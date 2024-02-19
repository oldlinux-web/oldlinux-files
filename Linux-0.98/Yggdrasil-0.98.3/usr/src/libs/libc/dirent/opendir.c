/*
 * Simple dirent routines for Linux.
 *
 * (C) 1991 Linus Torvalds
 */

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

DIR * opendir(const char * dirname)
{
	int fd;
	struct stat stat_buf;
	DIR * ptr;

	if ((fd = open(dirname,O_RDONLY))<0)
		return NULL;
	if (fstat(fd,&stat_buf)<0 ||
	    !S_ISDIR(stat_buf.st_mode) ||
	    !(ptr=malloc(sizeof(*ptr)))) {
		close(fd);
		return NULL;
	}
	memset(ptr,0,sizeof(*ptr));
	ptr->dd_fd = fd;
	return ptr;
}
