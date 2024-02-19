/*
 *  linux/fs/ext/truncate.c
 *
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *
 *  from
 *
 *  linux/fs/minix/truncate.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/ext_fs.h>
#include <linux/tty.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>

/*
 * Truncate has the most races in the whole filesystem: coding it is
 * a pain in the a**. Especially as I don't do any locking...
 *
 * The code may look a bit weird, but that's just because I've tried to
 * handle things like file-size changes in a somewhat graceful manner.
 * Anyway, truncating a file at the same time somebody else writes to it
 * is likely to result in pretty weird behaviour...
 *
 * The new code handles normal truncates (size = 0) as well as the more
 * general case (size = XXX). I hope.
 */

static int trunc_direct(struct inode * inode)
{
	int i, tmp;
	unsigned long * p;
	struct buffer_head * bh;
	int retry = 0;
#define DIRECT_BLOCK ((inode->i_size + 1023) >> 10)

repeat:
	for (i = DIRECT_BLOCK ; i < 9 ; i++) {
		p = inode->u.ext_i.i_data+i;
		if (!(tmp = *p))
			continue;
		bh = getblk(inode->i_dev,tmp,BLOCK_SIZE);
		if (i < DIRECT_BLOCK) {
			brelse(bh);
			goto repeat;
		}
		if ((bh && bh->b_count != 1) || tmp != *p) {
			retry = 1;
			brelse(bh);
			continue;
		}
		*p = 0;
		inode->i_dirt = 1;
		brelse(bh);
		ext_free_block(inode->i_sb,tmp);
	}
	return retry;
}

static int trunc_indirect(struct inode * inode, int offset, unsigned long * p)
{
	int i, tmp;
	struct buffer_head * bh;
	struct buffer_head * ind_bh;
	unsigned long * ind;
	int retry = 0;
#define INDIRECT_BLOCK (DIRECT_BLOCK-offset)

	tmp = *p;
	if (!tmp)
		return 0;
	ind_bh = bread(inode->i_dev, tmp, BLOCK_SIZE);
	if (tmp != *p) {
		brelse(ind_bh);
		return 1;
	}
	if (!ind_bh) {
		*p = 0;
		return 0;
	}
repeat:
	for (i = INDIRECT_BLOCK ; i < 256 ; i++) {
		if (i < 0)
			i = 0;
		if (i < INDIRECT_BLOCK)
			goto repeat;
		ind = i+(unsigned long *) ind_bh->b_data;
		tmp = *ind;
		if (!tmp)
			continue;
		bh = getblk(inode->i_dev,tmp,BLOCK_SIZE);
		if (i < INDIRECT_BLOCK) {
			brelse(bh);
			goto repeat;
		}
		if ((bh && bh->b_count != 1) || tmp != *ind) {
			retry = 1;
			brelse(bh);
			continue;
		}
		*ind = 0;
		ind_bh->b_dirt = 1;
		brelse(bh);
		ext_free_block(inode->i_sb,tmp);
	}
	ind = (unsigned long *) ind_bh->b_data;
	for (i = 0; i < 256; i++)
		if (*(ind++))
			break;
	if (i >= 256)
		if (ind_bh->b_count != 1)
			retry = 1;
		else {
			tmp = *p;
			*p = 0;
			inode->i_dirt = 1;
			ext_free_block(inode->i_sb,tmp);
		}
	brelse(ind_bh);
	return retry;
}

static int trunc_dindirect(struct inode * inode, int offset, unsigned long * p)
{
	int i,tmp;
	struct buffer_head * dind_bh;
	unsigned long * dind;
	int retry = 0;
#define DINDIRECT_BLOCK ((DIRECT_BLOCK-offset)>>8)

	tmp = *p;
	if (!tmp)
		return 0;
	dind_bh = bread(inode->i_dev, tmp, BLOCK_SIZE);
	if (tmp != *p) {
		brelse(dind_bh);
		return 1;
	}
	if (!dind_bh) {
		*p = 0;
		return 0;
	}
repeat:
	for (i = DINDIRECT_BLOCK ; i < 256 ; i ++) {
		if (i < 0)
			i = 0;
		if (i < DINDIRECT_BLOCK)
			goto repeat;
		dind = i+(unsigned long *) dind_bh->b_data;
		tmp = *dind;
		if (!tmp)
			continue;
		retry |= trunc_indirect(inode,offset+(i<<8),dind);
		dind_bh->b_dirt = 1;
	}
	dind = (unsigned long *) dind_bh->b_data;
	for (i = 0; i < 256; i++)
		if (*(dind++))
			break;
	if (i >= 256)
		if (dind_bh->b_count != 1)
			retry = 1;
		else {
			tmp = *p;
			*p = 0;
			inode->i_dirt = 1;
			ext_free_block(inode->i_sb,tmp);
		}
	brelse(dind_bh);
	return retry;
}

static int trunc_tindirect(struct inode * inode)
{
	int i,tmp;
	struct buffer_head * tind_bh;
	unsigned long * tind, * p;
	int retry = 0;
#define TINDIRECT_BLOCK ((DIRECT_BLOCK-(256*256+256+9))>>16)

	p = inode->u.ext_i.i_data+11;
	if (!(tmp = *p))
		return 0;
	tind_bh = bread(inode->i_dev, tmp, BLOCK_SIZE);
	if (tmp != *p) {
		brelse(tind_bh);
		return 1;
	}
	if (!tind_bh) {
		*p = 0;
		return 0;
	}
repeat:
	for (i = TINDIRECT_BLOCK ; i < 256 ; i ++) {
		if (i < 0)
			i = 0;
		if (i < TINDIRECT_BLOCK)
			goto repeat;
		tind = i+(unsigned long *) tind_bh->b_data;
		retry |= trunc_dindirect(inode,9+256+256*256+(i<<16),tind);
		tind_bh->b_dirt = 1;
	}
	tind = (unsigned long *) tind_bh->b_data;
	for (i = 0; i < 256; i++)
		if (*(tind++))
			break;
	if (i >= 256)
		if (tind_bh->b_count != 1)
			retry = 1;
		else {
			tmp = *p;
			*p = 0;
			inode->i_dirt = 1;
			ext_free_block(inode->i_sb,tmp);
		}
	brelse(tind_bh);
	return retry;
}

void ext_truncate(struct inode * inode)
{
	int retry;

	if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	     S_ISLNK(inode->i_mode)))
		return;
	while (1) {
		retry = trunc_direct(inode);
		retry |= trunc_indirect(inode,9,inode->u.ext_i.i_data+9);
		retry |= trunc_dindirect(inode,9+256,inode->u.ext_i.i_data+10);
		retry |= trunc_tindirect(inode);
		if (!retry)
			break;
		current->counter = 0;
		schedule();
	}
	inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_dirt = 1;
}

/*
 * Called when a inode is released. Note that this is different
 * from ext_open: open gets called at every open, but release
 * gets called only when /all/ the files are closed.
 */
void ext_release(struct inode * inode, struct file * filp)
{
	printk("ext_release not implemented\n");
}
