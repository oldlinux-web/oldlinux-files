/*
 *  linux/fs/ext/inode.c
 *
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *
 *  from
 *
 *  linux/fs/minix/inode.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/ext_fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/system.h>
#include <asm/segment.h>

void ext_put_inode(struct inode *inode)
{
	if (inode->i_nlink)
		return;
	inode->i_size = 0;
	ext_truncate(inode);
	ext_free_inode(inode);
}

void ext_put_super(struct super_block *sb)
{

	lock_super(sb);
	sb->s_dev = 0;
	if (sb->u.ext_sb.s_firstfreeinodeblock)
		brelse (sb->u.ext_sb.s_firstfreeinodeblock);
	if (sb->u.ext_sb.s_firstfreeblock)
		brelse (sb->u.ext_sb.s_firstfreeblock);
	unlock_super(sb);
	return;
}

static struct super_operations ext_sops = { 
	ext_read_inode,
	ext_write_inode,
	ext_put_inode,
	ext_put_super,
	ext_write_super,
	ext_statfs
};

struct super_block *ext_read_super(struct super_block *s,void *data)
{
	struct buffer_head *bh;
	struct ext_super_block *es;
	int dev = s->s_dev,block;

	lock_super(s);
	if (!(bh = bread(dev, 1, BLOCK_SIZE))) {
		s->s_dev=0;
		unlock_super(s);
		printk("bread failed\n");
		return NULL;
	}
	es = (struct ext_super_block *) bh->b_data;
	s->s_blocksize = 1024;
	s->u.ext_sb.s_ninodes = es->s_ninodes;
	s->u.ext_sb.s_nzones = es->s_nzones;
	s->u.ext_sb.s_firstdatazone = es->s_firstdatazone;
	s->u.ext_sb.s_log_zone_size = es->s_log_zone_size;
	s->u.ext_sb.s_max_size = es->s_max_size;
	s->s_magic = es->s_magic;
	s->u.ext_sb.s_firstfreeblocknumber = es->s_firstfreeblock;
	s->u.ext_sb.s_freeblockscount = es->s_freeblockscount;
	s->u.ext_sb.s_firstfreeinodenumber = es->s_firstfreeinode;
	s->u.ext_sb.s_freeinodescount = es->s_freeinodescount;
	brelse(bh);
	if (s->s_magic != EXT_SUPER_MAGIC) {
		s->s_dev = 0;
		unlock_super(s);
		printk("magic match failed\n");
		return NULL;
	}
	if (!s->u.ext_sb.s_firstfreeblocknumber)
		s->u.ext_sb.s_firstfreeblock = NULL;
	else
		if (!(s->u.ext_sb.s_firstfreeblock = bread(dev,
			s->u.ext_sb.s_firstfreeblocknumber, BLOCK_SIZE))) {
			printk ("ext_read_super: unable to read first free block\n");
			s->s_dev = 0;
			unlock_super(s);
			return NULL;
		}
	if (!s->u.ext_sb.s_firstfreeinodenumber)
		s->u.ext_sb.s_firstfreeinodeblock = NULL;
	else {
		block = 2 + (s->u.ext_sb.s_firstfreeinodenumber - 1) / EXT_INODES_PER_BLOCK;
		if (!(s->u.ext_sb.s_firstfreeinodeblock = bread(dev, block, BLOCK_SIZE))) {
			printk ("ext_read_super: unable to read first free inode block\n");
			brelse(s->u.ext_sb.s_firstfreeblock);
			s->s_dev = 0;
			unlock_super (s);
			return NULL;
		}
	}
	unlock_super(s);
	/* set up enough so that it can read an inode */
	s->s_dev = dev;
	s->s_op = &ext_sops;
	if (!(s->s_mounted = iget(s,EXT_ROOT_INO))) {
		s->s_dev=0;
		printk("get root inode failed\n");
		return NULL;
	}
	return s;
}

void ext_write_super (struct super_block *sb)
{
	struct buffer_head * bh;
	struct ext_super_block * es;

	if (!(bh = bread(sb->s_dev, 1, BLOCK_SIZE))) {
		printk ("ext_write_super: bread failed\n");
		return;
	}
	es = (struct ext_super_block *) bh->b_data;
	es->s_firstfreeblock = sb->u.ext_sb.s_firstfreeblocknumber;
	es->s_freeblockscount = sb->u.ext_sb.s_freeblockscount;
	es->s_firstfreeinode = sb->u.ext_sb.s_firstfreeinodenumber;
	es->s_freeinodescount = sb->u.ext_sb.s_freeinodescount;
	bh->b_dirt = 1;
	brelse (bh);
	sb->s_dirt = 0;
}

void ext_statfs (struct super_block *sb, struct statfs *buf)
{
	long tmp;

	put_fs_long(EXT_SUPER_MAGIC, &buf->f_type);
	put_fs_long(1024, &buf->f_bsize);
	put_fs_long(sb->u.ext_sb.s_nzones << sb->u.ext_sb.s_log_zone_size,
		&buf->f_blocks);
	tmp = ext_count_free_blocks(sb);
	put_fs_long(tmp, &buf->f_bfree);
	put_fs_long(tmp, &buf->f_bavail);
	put_fs_long(sb->u.ext_sb.s_ninodes, &buf->f_files);
	put_fs_long(ext_count_free_inodes(sb), &buf->f_ffree);
	/* Don't know what value to put in buf->f_fsid */
}

#define inode_bmap(inode,nr) ((inode)->u.ext_i.i_data[(nr)])

static int block_bmap(struct buffer_head * bh, int nr)
{
	int tmp;

	if (!bh)
		return 0;
	tmp = ((unsigned long *) bh->b_data)[nr];
	brelse(bh);
	return tmp;
}

int ext_bmap(struct inode * inode,int block)
{
	int i;

	if (block<0) {
		printk("ext_bmap: block<0");
		return 0;
	}
	if (block >= 9+256+256*256+256*256*256) {
		printk("ext_bmap: block>big");
		return 0;
	}
	if (block<9)
		return inode_bmap(inode,block);
	block -= 9;
	if (block<256) {
		i = inode_bmap(inode,9);
		if (!i)
			return 0;
		return block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block);
	}
	block -= 256;
	if (block<256*256) {
		i = inode_bmap(inode,10);
		if (!i)
			return 0;
		i = block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block>>8);
		if (!i)
			return 0;
		return block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block & 255);
	}
	block -= 256*256;
	i = inode_bmap(inode,11);
	if (!i)
		return 0;
	i = block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block>>16);
	if (!i)
		return 0;
	i = block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),(block>>8) & 255);
	if (!i)
		return 0;
	return block_bmap(bread(inode->i_dev,i,BLOCK_SIZE),block & 255);
}

static struct buffer_head * inode_getblk(struct inode * inode, int nr, int create)
{
	int tmp;
	unsigned long * p;
	struct buffer_head * result;

	p = inode->u.ext_i.i_data + nr;
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
	tmp = ext_new_block(inode->i_sb);
	if (!tmp)
		return NULL;
	result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
	if (*p) {
		ext_free_block(inode->i_sb,tmp);
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
	unsigned long * p;
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
	p = nr + (unsigned long *) bh->b_data;
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
	tmp = ext_new_block(inode->i_sb);
	if (!tmp) {
		brelse(bh);
		return NULL;
	}
	result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
	if (*p) {
		ext_free_block(inode->i_sb,tmp);
		brelse(result);
		goto repeat;
	}
	*p = tmp;
	bh->b_dirt = 1;
	brelse(bh);
	return result;
}

struct buffer_head * ext_getblk(struct inode * inode, int block, int create)
{
	struct buffer_head * bh;

	if (block<0) {
		printk("ext_getblk: block<0\n");
		return NULL;
	}
	if (block >= 9+256+256*256+256*256*256) {
		printk("ext_getblk: block>big\n");
		return NULL;
	}
	if (block<9)
		return inode_getblk(inode,block,create);
	block -= 9;
	if (block<256) {
		bh = inode_getblk(inode,9,create);
		return block_getblk(inode,bh,block,create);
	}
	block -= 256;
	if (block<256*256) {
		bh = inode_getblk(inode,10,create);
		bh = block_getblk(inode,bh,block>>8,create);
		return block_getblk(inode,bh,block & 255,create);
	}
	block -= 256*256;
	bh = inode_getblk(inode,11,create);
	bh = block_getblk(inode,bh,block>>16,create);
	bh = block_getblk(inode,bh,(block>>8) & 255,create);
	return block_getblk(inode,bh,block & 255,create);
}

struct buffer_head * ext_bread(struct inode * inode, int block, int create)
{
	struct buffer_head * bh;

	bh = ext_getblk(inode,block,create);
	if (!bh || bh->b_uptodate) 
		return bh;
	ll_rw_block(READ,bh);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
		return bh;
	brelse(bh);
	return NULL;
}

void ext_read_inode(struct inode * inode)
{
	struct buffer_head * bh;
	struct ext_inode * raw_inode;
	int block;

	block = 2 + (inode->i_ino-1)/EXT_INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev, block, BLOCK_SIZE)))
		panic("unable to read i-node block");
	raw_inode = ((struct ext_inode *) bh->b_data) +
		(inode->i_ino-1)%EXT_INODES_PER_BLOCK;
	inode->i_mode = raw_inode->i_mode;
	inode->i_uid = raw_inode->i_uid;
	inode->i_gid = raw_inode->i_gid;
	inode->i_nlink = raw_inode->i_nlinks;
	inode->i_size = raw_inode->i_size;
	inode->i_mtime = inode->i_atime = inode->i_ctime = raw_inode->i_time;
	inode->i_blocks = inode->i_blksize = 0;
	if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
		inode->i_rdev = raw_inode->i_zone[0];
	else for (block = 0; block < 12; block++)
		inode->u.ext_i.i_data[block] = raw_inode->i_zone[block];
	brelse(bh);
	inode->i_op = NULL;
	if (S_ISREG(inode->i_mode))
		inode->i_op = &ext_file_inode_operations;
	else if (S_ISDIR(inode->i_mode))
		inode->i_op = &ext_dir_inode_operations;
	else if (S_ISLNK(inode->i_mode))
		inode->i_op = &ext_symlink_inode_operations;
	else if (S_ISCHR(inode->i_mode))
		inode->i_op = &ext_chrdev_inode_operations;
	else if (S_ISBLK(inode->i_mode))
		inode->i_op = &ext_blkdev_inode_operations;
	else if (S_ISFIFO(inode->i_mode)) {
		inode->i_op = &ext_fifo_inode_operations;
		inode->i_pipe = 1;
		PIPE_BASE(*inode) = NULL;
		PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;
		PIPE_READ_WAIT(*inode) = PIPE_WRITE_WAIT(*inode) = NULL;
		PIPE_READERS(*inode) = PIPE_WRITERS(*inode) = 0;
	}
}

void ext_write_inode(struct inode * inode)
{
	struct buffer_head * bh;
	struct ext_inode * raw_inode;
	int block;

	block = 2 + (inode->i_ino-1)/EXT_INODES_PER_BLOCK;
	if (!(bh=bread(inode->i_dev, block, BLOCK_SIZE)))
		panic("unable to read i-node block");
	raw_inode = ((struct ext_inode *)bh->b_data) +
		(inode->i_ino-1)%EXT_INODES_PER_BLOCK;
	raw_inode->i_mode = inode->i_mode;
	raw_inode->i_uid = inode->i_uid;
	raw_inode->i_gid = inode->i_gid;
	raw_inode->i_nlinks = inode->i_nlink;
	raw_inode->i_size = inode->i_size;
	raw_inode->i_time = inode->i_mtime;
	if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
		raw_inode->i_zone[0] = inode->i_rdev;
	else for (block = 0; block < 12; block++)
		raw_inode->i_zone[block] = inode->u.ext_i.i_data[block];
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
}
