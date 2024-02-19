#ifdef _SYS_DIRENT_H
#error "Can't include both sys/dir.h and sys/dirent.h"
#define _SYS_DIR_H
#endif

#ifndef _SYS_DIR_H
#define _SYS_DIR_H

#include <traditional.h>
#include <limits.h>
#include <sys/types.h>

#ifndef DIRSIZ
#define DIRSIZ NAME_MAX
#endif

struct direct {
	long		d_ino;
	off_t		d_off;
	unsigned short	d_reclen;
	char		d_name[NAME_MAX+1];
};

typedef struct {
  int dd_fd;			/* file descriptor */
  int dd_loc;			/* offset in buffer */
  int dd_size;			/* # of valid entries in buffer */
  struct direct *dd_buf;	/* -> directory buffer */
} DIR;				/* stream data from opendir() */

#ifdef __cplusplus
extern "C" {
#endif

extern DIR		*opendir _ARGS ((const char * __dirname));
extern struct direct	*readdir _ARGS ((DIR *__dir));
extern off_t		telldir _ARGS ((DIR *__dir));
extern void		seekdir _ARGS ((DIR * __dir, off_t __loc));
extern void		rewinddir _ARGS ((DIR *__dir));
extern int		closedir _ARGS ((DIR *__dir));

#ifdef __cplusplus
}
#endif

#endif
