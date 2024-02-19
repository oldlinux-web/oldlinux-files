#ifndef _LINUX_LIMITS_H
#define _LINUX_LIMITS_H

#define NAME_MAX 255

#define NR_OPEN 32
#define NR_INODE 128
#define NR_FILE 128
#define NR_SUPER 8
#define NR_HASH 997
#define NR_FILE_LOCKS 32
#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS 10
#define MAX_CHRDEV 16
#define MAX_BLKDEV 16

#define NGROUPS_MAX       32	/* supplemental group IDs are available */
#define ARG_MAX       131072	/* # bytes of args + environ for exec() */
#define CHILD_MAX        999    /* no limit :-) */
#define OPEN_MAX          32	/* # open files a process may have */
#define LINK_MAX         127	/* # links a file may have */
#define MAX_CANON        255	/* size of the canonical input queue */
#define MAX_INPUT        255	/* size of the type-ahead buffer */
#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        1024	/* # chars in a path name */
#define PIPE_BUF        4095	/* # bytes in atomic write to a pipe */

#endif
