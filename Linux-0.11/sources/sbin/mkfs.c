/*
 * mkfs.c - make a linux (minix) file-system.
 *
 * (C) 1991 Linus Torvalds. This file may be redistributed as per
 * the Linux copyright.
 */

/*
 * 24.11.91  -	time began. Used the fsck sources to get started.
 *
 * 25.11.91  -	corrected some bugs. Added support for ".badblocks"
 *		The algorithm for ".badblocks" is a bit weird, but
 *		it should work. Oh, well.
 *
 * Usuage: mkfs [-c] device size-in-blocks
 *
 *	-c for readablility checking (SLOW!)
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
#define BAD_INO 2

#define TEST_BUFFER_BLOCKS 32
#define MAX_GOOD_BLOCKS 512

#define UPPER(size,n) ((size+((n)-1))/(n))
#define INODE_SIZE (sizeof(struct d_inode))
#define INODE_BLOCKS UPPER(INODES,INODES_PER_BLOCK)
#define INODE_BUFFER_SIZE (INODE_BLOCKS * BLOCK_SIZE)

#define BITS_PER_BLOCK (BLOCK_SIZE<<3)

static char * program_name = "mkfs";
static char * device_name = NULL;
static int DEV = -1;
static long BLOCKS = 0;
static int check = 0;
static int badblocks = 0;

#define ROOT_INO_STRING "\001\000"
#define BAD_INO_STRING "\002\000"

static char root_block[BLOCK_SIZE] =
ROOT_INO_STRING ".\0\0\0\0\0\0\0\0\0\0\0\0\0"
ROOT_INO_STRING "..\0\0\0\0\0\0\0\0\0\0\0\0"
BAD_INO_STRING  ".badblocks\0\0\0\0";

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

static unsigned short good_blocks_table[MAX_GOOD_BLOCKS];
static int used_good_blocks = 0;

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

#define inode_in_use(x) (bit(inode_map,(x)))
#define zone_in_use(x) (bit(zone_map,(x)-FIRSTZONE+1))

#define mark_inode(x) (setbit(inode_map,(x)))
#define unmark_inode(x) (clrbit(inode_map,(x)))

#define mark_zone(x) (setbit(zone_map,(x)-FIRSTZONE+1))
#define unmark_zone(x) (clrbit(zone_map,(x)-FIRSTZONE+1))

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

#define usage() fatal_error("Usage: %s [-c] /dev/name blocks\n")
#define die(str) fatal_error("%s: " str "\n")

void write_tables(void)
{
	if (BLOCK_SIZE != lseek(DEV, BLOCK_SIZE, SEEK_SET))
		die("seek failed in write_tables");
	if (BLOCK_SIZE != write(DEV, super_block_buffer, BLOCK_SIZE))
		die("unable to write super-block");
	if (IMAPS*BLOCK_SIZE != write(DEV,inode_map,IMAPS*BLOCK_SIZE))
		die("Unable to write inode map");
	if (ZMAPS*BLOCK_SIZE != write(DEV,zone_map,ZMAPS*BLOCK_SIZE))
		die("Unable to write zone map");
	if (INODE_BUFFER_SIZE != write(DEV,inode_buffer,INODE_BUFFER_SIZE))
		die("Unable to write inodes");
}

void write_block(int blk, char * buffer)
{
	if (blk*BLOCK_SIZE != lseek(DEV, blk*BLOCK_SIZE, SEEK_SET))
		die("seek failed in write_block");
	if (BLOCK_SIZE != write(DEV, buffer, BLOCK_SIZE))
		die("write failed in write_block");
}

int get_free_block(void)
{
	int blk;

	if (used_good_blocks+1 >= MAX_GOOD_BLOCKS)
		die("too many bad blocks");
	if (used_good_blocks)
		blk = good_blocks_table[used_good_blocks-1]+1;
	else
		blk = FIRSTZONE;
	while (blk < ZONES && zone_in_use(blk))
		blk++;
	if (blk >= ZONES)
		die("not enough good blocks");
	good_blocks_table[used_good_blocks] = blk;
	used_good_blocks++;
	return blk;
}

void mark_good_blocks(void)
{
	int blk;

	for (blk=0 ; blk < used_good_blocks ; blk++)
		mark_zone(good_blocks_table[blk]);
}

inline int next(int zone)
{
	if (!zone)
		zone = FIRSTZONE-1;
	while (++zone < ZONES)
		if (zone_in_use(zone))
			return zone;
	return 0;
}

void make_bad_inode(void)
{
	struct d_inode * inode = &Inode[BAD_INO];
	int i,j,zone;
	int ind=0,dind=0;
	unsigned short ind_block[BLOCK_SIZE>>1];
	unsigned short dind_block[BLOCK_SIZE>>1];

#define NEXT_BAD (zone = next(zone))

	if (!badblocks)
		return;
	mark_inode(BAD_INO);
	inode->i_nlinks = 1;
	inode->i_time = time(NULL);
	inode->i_mode = S_IFREG + 0000;
	inode->i_size = badblocks*BLOCK_SIZE;
	zone = next(0);
	for (i=0 ; i<7 ; i++) {
		inode->i_zone[i] = zone;
		if (!NEXT_BAD)
			goto end_bad;
	}
	inode->i_zone[7] = ind = get_free_block();
	memset(ind_block,0,BLOCK_SIZE);
	for (i=0 ; i<512 ; i++) {
		ind_block[i] = zone;
		if (!NEXT_BAD)
			goto end_bad;
	}
	inode->i_zone[8] = dind = get_free_block();
	memset(dind_block,0,BLOCK_SIZE);
	for (i=0 ; i<512 ; i++) {
		write_block(ind,(char *) ind_block);
		dind_block[i] = ind = get_free_block();
		memset(ind_block,0,BLOCK_SIZE);
		for (j=0 ; j<512 ; j++) {
			ind_block[j] = zone;
			if (!NEXT_BAD)
				goto end_bad;
		}
	}
	die("too many bad blocks");
end_bad:
	if (ind)
		write_block(ind, (char *) ind_block);
	if (dind)
		write_block(dind, (char *) dind_block);
}

void make_root_inode(void)
{
	struct d_inode * inode = &Inode[ROOT_INO];

	mark_inode(ROOT_INO);
	inode->i_zone[0] = get_free_block();
	inode->i_nlinks = 2;
	inode->i_time = time(NULL);
	if (badblocks)
		inode->i_size = 48;
	else
		inode->i_size = 32;
	inode->i_mode = S_IFDIR + 0755;
	write_block(inode->i_zone[0],root_block);
}

void setup_tables(void)
{
	int i;

	memset(inode_map,0xff,sizeof(inode_map));
	memset(zone_map,0xff,sizeof(zone_map));
	memset(super_block_buffer,0,BLOCK_SIZE);
	MAGIC = SUPER_MAGIC;
	ZONESIZE = 0;
	MAXSIZE = (7+512+512*512)*1024;
	ZONES = BLOCKS;
/* some magic nrs: 1 inode / 3 blocks */
	INODES = BLOCKS/3;
/* I don't want some off-by-one errors, so this hack... */
	if ((INODES & 8191) > 8188)
		INODES -= 5;
	if ((INODES & 8191) < 10)
		INODES -= 20;
	IMAPS = UPPER(INODES,BITS_PER_BLOCK);
	ZMAPS = 0;
	while (ZMAPS != UPPER(BLOCKS - NORM_FIRSTZONE,BITS_PER_BLOCK))
		ZMAPS = UPPER(BLOCKS - NORM_FIRSTZONE,BITS_PER_BLOCK);
	FIRSTZONE = NORM_FIRSTZONE;
	for (i = FIRSTZONE ; i<ZONES ; i++)
		unmark_zone(i);
	for (i = ROOT_INO ; i<INODES ; i++)
		unmark_inode(i);
	inode_buffer = malloc(INODE_BUFFER_SIZE);
	if (!inode_buffer)
		die("Unable to allocate buffer for inodes");
	memset(inode_buffer,0,INODE_BUFFER_SIZE);
	printf("%d inodes\n",INODES);
	printf("%d blocks\n",ZONES);
	printf("Firstdatazone=%d (%d)\n",FIRSTZONE,NORM_FIRSTZONE);
	printf("Zonesize=%d\n",BLOCK_SIZE<<ZONESIZE);
	printf("Maxsize=%d\n\n",MAXSIZE);
}

void check_blocks(void)
{
	unsigned int current_block=0;
	int try,got;
	static char buffer[BLOCK_SIZE * TEST_BUFFER_BLOCKS];

	while (current_block < ZONES) {
		if (lseek(DEV,current_block*BLOCK_SIZE,SEEK_SET) !=
		current_block*BLOCK_SIZE)
			die("seek failed in check_blocks");
		try = TEST_BUFFER_BLOCKS;
		if (current_block + try > ZONES)
			try = ZONES-current_block;
		got = read(DEV, buffer, try * BLOCK_SIZE);
		if (got<0)
			got = 0;
		if (got & (BLOCK_SIZE-1))
			printf("Weird values in check_blocks: probably bugs\n");
		got /= BLOCK_SIZE;
		current_block += got;
		if (got == try)
			continue;
		if (current_block < FIRSTZONE)
			die("bad blocks before data-area: cannot make fs");
		mark_zone(current_block);
		badblocks++;
		current_block++;
	}
	if (badblocks)
		printf("%d bad block%s\n",badblocks,(badblocks>1)?"s":"");
}

int main(int argc, char ** argv)
{
	char * tmp;
	struct stat statbuf;

	if (argc && *argv)
		program_name = *argv;
	if (INODE_SIZE * INODES_PER_BLOCK != BLOCK_SIZE)
		die("bad inode size");
	while (argc-- > 1) {
		argv++;
		if (argv[0][0] != '-')
			if (device_name) {
				BLOCKS = strtol(argv[0],&tmp,0);
				if (*tmp)
					usage();
			} else
				device_name = argv[0];
		else while (*++argv[0])
			switch (argv[0][0]) {
				case 'c': check=1; break;
				default: usage();
			}
	}
	if (!device_name || BLOCKS<10 || BLOCKS > 65536)
		usage();
	DEV = open(device_name,O_RDWR);
	if (DEV<0)
		die("unable to open %s");
	if (fstat(DEV,&statbuf)<0)
		die("unable to stat %s");
	if (!S_ISBLK(statbuf.st_mode))
		check=0;
	else if (statbuf.st_rdev == 0x0300 || statbuf.st_rdev == 0x0305)
		die("Will not try to make filesystem on '%s'");
	setup_tables();
	if (check)
		check_blocks();
	make_root_inode();
	make_bad_inode();
	mark_good_blocks();
	write_tables();
	return 0;
}
