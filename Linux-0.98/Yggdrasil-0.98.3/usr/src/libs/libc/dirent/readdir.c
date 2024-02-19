/*
 * Simple dirent routines for Linux.
 *
 * (C) 1991 Linus Torvalds
 */

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/dir.h>
#include <string.h>
#include <errno.h>

static struct dirent result;

struct dirent * readdir(DIR * dir)
{
	struct direct * ptr;

	if (!dir) {
		errno = EBADF;
		return NULL;
	}
	if (!dir->dd_buf)
		if (!(dir->dd_buf = malloc(DIRBUF)))
			return NULL;
		else 
			dir->dd_size = dir->dd_loc = 0;
	while (1) {
		if (dir->dd_size <= dir->dd_loc) {
			dir->dd_loc = 0;
			dir->dd_size = read(dir->dd_fd,dir->dd_buf,DIRBUF);
		}
		if (dir->dd_size <= 0)
			return NULL;
		ptr = (struct direct *) (dir->dd_loc + dir->dd_buf);
		dir->dd_loc += sizeof (*ptr);
		if (!ptr->d_ino)
			continue;
		result.d_ino = ptr->d_ino;
		strncpy(result.d_name,ptr->d_name,NAME_MAX);
		result.d_name[NAME_MAX] = 0;
		result.d_reclen = strlen(result.d_name);
		return &result;
	}
}
