/*
 *  linux/fs/proc/root.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  proc root directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>

static int proc_readroot(struct inode *, struct file *, struct dirent *, int);
static int proc_lookuproot(struct inode *,const char *,int,struct inode **);

static struct file_operations proc_root_operations = {
	NULL,			/* lseek - default */
	NULL,			/* read - bad */
	NULL,			/* write - bad */
	proc_readroot,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

/*
 * proc directories can do almost nothing..
 */
struct inode_operations proc_root_inode_operations = {
	&proc_root_operations,	/* default base directory file-ops */
	NULL,			/* create */
	proc_lookuproot,	/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};

static int proc_lookuproot(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	unsigned int pid, c;
	int i, ino;

	*result = NULL;
	if (!dir)
		return -ENOENT;
	if (!S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}
	pid = 0;
	if (!len || (get_fs_byte(name) == '.' && (len == 1 ||
	    (get_fs_byte(name+1) == '.' && len == 2)))) {
		*result = dir;
		return 0;
	}
	while (len-- > 0) {
		c = get_fs_byte(name) - '0';
		name++;
		if (c > 9) {
			pid = 0;
			break;
		}
		pid *= 10;
		pid += c;
		if (pid & 0xffff0000) {
			pid = 0;
			break;
		}
	}
	for (i = 0 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid)
			break;
	if (!pid || i >= NR_TASKS) {
		iput(dir);
		return -ENOENT;
	}
	ino = (pid << 16) + 2;
	if (!(*result = iget(dir->i_sb,ino))) {
		iput(dir);
		return -ENOENT;
	}
	iput(dir);
	return 0;
}

static int proc_readroot(struct inode * inode, struct file * filp,
	struct dirent * dirent, int count)
{
	struct task_struct * p;
	unsigned int pid;
	int i,j;

	if (!inode || !S_ISDIR(inode->i_mode))
		return -EBADF;
	while ((pid = filp->f_pos) < NR_TASKS+2) {
		filp->f_pos++;
		if (pid < 2) {
			i = j = pid+1;
			put_fs_long(1, &dirent->d_ino);
			put_fs_word(i, &dirent->d_reclen);
			put_fs_byte(0, i+dirent->d_name);
			while (i--)
				put_fs_byte('.', i+dirent->d_name);
			return j;
		}			
		p = task[pid-2];
		if (!p || !(pid = p->pid))
			continue;
		if (pid & 0xffff0000)
			continue;
		j = 10;
		i = 1;
		while (pid >= j) {
			j *= 10;
			i++;
		}
		j = i;
		put_fs_long((pid << 16)+2, &dirent->d_ino);
		put_fs_word(i, &dirent->d_reclen);
		put_fs_byte(0, i+dirent->d_name);
		while (i--) {
			put_fs_byte('0'+(pid % 10), i+dirent->d_name);
			pid /= 10;
		}
		return j;
	}
	return 0;
}
