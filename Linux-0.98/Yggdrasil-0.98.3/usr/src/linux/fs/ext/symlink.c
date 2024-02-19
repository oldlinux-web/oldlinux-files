/*
 *  linux/fs/ext/symlink.c
 *
 *  Copyright (C) 1992 Remy Card (card@masi.ibp.fr)
 *
 *  from
 *
 *  linux/fs/minix/symlink.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  ext symlink handling code
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/ext_fs.h>
#include <linux/stat.h>

static int ext_readlink(struct inode *, char *, int);
static int ext_follow_link(struct inode *, struct inode *, int, int, struct inode **);

/*
 * symlinks can't do much...
 */
struct inode_operations ext_symlink_inode_operations = {
	NULL,			/* no file-operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	ext_readlink,		/* readlink */
	ext_follow_link,	/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};

static int ext_follow_link(struct inode * dir, struct inode * inode,
	int flag, int mode, struct inode ** res_inode)
{
	int error;
	unsigned short fs;
	struct buffer_head * bh;

	*res_inode = NULL;
	if (!dir) {
		dir = current->root;
		dir->i_count++;
	}
	if (!inode) {
		iput(dir);
		return -ENOENT;
	}
	if (!S_ISLNK(inode->i_mode)) {
		iput(dir);
		*res_inode = inode;
		return 0;
	}
	if (current->link_count > 5) {
		iput(dir);
		iput(inode);
		return -ELOOP;
	}
	if (!(bh = ext_bread(inode, 0, 0))) {
		iput(inode);
		iput(dir);
		return -EIO;
	}
	iput(inode);
	__asm__("mov %%fs,%0":"=r" (fs));
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	current->link_count++;
	error = open_namei(bh->b_data,flag,mode,res_inode,dir);
	current->link_count--;
	__asm__("mov %0,%%fs"::"r" (fs));
	brelse(bh);
	return error;
}

static int ext_readlink(struct inode * inode, char * buffer, int buflen)
{
	struct buffer_head * bh;
	int i;
	char c;

	if (!S_ISLNK(inode->i_mode)) {
		iput(inode);
		return -EINVAL;
	}
	if (buflen > 1023)
		buflen = 1023;
	bh = ext_bread(inode, 0, 0);
	iput(inode);
	if (!bh)
		return 0;
	i = 0;
	while (i<buflen && (c = bh->b_data[i])) {
		i++;
		put_fs_byte(c,buffer++);
	}
	brelse(bh);
	return i;
}
