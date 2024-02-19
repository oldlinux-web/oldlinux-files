/*
 * efsck.c - a consistency checker for the extended file system.
 *
 * Copyright (C) 1992 Remy Card (card@masi.ibp.fr)
 *
 * Copyright (C) 1991 Linus Torvalds. This file may be redistributed as per
 * the GNU copyright.
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
 * 16.11.91  -  More corrections (thanks to Mika Jalava). Most
 *		things seem to work now. Yeah, sure.
 *
 * 14.07.92  -  Modifications by Remy Card (card@masi.ibp.fr)
 *		Used the fsck source to create efsck.c
 *
 * 19.07.92  -	Many bugs fixed [the first release was full of bugs :-(]
 *		- init_zone_bitmap() and init_inode_bitmap() now check that
 *		  the zones/inodes free lists are not corrupted. If they are,
 *		  all zones/inodes are marked as unused and are corrected (I
 *		  hope) by check_counts().
 *		- init_zone_bitmap() and init_inode_bitmap() now check for
 *		  cycles in the free lists. Before, in case of a cycle, efsck
 *		  entered an infinite loop ...
 *		- salvage_inode_freelist() was wrong. It used inode-1 as index
 *		  instead of inode.
 *		- salvage_zone_freelist() and salvage_inode_freelist() now
 *		  try to keep the same first free zone/inode so there should
 *		  be less problems when checking a mounted file system (for
 *		  a mounted ext file system, the first free zone/inode numbers
 *		  are stored in the copy of the super block in memory and are
 *		  rewritten to disk on sync() and umount(), modifying the
 *		  first free zone/inode can lead to some inconsistency).
 *		- corrected the file name printing in get_inode().
 *		- corrected the "inode not used" test which was never true ...
 *		- added size checks : compare the size of each inode with the
 *		  number of blocks allocated for the inode.
 *		Remaining problem :
 *		- I think that there is some more work to do to correct the
 *		  free lists. Currently, efsck salvages (rebuilds) them and it
 *		  is a very primitive way to handle errors. Perhaps, it should
 *		  act in a more clever way by adding or removing zones/inodes
 *		  from the free lists. I don't know if it is very important ...
 *
 *  21.07.92  - Corrected check_sizes():
 *		- to count the last allocated block and	NOT the allocated blocks
 *		  count, wo it now counts holes,
 *		- fixed the bug causing the message 'incorrect size 0
 *		  (counted = 0)'.
 *
 *  26.07.92  - efsck now understands triple indirection
 *
 *  11.08.92  -	Changes by Wayne Davison (davison@borland.com)
 *		- The badblock inode (2 on my system) is always read in and
 *		  the resulting bitmap is used to ensure that files don't use
 *		  these blocks in their data.  A minor tweak keeps efsck from
 *		  complaining about the inode not being referenced.
 *		- The -t option has been added to perform a read test of the
 *		  disk looking for new bad blocks.  Using -t gives efsck write-
 *		  permission for just the bad block changes (which might include
 *		  a rewrite of the free lists, inodes & root).  If no repair
 *		  options were specified and a file uses a bad block only a
 *		  warning is generated.  A block must be either unused or
 *		  "repaired" (dropped from the file) for it to be added to the
 *		  badblock inode.
 *		- Minor changes were made to the buffers to reduce their number.
 *		- All the pluralizing outputs were changed so that 0's come out
 *		  plural (e.g. 0 bad blocks, not 0 bad block).
 *		- Fixed an off-by-one error in the INODE code.
 *		- Fixed a bug in the directory loop where it could infinite loop
 *		  if we're checking bogus directory data (with a zero rec_len).
 *		- Fixed a bug in the size counting when dealing with the triple-
 *		  indirect zone list.
 *		- Twiddled the verbose code to use the counts stored in the
 *		  super block (which were just verified) rather than counting
 *		  them again.
 *		- The verbose code outputs the number of bad blocks on the disk.
 *		- I removed 'm' from the usage list of options.
 *
 *  12.08.92  - Corrected (again) check_sizes() : it now complains only
 *		when the size of a file is less than the last allocated
 *		block.
 *
 *  13.08.92  - Changes to support the existing .badblocks file
 *	      - Added the -S argument which allows salvaging the free lists
 *
 *  16.08.92  - Added some sanity checks on the directory entries
 *	      - corrected the test on rec_len to ensure that it is greater than
 *		or equal to 8 (was 16 before) because there may be 8 bytes
 *		directory entries (the last unused entry in a block can be 8
 *		bytes long)
 *	      - Use getopt() to get arguments
 *
 *  25.08.92  - Changes by Wayne Davison to create a "new" bad blocks inode
 *		from the old ".badblocks" file
 *
 *  27.08.92  - Fixed two stupid errors :
 *		- when the free lists were salvaged, the tables were not
 *		  written back to the disk
 *		- when the free inodes count and/or the free blocks count
 *		  were modified, the tables were not written back to the
 *		  disk
 *
 *  28.08.92  - Corrected init_zone_bitmap() and init_inode_bitmap() : sanity
 *		checks on the free zones/inodes are done BEFORE accessing the
 *		bitmaps.  When it was done after accessing the bitmaps, efsck
 *		could dump core when the list(s) was(were) corrupted.
 *
 *  03.09.92  - Corrected check_sizes() to ignore special files
 *
 * I've had no time to add comments - hopefully the function names
 * are comments enough. As with all file system checkers, this assumes
 * the file system is quiescent - don't use it on a mounted device
 * unless you can be sure nobody is writing to it (and remember that the
 * kernel can write to it when it searches for files).
 *
 * Usage: fsck [-larvsmdtS] device
 *	-l for a listing of all the filenames
 *	-a for automatic repairs
 *	-r for repairs (interactive)
 *	-v for verbose (tells how many files)
 *	-s for super-block info
 *	-d for debugging this program
 *	-t for testing bad blocks on the fs
 *	-S for salvaging the free blocks/inodes lists
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
#include <getopt.h>

#include <linux/fs.h>
#include <linux/ext_fs.h>

#define NAME_LEN EXT_NAME_LEN

#ifndef __GNUC__
#error "needs gcc for the bitop-__asm__'s"
#endif

#ifndef __linux__
#define volatile
#endif

#define ROOT_INO 1
#define BAD_INO  2

#define TEST_BUFFER_BLOCKS	 16

static char blkbuf[BLOCK_SIZE * TEST_BUFFER_BLOCKS];

#define d_inode ext_inode

#define UPPER(size,n)		((size + ((n) - 1)) / (n))
#define INODE_SIZE		(sizeof (struct d_inode))
#define INODE_BLOCKS		UPPER(INODES, EXT_INODES_PER_BLOCK)
#define INODE_BUFFER_SIZE	(INODE_BLOCKS * BLOCK_SIZE)

#define BITS_PER_BLOCK		(BLOCK_SIZE<<3)

extern int isatty(int);

static char * program_name = "efsck";
static char * device_name = NULL;
static int IN;
static int repair = 0, automatic = 0, verbose = 0, list = 0, show = 0, 
		debug = 0, test_disk = 0, salvage = 0, no_bad_inode = 0;
static int directory = 0, regular = 0, blockdev = 0, chardev = 0, links = 0,
		symlinks = 0, fifo = 0, total = 0, badblocks = 0;

static int changed = 0; /* flags if the filesystem has been changed */
static int inode_freelist_to_salvage = 0, zone_freelist_to_salvage = 0;

/* File-name data */
#define MAX_DEPTH 50
static int name_depth = 0;   /* -1 when we're parsing the bad zone list */
static char name_list [MAX_DEPTH] [NAME_LEN+1];

static char * inode_buffer = NULL;
#define Inode		(((struct d_inode *) inode_buffer) - 1)
static char super_block_buffer[BLOCK_SIZE];
#define Super		(* (struct ext_super_block *) super_block_buffer)
#define INODES		(Super.s_ninodes)
#define ZONES		(Super.s_nzones)
#define FIRSTFREEBLOCK	(Super.s_firstfreeblock)
#define FREEBLOCKSCOUNT	(Super.s_freeblockscount)
#define FIRSTFREEINODE	(Super.s_firstfreeinode)
#define FREEINODESCOUNT	(Super.s_freeinodescount)
#define FIRSTZONE	(Super.s_firstdatazone)
#define ZONESIZE	(Super.s_log_zone_size)
#define MAXSIZE		(Super.s_max_size)
#define MAGIC		(Super.s_magic)
#define NORM_FIRSTZONE	(2 + INODE_BLOCKS)

static char * inode_map = NULL;
static char * zone_map = NULL;
static char * bad_map = NULL;

static unsigned short * inode_count = NULL;
static unsigned short * zone_count = NULL;
static unsigned short * inode_free_count = NULL;
static unsigned short * zone_free_count = NULL;

static unsigned long last_block_read;

void recursive_check(unsigned int ino);

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
	int __res; \
	__asm__ __volatile__("bt" op "l %1,%2; adcl $0,%0" \
		:"=g" (__res) \
		:"r" (nr),"m" (*(addr)),"0" (0)); \
	return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

#define inode_in_use(x)	(bit (inode_map, (x)))
#define zone_in_use(x)	(bit (zone_map, (x) - FIRSTZONE + 1))
#define zone_is_bad(x)	(bit (bad_map, (x) - FIRSTZONE + 1))

#define mark_inode(x)	(setbit (inode_map, (x)), changed = 1,\
				inode_freelist_to_salvage = 1)
#define unmark_inode(x)	(clrbit (inode_map, (x)), changed = 1,\
				inode_freelist_to_salvage = 1)

#define mark_zone(x)	(setbit (zone_map, (x) - FIRSTZONE + 1),changed = 1,\
				zone_freelist_to_salvage = 1)
#define unmark_zone(x)	(clrbit (zone_map, (x) - FIRSTZONE + 1),changed = 1,\
				zone_freelist_to_salvage = 1)

#define mark_bad(x)	(setbit (bad_map, (x) - FIRSTZONE + 1), badblocks++)

/*
 * Volatile to let gcc know that this doesn't return.
 */
volatile void fatal_error (const char * fmt_string)
{
	fflush (stdout);
	fprintf (stderr, fmt_string, program_name, device_name);
	exit (1);
}

#define usage()		fatal_error("Usage: %s [-larvsdtS] /dev/name\n")
#define die(str)	fatal_error("%s: " str "\n")

/*
 * This simply goes through the file-name data and prints out the
 * current file.
 */
void print_current_name (void)
{
	int i = 0;

	while (i < name_depth)
		printf ("/%.255s", name_list [i++]);
	if (!name_depth)
		printf ("/");
	else if (name_depth == -1)
		printf ("{Bad-Block-List}");
}

int ask (const char * string, int def)
{
	int c;

	if (!repair)
	{
		printf ("\n");
		return 0;
	}
	if (automatic)
	{
		printf ("\n");
		return def;
	}
	printf (def ? "%s (y/n)? " : "%s (n/y)? ", string);
	for (;;)
	{
		fflush (stdout);
		if ((c = getchar()) == EOF)
			return def;
		c = toupper(c);
		if (c == 'Y')
		{
			def = 1;
			break;
		}
		else if (c == 'N')
		{
			def = 0;
			break;
		}
		else if (c == ' ' || c == '\n')
			break;
	}
	if (def)
		printf ("y\n");
	else
		printf ("n\n");
	return def;
}

/*
 * check_zone_nr checks to see that *nr is a valid zone nr. If it
 * isn't, it will possibly be repaired. Check_zone_nr returns != 0
 * if it changed the nr.
 */
int check_zone_nr (unsigned long * nr)
{
	if (debug)
		printf ("DEBUG: check_zone_nr (&%d)\n", *nr);
	if (!*nr)
		return 0;
	if (*nr < FIRSTZONE)
		printf ("Zone nr %d < FIRSTZONE in file `", *nr);
	else if (*nr >= ZONES)
		printf ("Zone nr %d > ZONES in file `", *nr);
	else
		return 0;
	print_current_name ();
	printf ("'. ");
	if (ask ("Remove block", 1))
	{
		*nr = 0;
		changed = 1;
		return 1;
	}
	return 0;
}

/*
 * read-block reads block *nr into the buffer at addr. It returns
 * 0 if the *nr is unchanged, 1 if it was changed.
 */
int read_block (unsigned long * nr, char * addr)
{
	int blk_chg = check_zone_nr(nr);

	if (debug)
		printf ("DEBUG: read_block (&%d, %d)\n", *nr, (int) addr);
	last_block_read = *nr;

	if (!*nr || *nr >= ZONES)
	{
		memset (addr, 0, BLOCK_SIZE);
		return changed;
	}
	if (BLOCK_SIZE * (*nr) != lseek (IN, BLOCK_SIZE * (*nr), SEEK_SET))
		die ("seek failed in read_block");
	if (BLOCK_SIZE != read (IN, addr, BLOCK_SIZE))
	{
		printf ("Read error: bad block in file '");
		print_current_name ();
		printf ("'\n");
		memset(addr, 0, BLOCK_SIZE);
	}
	return blk_chg;
}

/*
 * write_block writes block nr to disk.
 */
inline void write_block (unsigned int nr, char * addr)
{
	if (debug)
		printf ("DEBUG: write_block(%d, %d)\n", nr, (int) addr);
	if (!nr)
		return;
	if (nr < FIRSTZONE || nr >= ZONES)
	{
		printf ("Internal error: trying to write bad block (%d)\n"
			"Write request ignored\n", nr);
		return;
	}
	if (BLOCK_SIZE * nr != lseek (IN, BLOCK_SIZE * nr, SEEK_SET))
		die ("seek failed in write_block");
	if (BLOCK_SIZE != write (IN, addr, BLOCK_SIZE))
	{
		printf ("Write error: bad block (%d) in file '", nr);
		print_current_name ();
		printf ("'\n");
	}
}

/*
 * mapped-read-block reads block nr blknr from the specified file.
 * it returns 1 if the inode has been changed due to bad zone nrs
 */
inline int mapped_read_block (struct d_inode * inode,
	unsigned int blknr, char * addr)
{
	unsigned long *ind  = (long *)(blkbuf + BLOCK_SIZE);
	unsigned long *dind = (long *)(blkbuf + BLOCK_SIZE * 2);
	unsigned long *tind = (long *)(blkbuf + BLOCK_SIZE * 3);
	int result;

	if (debug)
		printf ("DEBUG: mapped_read_block(%d,%d,%d)\n", 
			(int)inode, blknr, (int) addr);
	if (blknr < 9)
		return read_block (blknr + inode->i_zone, addr);
	blknr -= 9;
	if (blknr < 256)
	{
		result = read_block (9 + inode->i_zone, (char *) ind);
		if (read_block (blknr + ind, addr))
			write_block (inode->i_zone[9], (char *) ind);
		return result;
	}
	blknr -= 256;
	if (blknr < 256 * 256)
	{
		result = read_block (10 + inode->i_zone, (char *) dind);
		if (read_block (blknr / 256 + dind, (char *) ind))
			write_block (inode->i_zone[10], (char *) dind);
		if (read_block (blknr % 256 + ind, addr))
			write_block (dind [blknr / 256], (char *) ind);
		return result;
	}
	blknr -= 256 * 256;
	result = read_block (11 + inode->i_zone, (char *) tind);
	if (read_block ((blknr >> 16) + tind, (char *) dind))
		write_block (inode->i_zone[11], (char *) tind);
	if (read_block (((blknr >> 8) & 255) + dind, (char *) ind))
		write_block (tind [blknr >> 16], (char *) dind);
	if (read_block (blknr % 256 + ind, addr))
		write_block (dind [(blknr >> 8) & 255], (char *) ind);
	return result;
}

void write_tables (void)
{
	if (debug)
		printf ("DEBUG: write_tables()\n");
	if (BLOCK_SIZE != lseek (IN, BLOCK_SIZE, SEEK_SET))
		die ("seek failed in write_tables");
	if (BLOCK_SIZE != write (IN, super_block_buffer, BLOCK_SIZE))
		die ("unable to write super-block");
	if (INODE_BUFFER_SIZE != write (IN, inode_buffer, INODE_BUFFER_SIZE))
		die ("Unable to write inodes");
}

void read_tables (void)
{
	if (debug)
		printf ("DEBUG: read_tables()\n");
	if (BLOCK_SIZE != lseek (IN, BLOCK_SIZE, SEEK_SET))
		die ("seek failed");
	if (BLOCK_SIZE != read (IN, super_block_buffer, BLOCK_SIZE))
		die ("unable to read super block");
	if (MAGIC != EXT_SUPER_MAGIC)
		die ("bad magic number in super-block");
	if (ZONESIZE != 0 || BLOCK_SIZE != 1024)
		die("Only 1k blocks/zones supported");
	inode_buffer = malloc (INODE_BUFFER_SIZE);
	if (!inode_buffer)
		die ("Unable to allocate buffer for inodes");
	inode_count = malloc (INODES * sizeof (*inode_count));
	if (!inode_count)
		die ("Unable to allocate buffer for inode count");
	inode_free_count = malloc(INODES * sizeof (*inode_free_count));
	if (!inode_free_count)
		die("Unable to allocate buffer for inode count");
	zone_count = malloc(ZONES * sizeof (*zone_count));
	if (!zone_count)
		die ("Unable to allocate buffer for zone count");
	zone_free_count = malloc(ZONES * sizeof (*zone_free_count));
	if (!zone_free_count)
		die ("Unable to allocate buffer for zone count");
	inode_map = malloc ((INODES / 8) + 1);
	if (!inode_map)
		die ("Unable to allocate inodes bitmap\n");
	memset (inode_map, 0, (INODES / 8) + 1);
	zone_map = malloc (((ZONES - FIRSTZONE) / 8) + 1);
	if (!zone_map)
		die ("Unable to allocate zones bitmap\n");
	memset(zone_map, 0, ((ZONES - FIRSTZONE) / 8) + 1);
	bad_map = malloc (((ZONES - FIRSTZONE) / 8) + 1);
	if (!bad_map)
		die ("Unable to allocate bad zone bitmap\n");
	memset(bad_map, 0, ((ZONES - FIRSTZONE) / 8) + 1);
	if (INODE_BUFFER_SIZE != read (IN, inode_buffer, INODE_BUFFER_SIZE))
		die ("Unable to read inodes");
	if (NORM_FIRSTZONE != FIRSTZONE)
		printf ("Warning: Firstzone != Norm_firstzone\n");
	if (show)
	{
		printf ("%d inode%s\n", INODES, (INODES != 1) ? "s" : "");
		printf ("%d block%s\n", ZONES, (ZONES != 1) ? "s" : "");
		printf ("Firstdatazone=%d (%d)\n", FIRSTZONE, NORM_FIRSTZONE);
		printf ("%d free block%s\n", FREEBLOCKSCOUNT,
			(FREEBLOCKSCOUNT != 1) ? "s" : "");
		printf ("Firstfreeblock=%d\n", FIRSTFREEBLOCK);
		printf ("%d free inode%s\n", FREEINODESCOUNT,
			(FREEINODESCOUNT != 1) ? "s" : "");
		printf ("Firstfreeinode=%d\n", FIRSTFREEINODE);
		printf ("Zonesize=%d\n", BLOCK_SIZE << ZONESIZE);
		printf ("Maxsize=%d\n\n", MAXSIZE);
	}
}

struct d_inode * get_inode (unsigned int nr)
{
	struct d_inode * inode;

	if (debug)
		printf ("DEBUG: get_inode (%d)\n", nr);
	if (!nr || nr > INODES)
		return NULL;
	total++;
	inode = Inode + nr;
	if (!inode_count [nr])
	{
		if (!inode_in_use (nr))
		{
			printf ("Inode %d marked not used, but used for file '",
				nr);
			print_current_name ();
			printf ("'\n");
			if (ask ("Mark in use", 1))
				mark_inode (nr);
		}
		if (S_ISDIR (inode->i_mode))
			directory++;
		else if (S_ISREG (inode->i_mode))
			regular++;
		else if (S_ISCHR (inode->i_mode))
			chardev++;
		else if (S_ISBLK (inode->i_mode))
			blockdev++;
		else if (S_ISLNK (inode->i_mode))
			symlinks++;
		else if (S_ISFIFO (inode->i_mode))
			fifo++;
	}
	else
		links++;
	if (! ++inode_count [nr])
	{
		printf ("Warning: inode count too big.\n");
		inode_count [nr] --;
	}
	return inode;
}

void check_root (void)
{
	struct d_inode * inode = Inode + ROOT_INO;

	if (debug)
		printf ("DEBUG: check_root()\n");
	if (!inode || !S_ISDIR (inode->i_mode))
		die ("root inode isn't a directory");
}

static int add_zone (unsigned long * znr, int badflg)
{
	int result;

	if (debug)
		printf ("DEBUG: add_zone(&%d,%d)\n", *znr, badflg);
	result = check_zone_nr(znr);
	if (! *znr || *znr >= ZONES)
		return result;
	if (!badflg && zone_is_bad (*znr))
	{
		printf ("Bad block used in file `");
		print_current_name ();
		printf ("'. ");
		if (ask ("Clear", 1))
		{
			*znr = 0;
			changed = 1;
			return result;
		}
	}
	else if (zone_count [*znr])
	{
		printf ("Block has been used before. Now in file `");
		print_current_name ();
		printf ("'. ");
		if (ask ("Clear", 1))
		{
			*znr = 0;
			changed = 1;
			return result;
		}
	}
	if (!zone_in_use (*znr))
	{
		printf ("Block %d in file `", *znr);
		print_current_name ();
		printf ("' is marked not in use. ");
		if (ask ("Correct", 1))
			mark_zone (*znr);
	}
	if (badflg)
	{
		if (debug)
			printf ("DEBUG: Marking %d as bad.\n", *znr);
		mark_bad (*znr);
	}
	if (! ++ zone_count [*znr])
		zone_count [*znr] --;
	return result;
}

static int add_zone_ind (unsigned long * znr, int badflg)
{
	char *blk = blkbuf + BLOCK_SIZE;
	int i, result, chg_blk = 0;

	if (debug)
		printf ("DEBUG: add_zone_ind (&%d,%d)\n", *znr, badflg);
	result = add_zone (znr, 0);
	if (! *znr || *znr >= ZONES || zone_is_bad (*znr))
		return result;
	read_block(znr, blk);
	for (i = 0; i < (BLOCK_SIZE >> 2); i++)
		chg_blk |= add_zone (i + (unsigned long *) blk, badflg);
	if (chg_blk)
		write_block (*znr, blk);
	return result;
}

static int add_zone_dind (unsigned long * znr, int badflg)
{
	char *blk = blkbuf + BLOCK_SIZE * 2;
	int i, result, blk_chg = 0;

	if (debug)
		printf ("DEBUG: add_zone_dind (&%d,%d)\n", *znr, badflg);
	result = add_zone (znr, 0);
	if (! *znr || *znr >= ZONES || zone_is_bad (*znr))
		return result;
	read_block(znr, blk);
	for (i = 0; i < (BLOCK_SIZE >> 2); i++)
		blk_chg |= add_zone_ind (i + (unsigned long *) blk, badflg);
	if (blk_chg)
		write_block (*znr, blk);
	return result;
}

static int add_zone_tind (unsigned long * znr, int badflg)
{
	char *blk = blkbuf + BLOCK_SIZE * 3;
	int i, result, blk_chg = 0;

	if (debug)
		printf ("DEBUG: add_zone_tind (&%d,%d)\n", *znr, badflg);
	result = add_zone (znr, 0);
	if (! *znr || *znr >= ZONES || zone_is_bad (*znr))
		return result;
	read_block(znr, blk);
	for (i = 0; i < (BLOCK_SIZE >> 2); i++)
		blk_chg |= add_zone_dind (i + (unsigned long *) blk, badflg);
	if (blk_chg)
		write_block (*znr, blk);
	return result;
}

/*
 * Perform a test of a block; return the number of blocks readable/writeable.
 */
long do_test (char * buffer, int try, unsigned int current_block)
{
	long got;

	if (debug)
		printf ("DEBUG: do_test (buf,%d,%d)\n", try, current_block);
	
	/* Seek to the correct loc. */
	if (lseek (IN, current_block * BLOCK_SIZE, SEEK_SET) !=
                       current_block * BLOCK_SIZE)
                 die("seek failed during testing of blocks");

	/* Try the read */
	got = read (IN, buffer, try * BLOCK_SIZE);
	if (got < 0)
		got = 0;	
	if (got & (BLOCK_SIZE - 1))
		printf ("Weird values in do_test: probably bugs\n");
	got /= BLOCK_SIZE;
	return got;
}

static unsigned int currently_testing = 0;

void alarm_intr (int alnum)
{
	if (currently_testing >= ZONES)
		return;
	signal (SIGALRM, alarm_intr);
	alarm (5);
	if (!currently_testing)
		return;
	printf ("%d... ", currently_testing);
	fflush (stdout);
}

void test_blocks (void)
{
	int try, got;
	int currently_bad = badblocks;

	if (debug)
		printf ("DEBUG: test_blocks ()\n");
	if (verbose)
		printf ("Testing the disk for bad blocks ...\n");
	currently_testing = 0;
	signal (SIGALRM, alarm_intr);
	alarm (5);
	while (currently_testing < ZONES)
	{
		try = TEST_BUFFER_BLOCKS;
		if (currently_testing + try > ZONES)
			try = ZONES - currently_testing;
		if (currently_testing >= FIRSTZONE)
		{
			int i;

			/* Skip zones that are known to be bad */
			while (currently_testing < ZONES &&
			       zone_is_bad (currently_testing))
				currently_testing++;
			if (currently_testing >= ZONES)
				break;
			/* Shorten group if it contains a bad block */
			for (i = try - 1; i; i--)
			{
				if (zone_is_bad (currently_testing + try - i))
				{
					try -= i;
					break;
				}
			}
		}
		else if (currently_testing + try > FIRSTZONE)
			try = FIRSTZONE - currently_testing;
		got = do_test (blkbuf, try, currently_testing);
		currently_testing += got;
		if (got == try)
			continue;
		if (debug)
			printf ("Only got %d\n", got);
		if (currently_testing < FIRSTZONE)
			die ("bad blocks before data-area: cannot fix!");
		mark_bad (currently_testing);
		/* If this block is in use, we'll warn about it later.
		   For now, just make sure it is marked as used. */
		if (!zone_in_use (currently_testing))
		{
			mark_zone (currently_testing);
			FREEBLOCKSCOUNT--;
		}
		currently_testing++;
	}
	if (badblocks - currently_bad)
		printf ("found %d bad block%s\n", badblocks - currently_bad,
			(badblocks - currently_bad != 1) ? "s" : "");
}

void read_bad_zones (void)
{
	struct d_inode * inode = Inode + BAD_INO;
	int i;

	if (debug)
		printf ("DEBUG: read_bad_zones()\n");
	if (inode->i_mode || inode->i_nlinks)
	{
#if 1
		/* Temporary support for the old-style .badblock file. */
		if (inode_in_use(BAD_INO) && S_ISREG(inode->i_mode) &&
		    inode->i_nlinks == 1)
		{
			struct d_inode * dir = Inode + ROOT_INO;
			unsigned long ino;
			unsigned short name_len;
			char * name;

			read_block (dir->i_zone, blkbuf);
			name = blkbuf + 32 + 8;
			ino = * (unsigned long *) (name - 8);
			name_len = * (unsigned short *) (name - 2);
			if (ino == BAD_INO && name_len == 10 &&
			    !strncmp(name, ".badblocks", 10))
			{
				inode->i_mode = 0;
				inode->i_nlinks = 0;
				if (repair || test_disk)
				{
					/* Zero inode & write the dir block */
					printf ("Transforming .badblock file "
						"into reserved inode.\n");
					* (unsigned long *) (name - 8) = 0;
					write_block(dir->i_zone[0], blkbuf);
					changed = 1;
				}
				goto process_bad_blocks;
			}
		}
#endif
		printf ("Note: disk does not have a badblock inode.\n");
		if (test_disk)
			fatal_error ("You can't use -t on this disk.\n");
		no_bad_inode = 1;
		return;
	}
	if (!inode_in_use (BAD_INO))
	{
		printf ("The badblock inode is on the free list. ");
		if (test_disk && !repair)
			fatal_error ("\nYou need to specify -a or -r to repair this.");
		if (ask ("Mark in use", 1))
			mark_inode (BAD_INO);
	}
#if 1
process_bad_blocks:
#endif
	name_depth = -1;    /* Flag to indicate we're parsing the bad zones */
	for (i = 0; i < 9 ; i++)
		add_zone (i + inode->i_zone, 1);
	add_zone_ind (9 + inode->i_zone, 1);
	add_zone_dind (10 + inode->i_zone, 1);
	add_zone_tind (11 + inode->i_zone, 1);
	name_depth = 0;
}

void check_zones (unsigned int i)
{
	struct d_inode * inode;

	if (debug)
		printf ("DEBUG: check_zones(%d)\n", i);
	if (!i || i > INODES)
		return;
	if (inode_count [i] > 1)	/* have we counted this file already? */
		return;
	inode = Inode + i;
	if (!S_ISDIR (inode->i_mode) && !S_ISREG (inode->i_mode) &&
	    !S_ISLNK (inode->i_mode))
		return;
	for (i = 0; i < 9 ; i++)
		add_zone (i + inode->i_zone, 0);
	add_zone_ind (9 + inode->i_zone, 0);
	add_zone_dind (10 + inode->i_zone, 0);
	add_zone_tind (11 + inode->i_zone, 0);
}

unsigned short check_file (struct d_inode * dir, unsigned int offset)
{
	struct d_inode * inode;
	unsigned long ino;
	unsigned short rec_len;
	unsigned short name_len;
	char * name;
	int original_depth = name_depth;

	if (debug)
		printf ("DEBUG: check_file(%d,%d)\n", (int)dir, offset);
	changed |= mapped_read_block (dir, offset / BLOCK_SIZE, blkbuf);
	name = blkbuf + (offset % BLOCK_SIZE) + 8;
	ino = * (unsigned long *) (name - 8);
	rec_len = * (unsigned short *) (name - 4);
	name_len = * (unsigned short *) (name - 2);
	if (rec_len % 4 != 0 || rec_len < name_len + 8)
	{
		printf ("Bad directory entry in ");
		print_current_name ();
		printf ("\n");
		return 0;
	}
	if (ino && strcmp (".", name) && strcmp ("..", name))
	{
		if (name_depth < MAX_DEPTH)
		{
			strncpy (name_list [name_depth], name, name_len);
			name_list [name_depth] [name_len] = '\0';
		}
		name_depth++;	
	}
	if (!no_bad_inode && ino == BAD_INO)
	{
		printf ("File `");
		print_current_name ();
		printf ("' consists of the badblock list. ");
		if (ask ("Delete it", 1))
		{
			/* Zero the inode and write out the dir block */
			* (unsigned long *) (name - 8) = 0;
			write_block(last_block_read, blkbuf);
		}
		goto exit;
	}
	inode = get_inode (ino);
	if (!offset)
		if (!inode || strcmp (".",name))
		{
			print_current_name ();
			printf (": bad directory: '.' isn't first\n");
		}
		else
			goto exit;
	if (offset == 16)
		if (!inode || strcmp ("..",name))
		{
			print_current_name ();
			printf (": bad directory: '..' isn't second\n");
		}
		else
			goto exit;
	if (!inode)
		goto exit;
	if (list)
	{
		if (verbose)
			printf ("%6d %07o ", ino, inode->i_mode);
		print_current_name ();
		if (S_ISDIR (inode->i_mode))
			printf (":\n");
		else
			printf ("\n");
	}
	check_zones (ino);
	if (inode && S_ISDIR (inode->i_mode))
		recursive_check (ino);
exit:
	name_depth = original_depth;
	return rec_len;
}

void recursive_check (unsigned int ino)
{
	struct d_inode * dir;
	unsigned int offset;
	unsigned short rec_len;

	if (debug)
		printf ("DEBUG: recursive_check(%d)\n", ino);
	dir = Inode + ino;
	if (!S_ISDIR (dir->i_mode))
		die ("internal error");
	if (dir->i_size < 32)
	{
		print_current_name ();
		printf (": bad directory: size<32\n");
		return;
	}
	for (offset = 0; offset < dir->i_size; offset += rec_len)
		if ((rec_len = check_file(dir, offset)) < 8)
		{
			print_current_name ();
			printf (": bad directory: rec_len(%d) too short\n", rec_len);
			return;
		}
}

unsigned long get_free_block (void)
{
	static unsigned long blk = 0;

	if (!blk)
		blk = FIRSTZONE;
	while (blk < ZONES && (zone_in_use (blk) || blk == FIRSTFREEBLOCK))
		blk++;
	if (blk >= ZONES)
		die ("not enough good blocks");
	mark_zone(blk);
	zone_count [blk]++;
	FREEBLOCKSCOUNT--;
	return blk++;
}

inline unsigned long next_new_bad (unsigned long zone)
{
	if (!zone)
		zone = FIRSTZONE - 1;
	while (++zone < ZONES)
		if (zone_is_bad (zone) && !zone_count [zone])
		{
			zone_count [zone]++;
			return zone;
		}
	return 0;
}

inline int next_zone (unsigned long *znr, void *blk, unsigned long *pnr)
{
	if (*znr)
	{
		*pnr = *znr;
		read_block (znr, blk);
		return 0;
	}
	*pnr = *znr = get_free_block();
	memset (blk, 0, BLOCK_SIZE);
	return 1;
}

void update_bad_zone (void)
{
	struct ext_inode * inode = &Inode [BAD_INO];
	int i, j, k;
	unsigned long zone;
	int ind, dind, tind;
	int ind_dirty = 0, dind_dirty = 0, tind_dirty = 0;
	unsigned long *ind_block  = (long *)(blkbuf + BLOCK_SIZE);
	unsigned long *dind_block = (long *)(blkbuf + BLOCK_SIZE * 2);
	unsigned long *tind_block = (long *)(blkbuf + BLOCK_SIZE * 3);

	inode->i_time = time(NULL);
	inode->i_size = badblocks * BLOCK_SIZE;
	if (!badblocks)
		return;
	if (!(zone = next_new_bad (0)))
		return;
	for (i = 0; i < 9; i++)
	{
		if (inode->i_zone[i])
			continue;
		inode->i_zone[i] = zone;
		if (!(zone = next_new_bad (zone)))
			goto end_bad;
	}
	ind_dirty = next_zone (&inode->i_zone[9], ind_block, &ind);
	for (i = 0; i < 256; i++)
	{
		if (ind_block[i])
			continue;
		ind_block[i] = zone;
		ind_dirty = 1;
		if (!(zone = next_new_bad (zone)))
			goto end_bad;
	}
	dind_dirty = next_zone (&inode->i_zone[10], dind_block, &dind);
	for (i = 0; i < 256; i++)
	{
		if (ind_dirty)
		{
			write_block (ind, (char *) ind_block);
			ind_dirty = 0;
		}
		dind_dirty |= next_zone (&dind_block[i], ind_block, &ind);
		for (j = 0; j < 256; j++)
		{
			if (ind_block[j])
				continue;
			ind_block[j] = zone;
			ind_dirty = 1;
			if (!(zone = next_new_bad (zone)))
				goto end_bad;
		}
	}
	tind_dirty = next_zone (&inode->i_zone[11], tind_block, &tind);
	for (i = 0; i < 256; i++)
	{
		if (dind_dirty)
		{
			write_block (dind, (char *) dind_block);
			dind_dirty = 0;
		}
		tind_dirty |= next_zone (&tind_block[i], dind_block, &dind);
		for (j = 0; j < 256; j++)
		{
			if (ind_dirty)
			{
				write_block (ind, (char *) ind_block);
				ind_dirty = 0;
			}
			dind_dirty|=next_zone(&dind_block[j], ind_block, &ind);
			for (k = 0; k < 256; k++)
			{
				if (ind_block[k])
					continue;
				ind_block[k] = zone;
				ind_dirty = 1;
				if (!(zone = next_new_bad (zone)))
					goto end_bad;
			}
		}
	}
	printf ("Warning: there are too many bad blocks\n");
end_bad:
	if (ind_dirty)
		write_block (ind, (char *) ind_block);
	if (dind_dirty)
		write_block (dind, (char *) dind_block);
	if (tind_dirty)
		write_block (tind, (char *) tind_block);
}

void check_counts (void)
{
	int i;
	int free;

	if (debug)
		printf ("DEBUG: check_counts()\n");
	for (i = 1; i <= INODES; i++)
	{
		if (i == BAD_INO)
#if 1
		    if (!Inode[i].i_mode && !Inode[i].i_nlinks)
#endif
			continue;
		if (!inode_count[i] && inode_in_use (i))
		{
			printf ("Inode %d not used, "
				"not counted in the free list. ",
				i);
			if (ask ("Clear", 1))
				unmark_inode (i);
		}
		else if (inode_count[i] && !inode_in_use (i))
		{
			printf ("Inode %d used, "
				"counted in the free list. ",
				i);
			if (ask ("Set", 1))
				mark_inode (i);
		}
		if (inode_in_use (i) && Inode[i].i_nlinks != inode_count[i])
		{
			printf ("Inode %d, i_nlinks=%d, counted=%d. ",
				i, Inode[i].i_nlinks, inode_count[i]);
			if (ask ("Set i_nlinks to count", 1))
			{
				Inode[i].i_nlinks = inode_count[i];
				changed = 1;
			}
		}
	}
	for (i = 1; i <= INODES; i++)
	{
		if (inode_free_count[i] > 1)
		{
			printf ("Inode %d, counted %d times in the free list.",
				i, inode_free_count[i]);
			if (inode_freelist_to_salvage ||
				ask ("Salvage free list", 1))
			{
				printf ("Inode free list will be salvaged.\n");
				changed = 1;
				inode_freelist_to_salvage = 1;
			}
			else
				printf ("\n");
		}
	}
	for (i = 1, free=0; i <= INODES; i++)
		if (!inode_in_use (i))
			free++;
	if (free != FREEINODESCOUNT)
	{
		printf ("Free inodes count wrong (%d, counted=%d). ",
			FREEINODESCOUNT, free);
		if (inode_freelist_to_salvage)
			printf ("Free list will be salvaged.\n");
		else if (ask ("Correct", 1))
		{
			FREEINODESCOUNT = free;
			changed = 1;
		}
	}
	for (i = FIRSTZONE; i < ZONES; i++)
	{
/*		if (zone_in_use (i) == zone_count[i])
			continue;
*/
		if (!zone_count[i] && zone_in_use (i))
		{
			if (zone_is_bad (i))
				continue;
			printf ("Zone %d: marked in use, no file uses it. ", i);
			if (ask ("Unmark", 1))
				unmark_zone (i);
			continue;
		}
		if (zone_count[i] && !zone_in_use (i))
			printf ("Zone %d: %sin use, counted=%d\n",
				i, zone_in_use (i) ? "" : "not ", 
				zone_count[i]);
	}
	for (i = FIRSTZONE; i < ZONES; i++)
	{
		if (zone_free_count[i] > 1)
		{
			printf ("Zone %d, counted %d times in the free list. ",
				i, zone_free_count[i]);
			if (zone_freelist_to_salvage ||
				ask ("Salvage free list", 1))
			{
				printf ("Free zones list will be salvaged.\n");
				changed = 1;
				zone_freelist_to_salvage = 1;
			}
			else
				printf ("\n");
		}
	}
	for (i = FIRSTZONE, free=0; i < ZONES; i++)
		if (!zone_in_use (i))
			free++;
	if (free != FREEBLOCKSCOUNT)
	{
		printf ("Free zones count wrong (%d, counted=%d). ",
			FREEBLOCKSCOUNT, free);
		if (zone_freelist_to_salvage)
			printf (" Will be salvaged.\n");
		else if (ask ("Correct", 1))
		{
			FREEBLOCKSCOUNT = free;
			changed = 1;
		}
	}
}

static int count_ind (int *znr)
{
	char *block = blkbuf + BLOCK_SIZE;
	unsigned long * ptr;
	int count = 0;
	int i;

	if (!*znr || *znr > ZONES)
		return 0;
	read_block (znr, block);
	for (i = 0, ptr = (unsigned long *) block; i < (BLOCK_SIZE >> 2); i++)
		if (ptr[i])
			count = i + 1;
	return count;
}

static int count_dind (int *znr)
{
	char *block = blkbuf + BLOCK_SIZE * 2;
	unsigned long * ptr;
	int count = 0;
	int tmp;
	int i;

	if (!*znr || !znr > ZONES)
		return 0;
	read_block (znr, block);
	for (i = 0, ptr = (unsigned long *) block; i < (BLOCK_SIZE >> 2); i++)
	{
		tmp = count_ind (ptr + i);
		if (tmp)
			count = tmp + i * 256;
	}
	return count;
}

static int count_tind (int *znr)
{
	char *block = blkbuf + BLOCK_SIZE * 3;
	unsigned long * ptr;
	int count = 0;
	int tmp;
	int i;

	if (!*znr || !znr > ZONES)
		return 0;
	read_block (znr, block);
	for (i = 0, ptr = (unsigned long *) block; i < (BLOCK_SIZE >> 2); i++)
	{
		tmp = count_dind (ptr + i);
		if (tmp)
			count = tmp + i * 256 * 256;
	}
	return count;
}

void check_sizes (void)
{
	int i;
	int ino;
	struct d_inode * inode;
	int count = 0;
	int tmp;

	for (ino = 1; ino <= INODES; ino++)
	{
		if (!inode_in_use (ino))
			continue;
		inode = & Inode[ino];
		if (!S_ISDIR (inode->i_mode) && !S_ISREG (inode->i_mode) &&
		    !S_ISLNK (inode->i_mode))
			continue;
		count = 0;
		for (i = 0; i < 9; i++)
			if (inode->i_zone[i])
				count = i + 1;
		tmp = count_ind (inode->i_zone + 9);
		if (tmp)
			count = tmp + 9;
		tmp = count_dind (inode->i_zone + 10);
		if (tmp)
			count = tmp + 9 + 256;
		tmp = count_tind (inode->i_zone + 11);
		if (tmp)
			count = tmp + 9 + 256 + 256 * 256;
		if ((inode->i_size || count) &&
			inode->i_size < (count - 1) * BLOCK_SIZE)
		{
			printf ("Inode %d, incorrect size, %d (counted = %d). ",
				ino, inode->i_size, count * BLOCK_SIZE);
			if (ask ("Correct", 1))
			{
				inode->i_size = count * BLOCK_SIZE;
				changed = 1;
			}
		}
	}
}

void check(void)
{
	if (debug)
		printf ("DEBUG: check()\n");
	memset (inode_count, 0, INODES * sizeof (*inode_count));
	memset (zone_count, 0, ZONES * sizeof (*zone_count));
	memset (inode_free_count, 0, INODES * sizeof (*inode_free_count));
	memset (zone_free_count, 0, ZONES * sizeof (*zone_free_count));
	read_bad_zones ();
	if (test_disk)
		test_blocks ();
	check_zones (ROOT_INO);
	recursive_check (ROOT_INO);
	if (test_disk)
		update_bad_zone ();
	check_counts ();
	check_sizes ();
}

void salvage_zone_freelist (void)
{
	struct ext_free_block efb;
	unsigned long blk;
	unsigned long old_firstfreeblock;
	unsigned long block_to_write = 0;
	unsigned long next = 0;
	unsigned long count = 0;

	if (debug)
		printf ("DEBUG: salvage_zone_freelist()\n");
	if (verbose)
		printf ("Salvaging free zones list ...\n");
	old_firstfreeblock = FIRSTFREEBLOCK;
	blk = ZONES - 1;
	while (blk >= FIRSTZONE)
	{
		if (zone_in_use (blk) || blk == old_firstfreeblock)
		{
			blk--;
			continue;
		}
		if (!block_to_write)
		{
			block_to_write = blk;
			FIRSTFREEBLOCK = blk;
			efb.count = 0;
		}
		else
			efb.free[efb.count++] = blk;
		if (efb.count == 254)
		{
			efb.next = next;
			count += efb.count + 1;
			write_block (block_to_write, (char *) &efb);
			next = block_to_write;
			block_to_write = 0;
		}
		blk --;
	}
	if (block_to_write)
	{
		efb.next = next;
		write_block (block_to_write, (char *) &efb);
		count += efb.count + 1;
		next = block_to_write;
	}
	/* Try to keep the same first free block */
	if (!zone_in_use (old_firstfreeblock))
	{
		efb.count = 0;
		efb.next = next;
		write_block (old_firstfreeblock, (char *) &efb);
		count ++;
		FIRSTFREEBLOCK = old_firstfreeblock;
	}
	FREEBLOCKSCOUNT = count;
	if (verbose)
	{
		printf ("Free blocks count : %d\n", count);
		printf ("First free zone : %d\n", FIRSTFREEBLOCK);
	}
	if (old_firstfreeblock != FIRSTFREEBLOCK)
		printf ("First free block changed from %d to %d !!!\n"
			"If the file system is mounted, unmount it and run %s"
			" again.\n", old_firstfreeblock, FIRSTFREEBLOCK,
			program_name);
}

void salvage_inode_freelist ()
{
	struct ext_free_inode * inode_table = (struct ext_free_inode *) inode_buffer;
	struct ext_free_inode * efi = NULL;
	unsigned long inode;
	unsigned long old_firstfreeinode;
	unsigned long next = 0;
	unsigned long count = 0;

	if (debug)
		printf ("DEBUG: salvage_inode_freelist()\n");
	if (verbose)
		printf ("Salvaging free inodes list ...\n");
	old_firstfreeinode = FIRSTFREEINODE;
	inode = INODES;
	while (inode > 0)
	{
		if (inode_in_use (inode) || inode == old_firstfreeinode)
		{
			inode --;
			continue;
		}
		if (!efi)
		{
			efi = &inode_table[inode-1];
			efi->count = 0;
			efi->next = next;
			FIRSTFREEINODE = inode;
			next = inode;
		}
		else
			efi->free[efi->count++] = inode;
		if (efi->count == 14)
		{
			count += efi->count + 1;
			efi = NULL;
		}
		inode --;
	}
	if (efi)
		count += efi->count + 1;
	/* Try to keep the same first free inode */
	if (!inode_in_use (old_firstfreeinode))
	{
		efi = &inode_table[old_firstfreeinode - 1];
		efi->count = 0;
		efi->next = next;
		count ++;
		FIRSTFREEINODE = old_firstfreeinode;
	}
	FREEINODESCOUNT = count;
	if (verbose)
	{
		printf ("Free inodes count : %d\n", count);
		printf ("First free inode : %d\n", FIRSTFREEINODE);
	}
	if (old_firstfreeinode != FIRSTFREEINODE)
		printf ("First free inode changed from %d to %d !!!\n"
			"If the file system is mounted, unmount it and run %s"
			" again.\n", old_firstfreeinode, FIRSTFREEINODE,
			program_name);
}

void init_zone_bitmap (void)
{
	int i;
	unsigned long blknr;
	struct ext_free_block block;

	if (debug)
		printf ("DEBUG: init_zone_bitmap()\n");
	for (i = 0; i < ZONES; i++)
		mark_zone (i);
	/* Scan the free blocks list to unmark_zones */
	if (FIRSTFREEBLOCK)
	{
		blknr = FIRSTFREEBLOCK;
		read_block (&blknr, (char *) &block);
		while (blknr)
		{
			if (blknr < FIRSTZONE || blknr > ZONES)
			{
				printf ("Free blocks list corrupted. ");
				for (i = FIRSTZONE; i < ZONES; i++)
					unmark_zone (i);
				if (ask ("Salvage free list", 1))
				{
					changed = 1;
					zone_freelist_to_salvage = 1;
				}
				else
				{
					changed = 0;
					zone_freelist_to_salvage = 0;
				}
				return;
			}
			if (!zone_in_use (blknr))
			{
				printf ("Cycle in the free blocks list. ");
				if (ask ("Salvage free list", 1))
				{
					changed = 1;
					zone_freelist_to_salvage = 1;
				}
				else
				{
					changed = 0;
					zone_freelist_to_salvage = 0;
				}
				return;
			}
			if (debug)
				printf ("%d ", blknr);
			unmark_zone (blknr);
			zone_free_count [blknr] ++;
			for (i = 0; i < block.count; i++)
			{
				if (block.free[i] < FIRSTZONE ||
					block.free[i] > ZONES)
				{
					printf ("Free blocks list corrupted. ");
					for (i = FIRSTZONE; i < ZONES; i++)
						unmark_zone (i);
					if (ask ("Salvage free list", 1))
					{
						changed = 1;
						zone_freelist_to_salvage = 1;
					}
					else
					{
						changed = 0;
						zone_freelist_to_salvage = 0;
					}
					return;
				}
				if (debug)
					printf ("%d ", block.free[i]);
				unmark_zone (block.free[i]);
				zone_free_count [block.free[i]] ++;
			}
			blknr = block.next;
			if (blknr)
				read_block (&blknr, (char *) &block);
		}
	}
	changed = 0;
	zone_freelist_to_salvage = 0;
}

void init_inode_bitmap (void)
{
	int i;
	unsigned long ino;
	struct ext_free_inode * inode;

	if (debug)
		printf ("DEBUG: init_inode_bitmap()\n");
	for (i = 1; i <= INODES; i++)
		mark_inode(i);
	/* Scan the free inodes list to unmark inodes */
	if (FIRSTFREEINODE)
	{
		ino = FIRSTFREEINODE;
		inode = (struct ext_free_inode *) (Inode + ino);
		while (ino)
		{
			if (!ino || ino > INODES)
			{
				printf ("Free inodes list corrupted. ");
				for (i = 1; i <= INODES; i++)
					unmark_inode(i);
				if (ask ("Salvage free list", 1))
				{
					changed = 1;
					inode_freelist_to_salvage = 1;
				}
				else
				{
					changed = 0;
					inode_freelist_to_salvage = 0;
				}
				return;
			}
			if (!inode_in_use (ino))
			{
				printf ("Cycle in the free inodes list. ");
				if (ask ("Salvage free list", 1))
				{
					changed = 1;
					inode_freelist_to_salvage = 1;
				}
				else
				{
					changed = 0;
					inode_freelist_to_salvage = 0;
				}
				return;
			}
			unmark_inode (ino);
			if (debug)
				printf ("%d ", ino);
			inode_free_count [ino] ++;
			for (i = 0; i < inode->count; i++)
			{
				if (!inode->free[i] || inode->free[i] > INODES)
				{
					printf ("Free inodes list corrupted. ");
					for (i = 1; i <= INODES; i++)
						unmark_inode(i);
					if (ask ("Salvage free list", 1))
					{
						changed = 1;
						inode_freelist_to_salvage = 1;
					}
					else
					{
						changed = 0;
						inode_freelist_to_salvage = 0;
					}
					return;
				}
				if (debug)
					printf ("%d ", inode->free[i]);
				unmark_inode (inode->free[i]);
				inode_free_count [inode->free[i]] ++;
			}
			ino = inode->next;
			if (ino)
				inode = (struct ext_free_inode *) (Inode + ino);
		}
	}
	changed = 0;
	inode_freelist_to_salvage = 0;
}

int main (int argc, char ** argv)
{
	struct termios termios, tmp;
	int count;
	char c;

	printf ("efsck version alpha 8 - 07.09.92\n");
	if (argc && *argv)
		program_name = *argv;
	if (INODE_SIZE * EXT_INODES_PER_BLOCK != BLOCK_SIZE)
		die("bad inode size");
	while ((c = getopt (argc, argv, "larvstSd")) != EOF)
		switch (c)
		{
			case 'l': list=1; break;
			case 'a': automatic=1; repair=1; break;
			case 'r': automatic=0; repair=1; break;
			case 'v': verbose=1; break;
			case 's': show=1; break;
			case 't': test_disk=1; break;
			case 'S': salvage=1; break;
			case 'd': debug=1; break;
			default: usage();
		}
	if (optind != argc - 1)
		usage ();
	device_name = argv[optind];
	if (repair && !automatic)
	{
		if (! isatty (0) || ! isatty (1))
			die ("need terminal for interactive repairs");
		tcgetattr (0, &termios);
		tmp = termios;
		tmp.c_lflag &= ~(ICANON | ECHO);
		tcsetattr (0, TCSANOW, &tmp);
	}
	IN = open (device_name, repair || test_disk || salvage ? O_RDWR : O_RDONLY);
	if (IN < 0)
		die("unable to open '%s'");
	for (count = 0 ; count < 3; count++)
		sync();
	read_tables ();
	init_zone_bitmap ();
	init_inode_bitmap ();
	check_root ();
	check ();
	if (changed)
	{
		if (inode_freelist_to_salvage || salvage)
			salvage_inode_freelist ();
		if (zone_freelist_to_salvage || salvage)
			salvage_zone_freelist ();
		write_tables ();
		printf ("----------------------------\n"
			"FILE SYSTEM HAS BEEN CHANGED\n"
			"----------------------------\n");
		for (count = 0; count < 3; count++)
			sync ();
	}
	else if (salvage)
	{
		salvage_inode_freelist ();
		salvage_zone_freelist ();
		write_tables ();
	}
	if (verbose)
	{
		int free;

		free = FREEINODESCOUNT;
		printf ("\n%6d inode%s used (%d%%)\n", (INODES - free),
			((INODES - free) != 1) ? "s" : "",
			100 * (INODES - free) / INODES);
		free = FREEBLOCKSCOUNT;
		printf ("%6d zone%s used (%d%%)\n"
			"%6d bad block%s\n", (ZONES - free),
			((ZONES - free) != 1) ? "s" : "",
			100 * (ZONES - free) / ZONES, badblocks,
			badblocks != 1 ? "s" : "");
		printf ("\n%6d regular file%s\n"
			"%6d director%s\n"
			"%6d character device file%s\n"
			"%6d block device file%s\n"
			"%6d fifo%s\n"
			"%6d link%s\n"
			"%6d symbolic link%s\n"
			"------\n"
			"%6d file%s\n",
			regular, (regular != 1) ? "s" : "",
			directory, (directory != 1) ? "ies" : "y",
			chardev, (chardev != 1) ? "s" : "",
			blockdev, (blockdev != 1) ? "s" : "",
			fifo, (fifo != 1) ? "s" : "",
			links - 2 * directory + 1,
			((links - 2 * directory + 1) != 1) ? "s" : "",
			symlinks, (symlinks != 1) ? "s" : "",
			total - 2 * directory + 1,
			((total - 2 * directory + 1) != 1) ? "s" : "");
	}
	if (repair && !automatic)
		tcsetattr (0, TCSANOW, &termios);
	return (0);
}
