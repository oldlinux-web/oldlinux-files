/*
	<dirent.h> -- definitions for SVR3 directory access routines

	last edit:	25-Apr-1987	D A Gwyn

	Prerequisite:	<sys/types.h>
*/

/* NOTE! The actual routines by D A Gwyn aren't used in linux - I though
 * they were too complicated, so I wrote my own. I use the header files,
 * though, as I didn't know what should be in them.
 */

#ifndef _DIRENT_H
#define _DIRENT_H

#include <traditional.h>
#include <sys/types.h>
#include <sys/dirent.h>

/* buffer size for fs-indep. dirs
 * must in general be larger than the filesystem buffer size
 */
/* For now, syscall readdir () only supports one entry at a time. It
 * will be changed in the future.
#define NUMENT		3
 */
#ifndef NUMENT
#define	NUMENT		1
#endif

typedef struct {
  int dd_fd;			/* file descriptor */
  int dd_loc;			/* offset in buffer */
  int dd_size;			/* # of valid entries in buffer */
  struct dirent *dd_buf;	/* -> directory buffer */
} DIR;				/* stream data from opendir() */

#ifdef __cplusplus
extern "C" {
#endif

extern DIR		*opendir _ARGS ((const char * __dirname));
extern struct dirent	*readdir _ARGS ((DIR *__dir));
extern off_t		telldir _ARGS ((DIR *__dir));
extern void		seekdir _ARGS ((DIR * __dir, off_t __loc));
extern void		rewinddir _ARGS ((DIR *__dir));
extern int		closedir _ARGS ((DIR *__dir));

typedef int (*__dir_select_fn_t)(const struct dirent *);
typedef int (*__dir_compar_fn_t)(const struct dirent * const *,
		const struct dirent * const *);

extern int scandir _ARGS ((const char *__dirname,
			struct dirent ***__namelist,
			__dir_select_fn_t __select,
			__dir_compar_fn_t __compar));

extern int	alphasort _ARGS ((const struct dirent * const *__d1,
			    const struct dirent * const *__d2));

#ifdef __cplusplus
}
#endif

#endif
