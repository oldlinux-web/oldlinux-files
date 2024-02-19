#ifndef _EXT_FS_I
#define _EXT_FS_I

/*
 * extended file system inode data in memory
 */
struct ext_inode_info {
	unsigned long i_data[16];
};

#endif
