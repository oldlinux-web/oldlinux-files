/*
 *  linux/fs/truncate.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/tty.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

static int minix_free_ind(int dev,int block)
{
	struct buffer_head * bh;
	unsigned short * p;
	int i;
	int block_busy;

	if (!block)
		return 1;
	block_busy = 0;
	if (bh=bread(dev,block)) {
		p = (unsigned short *) bh->b_data;
		for (i=0;i<512;i++,p++)
			if (*p)
				if (minix_free_block(dev,*p)) {
					*p = 0;
					bh->b_dirt = 1;
				} else
					block_busy = 1;
		brelse(bh);
	}
	if (block_busy)
		return 0;
	else
		return minix_free_block(dev,block);
}

static int minix_free_dind(int dev,int block)
{
	struct buffer_head * bh;
	unsigned short * p;
	int i;
	int block_busy;

	if (!block)
		return 1;
	block_busy = 0;
	if (bh=bread(dev,block)) {
		p = (unsigned short *) bh->b_data;
		for (i=0;i<512;i++,p++)
			if (*p)
				if (minix_free_ind(dev,*p)) {
					*p = 0;
					bh->b_dirt = 1;
				} else
					block_busy = 1;
		brelse(bh);
	}
	if (block_busy)
		return 0;
	else
		return minix_free_block(dev,block);
}

void minix_truncate(struct inode * inode)
{
	int i;
	int block_busy;

	if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	     S_ISLNK(inode->i_mode)))
		return;
repeat:
	block_busy = 0;
	for (i=0;i<7;i++)
		if (inode->i_data[i]) {
			if (minix_free_block(inode->i_dev,inode->i_data[i]))
				inode->i_data[i]=0;
			else
				block_busy = 1;
		}
	if (minix_free_ind(inode->i_dev,inode->i_data[7]))
		inode->i_data[7] = 0;
	else
		block_busy = 1;
	if (minix_free_dind(inode->i_dev,inode->i_data[8]))
		inode->i_data[8] = 0;
	else
		block_busy = 1;
	inode->i_dirt = 1;
	if (block_busy) {
		current->counter = 0;
		schedule();
		goto repeat;
	}
	inode->i_size = 0;
	inode->i_mtime = inode->i_ctime = CURRENT_TIME;
}

/*
 * Called when a inode is released. Note that this is different
 * from minix_open: open gets called at every open, but release
 * gets called only when /all/ the files are closed.
 */
void minix_release(struct inode * inode, struct file * filp)
{
	printk("minix_release not implemented\n");
}

static int check_char_dev(struct inode * inode, struct file * filp)
{
	struct tty_struct *tty;
	int min, dev;

	dev = inode->i_rdev;
	if (MAJOR(dev) == 4 || MAJOR(dev) == 5) {
		if (MAJOR(dev) == 5)
			min = current->tty;
		else
			min = MINOR(dev);
		if (min < 0)
			return -1;
		if ((IS_A_PTY_MASTER(min)) && (inode->i_count>1))
			return -1;
		tty = TTY_TABLE(min);
		if (!(filp->f_flags & O_NOCTTY) &&
		    current->leader &&
		    current->tty<0 &&
		    tty->session==0) {
			current->tty = min;
			tty->session= current->session;
			tty->pgrp = current->pgrp;
		}
	}
	return 0;
}

/*
 * Called every time a minix-file is opened
 */
int minix_open(struct inode * inode, struct file * filp)
{
	if (S_ISCHR(inode->i_mode)) {
		if (check_char_dev(inode,filp))
			return -EAGAIN;
	} else if (S_ISBLK(inode->i_mode))
		check_disk_change(inode->i_rdev);
	filp->f_op = &minix_file_operations;
	return 0;
}
