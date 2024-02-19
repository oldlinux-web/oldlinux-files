/*
 * fsck.c - a file system consistency checker for Linux.
 *
 * (C) 1991 Linus Torvalds. This file may be redistributed as per
 * the Linux copyright.
 */

/*
 * 09.11.91  -  made the first rudimetary functions
 *
 * 10.11.91  -  updated, does checking, no repairs yet.
 *		Sent out to the mailing-list for testing.
 *
 * 14.11.91  -	Testing seems to have gone well. Added some
 *		correction-code, and changed some functions.
 *
 * 15.11.91  -  More correction code. Hopefully it notices most
 *		cases now, and tries to do something about them.
 *
 * 16.11.91  -  More corrections (thanks to Matti Jalava). Most
 *		things seem to work now.
 *
 * I've had no time to add comments - hopefully the function names
 * are comments enough. As with all file system checkers, this assumes
 * the file system is quiescent - don't use it on a mounted device
 * unless you can be sure nobody is writing to it (and remember that the
 * kernel can write to it when it searches for files).
 *
 * Usuage: fsck [-larvsm] device
 *	-l for a listing of all the filenames
 *	-a for automatic repairs (not implemented)
 *	-r for repairs (interactive) (not implemented)
 *	-v for verbose (tells how many files)
 *	-s for super-block info
 *	-m for minix-like "mode not cleared" warnings
 *
 * The device may be a block device or a image of one, but this isn't
 * enforced (but it's not much fun on a character device :-). 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/stat.h>

#include <linux/fs.h>

#ifndef __GNUC__
#error "needs gcc for the bitop-__asm__'s"
#endif

#ifndef __linux__
#define volatile
#endif

#define ROOT_INO 1

#define UPPER(size,n) ((size+((n)-1))/(n))
#define INODE_SIZE (sizeof(struct d_inode))
#define INODE_BLOCKS UPPER(INODES,INODES_PER_BLOCK)
#define INODE_BUFFER_SIZE (INODE_BLOCKS * BLOCK_SIZE)

#define BITS_PER_BLOCK (BLOCK_SIZE<<3)

static char * program_name = "fsck";
static char * device_name = NULL;
static int IN;
static int repair=0, automatic=0, verbose=0, list=0, show=0, warn_mode=0;
static int directory=0, regular=0, blockdev=0, chardev=0, links=0, total=0;

/* this is used to implement the (coming) two-pass zone checking. */
static int trust_zone_bit_map=0;

static int changed = 0; /* flags if the filesystem has been changed */

/* File-name data */
#define MAX_DEPTH 50
static int name_depth = 0;
static char name_list[MAX_DEPTH][NAME_LEN+1];

static char * inode_buffer = NULL;
#define Inode (((struct d_inode *) inode_buffer)-1)
static char super_block_buffer[BLOCK_SIZE];
#define Super (*(struct super_block *)super_block_buffer)
#define INODES ((unsigned long)Super.s_ninodes)
#define ZONES ((unsigned long)Super.s_nzones)
#define IMAPS ((unsigned long)Super.s_imap_blocks)
#define ZMAPS ((unsigned long)Super.s_zmap_blocks)
#define FIRSTZONE ((unsigned long)Super.s_firstdatazone)
#define ZONESIZE ((unsigned long)Super.s_log_zone_size)
#define MAXSIZE ((unsigned long)Super.s_max_size)
#define MAGIC (Super.s_magic)
#define NORM_FIRSTZONE (2+IMAPS+ZMAPS+INODE_BLOCKS)

static char inode_map[BLOCK_SIZE * I_MAP_SLOTS];
static char zone_map[BLOCK_SIZE * Z_MAP_SLOTS];

static unsigned char * inode_count = NULL;
static unsigned char * zone_count = NULL;

void recursive_check(unsigned int ino);

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"c")

#define inode_in_use(x) (bit(inode_map,(x)))
#define zone_in_use(x) (bit(zone_map,(x)-FIRSTZONE+1))

#define mark_inode(x) (setbit(inode_map,(x)),changed=1)
#define unmark_inode(x) (clrbit(inode_map,(x)),changed=1)

#define mark_zone(x) (setbit(zone_map,(x)-FIRSTZONE+1),changed=1)
#define unmark_zone(x) (clrbit(zone_map,(x)-FIRSTZONE+1),changed=1)

/*
 * Volatile to let gcc know that this doesn't return. When trying
 * to compile this under minix, volatile gives a warning, as
 * exit() isn't defined as volatile under minix.
 */
volatile void fatal_error(const char * fmt_string)
{
	fprintf(stderr,fmt_string,program_name,device_name);
	exit(1);
}

#define usage() fatal_error("Usage: %s [-larvsm] /dev/name\n")
#define die(str) fatal_error("%s: " str "\n")

/*
 * This simply goes through the file-name data and prints out the
 * current file.
 */
void print_current_name(void)
{
	int i=0;

	while (i<name_depth)
		printf("/%.14s",name_list[i++]);
}

int ask(const char * string,int def)
{
	int c;

	if (!repair) {
		printf("\n");
		return 0;
	}
	if (automatic) {
		printf("\n");
		return def;
	}
	printf(def?"%s (y/n)? ":"%s (n/y)? ",string);
	for (;;) {
		fflush(stdout);
		if ((c=getchar())==EOF)
			return def;
		c=toupper(c);
		if (c == 'Y') {
			def = 1;
			break;
		} else if (c == 'N') {
			def = 0;
			break;
		} else if (c == ' ' || c == '\n')
			break;
	}
	if (def)
		printf("y\n");
	else
		printf("n\n");
	return def;
}

/*
 * check_zone_nr checks to see that *nr is a valid zone nr. If it
 * isn't, it will possibly be repaired. Check_zone_nr returns != 0
 * if it changed the nr.
 */
int check_zone_nr(unsigned short * nr)
{
	if (!*nr)
		return 0;
	if (*nr < FIRSTZONE)
		printf("Zone nr < FIRSTZONE in file `");
	else if (*nr >= ZONES)
		printf("Zone nr > ZONES in file `");
	else
		return 0;
	print_current_name();
	printf("'.");
	if (ask("Remove block",1)) {
		*nr=0;
		changed = 1;
		return 1;
	}
	return 0;
}

/*
 * read-block reads block *nr into the buffer at addr. It returns
 * 0 if the *nr is unchanged, 1 if it was changed.
 */
int read_block(unsigned short * nr, char * addr)
{
	int blk_chg = check_zone_nr(nr);

	if (!*nr || *nr >= ZONES) {
		memset(addr,0,BLOCK_SIZE);
		return changed;
	}
	if (BLOCK_SIZE*(*nr) != lseek(IN, BLOCK_SIZE*(*nr), SEEK_SET))
		die("seek failed in read_block");
	if (BLOCK_SIZE != read(IN, addr, BLOCK_SIZE)) {
		printf("Read error: bad block in file '");
		print_current_name();
		printf("'\n");
		memset(addr,0,BLOCK_SIZE);
	}
	return blk_chg;
}

/*
 * write_block writes block nr to disk.
 */
inline void write_block(unsigned int nr, char * addr)
{
	if (!nr)
		return;
	if (nr < FIRSTZONE || nr >= ZONES) {
		printf("Internal error: trying to write bad block\n"
		"Write request ignored\n");
		return;
	}
	if (BLOCK_SIZE*nr != lseek(IN, BLOCK_SIZE*nr, SEEK_SET))
		die("seek failed in write_block");
	if (BLOCK_SIZE != write(IN, addr, BLOCK_SIZE)) {
		printf("Write error: bad block in file '");
		print_current_name();
		printf("'\n");
	}
}

/*
 * mapped-read-block reads block nr blknr from the specified file.
 * it returns 1 if the inode has been changed due to bad zone nrs
 */
inline int mapped_read_block(struct d_inode * inode,
	unsigned int blknr, char * addr)
{
	unsigned short ind[BLOCK_SIZE>>1];
	unsigned short dind[BLOCK_SIZE>>1];
	int result;

	if (blknr<7)
		return read_block(blknr + inode->i_zone,addr);
	blknr -= 7;
	if (blknr<512) {
		result = read_block(7 + inode->i_zone, (char *) ind);
		if (read_block(blknr + ind,addr))
			write_block(inode->i_zone[7], (char *) ind);
		return result;
	}
	blknr -= 512;
	result = read_block(8 + inode->i_zone, (char *) dind);
	if (read_block(blknr/512 + dind, (char *) ind))
		write_block(inode->i_zone[8], (char *) dind);
	if (read_block(blknr%512 + ind,addr))
		write_block(dind[blknr/512], (char *) ind);
	return result;
}

void write_tables(void)
{
	if (BLOCK_SIZE != lseek(IN, BLOCK_SIZE, SEEK_SET))
		die("seek failed in write_tables");
	if (BLOCK_SIZE != write(IN, super_block_buffer, BLOCK_SIZE))
		die("unable to write super-block");
	if (IMAPS*BLOCK_SIZE != write(IN,inode_map,IMAPS*BLOCK_SIZE))
		die("Unable to write inode map");
	if (ZMAPS*BLOCK_SIZE != write(IN,zone_map,ZMAPS*BLOCK_SIZE))
		die("Unable to write zone map");
	if (INODE_BUFFER_SIZE != write(IN,inode_buffer,INODE_BUFFER_SIZE))
		die("Unable to write inodes");
}

void read_tables(void)
{
	memset(inode_map,0,sizeof(inode_map));
	memset(zone_map,0,sizeof(zone_map));
	if (BLOCK_SIZE != lseek(IN, BLOCK_SIZE, SEEK_SET))
		die("seek failed");
	if (BLOCK_SIZE != read(IN, super_block_buffer, BLOCK_SIZE))
		die("unable to read super block");
	if (MAGIC != SUPER_MAGIC)
		die("bad magic number in super-block");
	if (ZONESIZE != 0 || BLOCK_SIZE != 1024)
		die("Only 1k blocks/zones supported");
	if (!IMAPS || IMAPS > I_MAP_SLOTS)
		die("bad s_imap_blocks field in super-block");
	if (!ZMAPS || ZMAPS > Z_MAP_SLOTS)
		die("bad s_zmap_blocks field in super-block");
	inode_buffer = malloc(INODE_BUFFER_SIZE);
	if (!inode_buffer)
		die("Unable to allocate buffer for inodes");
	inode_count = malloc(INODES);
	if (!inode_count)
		die("Unable to allocate buffer for inode count");
	zone_count = malloc(ZONES);
	if (!zone_count)
		die("Unable to allocate buffer for zone count");
	if (IMAPS*BLOCK_SIZE != read(IN,inode_map,IMAPS*BLOCK_SIZE))
		die("Unable to read inode map");
	if (ZMAPS*BLOCK_SIZE != read(IN,zone_map,ZMAPS*BLOCK_SIZE))
		die("Unable to read zone map");
	if (INODE_BUFFER_SIZE != read(IN,inode_buffer,INODE_BUFFER_SIZE))
		die("Unable to read inodes");
	if (NORM_FIRSTZONE != FIRSTZONE)
		printf("Warning: Firstzone != Norm_firstzone\n");
	if (show) {
		printf("%d inodes\n",INODES);
		printf("%d blocks\n",ZONES);
		printf("Firstdatazone=%d (%d)\n",FIRSTZONE,NORM_FIRSTZONE);
		printf("Zonesize=%d\n",BLOCK_SIZE<<ZONESIZE);
		printf("Maxsize=%d\n\n",MAXSIZE);
	}
}

struct d_inode * get_inode(unsigned int nr)
{
	struct d_inode * inode;

	if (!nr || nr > INODES)
		return NULL;
	total++;
	inode = Inode + nr;
	if (!inode_count[nr]) {
		if (!inode_in_use(nr)) {
			printf("Inode %d marked not used, but used for file '",
				nr);
			print_current_name();
			printf("'\n");
			if (repair)
				if (ask("Mark in use",1))
					mark_inode(nr);
		}
		if (S_ISDIR(inode->i_mode))
			directory++;
		else if (S_ISREG(inode->i_mode))
			regular++;
		else if (S_ISCHR(inode->i_mode))
			chardev++;
		else if (S_ISBLK(inode->i_mode))
			blockdev++;
	} else
		links++;
	if (!++inode_count[nr]) {
		printf("Warning: inode count too big.\n");
		inode_count[nr]--;
	}
	return inode;
}

void check_root(void)
{
	struct d_inode * inode = Inode + ROOT_INO;

	if (!inode || !S_ISDIR(inode->i_mode))
		die("root inode isn't a directory");
}

static int add_zone(unsigned short * znr)
{
	int result;

	result=check_zone_nr(znr);
	if (!*znr || *znr >= ZONES)
		return result;
	if (zone_count[*znr]) {
		printf("Block has been used before. Now in file `");
		print_current_name();
		printf("'.");
		if (ask("Clear",1)) {
			*znr = 0;
			changed = 1;
		}
	}
	if (!*znr || *znr >= ZONES)
		return result;
	if (!zone_in_use(*znr)) {
		printf("Block %d in file `",*znr);
		print_current_name();
		printf("' is marked not in use.");
		if (ask("Correct",1))
			mark_zone(*znr);
	}
	if (!++zone_count[*znr])
		zone_count[*znr]--;
	return result;
}

static int add_zone_ind(unsigned short * znr)
{
	static char blk[BLOCK_SIZE];
	int i, result, chg_blk=0;

	result = add_zone(znr);
	if (!*znr || *znr>=ZONES)
		return result;
	read_block(znr,blk);
	for (i=0 ; i < (BLOCK_SIZE>>1) ; i++)
		chg_blk |= add_zone(i + (unsigned short *) blk);
	if (chg_blk)
		write_block(*znr,blk);
	return result;
}

static int add_zone_dind(unsigned short * znr)
{
	static char blk[BLOCK_SIZE];
	int i, result, blk_chg=0;

	result = add_zone(znr);
	if (!*znr || *znr >= ZONES)
		return result;
	read_block(znr,blk);
	for (i=0 ; i < (BLOCK_SIZE>>1) ; i++)
		blk_chg |= add_zone_ind(i + (unsigned short *) blk);
	if (blk_chg)
		write_block(*znr,blk);
	return result;
}

void check_zones(unsigned int i)
{
	struct d_inode * inode;

	if (!i || i >= INODES)
		return;
	if (inode_count[i] > 1)	/* have we counted this file already? */
		return;
	inode = Inode + i;
	if (!S_ISDIR(inode->i_mode) && !S_ISREG(inode->i_mode))
		return;
	for (i=0 ; i<7 ; i++)
		add_zone(i + inode->i_zone);
	add_zone_ind(7 + inode->i_zone);
	add_zone_dind(8 + inode->i_zone);
}

void check_file(struct d_inode * dir, unsigned int offset)
{
	static char blk[BLOCK_SIZE];
	struct d_inode * inode;
	int ino;
	char * name;

	changed |= mapped_read_block(dir,offset/BLOCK_SIZE,blk);
	name = blk + (offset % BLOCK_SIZE) + 2;
	ino = * (unsigned short *) (name-2);
	inode = get_inode(ino);
	if (!offset)
		if (!inode || strcmp(".",name)) {
			print_current_name();
			printf(": bad directory: '.' isn't first\n");
		} else return;
	if (offset == 16)
		if (!inode || strcmp("..",name)) {
			print_current_name();
			printf(": bad directory: '..' isn't second\n");
		} else return;
	if (!inode)
		return;
	if (name_depth < MAX_DEPTH)
		strncpy(name_list[name_depth],name,14);
	name_depth++;	
	if (list) {
		if (verbose)
			printf("%6d %07o ",ino,inode->i_mode);
		print_current_name();
		if (S_ISDIR(inode->i_mode))
			printf(":\n");
		else
			printf("\n");
	}
	check_zones(ino);
	if (inode && S_ISDIR(inode->i_mode))
		recursive_check(ino);
	name_depth--;
	return;
}

void recursive_check(unsigned int ino)
{
	struct d_inode * dir;
	unsigned int offset;

	dir = Inode + ino;
	if (!S_ISDIR(dir->i_mode))
		die("internal error");
	for (offset = 0 ; offset < dir->i_size ; offset += 16)
		check_file(dir,offset);
}

void check_counts(void)
{
	int i;

	for (i=1 ; i < INODES ; i++) {
		if (!inode_in_use(i) && Inode[i].i_mode && warn_mode) {
			printf("Inode %d mode not cleared.",i);
			if (ask("Clear",1)) {
				Inode[i].i_mode = 0;
				changed = 1;
			}
		}
		if (inode_in_use(i)*Inode[i].i_nlinks == inode_count[i])
			continue;
		if (!inode_count[i]) {
			printf("Inode %d not used, marked used in the bitmap.",
				i);
			if (ask("Clear",1))
				unmark_inode(i);
		} else if (!inode_in_use(i)) {
			printf("Inode %d used, marked unused in the bitmap.",
				i);
			if (ask("Set",1))
				mark_inode(i);
		}
		if (inode_in_use(i) && Inode[i].i_nlinks != inode_count[i]) {
			printf("Inode %d, i_nlinks=%d, counted=%d.",
				i,Inode[i].i_nlinks,inode_count[i]);
			if (ask("Set i_nlinks to count",1)) {
				Inode[i].i_nlinks=inode_count[i];
				changed=1;
			}
		}
	}
	for (i=FIRSTZONE ; i < ZONES ; i++) {
		if (zone_in_use(i) == zone_count[i])
			continue;
		printf("Zone %d: %s in use, counted=%d\n",
		i,zone_in_use(i)?"":"not",zone_count[i]);
	}
}

void check(void)
{
	memset(inode_count,0,INODES*sizeof(*inode_count));
	memset(zone_count,0,ZONES*sizeof(*zone_count));
	check_zones(ROOT_INO);
	recursive_check(ROOT_INO);
	check_counts();
}

int main(int argc, char ** argv)
{
	struct termios termios,tmp;

	if (argc && *argv)
		program_name = *argv;
	if (INODE_SIZE * INODES_PER_BLOCK != BLOCK_SIZE)
		die("bad inode size");
	while (argc-- > 1) {
		argv++;
		if (argv[0][0] != '-')
			if (device_name)
				usage();
			else
				device_name = argv[0];
		else while (*++argv[0])
			switch (argv[0][0]) {
				case 'l': list=1; break;
				case 'a': automatic=1; repair=1; break;
				case 'r': automatic=0; repair=1; break;
				case 'v': verbose=1; break;
				case 's': show=1; break;
				case 'm': warn_mode=1; break;
				default: usage();
			}
	}
	if (!device_name)
		usage();
	if (repair && !automatic) {
		if (!isatty(0) || !isatty(1))
			die("need terminal for interactive repairs");
		tcgetattr(0,&termios);
		tmp = termios;
		tmp.c_lflag &= ~(ICANON|ECHO);
		tcsetattr(0,TCSANOW,&tmp);
	}
	IN = open(device_name,repair?O_RDWR:O_RDONLY);
	if (IN < 0)
		die("unable to open '%s'");
	read_tables();
	check_root();
	check();
	if (verbose) {
		int i, free;

		for (i=1,free=0 ; i<INODES ; i++)
			if (!inode_in_use(i))
				free++;
		printf("\n%6d inodes used (%d%%)\n",(INODES-free),
			100*(INODES-free)/INODES);
		for (i=FIRSTZONE,free=0 ; i<ZONES ; i++)
			if (!zone_in_use(i))
				free++;
		printf("%6d zones used (%d%%)\n",(ZONES-free),
			100*(ZONES-free)/ZONES);
		printf("\n%6d regular files\n"
		"%6d directories\n"
		"%6d character device files\n"
		"%6d block device files\n"
		"%6d links\n"
		"------\n"
		"%6d files\n",
		regular,directory,chardev,blockdev,
		links-2*directory+1,total-2*directory+1);
	}
	if (changed) {
		write_tables();
		printf(	"----------------------------\n"
			"FILE SYSTEM HAS BEEN CHANGED\n"
			"----------------------------\n");
	}
	if (repair && !automatic)
		tcsetattr(0,TCSANOW,&termios);
	return (0);
}
