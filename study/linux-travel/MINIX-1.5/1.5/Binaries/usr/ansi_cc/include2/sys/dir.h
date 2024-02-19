/* The <dir.h> header gives the layout of a directory. */

#ifndef _SYS_DIR_H
#define _SYS_DIR_H

#ifndef _SYS_TYPES_H		/* not quite right */
#include <sys/types.h>
#endif

#define	DIRBLKSIZ	512	/* size of directory block */

#ifndef DIRSIZ
#define	DIRSIZ	14
#endif

struct direct {
  ino_t d_ino;
  char d_name[DIRSIZ];
};

#endif /* _SYS_DIR_H */
