/*
 * Simple dirent routines for Linux.
 *
 * (C) 1991 Linus Torvalds
 */

#include <unistd.h>
#include <dirent.h>
#include <errno.h>

void rewinddir(DIR * dir)
{
	if (!dir) {
		errno = EBADF;
		return;
	}
	dir->dd_size = dir->dd_loc = 0;
	lseek(dir->dd_fd,0L,SEEK_SET);
}
