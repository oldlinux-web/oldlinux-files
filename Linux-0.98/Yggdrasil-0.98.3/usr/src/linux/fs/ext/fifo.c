/*
 *  linux/fs/fifo.c
 *
 *  written by Paul H. Hargrove.
 */

#include <linux/sched.h>
#include <linux/ext_fs.h>

extern struct file_operations def_fifo_fops;

struct inode_operations ext_fifo_inode_operations = {
	&def_fifo_fops,		/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};
