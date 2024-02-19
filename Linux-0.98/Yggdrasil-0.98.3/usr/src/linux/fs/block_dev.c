/*
 *  linux/fs/block_dev.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

extern int *blk_size[];

int block_write(struct inode * inode, struct file * filp, char * buf, int count)
{
	int block = filp->f_pos >> BLOCK_SIZE_BITS;
	int offset = filp->f_pos & (BLOCK_SIZE-1);
	int chars;
	int written = 0;
	int size;
	unsigned int dev;
	struct buffer_head * bh;
	register char * p;

	dev = inode->i_rdev;
	if (blk_size[MAJOR(dev)])
		size = blk_size[MAJOR(dev)][MINOR(dev)];
	else
		size = 0x7fffffff;
	while (count>0) {
		if (block >= size)
			return written;
		chars = BLOCK_SIZE - offset;
		if (chars > count)
			chars=count;
		if (chars == BLOCK_SIZE)
			bh = getblk(dev, block, BLOCK_SIZE);
		else
			bh = breada(dev,block,block+1,block+2,-1);
		block++;
		if (!bh)
			return written?written:-EIO;
		p = offset + bh->b_data;
		offset = 0;
		filp->f_pos += chars;
		written += chars;
		count -= chars;
		memcpy_fromfs(p,buf,chars);
		p += chars;
		buf += chars;
		bh->b_uptodate = 1;
		bh->b_dirt = 1;
		brelse(bh);
	}
	return written;
}

int block_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	unsigned int block = filp->f_pos >> BLOCK_SIZE_BITS;
	unsigned int offset = filp->f_pos & (BLOCK_SIZE-1);
	unsigned int chars;
	unsigned int size;
	unsigned int dev;
	int read = 0;
	struct buffer_head * bh;
	register char * p;

	dev = inode->i_rdev;
	if (blk_size[MAJOR(dev)])
		size = blk_size[MAJOR(dev)][MINOR(dev)];
	else
		size = 0x7fffffff;
	while (count>0) {
		if (block >= size)
			return read;
		chars = BLOCK_SIZE-offset;
		if (chars > count)
			chars = count;
		if (!(bh = breada(dev,block,block+1,block+2,-1)))
			return read?read:-EIO;
		block++;
		p = offset + bh->b_data;
		offset = 0;
		filp->f_pos += chars;
		read += chars;
		count -= chars;
		memcpy_tofs(buf,p,chars);
		p += chars;
		buf += chars;
		brelse(bh);
	}
	return read;
}
