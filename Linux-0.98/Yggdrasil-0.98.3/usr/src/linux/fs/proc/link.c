/*
 *  linux/fs/proc/link.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  /proc link-file handling code
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/minix_fs.h>
#include <linux/stat.h>

static int proc_readlink(struct inode *, char *, int);
static int proc_follow_link(struct inode *, struct inode *, int, int, struct inode **);

/*
 * links can't do much...
 */
struct inode_operations proc_link_inode_operations = {
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
	proc_readlink,		/* readlink */
	proc_follow_link,	/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};

static int proc_follow_link(struct inode * dir, struct inode * inode,
	int flag, int mode, struct inode ** res_inode)
{
	unsigned int pid, ino;
	struct task_struct * p;
	int i;

	*res_inode = NULL;
	if (dir)
		iput(dir);
	if (!inode)
		return -ENOENT;
	ino = inode->i_ino;
	pid = ino >> 16;
	ino &= 0x0000ffff;
	iput(inode);
	for (i = 0 ; i < NR_TASKS ; i++)
		if ((p = task[i]) && p->pid == pid)
			break;
	if (i >= NR_TASKS)
		return -ENOENT;
	inode = NULL;
	switch (ino) {
		case 4:
			inode = p->pwd;
			break;
		case 5:
			inode = p->root;
			break;
		case 6:
			inode = p->executable;
			break;
		default:
			switch (ino >> 8) {
				case 1:
					ino &= 0xff;
					if (ino < NR_OPEN && p->filp[ino])
						inode = p->filp[ino]->f_inode;
					break;
				case 2:
					ino &= 0xff;
					if (ino < p->numlibraries)
						inode = p->libraries[ino].library;
			}
	}
	if (!inode)
		return -ENOENT;
	*res_inode = inode;
	inode->i_count++;
	return 0;
}

static int proc_readlink(struct inode * inode, char * buffer, int buflen)
{
	int i;

	iput(inode);
	if (buflen > 3)
		buflen = 3;
	i = 0;
	while (i++ < buflen)
		put_fs_byte('-',buffer++);
	return i;
}
