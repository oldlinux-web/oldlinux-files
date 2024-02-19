#ifndef _LINUX_VFS_H
#define _LINUX_VFS_H

typedef struct {
	long    val[2];
} fsid_t;

struct statfs {
	long f_type;
	long f_bsize;
	long f_blocks;
	long f_bfree;
	long f_bavail;
	long f_files;
	long f_ffree;
	fsid_t f_fsid;
	long f_spare[7];
};

#endif
