/*
 *  linux/fs/read_write.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>

int sys_lseek(unsigned int fd,off_t offset, unsigned int origin)
{
	struct file * file;
	int tmp;

	if (fd >= NR_OPEN || !(file=current->filp[fd]) || !(file->f_inode))
		return -EBADF;
	if (origin > 2)
		return -EINVAL;
	if (file->f_inode->i_pipe)
		return -ESPIPE;
	if (file->f_op && file->f_op->lseek)
		return file->f_op->lseek(file->f_inode,file,offset,origin);
/* this is the default handler if no lseek handler is present */
	switch (origin) {
		case 0:
			if (offset<0) return -EINVAL;
			file->f_pos=offset;
			break;
		case 1:
			if (file->f_pos+offset<0) return -EINVAL;
			file->f_pos += offset;
			break;
		case 2:
			if ((tmp=file->f_inode->i_size+offset) < 0)
				return -EINVAL;
			file->f_pos = tmp;
	}
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
	if (!count)
		return 0;
	verify_area(buf,count);
	if (file->f_op && file->f_op->read)
		return file->f_op->read(inode,file,buf,count);
/* these are the default read-functions */
	if (inode->i_pipe)
		return pipe_read(inode,file,buf,count);
	if (S_ISCHR(inode->i_mode))
		return char_read(inode,file,buf,count);
	if (S_ISBLK(inode->i_mode))
		return block_read(inode,file,buf,count);
	if (S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode))
		return minix_file_read(inode,file,buf,count);
	printk("(Read)inode->i_mode=%06o\n\r",inode->i_mode);
	return -EINVAL;
}

int sys_write(unsigned int fd,char * buf,unsigned int count)
{
	struct file * file;
	struct inode * inode;
	
	if (fd>=NR_OPEN || !(file=current->filp[fd]) || !(inode=file->f_inode))
		return -EBADF;
	if (!(file->f_mode&2))
		return -EBADF;
	if (!count)
		return 0;
	if (file->f_op && file->f_op->write)
		return file->f_op->write(inode,file,buf,count);
/* these are the default read-functions */
	if (inode->i_pipe)
		return pipe_write(inode,file,buf,count);
	if (S_ISCHR(inode->i_mode))
		return char_write(inode,file,buf,count);
	if (S_ISBLK(inode->i_mode))
		return block_write(inode,file,buf,count);
	if (S_ISREG(inode->i_mode))
		return minix_file_write(inode,file,buf,count);
	printk("(Write)inode->i_mode=%06o\n\r",inode->i_mode);
	return -EINVAL;
}
