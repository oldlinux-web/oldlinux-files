/*
 *  linux/fs/isofs/symlink.c
 *
 *  (C) 1992  Eric Youngdale Modified for ISO9660 filesystem.
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  isofs symlink handling code.  This is only used with the Rock Ridge
 *  extensions to iso9660
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/iso_fs.h>
#include <linux/stat.h>

static int isofs_readlink(struct inode *, char *, int);
static int isofs_follow_link(struct inode *, struct inode *, int, int, struct inode **);

/*
 * symlinks can't do much...
 */
struct inode_operations isofs_symlink_inode_operations = {
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
	isofs_readlink,		/* readlink */
	isofs_follow_link,	/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};

static int isofs_follow_link(struct inode * dir, struct inode * inode,
	int flag, int mode, struct inode ** res_inode)
{
	int error;
	unsigned short fs;
	char * pnt;

	if (!dir) {
		dir = current->root;
		dir->i_count++;
	}
	if (!inode) {
		iput(dir);
		*res_inode = NULL;
		return -ENOENT;
	}
	if (!S_ISLNK(inode->i_mode)) {
		iput(dir);
		*res_inode = inode;
		return 0;
	}
	__asm__("mov %%fs,%0":"=r" (fs));
	if ((current->link_count > 5) ||
	   !(pnt = get_rock_ridge_symlink(inode))) {
		iput(dir);
		iput(inode);
		*res_inode = NULL;
		return -ELOOP;
	}
	iput(inode);
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	current->link_count++;
	error = open_namei(pnt,flag,mode,res_inode,dir);
	current->link_count--;
	__asm__("mov %0,%%fs"::"r" (fs));
	free(pnt);
	return error;
}

static int isofs_readlink(struct inode * inode, char * buffer, int buflen)
{
        char * pnt;
	int i;
	char c;

	if (!S_ISLNK(inode->i_mode)) {
		iput(inode);
		return -EINVAL;
	}

	if (buflen > 1023)
		buflen = 1023;
	pnt = get_rock_ridge_symlink(inode);

	iput(inode);
	if (!pnt)
		return 0;
	i = 0;

	while (i<buflen && (c = pnt[i])) {
		i++;
		put_fs_byte(c,buffer++);
	}
	free(pnt);
	return i;
}
