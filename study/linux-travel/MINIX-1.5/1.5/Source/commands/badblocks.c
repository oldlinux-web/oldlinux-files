/* badblocks - collect bad blocks in a file	Author: Jacob Bunschoten */

/* Usage "badblocks block_special [Up_to_7_blocks]" */

/* This program is written to handle BADBLOCKS on a hard or floppy disk.
 * The program asks for block_numbers. These numbers can be obtained with
 * the program disk_check; written by A. Tanenbaum.  It then creates a
 * file on the disk containing up to 7 bad blocks.
 *
 * BUG:
 *
 *	When the zone_size > block_size it can happen that
 *	the zone is already allocated. This means some
 *	file is using this zone and may use all the blocks including
 *	the bad one. This can be cured by inspecting the zone_bitmap
 *	(is already done) and change the file if this zone is used.
 *	This means that another zone must be allocated and
 *	the inode wich claims this zone must be found and changed.
 *
 */

#include <minix/config.h>
#include <minix/type.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>



/* 		Super block table.
 *
 * 	The disk layout is:
 *
 *      Item        # block
 *    boot block      1
 *    super block     1
 *    inode map     s_imap_blocks
 *    zone map      s_zmap_blocks
 *    inodes        (s_ninodes + 1 + INODES_PER_BLOCK - 1)/INODES_PER_BLOCK
 *    unused
 *    data zones    (s_nzones - s_firstdatazone) << s_log_zone_size
 *
 */


struct super_block {
  ino_t s_ninodes;		/* # usable inodes on the minor device */
  zone_nr s_nzones;		/* total device size, including bit maps etc */
  unshort s_imap_block;		/* # of block used by inode bit map */
  unshort s_zmap_block;		/* # of block used by zone bit map */
  zone_nr s_firstdatazone;	/* number of first data zone */
  short int s_log_zone_size;	/* log2 of block/zone */
  off_t s_max_size;		/* maximum file size on this device */
  int s_magic;			/* magic number to recognize super-block */
} super_block;

#define SUPER_MAGIC	0x137F


#define NR_ZONE_NUMS	9
#define NR_DZONE_NUMS	(NR_ZONE_NUMS -2 )

struct d_inode {		/* disk inode. */
  mode_t i_mode;		/* file type, protection, etc. */
  uid_t i_uid;			/* user id of the file's owner */
  off_t i_size;			/* current file size in bytes */
  time_t i_mtime;		/* when was file data last changed */
  gid_t i_gid;			/* group number */
  nlink_t i_nlinks;		/* how many links to this file */
  zone_nr i_zone[NR_ZONE_NUMS];	/* blocks nums for direct, ind, and dbl ind */
} d_inode;



#define OK	0
#define NOT_OK	1
#define QUIT	2

#define READ 	0
#define WRITE	1

#define HARMLESS	0
#define DIR_CREATED	1
#define DEV_MOUNTED	2
#define FILE_EXISTS	3
#define SUCCESS		4

#define BLOCK_SIZE	1024
#define INODES_PER_BLOCK	(BLOCK_SIZE/sizeof(struct d_inode))

#define INODE_SIZE (sizeof (struct d_inode) )
#define SUPER_SIZE (sizeof (struct super_block) )
#define SIZE_OF_INT   (sizeof (int) )

 /* ====== globals ======= */

char *dev_name;
char f_name[] = ".Bad_XXXXXX";
char file_name[50];
char dir_name[] = "/tmpXXXXXX";

int block[NR_DZONE_NUMS + 1];	/* last block contains zero */
int interactive;		/* 1 if interactive (argc == 2) */
int position = 2;		/* next block # is argv[position] */

FILE *fp, *fopen();
int fd;
int eofseen;			/* set if '\n' seen */
struct stat stat_buf;
struct d_inode *ip;
struct super_block *sp;

extern char *strcat();


 /* ====== super block routines ======= */

rw_super(flag)
{				/* read or write a superblock */
  int rwd;

  lseek(fd, 0L, SEEK_SET);	/* rewind */
  lseek(fd, (long) BLOCK_SIZE, SEEK_SET);	/* seek */

  if (flag == READ)
	rwd = read(fd, (char *) sp, SUPER_SIZE);
  else
	rwd = write(fd, (char *) sp, SUPER_SIZE);
  if (rwd != SUPER_SIZE) {	/* ok ? */
	printf("Bad %s in get_super() (should be %d is %d)\n",
	       flag == READ ? "read" : "write",
	       SUPER_SIZE, rwd);
	done(DIR_CREATED);
  }
}

get_super()
 /* Get super_block. global pointer sp is used */
{
  rw_super(READ);

  if (sp->s_magic != SUPER_MAGIC) {	/* check */
	printf("Bad magic number in super_block?!\n");
	done(DIR_CREATED);
  }
}


put_super()
{
  rw_super(WRITE);
}

 /* ========== inode routines =========== */

rw_inode(stat_ptr, rw_mode)
struct stat *stat_ptr;
{
  int rwd, i_num;
  long blk, offset;


  i_num = stat_ptr->st_ino;

  blk = (long) (2 + sp->s_imap_block + sp->s_zmap_block);
  blk += (long) ((i_num - 1) / INODES_PER_BLOCK);
  blk *= (long) (BLOCK_SIZE);	/* this block */

  offset = (long) ((i_num - 1) % INODES_PER_BLOCK);
  offset *= (long) (INODE_SIZE);/* and this offset */

  lseek(fd, 0L, SEEK_SET);	/* rewind */
  lseek(fd, (long) (blk + offset), SEEK_SET);	/* seek */

  /* Pointer is at the inode */
  if (rw_mode == READ) {	/* read it */
	rwd = read(fd, (char *) ip, INODE_SIZE);
  } else {			/* write it */
	rwd = write(fd, (char *) ip, INODE_SIZE);
  }
  if (rwd != INODE_SIZE) {	/* ok ? */
	printf("Bad %s in get_inode()\n", (rw_mode == READ) ? "read" :
	       "write");
	done(DIR_CREATED);
  }
}

get_inode(stat_ptr)
struct stat *stat_ptr;
{

  int cnt;

  rw_inode(stat_ptr, READ);

  for (cnt = 0; cnt < NR_ZONE_NUMS; cnt++)
	ip->i_zone[cnt] = 0;	/* Just to be safe */
}

put_inode(stat_ptr)
struct stat *stat_ptr;
{
  rw_inode(stat_ptr, WRITE);
}


 /* ==============  main program ================= */
main(argc, argv)
char *argv[];
{
  int cnt, finished;
  unsigned blk_nr;
  struct stat dev_stat;

  sp = &super_block;
  ip = &d_inode;

  if (argc < 2 || argc > 9) {
	fprintf(stderr, "Usage: %s block_special [up_to_7_blocks]\n", argv[0]);
	done(HARMLESS);
  }

  interactive = (argc == 2 ? 1 : 0);

  /* Do some test. */
  if (geteuid()) {
	printf("Sorry, not in superuser mode \n");
	printf("Set_uid bit must be on or you must become super_user\n");
	done(HARMLESS);
  }
  dev_name = argv[1];
  mktemp(dir_name);
  if (mkdir(dir_name, 0777) == -1) {
	fprintf(stderr, "%s is already used in system\n", dir_name);
	done(HARMLESS);
  }

  /* Mount device. This call may fail. */
  mount(dev_name, dir_name, 0);
  /* Succes. dev was mounted, try to umount */

  /* Umount device. Playing with the file system while other processes
   * have access to this device is asking for trouble */
  if (umount(dev_name) == -1) {
	printf("Could not umount device %s.\n", dev_name);
	done(HARMLESS);
  }
  mktemp(f_name);
  /* Create "/tmpXXXXpid/.BadXXpid" */
  strcat(file_name, dir_name);
  strcat(file_name, "/");
  strcat(file_name, f_name);

  if (mount(dev_name, dir_name, 0) == -1) {	/* this call should work */
	fprintf(stderr, "Could not mount device anymore\n");
	done(HARMLESS);
  }
  if (stat(file_name, &stat_buf) != -1) {
	printf("File %s already exists\n", file_name);
	done(DEV_MOUNTED);
  }
  if ((fp = fopen(file_name, "w")) == NULL) {
	printf("Cannot create file %s\n", file_name);
	done(DEV_MOUNTED);
  }
  chmod(file_name, 0);		/* "useless" file */
  if (stat(file_name, &stat_buf) == -1) {
	printf("What? Second call from stat failed\n");
	done(FILE_EXISTS);
  }

  /* Stat buf must be safed. We can now calculate the inode on disk */
  fclose(fp);

  /* ===== the badblock file is created ===== */

  if (umount(dev_name) == -1) {
	printf("Can not umount device anymore??? \n");
	done(DIR_CREATED);
  }
  if ((fd = open(dev_name, O_RDWR)) == -1) {
	printf("Can not open device %s\n", dev_name);
	done(DEV_MOUNTED);
  }
  if (fstat(fd, &dev_stat) == -1) {
	printf("fstat on device %s failed\n", dev_name);
	done(DEV_MOUNTED);
  }
  if ((dev_stat.st_mode & S_IFMT) != S_IFBLK) {
	printf("Device \"%s\" is not a block_special.\n", dev_name);
	done(DEV_MOUNTED);
  }
  get_super();
  if (sp->s_log_zone_size) {
	printf("Block_size != zone_size.");
	printf("This program can not handle it\n");
	done(DIR_CREATED);
  }
  get_inode(&stat_buf);

  for (finished = 0; !finished;) {
	if (interactive)
		printf("Give up to %d bad block numbers separated by spaces\n",
							       NR_DZONE_NUMS);
	reset_blks();
	cnt = 0;		/* cnt keep track of the zone's */
	while (cnt < NR_DZONE_NUMS) {
		int tst;

		if (interactive)
			blk_nr = rd_num();
		else
			blk_nr = rd_cmdline(argc, argv);
		if (blk_nr == -1) break;
		tst = blk_ok(blk_nr);

		/* Test if this block is free */
		if (tst == OK) {
			cnt++;
			save_blk(blk_nr);
		} else if (tst == QUIT)
			break;
	}
	if (interactive) show_blks();
	if (!cnt) done(FILE_EXISTS);
	if (interactive) {
	   switch (ok("All these blocks ok <y/n/q> (y:Device will change) ")) {
		case OK:	finished = 1; break;
		case NOT_OK:	break;
		case QUIT:	done(FILE_EXISTS);
	   }
	} else {
		finished = 1;
	}
  }

  modify(cnt);
  close(fd);			/* free device */
  done(SUCCESS);
}

rd_cmdline(argc, argv)
int argc;
char *argv[];
{
  if (position == argc) return(-1);
  return(atoi(argv[position++]));
}


modify(nr_blocks)
{
  int i;

  if (nr_blocks == 0) return;
  for (i = 0; i < nr_blocks; i++) {
	set_bit(block[i]);
	ip->i_zone[i] = block[i];
  }
  ip->i_size = (long) (BLOCK_SIZE * nr_blocks);	/* give file size */
  put_inode(&stat_buf);		/* save the inode on disk */
  put_super();			/* bit_maps too */
}


static blk_cnt = 0;

save_blk(blk_num)
int blk_num;
{
  block[blk_cnt++] = blk_num;
}

reset_blks()
{
  int i;

  for (i = 0; i <= NR_DZONE_NUMS; i++)
	block[i] = 0;		/* Note: Last block_number is set to zero */
  blk_cnt = 0;
}

show_blks()
{
  int i;

  for (i = 0; i < blk_cnt; i++) printf("Block[%d] = %d\n", i, block[i]);
}

blk_is_used(blk_num)
int blk_num;
{				/* return TRUE(1) if used */
  int i;

  for (i = 0; block[i] && block[i] != blk_num; i++);
  return(block[i] != 0) ? 1 : 0;
}


 /* ===== bitmap handling ======	 */

#define BIT_MAP_SHIFT	13
#define INT_BITS	(SIZE_OF_INT << 3)

blk_ok(num)			/* is this zone free (y/n) */
unsigned num;
{
  long blk_offset;
  int rd;
  int block, offset, words, bit, tst_word;
  unsigned z_num;

  if (num < 0) {
	return QUIT;		/* negative number is not allowed */
  }
  if (blk_is_used(num)) {
	printf("Duplicate block (%d) given\n", num);
	return NOT_OK;
  }

  /* Assumption zone_size == block_size */

  z_num = num - (sp->s_firstdatazone - 1);	/* account offset */

  /* Calculate the word in the bitmap. */
  block = z_num >> BIT_MAP_SHIFT;	/* which block */
  offset = z_num - (block << BIT_MAP_SHIFT);	/* offset */
  words = z_num / INT_BITS;	/* which word */

  blk_offset = (long) (2 + sp->s_imap_block);	/* zone map */
  blk_offset *= (long) BLOCK_SIZE;	/* of course in block */
  blk_offset += (long) (words * SIZE_OF_INT);	/* offset */


  lseek(fd, 0L, SEEK_SET);	/* rewind */
  lseek(fd, blk_offset, SEEK_SET);	/* set pointer at word */

  rd = read(fd, (char *) &tst_word, SIZE_OF_INT);
  if (rd != SIZE_OF_INT) {
	printf("Read error in bitmap\n");
	done(DIR_CREATED);
  }

  /* We have the tst_word, check if bit was off */
  bit = offset % INT_BITS;

  if (((tst_word >> bit) & 01) == 0)	/* free */
	return OK;
  else {
	printf("Bad number %d. ", num);
	printf("This zone (block) is marked in bitmap\n");
	return NOT_OK;
  }
}

set_bit(num)			/* write in the bitmap */
unsigned num;
{
  int rwd;
  long blk_offset;
  int block, offset, words, tst_word, bit;
  unsigned z_num;
  char wrd_str[17], bit_str[17];

  z_num = num + 1 - sp->s_firstdatazone;

  block = z_num >> BIT_MAP_SHIFT;	/* which block */
  offset = z_num - (block << BIT_MAP_SHIFT);	/* offset in block */
  words = offset / INT_BITS;	/* which word */

  blk_offset = (long) (2 + sp->s_imap_block);
  blk_offset *= (long) BLOCK_SIZE;
  blk_offset += (long) (words * SIZE_OF_INT);


  lseek(fd, 0L, SEEK_SET);	/* rewind */
  lseek(fd, blk_offset, SEEK_SET);

  rwd = read(fd, (char *) &tst_word, SIZE_OF_INT);
  if (rwd != SIZE_OF_INT) {
	printf("Read error in bitmap\n");
	done(DEV_MOUNTED);
  }
  bit = offset % INT_BITS;
  if (((tst_word >> bit) & 01) == 0) {	/* free */
	lseek(fd, 0L, SEEK_SET);/* rewind */
	lseek(fd, blk_offset, SEEK_SET);
	tst_word |= (1 << bit);	/* not free anymore */
	rwd = write(fd, (char *) &tst_word, SIZE_OF_INT);
	if (rwd != SIZE_OF_INT) {
		printf("Bad write in zone map\n");
		printf("Check file system \n");
		done(DIR_CREATED);
	}
	return;
  }
  printf("Bit map indicates that block %u is in use. Not marked.\n",num);
/*  done(DIR_CREATED); */
  return;
}

 /* ======= interactive interface ======= */

rd_num()
{				/* read a number from stdin */
  static num;
  int c;

  if (eofseen) return(-1);
  do {
	c = getchar();
	if (c == EOF || c == '\n') return(-1);
  } while (c != '-' && (c < '0' || c > '9'));

  if (c == '-') {
	printf("Block numbers must be positive\n");
	exit(1);
  }
  num = 0;
  while (c >= '0' && c <= '9') {
	num *= 10;
	num += c - '0';
	c = getchar();
	if (c == '\n') eofseen = 1;
  }
  return num;
}



ok(str)
char *str;
{
  int c;

  for (;;) {
	printf("%s", str);
	while ((c = getchar()) != EOF &&
	       c != 'y' && c != 'n' && c != 'q')
		if (c != '\n') printf(" Bad character %c\n", (char) c);
	switch (c) {
	    case EOF:		return QUIT;
	    case 'y':		return OK;
	    case 'n':		return NOT_OK;
	    case 'q':		return QUIT;
	}
	printf("\n");
  }
}


done(nr)
int nr;
{
  switch (nr) {
      case SUCCESS:
      case FILE_EXISTS:
	unlink(file_name);
      case DEV_MOUNTED:
	umount(dev_name);
      case DIR_CREATED:
	unlink(dir_name);
      case HARMLESS:;
  }
  sync();
  exit(nr == SUCCESS ? 0 : 1);
}

