#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <traditional.h>
#include <sys/types.h>
#undef __NOT_KERNEL
#define __NOT_KERNEL
#include <linux/stat.h>
#undef __NOT_KERNEL

#ifdef OLD_LINUX
struct stat {
	dev_t   st_dev;
	ino_t   st_ino;
	umode_t st_mode;
	nlink_t st_nlink;
	uid_t   st_uid;
	gid_t   st_gid;
	dev_t   st_rdev;
	off_t   st_size;
	time_t  st_atime;
	time_t  st_mtime;
	time_t  st_ctime;
};
#else
struct stat {
	dev_t		st_dev;
	unsigned short	__pad1;
	ino_t		st_ino;
	umode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	unsigned short	__pad2;
	off_t		st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	time_t		st_atime;
	unsigned long	__unused1;
	time_t		st_mtime;
	unsigned long	__unused2;
	time_t		st_ctime;
	unsigned long	__unused3;
	unsigned long	__unused4;
	unsigned long	__unused5;
};
#endif

/* Some synonyms used historically in the kernel and elsewhere */
#define S_IREAD		S_IRUSR /* read permission, owner */
#define S_IWRITE	S_IWUSR /* write permission, owner */
#define S_IEXEC		S_IXUSR /* execute/search permission, owner */

#ifdef __cplusplus
extern "C" {
#endif

extern int	chmod _ARGS ((const char *__path, mode_t __mode));
extern int	fstat _ARGS ((int __fildes, struct stat *__stat_buf));
extern int	mkdir _ARGS ((const char *__path, mode_t __mode));
extern int	mkfifo _ARGS ((const char *__path, mode_t __mode));
extern int	stat _ARGS ((const char *__filename,
			struct stat *__stat_buf));
extern int	lstat _ARGS ((const char *__filename,
			struct stat *__stat_buf));
extern mode_t	umask _ARGS ((mode_t __mask));

#ifdef __cplusplus
}
#endif

#endif
