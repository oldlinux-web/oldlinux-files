#ifndef _LINUX_DIRENT_H
#define _LINUX_DIRENT_H

struct dirent {
	long		d_ino;
	off_t		d_off;
	unsigned short	d_reclen;
	char		d_name[NAME_MAX+1];
};

#endif
