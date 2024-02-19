/*
 * linux/fs/minix/minix_op.c
 *
 * structures for the minix super_block/inode/file-operations
 */

#include <linux/fs.h>
#include <linux/minix_fs.h>

/*
 * These are the low-level inode operations for minix filesystem inodes.
 */
struct inode_operations minix_inode_operations = {
	minix_create,
	minix_lookup,
	minix_link,
	minix_unlink,
	minix_symlink,
	minix_mkdir,
	minix_rmdir,
	minix_mknod,
	minix_rename,
	minix_readlink,
	minix_open,
	minix_release,
	minix_follow_link
};

/*
 * We have just NULL's here: the current defaults are ok for
 * the minix filesystem.
 */
struct file_operations minix_file_operations = {
	NULL,	/* lseek */
	NULL,	/* read */
	NULL	/* write */
};
	
