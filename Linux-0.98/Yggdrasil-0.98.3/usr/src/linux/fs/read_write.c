/*
 *  linux/fs/read_write.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/minix_fs.h>

#include <asm/segment.h>

/*
 * Count is not yet used: but we'll probably support reading several entries
 * at once in the future. Use count=1 in the library for future expansions.
 */
int sys_readdir(unsigned int fd, struct dirent * dirent, unsigned int count)
{
	struct file * file;
	struct inode * inode;

	if (fd >= NR_OPEN || !(file = current->filp[fd]) ||
	    !(inode = file->f_inode))
		return -EBADF;
	if (file->f_op && file->f_op->readdir) {
		verify_area(dirent, sizeof (*dirent));
		return file->f_op->readdir(inode,file,dirent,count);
	}
	return -ENOTDIR;
}

int sys_lseek(unsigned int fd, off_t offset, unsigned int origin)
{
	struct file * file;
	int tmp = -1;

	if (fd >= NR_OPEN || !(file=current->filp[fd]) || !(file->f_inode))
		return -EBADF;
	if (origin > 2)
		return -EINVAL;
	if (file->f_op && file->f_op->lseek)
		return file->f_op->lseek(file->f_inode,file,offset,origin);

/* this is the default handler if no lseek handler is present */
	switch (origin) {
		case 0:
			tmp = offset;
			break;
		case 1:
			tmp = file->f_pos + offset;
			break;
		case 2:
			if (!file->f_inode)
				return -EINVAL;
			tmp = file->f_inode->i_size + offset;
			break;
	}
	if (tmp < 0)
		return -EINVAL;
	file->f_pos = tmp;
	file->f_reada = 0;
	return file->f_pos;
}

int sys_read(unsigned int fd,char * buf,unsigned int count)
{
	struct file * file;
	struct inode * inode;

	if (fd>=NR_OPEN || !(file=current->filp[fd]) || !(inode=file->f_inode))
		return -EBADF;
	if (!(file->f_mode & 1))
		return -EBADF;
	if (!file->f_op || !file->f_op->read)
		return -EINVAL;
	if (!count)
		return 0;
	verify_area(buf,count);
	return file->f_op->read(inode,file,buf,count);
}

int sys_write(unsigned int fd,char * buf,unsigned int count)
{
	struct file * file;
	struct inode * inode;
	
	if (fd>=NR_OPEN || !(file=current->filp[fd]) || !(inode=file->f_inode))
		return -EBADF;
	if (!(file->f_mode & 2))
		return -EBADF;
	if (!file->f_op || !file->f_op->write)
		return -EINVAL;
	if (!count)
		return 0;
	return file->f_op->write(inode,file,buf,count);
}
