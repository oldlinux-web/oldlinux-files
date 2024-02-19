/*
 *  linux/fs/pipe.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/termios.h>

static int pipe_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	int chars, size, read = 0;

	if (!(filp->f_flags & O_NONBLOCK))
		while (!PIPE_SIZE(*inode)) {
			wake_up(& PIPE_WRITE_WAIT(*inode));
			if (!PIPE_WRITERS(*inode)) /* are there any writers? */
				return 0;
			if (current->signal & ~current->blocked)
				return -ERESTARTSYS;
			interruptible_sleep_on(& PIPE_READ_WAIT(*inode));
		}
	while (count>0 && (size = PIPE_SIZE(*inode))) {
		chars = PAGE_SIZE-PIPE_TAIL(*inode);
		if (chars > count)
			chars = count;
		if (chars > size)
			chars = size;
		memcpy_tofs(buf, PIPE_BASE(*inode)+PIPE_TAIL(*inode), chars );
		read += chars;
		PIPE_TAIL(*inode) += chars;
		PIPE_TAIL(*inode) &= (PAGE_SIZE-1);
		count -= chars;
		buf += chars;
	}
	wake_up(& PIPE_WRITE_WAIT(*inode));
	return read?read:-EAGAIN;
}
	
static int pipe_write(struct inode * inode, struct file * filp, char * buf, int count)
{
	int chars, size, written = 0;

	if (!PIPE_READERS(*inode)) { /* no readers */
		send_sig(SIGPIPE,current,0);
		return -EPIPE;
	}
/* if count < PAGE_SIZE, we have to make it atomic */
	if (count < PAGE_SIZE)
		size = PAGE_SIZE-count;
	else
		size = PAGE_SIZE-1;
	while (count>0) {
		while (PIPE_SIZE(*inode) >= size) {
			if (!PIPE_READERS(*inode)) { /* no readers */
				send_sig(SIGPIPE,current,0);
				return written?written:-EPIPE;
			}
			if (current->signal & ~current->blocked)
				return written?written:-ERESTARTSYS;
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;
			else
				interruptible_sleep_on(&PIPE_WRITE_WAIT(*inode));
		}
		while (count>0 && (size = (PAGE_SIZE-1)-PIPE_SIZE(*inode))) {
			chars = PAGE_SIZE-PIPE_HEAD(*inode);
			if (chars > count)
				chars = count;
			if (chars > size)
				chars = size;
			memcpy_fromfs(PIPE_BASE(*inode)+PIPE_HEAD(*inode), buf, chars );
			written += chars;
			PIPE_HEAD(*inode) += chars;
			PIPE_HEAD(*inode) &= (PAGE_SIZE-1);
			count -= chars;
			buf += chars;
		}
		wake_up(& PIPE_READ_WAIT(*inode));
		size = PAGE_SIZE-1;
	}
	return written;
}

static int pipe_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
{
	return -ESPIPE;
}

static int pipe_readdir(struct inode * inode, struct file * file, struct dirent * de, int count)
{
	return -ENOTDIR;
}

static int bad_pipe_rw(struct inode * inode, struct file * filp, char * buf, int count)
{
	return -EBADF;
}

static int pipe_ioctl(struct inode *pino, struct file * filp,
	unsigned int cmd, unsigned int arg)
{
	switch (cmd) {
		case FIONREAD:
			verify_area((void *) arg,4);
			put_fs_long(PIPE_SIZE(*pino),(unsigned long *) arg);
			return 0;
		default:
			return -EINVAL;
	}
}

static int pipe_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
{
	switch (sel_type) {
		case SEL_IN:
			if (!PIPE_EMPTY(*inode) || !PIPE_WRITERS(*inode))
				return 1;
			select_wait(&PIPE_READ_WAIT(*inode), wait);
			return 0;
		case SEL_OUT:
			if (!PIPE_FULL(*inode) || !PIPE_WRITERS(*inode))
				return 1;
			select_wait(&PIPE_WRITE_WAIT(*inode), wait);
			return 0;
		case SEL_EX:
			if (!PIPE_READERS(*inode) || !PIPE_WRITERS(*inode))
				return 1;
			select_wait(&inode->i_wait,wait);
			return 0;
	}
	return 0;
}

/*
 * Ok, these three routines NOW keep track of readers/writers,
 * Linus previously did it with inode->i_count checking.
 */
static void pipe_read_release(struct inode * inode, struct file * filp)
{
	PIPE_READERS(*inode)--;
	wake_up(&PIPE_WRITE_WAIT(*inode));
}

static void pipe_write_release(struct inode * inode, struct file * filp)
{
	PIPE_WRITERS(*inode)--;
	wake_up(&PIPE_READ_WAIT(*inode));
}

static void pipe_rdwr_release(struct inode * inode, struct file * filp)
{
	PIPE_READERS(*inode)--;
	PIPE_WRITERS(*inode)--;
	wake_up(&PIPE_READ_WAIT(*inode));
	wake_up(&PIPE_WRITE_WAIT(*inode));
}

/*
 * The three file_operations structs are not static because they
 * are also used in linux/fs/fifo.c to do operations on fifo's.
 */
struct file_operations read_pipe_fops = {
	pipe_lseek,
	pipe_read,
	bad_pipe_rw,
	pipe_readdir,
	pipe_select,
	pipe_ioctl,
	NULL,		/* no mmap on pipes.. surprise */
	NULL,		/* no special open code */
	pipe_read_release
};

struct file_operations write_pipe_fops = {
	pipe_lseek,
	bad_pipe_rw,
	pipe_write,
	pipe_readdir,
	pipe_select,
	pipe_ioctl,
	NULL,		/* mmap */
	NULL,		/* no special open code */
	pipe_write_release
};

struct file_operations rdwr_pipe_fops = {
	pipe_lseek,
	pipe_read,
	pipe_write,
	pipe_readdir,
	pipe_select,
	pipe_ioctl,
	NULL,		/* mmap */
	NULL,		/* no special open code */
	pipe_rdwr_release
};

int sys_pipe(unsigned long * fildes)
{
	struct inode * inode;
	struct file * f[2];
	int fd[2];
	int i,j;

	verify_area(fildes,8);
	for(j=0 ; j<2 ; j++)
		if (!(f[j] = get_empty_filp()))
			break;
	if (j==1)
		f[0]->f_count--;
	if (j<2)
		return -ENFILE;
	j=0;
	for(i=0;j<2 && i<NR_OPEN;i++)
		if (!current->filp[i]) {
			current->filp[ fd[j]=i ] = f[j];
			j++;
		}
	if (j==1)
		current->filp[fd[0]]=NULL;
	if (j<2) {
		f[0]->f_count--;
		f[1]->f_count--;
		return -EMFILE;
	}
	if (!(inode=get_pipe_inode())) {
		current->filp[fd[0]] = NULL;
		current->filp[fd[1]] = NULL;
		f[0]->f_count--;
		f[1]->f_count--;
		return -ENFILE;
	}
	f[0]->f_inode = f[1]->f_inode = inode;
	f[0]->f_pos = f[1]->f_pos = 0;
	f[0]->f_flags = f[1]->f_flags = 0;
	f[0]->f_op = &read_pipe_fops;
	f[0]->f_mode = 1;		/* read */
	f[1]->f_op = &write_pipe_fops;
	f[1]->f_mode = 2;		/* write */
	put_fs_long(fd[0],0+fildes);
	put_fs_long(fd[1],1+fildes);
	return 0;
}
