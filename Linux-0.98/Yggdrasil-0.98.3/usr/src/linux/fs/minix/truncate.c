/*
 *  linux/fs/truncate.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/tty.h>
#include <linux/stat.h>
#include <linux/fcntl.h>

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
	unsigned short * p;
	struct buffer_head * bh;
	int i, tmp;
	int retry = 0;
#define DIRECT_BLOCK ((inode->i_size + 1023) >> 10)

repeat:
	for (i = DIRECT_BLOCK ; i < 7 ; i++) {
		p = i + inode->u.minix_i.i_data;
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
		minix_free_block(inode->i_sb,tmp);
	}
	return retry;
}

static int trunc_indirect(struct inode * inode, int offset, unsigned short * p)
{
	struct buffer_head * bh;
	int i, tmp;
	struct buffer_head * ind_bh;
	unsigned short * ind;
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
	for (i = INDIRECT_BLOCK ; i < 512 ; i++) {
		if (i < 0)
			i = 0;
		if (i < INDIRECT_BLOCK)
			goto repeat;
		ind = i+(unsigned short *) ind_bh->b_data;
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
		minix_free_block(inode->i_sb,tmp);
	}
	ind = (unsigned short *) ind_bh->b_data;
	for (i = 0; i < 512; i++)
		if (*(ind++))
			break;
	if (i >= 512)
		if (ind_bh->b_count != 1)
			retry = 1;
		else {
			tmp = *p;
			*p = 0;
			minix_free_block(inode->i_sb,tmp);
		}
	brelse(ind_bh);
	return retry;
}
		
static int trunc_dindirect(struct inode * inode)
{
	int i, tmp;
	struct buffer_head * dind_bh;
	unsigned short * dind, * p;
	int retry = 0;
#define DINDIRECT_BLOCK ((DIRECT_BLOCK-(512+7))>>9)

	p = 8 + inode->u.minix_i.i_data;
	if (!(tmp = *p))
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
	for (i = DINDIRECT_BLOCK ; i < 512 ; i ++) {
		if (i < 0)
			i = 0;
		if (i < DINDIRECT_BLOCK)
			goto repeat;
		dind = i+(unsigned short *) dind_bh->b_data;
		retry |= trunc_indirect(inode,7+512+(i<<9),dind);
		dind_bh->b_dirt = 1;
	}
	dind = (unsigned short *) dind_bh->b_data;
	for (i = 0; i < 512; i++)
		if (*(dind++))
			break;
	if (i >= 512)
		if (dind_bh->b_count != 1)
			retry = 1;
		else {
			tmp = *p;
			*p = 0;
			inode->i_dirt = 1;
			minix_free_block(inode->i_sb,tmp);
		}
	brelse(dind_bh);
	return retry;
}
		
void minix_truncate(struct inode * inode)
{
	int retry;

	if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	     S_ISLNK(inode->i_mode)))
		return;
	while (1) {
		retry = trunc_direct(inode);
		retry |= trunc_indirect(inode,7,inode->u.minix_i.i_data+7);
		retry |= trunc_dindirect(inode);
		if (!retry)
			break;
		current->counter = 0;
		schedule();
	}
	inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_dirt = 1;
}
