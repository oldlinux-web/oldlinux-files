/*
 *  linux/fs/isofs/blkdev.c
 *
 *  (C) 1992  Eric Youngdale Modified for ISO9660 filesystem.
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  isofs blkdev handling code.  This is only used with the Rock Ridge
 *  extensions to iso9660
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/iso_fs.h>
#include <linux/tty.h>
#include <linux/stat.h>
#include <linux/fcntl.h>

/*
 * Called every time a isofs block special file is opened
 */
static int blkdev_open(struct inode * inode, struct file * filp)
{
	int i;

	i = MAJOR(inode->i_rdev);
	if (i < MAX_BLKDEV) {
		filp->f_op = blkdev_fops[i];
		if (filp->f_op && filp->f_op->open)
			return filp->f_op->open(inode,filp);
	}
	return 0;
}	

/*
 * Dummy default file-operations: the only thing this does
 * is contain the open that then fills in the correct operations
 * depending on the special file...
 */
static struct file_operations def_blk_fops = {
	NULL,		/* lseek */
	NULL,		/* read */
	NULL,		/* write */
	NULL,		/* readdir */
	NULL,		/* select */
	NULL,		/* ioctl */
	blkdev_open,	/* open */
	NULL,		/* release */
};

struct inode_operations isofs_blkdev_inode_operations = {
	&def_blk_fops,		/* default file operations */
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
	isofs_bmap,		/* bmap */
	NULL			/* truncate */
};
