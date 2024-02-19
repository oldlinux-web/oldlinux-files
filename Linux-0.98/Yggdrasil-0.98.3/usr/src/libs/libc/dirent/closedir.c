/*
 * Simple dirent routines for Linux.
 *
 * (C) 1991 Linus Torvalds
 */

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

int closedir(DIR * dir)
{
	int fd;

	if (!dir) {
		errno = EBADF;
		return -1;
	}
	fd = dir->dd_fd;
	free(dir->dd_buf);
	free(dir);
	return close(fd);
}
