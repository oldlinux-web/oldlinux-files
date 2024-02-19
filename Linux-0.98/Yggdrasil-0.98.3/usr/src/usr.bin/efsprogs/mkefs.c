/*
 * mkefs.c - make a linux extended file-system.
 *
 * Copyright (C) 1992 Remy Card (card@masi.ibp.fr)
 *
 * Copyright (C) 1991 Linus Torvalds. This file may be redistributed as per
 * the Linux copyright.
 */

/*
 * 24.11.91  -	time began. Used the fsck sources to get started.
 *
 * 25.11.91  -	corrected some bugs. Added support for ".badblocks"
 *		The algorithm for ".badblocks" is a bit weird, but
 *		it should work. Oh, well.
 *
 * 25.01.92  -  Added the -l option for getting the list of bad blocks
 *              out of a named file. (Dave Rivers, rivers@ponds.uucp)
 *
 * 28.02.92  -	added %-information when using -c.
 *
 * 28.06.92  -	modifications by Remy Card (card@masi.ibp.fr)
 *		Used the mkfs sources to create mkefs.c
 *
 * 14.07.92  -	cleaned up the code
 *
 * 19.07.92  -	now use the same bit op than mkfs instead of an internal linked
 *		list of marked numbers
 *
 * 26.07.92  -	mkefs is now able to produce triple indirection for the
 *		badblocks file
 *
 * 11.08.92  -	changes by Wayne Davison (davison@borland.com)
 *		- Unified some of the buffers to make it use less memory at runtime.
 *		- Changed some ints to unsigned longs when referring to block
 *		  numbers (there's still more to "fix" in this area).
 *		- Fixed an off-by-one error in the alarm_intr() routine.
 *		- Removed a redundant lseek call in the block test code.
 *
 * 16.08.92  -  added the -i argument to specify the ratio of inodes
 *		use getopt() to get arguments
 *
 * 28.08.92  -  fixed the max size computation
 *
 * Usage:  mkefs [-c] [-i bytes-per-inode] device size-in-blocks
 *         mkefs [-l filename ] [-i bytes-per-inode] device size-in-blocks
 *
 *	-c for readablility checking (SLOW!)
 *      -l for getting a list of bad blocks from a file.
 *
 * Note:   even without the -c parameter, mkefs must scan all the file system
 *         to build the free blocks list
 *
 * The device may be a block device or a image of one, but this isn't
 * enforced (but it's not much fun on a character device :-). 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/stat.h>
#include <getopt.h>

#include <linux/fs.h>
#include <linux/ext_fs.h>

#ifndef __GNUC__
#error "needs gcc for the bitop-__asm__'s"
#endif

#ifndef __linux__
#define volatile
#endif

#define EXT_ROOT_INO	1
#define EXT_BAD_INO	2

#define TEST_BUFFER_BLOCKS	 16

static char blkbuf[BLOCK_SIZE * TEST_BUFFER_BLOCKS];

#define UPPER(size, n)		((size + ((n) - 1)) / (n))
#define INODE_SIZE		(sizeof (struct ext_inode))
#define INODE_BLOCKS		UPPER (INODES, EXT_INODES_PER_BLOCK)
#define INODE_BUFFER_SIZE	(INODE_BLOCKS * BLOCK_SIZE)

#define BITS_PER_BLOCK		(BLOCK_SIZE << 3)

static char * program_name = "mkefs";
static char * device_name = NULL;
static int inode_ratio = 4096;
static int DEV = -1;
static long BLOCKS = 0;
static int check = 0;
static int badblocks = 0;

#define ROOT_INO_STRING		"\001\000\000\000"

#define ROOT_RECLEN_STRING	"\020\000"

#define DOT_NAMELEN_STRING	"\001\000"
#define DOTDOT_NAMELEN_STRING	"\002\000"

static char root_block [BLOCK_SIZE] =
ROOT_INO_STRING ROOT_RECLEN_STRING DOT_NAMELEN_STRING    ".\0\0\0\0\0\0\0"
ROOT_INO_STRING ROOT_RECLEN_STRING DOTDOT_NAMELEN_STRING "..\0\0\0\0\0\0";

static char * inode_buffer = NULL;

#define Inode	(((struct ext_inode *) inode_buffer) - 1)

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
#define RESERVED1	(Super.s_reserved1)
#define RESERVED2	(Super.s_reserved2)
#define RESERVED3	(Super.s_reserved3)
#define RESERVED4	(Super.s_reserved4)
#define RESERVED5	(Super.s_reserved5)
#define MAGIC		(Super.s_magic)
#define NORM_FIRSTZONE	(2 + INODE_BLOCKS)

static char *inode_map = NULL;
static char *zone_map  = NULL;
static char *bad_map  = NULL;

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

#define inode_in_use(x)	(bit (inode_map, (x)))
#define zone_in_use(x)	(bit (zone_map, (x) - FIRSTZONE + 1))
#define zone_is_bad(x)  (bit (bad_map, (x) - FIRSTZONE + 1))

#define mark_inode(x)	(setbit (inode_map, (x)))
#define unmark_inode(x)	(clrbit (inode_map, (x)))

#define mark_zone(x)	(setbit (zone_map, (x) - FIRSTZONE + 1))
#define unmark_zone(x)	(clrbit (zone_map, (x) - FIRSTZONE + 1))

#define mark_bad(x)	(setbit (bad_map, (x) - FIRSTZONE + 1))

/*
 * Volatile to let gcc know that this doesn't return. When trying
 * to compile this under minix, volatile gives a warning, as
 * exit() isn't defined as volatile under minix.
 */
volatile void fatal_error (const char * fmt_string)
{
	fprintf (stderr, fmt_string, program_name, device_name);
	exit (1);
}

#define usage()	fatal_error ("usage: %s [-c | -l filename] [-i bytes-per-inode] /dev/name [blocks]\n")
#define die(str) fatal_error ("%s: " str "\n")

void write_tables (void)
{
	if (BLOCK_SIZE != lseek (DEV, BLOCK_SIZE, SEEK_SET))
		die ("seek failed in write_tables");
	if (BLOCK_SIZE != write (DEV, super_block_buffer, BLOCK_SIZE))
		die ("unable to write super-block");
	if (INODE_BUFFER_SIZE != write (DEV, inode_buffer, INODE_BUFFER_SIZE))
		die ("Unable to write inodes");
}

void write_block (int blk, char *buffer)
{

	if (blk * BLOCK_SIZE != lseek (DEV, blk * BLOCK_SIZE, SEEK_SET))
		die ("seek failed in write_block");
	if (BLOCK_SIZE != write(DEV, buffer, BLOCK_SIZE))
	{
		printf ("blk = %d\n", blk);
		die ("write failed in write_block");
	}
}

int get_free_block (void)
{
	static unsigned long blk = 0;

	if (!blk)
		blk = FIRSTZONE;
	while (blk < ZONES && zone_in_use (blk))
		blk++;
	if (blk >= ZONES)
		die ("not enough good blocks");
	mark_zone(blk);
	return blk++;
}

inline unsigned long next_bad (unsigned long zone)
{

	if (!zone)
		zone = FIRSTZONE - 1;
	while (++zone < ZONES)
		if (zone_is_bad (zone))
			return zone;
	return 0;
}

void make_bad_inode (void)
{
	struct ext_inode * inode = &Inode [EXT_BAD_INO];
	int i, j, k;
	unsigned long zone;
	int ind = 0, dind = 0, tind = 0;
	unsigned long *ind_block  = (long *)(blkbuf + BLOCK_SIZE);
	unsigned long *dind_block = (long *)(blkbuf + BLOCK_SIZE * 2);
	unsigned long *tind_block = (long *)(blkbuf + BLOCK_SIZE * 3);

#define NEXT_BAD (zone = next_bad (zone))

	mark_inode (EXT_BAD_INO);
	inode->i_nlinks = 0;
	inode->i_time = time(NULL);
	inode->i_mode = 0;
	inode->i_size = badblocks * BLOCK_SIZE;
	if (!badblocks)
		return;
	zone = next_bad (0);
	for (i = 0; i < 9; i++)
	{
		inode->i_zone[i] = zone;
		if (!NEXT_BAD)
			goto end_bad;
	}
	inode->i_zone[9] = ind = get_free_block();
	memset (ind_block, 0, BLOCK_SIZE);
	for (i = 0; i < 256; i++)
	{
		ind_block[i] = zone;
		if (!NEXT_BAD)
			goto end_bad;
	}
	inode->i_zone[10] = dind = get_free_block();
	memset (dind_block, 0, BLOCK_SIZE);
	for (i = 0; i < 256; i++)
	{
		write_block (ind, (char *) ind_block);
		dind_block[i] = ind = get_free_block();
		memset (ind_block, 0, BLOCK_SIZE);
		for (j = 0; j < 256; j++)
		{
			ind_block[j] = zone;
			if (!NEXT_BAD)
				goto end_bad;
		}
	}
	inode->i_zone[11] = tind = get_free_block();
	memset (tind_block, 0, BLOCK_SIZE);
	for (i = 0; i < 256; i++)
	{
		write_block (dind, (char *) dind_block);
		tind_block[i] = dind = get_free_block();
		memset (dind_block, 0, BLOCK_SIZE);
		for (j = 0; j < 256; j++)
		{
			write_block (ind, (char *) ind_block);
			dind_block[j] = ind = get_free_block();
			memset (ind_block, 0 , BLOCK_SIZE);
			for (k = 0; k < 256; k++)
			{
				ind_block[k] = zone;
				if (!NEXT_BAD)
					goto end_bad;
			}
		}
	}
	die ("too many bad blocks");
end_bad:
	if (ind)
		write_block (ind, (char *) ind_block);
	if (dind)
		write_block (dind, (char *) dind_block);
	if (tind)
		write_block (tind, (char *) tind_block);
}

void make_root_inode (void)
{
	struct ext_inode *inode = &Inode [EXT_ROOT_INO];

	mark_inode (EXT_ROOT_INO);
	inode->i_zone[0] = get_free_block();
	inode->i_nlinks = 2;
	inode->i_time = time(NULL);
	inode->i_size = 32;
	inode->i_mode = S_IFDIR + 0755;
	write_block (inode->i_zone[0], root_block);
}

void setup_tables (void)
{
	int i;

	memset (super_block_buffer, 0, BLOCK_SIZE);
	MAGIC = EXT_SUPER_MAGIC;
	ZONESIZE = 0;
	MAXSIZE = (1 << 31) - 1;
	RESERVED1 = 0;
	RESERVED2 = 0;
	RESERVED3 = 0;
	RESERVED4 = 0;
	RESERVED5 = 0;
	ZONES = BLOCKS;
/* some magic nrs: 1 inode / 4096 bytes (now use the inode ratio) */
	INODES = (BLOCKS * BLOCK_SIZE) / inode_ratio;
/* I don't want some off-by-one errors, so this hack... */
	if ((INODES & 8191) > 8188)
		INODES -= 5;
	if ((INODES & 8191) < 10)
		INODES -= 20;
/* Round the inode count to fully use the last block in the inode table */
	if (INODES % EXT_INODES_PER_BLOCK)
		INODES = ((INODES / EXT_INODES_PER_BLOCK) + 1) *
			 EXT_INODES_PER_BLOCK;
	FIRSTZONE = NORM_FIRSTZONE;
	inode_map = malloc (INODES / 8 + 1);
	if (!inode_map)
		die ("cannot allocate inode bitmap\n");
	zone_map = malloc ((ZONES - FIRSTZONE) / 8 + 1);
	if (!zone_map)
		die ("cannot allocate zone bitmap\n");
	bad_map = malloc ((ZONES - FIRSTZONE) / 8 + 1);
	if (!bad_map)
		die ("cannot allocate bad block bitmap\n");
	memset (inode_map, 0xff, INODES / 8 + 1);
	memset (zone_map, 0xff, (ZONES - FIRSTZONE) / 8 + 1);
	memset (bad_map, 0, (ZONES - FIRSTZONE) / 8 + 1);
	for (i = FIRSTZONE; i < ZONES; i++)
		unmark_zone (i);
	for (i = EXT_ROOT_INO; i <= INODES; i++)
		unmark_inode (i);
	inode_buffer = malloc (INODE_BUFFER_SIZE);
	if (!inode_buffer)
		die ("Unable to allocate buffer for inodes");
	memset (inode_buffer, 0, INODE_BUFFER_SIZE);
	printf ("%d inodes\n", INODES);
	printf ("%d blocks\n", ZONES);
	printf ("Firstdatazone=%d (%d)\n", FIRSTZONE, NORM_FIRSTZONE);
	printf ("Zonesize=%d\n", BLOCK_SIZE << ZONESIZE);
	printf ("Maxsize=%d\n\n", MAXSIZE);
}

/*
 * Perform a test of a block; return the number of
 * blocks readable/writeable.
 */
long do_check (char * buffer, int try, unsigned int current_block)
{

	long got;
	
	/* Seek to the correct loc. */
	if (lseek (DEV, current_block * BLOCK_SIZE, SEEK_SET) !=
                       current_block * BLOCK_SIZE)
                 die("seek failed during testing of blocks");

	/* Try the read */
	got = read (DEV, buffer, try * BLOCK_SIZE);
	if (got < 0)
		got = 0;	
	if (got & (BLOCK_SIZE - 1))
		printf("Weird values in do_check: probably bugs\n");
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

void check_blocks (void)
{

	int try, got;

	currently_testing = 0;
	signal (SIGALRM, alarm_intr);
	alarm (5);
	while (currently_testing < ZONES)
	{
		try = TEST_BUFFER_BLOCKS;
		if (currently_testing + try > ZONES)
			try = ZONES - currently_testing;
		got = do_check (blkbuf, try, currently_testing);
		currently_testing += got;
		if (got == try)
			continue;
		if (currently_testing < FIRSTZONE)
			die ("bad blocks before data-area: cannot make fs");
		mark_bad (currently_testing);
		mark_zone (currently_testing);
		badblocks++;
		currently_testing++;
	}
	if (badblocks)
		printf ("%d bad block%s\n", badblocks,
			(badblocks>1) ? "s" : "");
}

void get_list_blocks (char *filename)
{

	FILE *listfile;
	unsigned long blockno;

	listfile = fopen (filename,"r");
	if (listfile == (FILE *)NULL)
		die ("Can't open file of bad blocks");
	while (!feof (listfile))
	{
		fscanf (listfile, "%d\n", &blockno);
		mark_bad (blockno);
		mark_zone (blockno);
		badblocks++;
	}
	if (badblocks)
		printf("%d bad block%s\n", badblocks,
			(badblocks>1) ? "s" : "");
}

void make_free_blocks_list (void)
{
	struct ext_free_block efb;
	unsigned long blk;
	unsigned long block_to_write = 0;
	unsigned long next = 0;
	unsigned long count = 0;

	printf ("\nCreating free blocks list ...\n");
	efb.count = 0;
	blk = ZONES - 1;
	while (blk >= FIRSTZONE)
	{
		if (zone_in_use (blk))
		{
			blk--;
			continue;
		}
		if (!block_to_write)
		{
			block_to_write = blk;
			FIRSTFREEBLOCK = blk;
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
			efb.count = 0;
		}
		blk --;
	}
	if (block_to_write)
	{
		efb.next = next;
		write_block (block_to_write, (char *) &efb);
		count += efb.count + 1;
	}
	FREEBLOCKSCOUNT = count;
	printf ("Free blocks count : %d\n", count);
	printf ("First free block : %d\n", FIRSTFREEBLOCK);
}

void make_free_inodes_list (void)
{
	struct ext_free_inode * inode_table =
				(struct ext_free_inode *) inode_buffer;
	struct ext_free_inode * efi = NULL;
	unsigned long inode;
	unsigned long next = 0;
	unsigned long count = 0;

	printf ("\nCreating free inodes list ...\n");
	inode = INODES;
	while (inode > 0)
	{
		if (inode_in_use (inode))
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
	FREEINODESCOUNT = count;
	printf ("Free inodes count : %d\n", count);
	printf ("First free inode : %d\n", FIRSTFREEINODE);
}

static int
valid_offset( int fd, int offset )
{
    char ch;

    if( lseek( fd, offset, 0 ) < 0 ) return 0;
    if( read( fd, &ch, 1 ) < 1 ) return 0;
    return 1;
}

static int
countblocks( char *filename )
{
    unsigned long high, low;
    int fd;

    if( (fd = open( filename, O_RDONLY )) < 0 ) {
	perror (filename);
	exit (1);
    }
    low = 0;

    for( high = 1; valid_offset( fd, high ); high *= 2 ) {
	low = high;
    }
    while( low < high - 1 ) {
	const int mid = (low + high) / 2;
	if ( valid_offset( fd, mid)) {
	    low = mid;
	}
	else {
	    high = mid;
	}
    }
    valid_offset( fd, 0 );
    close( fd );
    return (low+1) / 1024 ;
}

int main (int argc, char **argv)
{
	char c;
	char * tmp;
	struct stat statbuf;
	char * listfile = NULL;

	fprintf (stderr, "mkefs version alpha 8 - 07.09.92\n");
	if (argc && *argv)
		program_name = *argv;
	if (INODE_SIZE * EXT_INODES_PER_BLOCK != BLOCK_SIZE)
		die("bad inode size");
	while ((c = getopt (argc, argv, "ci:l:")) != EOF)
		switch (c)
		{
			case 'c': check = 1; break;
			case 'i':
				inode_ratio = strtol (optarg, &tmp, 0);
				if (*tmp || inode_ratio < 4096)
				{
					printf ("bad inode raatio : %s\n",
						optarg);
					usage ();
				}
				break;
			case 'l':
				listfile = optarg;
				break;
			default:
				usage ();
		}
	device_name = argv [optind];
	if (optind == argc - 2)
		BLOCKS = strtol (argv[optind + 1], &tmp, 0);
	else if (optind == argc - 1) {
		BLOCKS = countblocks(device_name);
		tmp = "";
	    }
	else
		usage ();
	if (*tmp)
	{
		printf ("bad block count : %s\n", argv[optind + 1]);
		usage ();
	}
	if (check && listfile)
		die("-c and -l are incompatible\n");
	if (BLOCKS < 10)
		die("number of blocks must be greater than 10.");
	DEV = open (device_name, O_RDWR);
	if (DEV < 0)
		die("unable to open %s");
	if (fstat (DEV, &statbuf) < 0)
		die ("unable to stat %s");
	if (!S_ISBLK (statbuf.st_mode))
		check = 0;
	else if (statbuf.st_rdev == 0x0300 || statbuf.st_rdev == 0x0340)
		die ("will not try to make filesystem on '%s'");
	setup_tables ();
	if (check)
		check_blocks ();
        else if (listfile)
                get_list_blocks (listfile);
	make_root_inode ();
	make_bad_inode ();
	make_free_blocks_list ();
	make_free_inodes_list ();
	write_tables ();
	putchar ('\n');
	return 0;
}

