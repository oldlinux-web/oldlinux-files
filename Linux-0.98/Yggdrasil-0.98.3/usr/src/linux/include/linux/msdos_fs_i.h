#ifndef _MSDOS_FS_I
#define _MSDOS_FS_I

/*
 * msdos file system inode data in memory
 */
struct msdos_inode_info {
	int i_start;	/* first cluster or 0 */
	int i_attrs;	/* unused attribute bits */
	int i_busy;	/* file is either deleted but still open, or
			   inconsistent (mkdir) */
	struct inode *i_depend; /* pointer to inode that depends on the
				   current inode */
	struct inode *i_old;	/* pointer to the old inode this inode
				   depends on */
	int i_binary;	/* file contains non-text data */
};

#endif
