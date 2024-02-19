/*
 *  linux/fs/minix/inode.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/system.h>
#include <asm/segment.h>

void minix_put_inode(struct inode *inode)
{
	if (inode->i_nlink)
		return;
	inode->i_size = 0;
	minix_truncate(inode);
	minix_free_inode(inode);
}

void minix_put_super(struct super_block *sb)
{
	int i;

	lock_super(sb);
	sb->s_dev = 0;
	for(i = 0 ; i < MINIX_I_MAP_SLOTS ; i++)
		brelse(sb->u.minix_sb.s_imap[i]);
	for(i = 0 ; i < MINIX_Z_MAP_SLOTS ; i++)
		brelse(sb->u.minix_sb.s_zmap[i]);
	unlock_super(sb);
	return;
}

static struct super_operations minix_sops = { 
	minix_read_inode,
	minix_write_inode,
	minix_put_inode,
	minix_put_super,
	NULL,
	minix_statfs
};

struct super_block *minix_read_super(struct super_block *s,void *data)
{
	struct buffer_head *bh;
	struct minix_super_block *ms;
	int i,dev=s->s_dev,block;

	lock_super(s);
	if (!(bh = bread(dev,1,BLOCK_SIZE))) {
		s->s_dev=0;
		unlock_super(s);
		printk("bread failed\n");
		return NULL;
	}
	ms = (struct minix_super_block *) bh->b_data;
	s->s_blocksize = 1024;
	s->u.minix_sb.s_ninodes = ms->s_ninodes;
	s->u.minix_sb.s_nzones = ms->s_nzones;
	s->u.minix_sb.s_imap_blocks = ms->s_imap_blocks;
	s->u.minix_sb.s_zmap_blocks = ms->s_zmap_blocks;
	s->u.minix_sb.s_firstdatazone = ms->s_firstdatazone;
	s->u.minix_sb.s_log_zone_size = ms->s_log_zone_size;
	s->u.minix_sb.s_max_size = ms->s_max_size;
	s->s_magic = ms->s_magic;
	brelse(bh);
	if (s->s_magic != MINIX_SUPER_MAGIC) {
		s->s_dev = 0;
		unlock_super(s);
		printk("magic match failed\n");
		return NULL;
	}
	for (i=0;i < MINIX_I_MAP_SLOTS;i++)
		s->u.minix_sb.s_imap[i] = NULL;
	for (i=0;i < MINIX_Z_MAP_SLOTS;i++)
		s->u.minix_sb.s_zmap[i] = NULL;
	block=2;
	for (i=0 ; i < s->u.minix_sb.s_imap_blocks ; i++)
		if (s->u.minix_sb.s_imap[i]=bread(dev,block,BLOCK_SIZE))
			block++;
		else
			break;
	for (i=0 ; i < s->u.minix_sb.s_zmap_blocks ; i++)
		if (s->u.minix_sb.s_zmap[i]=bread(dev,block,BLOCK_SIZE))
			block++;
		else
			break;
	if (block != 2+s->u.minix_sb.s_imap_blocks+s->u.minix_sb.s_zmap_blocks) {
		for(i=0;i<MINIX_I_MAP_SLOTS;i++)
			brelse(s->u.minix_sb.s_imap[i]);
		for(i=0;i<MINIX_Z_MAP_SLOTS;i++)
			brelse(s->u.minix_sb.s_zmap[i]);
		s->s_dev=0;
		unlock_super(s);
		printk("block failed\n");
		return NULL;
	}
	s->u.minix_sb.s_imap[0]->b_data[0] |= 1;
	s->u.minix_sb.s_zmap[0]->b_data[0] |= 1;
	/* set up enough so that it can read an inode */
	s->s_dev = dev;
	s->s_op = &minix_sops;
	s->s_mounted = iget(s,MINIX_ROOT_INO);
	unlock_super(s);
	if (!s->s_mounted) {
		s->s_dev = 0;
		printk("get root inode failed\n");
		return NULL;
	}
	return s;
}

void minix_statfs(struct super_block *sb, struct statfs *buf)
{
	long tmp;

	put_fs_long(MINIX_SUPER_MAGIC, &buf->f_type);
	put_fs_long(1024, &buf->f_bsize);
	put_fs_long(sb->u.minix_sb.s_nzones << sb->u.minix_sb.s_log_zone_size, &buf->f_blocks);
	tmp = minix_count_free_blocks(sb);
	put_fs_long(tmp, &buf->f_bfree);
	put_fs_long(tmp, &buf->f_bavail);
	put_fs_long(sb->u.minix_sb.s_ninodes, &buf->f_files);
	put_fs_long(minix_count_free_inodes(sb), &buf->f_ffree);
	/* Don't know what value to put in buf->f_fsid */
}

#define inode_bmap(inode,nr) ((inode)->u.minix_i.i_data[(nr)])

static int block_bmap(struct buffer_head * bh, int nr)
{
	int tmp;

	if (!bh)
		return 0;
	tmp = ((unsigned short *) bh->b_data)[nr];
	brelse(bh);
	return tmp;
}

int minix_bmap(struct inode * inode,int block)
{
	int i;

	if (block<0) {
		printk("minix_bmap: block<0");
		return 0;
	}
	if (block >= 7+512+512*512) {
		printk("minix_bmap: block>big");
		return 0;
	}
	if (block < 7)
		return inode_bmap(inode,block);
	block -= 7;
	if (block < 512) {
		i = inode_bmap(inode,7);
		if (!i)
			return 0;
		return block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block);
	}
	block -= 512;
	i = inode_bmap(inode,8);
	if (!i)
		return 0;
	i = block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block>>9);
	if (!i)
		return 0;
	return block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block & 511);
}

static struct buffer_head * inode_getblk(struct inode * inode, int nr, int create)
{
	int tmp;
	unsigned short *p;
	struct buffer_head * result;

	p = inode->u.minix_i.i_data + nr;
repeat:
	tmp = *p;
	if (tmp) {
		result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
		if (tmp == *p)
			return result;
		brelse(result);
		goto repeat;
	}
	if (!create)
		return NULL;
	tmp = minix_new_block(inode->i_sb);
	if (!tmp)
		return NULL;
	result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
	if (*p) {
		minix_free_block(inode->i_sb,tmp);
		brelse(result);
		goto repeat;
	}
	*p = tmp;
	inode->i_ctime = CURRENT_TIME;
	inode->i_dirt = 1;
	return result;
}

static struct buffer_head * block_getblk(struct inode * inode, 
	struct buffer_head * bh, int nr, int create)
{
	int tmp;
	unsigned short *p;
	struct buffer_head * result;

	if (!bh)
		return NULL;
	if (!bh->b_uptodate) {
		ll_rw_block(READ,bh);
		wait_on_buffer(bh);
		if (!bh->b_uptodate) {
			brelse(bh);
			return NULL;
		}
	}
	p = nr + (unsigned short *) bh->b_data;
repeat:
	tmp = *p;
	if (tmp) {
		result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
		if (tmp == *p) {
			brelse(bh);
			return result;
		}
		brelse(result);
		goto repeat;
	}
	if (!create) {
		brelse(bh);
		return NULL;
	}
	tmp = minix_new_block(inode->i_sb);
	if (!tmp) {
		brelse(bh);
		return NULL;
	}
	result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
	if (*p) {
		minix_free_block(inode->i_sb,tmp);
		brelse(result);
		goto repeat;
	}
	*p = tmp;
	bh->b_dirt = 1;
	brelse(bh);
	return result;
}

struct buffer_head * minix_getblk(struct inode * inode, int block, int create)
{
	struct buffer_head * bh;

	if (block<0) {
		printk("minix_getblk: block<0");
		return NULL;
	}
	if (block >= 7+512+512*512) {
		printk("minix_getblk: block>big");
		return NULL;
	}
	if (block < 7)
		return inode_getblk(inode,block,create);
	block -= 7;
	if (block < 512) {
		bh = inode_getblk(inode,7,create);
		return block_getblk(inode, bh, block, create);
	}
	block -= 512;
	bh = inode_getblk(inode,8,create);
	bh = block_getblk(inode, bh, block>>9, create);
	return block_getblk(inode, bh, block & 511, create);
}

struct buffer_head * minix_bread(struct inode * inode, int block, int create)
{
	struct buffer_head * bh;

	bh = minix_getblk(inode,block,create);
	if (!bh || bh->b_uptodate)
		return bh;
	ll_rw_block(READ,bh);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
		return bh;
	brelse(bh);
	return NULL;
}

void minix_read_inode(struct inode * inode)
{
	struct buffer_head * bh;
	struct minix_inode * raw_inode;
	int block;

	block = 2 + inode->i_sb->u.minix_sb.s_imap_blocks + inode->i_sb->u.minix_sb.s_zmap_blocks +
		(inode->i_ino-1)/MINIX_INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev,block, BLOCK_SIZE)))
		panic("unable to read i-node block");
	raw_inode = ((struct minix_inode *) bh->b_data) +
		(inode->i_ino-1)%MINIX_INODES_PER_BLOCK;
	inode->i_mode = raw_inode->i_mode;
	inode->i_uid = raw_inode->i_uid;
	inode->i_gid = raw_inode->i_gid;
	inode->i_nlink = raw_inode->i_nlinks;
	inode->i_size = raw_inode->i_size;
	inode->i_mtime = inode->i_atime = inode->i_ctime = raw_inode->i_time;
	inode->i_blocks = inode->i_blksize = 0;
	if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
		inode->i_rdev = raw_inode->i_zone[0];
	else for (block = 0; block < 9; block++)
		inode->u.minix_i.i_data[block] = raw_inode->i_zone[block];
	brelse(bh);
	inode->i_op = NULL;
	if (S_ISREG(inode->i_mode))
		inode->i_op = &minix_file_inode_operations;
	else if (S_ISDIR(inode->i_mode))
		inode->i_op = &minix_dir_inode_operations;
	else if (S_ISLNK(inode->i_mode))
		inode->i_op = &minix_symlink_inode_operations;
	else if (S_ISCHR(inode->i_mode))
		inode->i_op = &minix_chrdev_inode_operations;
	else if (S_ISBLK(inode->i_mode))
		inode->i_op = &minix_blkdev_inode_operations;
	else if (S_ISFIFO(inode->i_mode)) {
		inode->i_op = &minix_fifo_inode_operations;
		inode->i_pipe = 1;
		PIPE_BASE(*inode) = NULL;
		PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;
		PIPE_READ_WAIT(*inode) = PIPE_WRITE_WAIT(*inode) = NULL;
		PIPE_READERS(*inode) = PIPE_WRITERS(*inode) = 0;
	}
}

void minix_write_inode(struct inode * inode)
{
	struct buffer_head * bh;
	struct minix_inode * raw_inode;
	int block;

	block = 2 + inode->i_sb->u.minix_sb.s_imap_blocks + inode->i_sb->u.minix_sb.s_zmap_blocks +
		(inode->i_ino-1)/MINIX_INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev, block, BLOCK_SIZE)))
		panic("unable to read i-node block");
	raw_inode = ((struct minix_inode *)bh->b_data) +
		(inode->i_ino-1)%MINIX_INODES_PER_BLOCK;
	raw_inode->i_mode = inode->i_mode;
	raw_inode->i_uid = inode->i_uid;
	raw_inode->i_gid = inode->i_gid;
	raw_inode->i_nlinks = inode->i_nlink;
	raw_inode->i_size = inode->i_size;
	raw_inode->i_time = inode->i_mtime;
	if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
		raw_inode->i_zone[0] = inode->i_rdev;
	else for (block = 0; block < 9; block++)
		raw_inode->i_zone[block] = inode->u.minix_i.i_data[block];
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
}
