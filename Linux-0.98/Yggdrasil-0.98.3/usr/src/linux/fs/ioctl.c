/*
 *  linux/fs/ioctl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/termios.h>
#include <linux/fcntl.h> /* for f_flags values */

static int file_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	int block;

	switch (cmd) {
		case FIBMAP:
			if (filp->f_inode->i_op == NULL)
				return -EBADF;
		    	if (filp->f_inode->i_op->bmap == NULL)
				return -EINVAL;
			verify_area((void *) arg,4);
			block = get_fs_long((long *) arg);
			block = filp->f_inode->i_op->bmap(filp->f_inode,block);
			put_fs_long(block,(long *) arg);
			return 0;
		case FIGETBSZ:
			if (filp->f_inode->i_sb == NULL)
				return -EBADF;
			verify_area((void *) arg,4);
			put_fs_long(filp->f_inode->i_sb->s_blocksize,
			    (long *) arg);
			return 0;
		case FIONREAD:
			verify_area((void *) arg,4);
			put_fs_long(filp->f_inode->i_size - filp->f_pos,
			    (long *) arg);
			return 0;
		default:
			return -EINVAL;
	}
}


int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{	
	struct file * filp;
	int on;

	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
		return -EBADF;
	switch (cmd) {
		case FIOCLEX:
			current->close_on_exec |= (1 << fd);
			return 0;

		case FIONCLEX:
			current->close_on_exec &= ~(1 << fd);
			return 0;

		case FIONBIO:
			on = get_fs_long((unsigned long *) arg);
			if (on)
				filp->f_flags |= O_NONBLOCK;
			else
				filp->f_flags &= ~O_NONBLOCK;
			return 0;

		case FIOASYNC: /* O_SYNC is not yet implemented,
				  but it's here for completeness. */
			on = get_fs_long ((unsigned long *) arg);
			if (on)
				filp->f_flags |= O_SYNC;
			else
				filp->f_flags &= ~O_SYNC;
			return 0;

		default:
			if (filp->f_inode && S_ISREG(filp->f_inode->i_mode))
				return file_ioctl(filp,cmd,arg);

			if (filp->f_op && filp->f_op->ioctl)
				return filp->f_op->ioctl(filp->f_inode, filp, cmd,arg);

			return -EINVAL;
	}
}
