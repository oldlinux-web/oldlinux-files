/* readfs - read a MINIX file system	Author: Paul Polderman */

/* Command: readfs - read and extract a MINIX filesystem.
 *
 * Syntax:  readfs [-li] block-special [directory]
 *
 * Flags: -l:	Extract files and dirs and produce a mkfs-listing on stdout
 * 	  -i:	Information only: give the listing, but do not extract files.
 *
 * Examples: readfs /dev/fd1		# extract all files from /dev/fd1.
 * 	     readfs -i /dev/hd2		# see what's on /dev/hd2.
 * 	     readfs -l /dev/at0 rootfs	# extract and list the filesystem
 * 					# of /dev/at0 and put the tree
 * 					# in the directory `rootfs'.
 *
 *   Readfs reads a MINIX filesystem and extracts recursively all directories
 * and files, and (optionally) produces a mkfs-listing of them on stdout.
 * The root directory contents are placed in the current directory, unless
 * a directory is given as argument, in which case the contents are put there.
 * Readfs tries to restore the attributes (mode/uid/gid/time) of the files
 * extracted to those of the original files.
 * Special files are created as ordinary files, but the mkfs-listing
 * enables mkfs to restore them to original.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

#include <minix/config.h>
#include <minix/const.h>
#include <minix/type.h>
#include "../fs/const.h"
#include "../fs/type.h"
#include "../fs/buf.h"
#include "../fs/super.h"


#undef printf			/* Definition used only in the kernel */

#define block_nr long		/* To allow big filesystems even if FS
			 * doesn't. */


/* Compile with -I/user0/ast/minix
 * (i.e. the directory containing the MINIX system sources)
 *
 *	Author: Paul Polderman (polder@cs.vu.nl) April 1987
 */

char verbose = 0;		/* give a mkfs-listing of the filesystem */
 /* And extracts its contents. */
char noaction = 0;		/* just give a mkfs-listing, do not extract
			 * files. */

struct super_block sb;
char pathname[1024];

main(argc, argv)
int argc;
char **argv;
{
  switch (argc) {
      case 2:
	pathname[0] = '\0';
	readfs(argv[1], pathname);
	break;
      case 3:
	if (argv[1][0] == '-') {
		get_flags(&argv[1][1]);
		pathname[0] = '\0';
		readfs(argv[2], pathname);
	} else {
		strcpy(pathname, argv[2]);
		readfs(argv[1], pathname);
	}
	break;
      case 4:
	if (argv[1][0] == '-') {
		get_flags(&argv[1][1]);
		strcpy(pathname, argv[3]);
		readfs(argv[2], pathname);
		break;
	}			/* else fall through .. */
      default:
	fprintf(stderr, "Usage: %s [-li] <special> [dirname]\n", argv[0]);
	exit(1);
  }
  exit(0);
}

get_flags(flags)
register char *flags;
{
  while (*flags) {
	switch (*flags) {
	    case 'L':
	    case 'l':	verbose = 1;	break;
	    case 'I':
	    case 'i':
		noaction = 1;
		verbose = 1;
		break;
	    default:
		fprintf(stderr, "Bad flag: %c\n", *flags);
		break;
	}
	flags++;
  }
}

#define	zone_shift	(sb.s_log_zone_size)	/* zone to block ratio */

readfs(special_file, directory)
char *special_file, *directory;
/* Readfs: opens the given special file (with MINIX filesystem),
 * and extracts its contents into the given directory.
 */
{
  d_inode root_inode;
  int special;
  off_t super_b;

  umask(0);

  /* Open the special file */
  if ((special = open(special_file, O_RDONLY)) < 0) {
	fprintf(stderr, "cannot open %s\n", special_file);
	return;
  }

  /* Read the superblock */
  super_b = (off_t) SUPER_BLOCK *(off_t) BLOCK_SIZE;
  if (lseek(special, super_b, SEEK_SET) != super_b) {
	fprintf(stderr, "cannot seek to superblock\n");
	return;
  }
  if (read(special, (char *) &sb, sizeof(struct super_block))
      != sizeof(struct super_block)) {
	fprintf(stderr, "cannot read superblock\n");
	return;
  }

  /* Is it really a MINIX filesystem ? */
  if (sb.s_magic != SUPER_MAGIC) {
	fprintf(stderr, "%s is not a valid MINIX filesystem\n",
		special_file);
	return;
  }

  /* Fetch the inode of the root directory */
  if (get_inode(special, (ino_t) ROOT_INODE, &root_inode) < 0) {
	fprintf(stderr, "cannot get inode of root directory\n");
	return;
  }

  /* Print number of blocks and inodes */
  if (verbose) printf("boot\n%ld %d\n",
	       (block_nr) sb.s_nzones << zone_shift, sb.s_ninodes);

  /* Extract (recursively) the root directory */
  dump_dir(special, &root_inode, directory);
}

/* Different type of blocks:	(used in routine get_block for caching) */

#define	B_INODE		0	/* Cache #0 is the inode cache */
#define	B_INDIRECT	1	/* Cache #1 is the (dbl) indirect block cache */
#define	B_DATA		2	/* No cache for data blocks (only read once) */

int get_inode(fd, inum, ip)
int fd;
ino_t inum;
d_inode *ip;

/* Get inode `inum' from the MINIX filesystem. (Uses the inode-cache) */
{
  struct buf bp;
  block_nr block;
  block_nr ino_block;
  unshort ino_offset;
  int r;

  /* Calculate start of i-list */
  block = SUPER_BLOCK + 1 + sb.s_imap_blocks + sb.s_zmap_blocks;

  /* Calculate block with inode inum */
  ino_block = ((inum - 1) / INODES_PER_BLOCK);
  ino_offset = ((inum - 1) % INODES_PER_BLOCK);
  block += ino_block;

  /* Fetch the block */
  if (get_block(fd, block, &bp, B_INODE) == 0) {
	bcopy(&bp.b_inode[ino_offset], ip, sizeof(d_inode));
	return(0);
  }

  /* Oeps, foutje .. */
  fprintf(stderr, "cannot find inode %d\n", inum);
  return(-1);
}

static int indent = 0;		/* current indent (used for mkfs-listing) */

dump_dir(special, ip, directory)
int special;
d_inode *ip;

char *directory;
/* Make the given directory (if non-NULL),
 * and recursively extract its contents.
 */
{
  register dir_struct *dp;
  register int n_entries;
  register char *name;
  block_nr b = 0;
  d_inode dip;
  struct buf bp;

  if (verbose) {
	show_info(directory, ip, "");
	indent++;
  }
  if (!noaction && *directory) {
	/* Try to make the directory if not already there */
	if (mkdir(directory) != 0 || chdir(directory) < 0) {
		fprintf(stderr, "mkdir %s failed\n", directory);
		return;
	}
  }
  for (name = directory; *name; name++)	/* Find end of pathname */
	;
  *name++ = '/';		/* Add trailing slash */

  n_entries = (int) (ip->i_size / (off_t) sizeof(dir_struct));
  while (n_entries > 0) {

	/* Read next block of the directory */
	if (get_fileblock(special, ip, b, &bp) < 0) return;
	dp = &bp.b_dir[0];
	if (b++ == (block_nr) 0) {
		dp += 2;	/* Skip "." and ".." */
		n_entries -= 2;
	}

	/* Extract the files/directories listed in the block */
	while (n_entries-- > 0 && dp < &bp.b_dir[NR_DIR_ENTRIES]) {
		if (dp->d_inum != (ino_t) 0) {
			if (get_inode(special, dp->d_inum, &dip) < 0) {
				/* Bad luck */
				dp++;
				continue;
			}

			/* Add new pathname-component to `pathname'. */
			strncpy(name, dp->d_name, NAME_MAX);
			name[NAME_MAX] = '\0';

			/* Call the right routine */
			if ((dip.i_mode & I_TYPE) == I_DIRECTORY)
				dump_dir(special, &dip, name);
			else
				dump_file(special, &dip, name);
		}
		dp++;		/* Next entry, please. */
	}
  }
  *--name = '\0';		/* Restore `pathname' to what it was. */
  if (!noaction && *directory) {
	chdir("..");		/* Go back up. */
	restore(directory, ip);	/* Restore mode/owner/accesstime */
  }
  if (verbose) {
	do_indent(--indent);	/* Let mkfs know we are done */
	printf("$\n");		/* with this directory. */
  }
}

dump_file(special, ip, filename)
int special;
d_inode *ip;
char *filename;
/* Extract given filename from the MINIX-filesystem,
 * and store it on the local filesystem.
 */
{
  dir_struct *dp;
  int file;
  block_nr b = 0;
  struct buf bp;
  off_t size;

  if (verbose) show_info(filename, ip, pathname);

  if (noaction) return(0);

  if (access(filename, 0) == 0) {
	/* Should not happen, but just in case .. */
	fprintf(stderr, "Will not create %s: file exists\n", filename);
	return(-1);
  }
  if ((file = creat(filename, (ip->i_mode & ALL_MODES))) < 0) {
	fprintf(stderr, "cannot create %s\n", filename);
	return(-1);
  }

  /* Don't try to extract /dev/hd0 */
  if ((ip->i_mode & I_TYPE) == I_REGULAR) {
	size = ip->i_size;
	while (size > (off_t) 0) {
		/* Get next block of file */
		if (get_fileblock(special, ip, b++, &bp) < 0) {
			close(file);
			return(-1);
		}

		/* Write it to the file */
		if (size > (off_t) BLOCK_SIZE)
			write(file, bp.b_data, BLOCK_SIZE);
		else
			write(file, bp.b_data, (int) size);

		size -= (off_t) BLOCK_SIZE;
	}
  }
  close(file);
  restore(filename, ip);	/* Restore mode/owner/filetimes */
  return(0);
}

int get_fileblock(special, ip, b, bp)
int special;
d_inode *ip;
block_nr b;
struct buf *bp;
/* Read the `b'-th block from the file whose inode is `ip'. */
{
  zone_nr zone, ind_zone;
  block_nr z, zone_index;
  int r;

  /* Calculate zone in which the datablock number is contained */
  zone = (zone_nr) (b >> zone_shift);

  /* Calculate index of the block number in the zone */
  zone_index = b - ((block_nr) zone << zone_shift);

  /* Go get the zone */
  if (zone < (zone_nr) NR_DZONE_NUM) {	/* direct block */
	zone = ip->i_zone[zone];
	z = ((block_nr) zone << zone_shift) + zone_index;
	r = get_block(special, z, bp, B_DATA);
	return(r);
  }

  /* The zone is not a direct one */
  zone -= (zone_nr) NR_DZONE_NUM;

  /* Is it single indirect ? */
  if (zone < (zone_nr) NR_INDIRECTS) {	/* single indirect block */
	ind_zone = ip->i_zone[NR_DZONE_NUM];
  } else {			/* double indirect block */
	/* Fetch the double indirect block */
	ind_zone = ip->i_zone[NR_DZONE_NUM + 1];
	z = (block_nr) ind_zone << zone_shift;
	r = get_block(special, z, bp, B_INDIRECT);
	if (r < 0) return(r);

	/* Extract the indirect zone number from it */
	zone -= (zone_nr) NR_INDIRECTS;
	ind_zone = bp->b_ind[zone / (zone_nr) NR_INDIRECTS];
	zone %= (zone_nr) NR_INDIRECTS;
  }

  /* Extract the datablock number from the indirect zone */
  z = (block_nr) ind_zone << zone_shift;
  r = get_block(special, z, bp, B_INDIRECT);
  if (r < 0) return(r);
  zone = bp->b_ind[zone];

  /* Calculate datablock number to be fetched */
  z = ((block_nr) zone << zone_shift) + zone_index;
  r = get_block(special, z, bp, B_DATA);
  return(r);
}

/* The following routines simulate a LRU block cache.
 *
 * Definition of a cache block:
 */

struct cache_block {
  block_nr b_block;		/* block number of block */
  long b_access;		/* counter value of last access */
  char b_buf[BLOCK_SIZE];	/* buffer for block */
};

#define	NR_CACHES	2	/* total number of caches */
#define	NR_CBLOCKS	5	/* number of blocks in a cache */

static struct cache_block cache[NR_CACHES][NR_CBLOCKS];
static long counter = 0L;	/* Counter used as a sense of time. */
 /* Incremented after each cache operation. */

get_block(fd, block, bp, type)
int fd;
block_nr block;
struct buf *bp;
int type;
/* Get the requested block from the device with filedescriptor fd.
 * If it is in the cache, no (floppy-) disk access is needed,
 * if not, allocate a cache block and read the block into it.
 */
{
  register int i;
  register struct cache_block *cache_p, *cp;

  if (block == (block_nr) NO_ZONE) {
	/* Should never happen in a good filesystem. */
	fprintf(stderr, "get_block: NO_ZONE requested !\n");
	return(-1);
  }
  if (type < 0 || type >= NR_CACHES)	/* No cache for this type */
	return(get_rawblock(fd, block, bp));

  cache_p = cache[type];
  cp = (struct cache_block *) 0;

  /* First find out if block requested is in the cache */
  for (i = 0; i < NR_CBLOCKS; i++) {
	if (cache_p[i].b_block == block) {	/* found right block */
		cp = &cache_p[i];
		break;
	}
  }

  if (cp == (struct cache_block *) 0) {	/* block is not in cache */
	cp = cache_p;		/* go find oldest buffer */
	for (i = 0; i < NR_CBLOCKS; i++) {
		if (cache_p[i].b_access < cp->b_access) cp = &cache_p[i];
	}

	/* Fill the buffer with the right block */
	if (get_rawblock(fd, block, cp->b_buf) < 0) return(-1);
  }

  /* Update/store last access counter */
  cp->b_access = ++counter;
  cp->b_block = block;
  bcopy(cp->b_buf, bp, BLOCK_SIZE);
  return(0);
}

int get_rawblock(special, blockno, bufp)
int special;
block_nr blockno;
char *bufp;
/* Read a block from the disk. */
{
  int r;
  off_t pos;

  /* Calculate the position of the block on the disk */
  pos = (off_t) blockno *(off_t) BLOCK_SIZE;

  /* Read the block from the disk */
  if (lseek(special, pos, SEEK_SET) == pos
      && read(special, bufp, BLOCK_SIZE) == BLOCK_SIZE)
	return(0);

  /* Should never get here .. */
  fprintf(stderr, "read block %d failed\n", blockno);
  return(-1);
}

restore(name, ip)
char *name;
d_inode *ip;
/* Restores given file's attributes.
 * `ip' contains the attributes of the file on the MINIX filesystem,
 * `name' is the filename of the extracted file on the local filesystem.
 */
{
  long ttime[2];

  chown(name, ip->i_uid, ip->i_gid);	/* Fails if not superuser */
  chmod(name, (ip->i_mode & ALL_MODES));
  ttime[0] = ttime[1] = ip->i_mtime;
  utime(name, ttime);
}

/* Characters to use as prefix to `mkfs' mode field */

static char special_chars[] = {
		       '-',	/* I_REGULAR */
		       'c',	/* I_CHAR_SPECIAL */
		       'd',	/* I_DIRECTORY */
		       'b'	/* I_BLOCK_SPECIAL */
};

show_info(name, ip, path)
char *name;
d_inode *ip;
char *path;
/* Show information about the given file/dir in `mkfs'-format */
{
  char c1, c2, c3;

  c1 = special_chars[(ip->i_mode >> 13) & 03];
  c2 = ((ip->i_mode & ALL_MODES & ~RWX_MODES) == I_SET_UID_BIT) ? 'u' : '-';
  c3 = ((ip->i_mode & ALL_MODES & ~RWX_MODES) == I_SET_GID_BIT) ? 'g' : '-';

  if (*name) {
	do_indent(indent);
	printf("%-14s ", name);
  }
  printf("%c%c%c%03o %d %d", c1, c2, c3,
         (ip->i_mode & RWX_MODES), ip->i_uid, ip->i_gid);

  switch (ip->i_mode & I_TYPE) {
      case I_DIRECTORY:
	break;
      case I_CHAR_SPECIAL:	/* Print major and minor dev numbers */
	printf(" %d %d", (ip->i_zone[0] >> MAJOR) & 0377,
	       (ip->i_zone[0] >> MINOR) & 0377);
	break;
      case I_BLOCK_SPECIAL:	/* Print major and minor dev numbers */
	printf(" %d %d", (ip->i_zone[0] >> MAJOR) & 0377,
	       (ip->i_zone[0] >> MINOR) & 0377);
	/* Also print the number of blocks on the device */
	printf(" %ld", (ip->i_size / (off_t) BLOCK_SIZE));
	break;
      default:			/* Just print the pathname */
	printf(" %s", path);
	break;
  }
  putchar('\n');
}

#define	INDENT_SIZE	4

do_indent(i)
int i;
{
  i *= INDENT_SIZE;
  while (i-- > 0) putchar(' ');
}

int mkdir(directory)
char *directory;
/* Make a directory, return exit status.
 * This routine is not necessary on systems that
 * have a system call to make directories.
 */
{
  int pid, status;

  if ((pid = fork()) == 0) {
	execl("/bin/mkdir", "mkdir", directory, (char *) 0);
	execl("/usr/bin/mkdir", "mkdir", directory, (char *) 0);
	exit(1);
  } else if (pid < 0)
	return(-1);
  while (wait(&status) != pid);
  return(status);
}

bcopy(src, dest, bytes)
register char *src, *dest;
register int bytes;
/* Copy bytes from src to dest.
 * May also be in the standard libc.a on some systems.
 */
{
  while (bytes--) *dest++ = *src++;
}
