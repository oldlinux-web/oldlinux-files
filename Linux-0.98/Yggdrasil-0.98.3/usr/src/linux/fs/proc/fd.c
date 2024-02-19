/*
 *  linux/fs/proc/fd.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  proc fd directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>

static int proc_readfd(struct inode *, struct file *, struct dirent *, int);
static int proc_lookupfd(struct inode *,const char *,int,struct inode **);

static struct file_operations proc_fd_operations = {
	NULL,			/* lseek - default */
	NULL,			/* read - bad */
	NULL,			/* write - bad */
	proc_readfd,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

/*
 * proc directories can do almost nothing..
 */
struct inode_operations proc_fd_inode_operations = {
	&proc_fd_operations,	/* default base directory file-ops */
	NULL,			/* create */
	proc_lookupfd,		/* lookup */
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

static int proc_lookupfd(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	unsigned int ino, pid, fd, c;
	struct task_struct * p;
	struct super_block * sb;
	int i;

	*result = NULL;
	ino = dir->i_ino;
	pid = ino >> 16;
	ino &= 0x0000ffff;
	ino -= 7;
	if (!dir)
		return -ENOENT;
	sb = dir->i_sb;
	if (!pid || ino > 1 || !S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}
	if (!len || (get_fs_byte(name) == '.' && (len == 1 ||
	    (get_fs_byte(name+1) == '.' && len == 2)))) {
		if (len < 2) {
			*result = dir;
			return 0;
		}
		if (!(*result = iget(sb,(pid << 16)+2))) {
			iput(dir);
			return -ENOENT;
		}
		iput(dir);
		return 0;
	}
	iput(dir);
	fd = 0;
	while (len-- > 0) {
		c = get_fs_byte(name) - '0';
		name++;
		if (c > 9) {
			fd = 0xfffff;
			break;
		}
		fd *= 10;
		fd += c;
		if (fd & 0xffff0000) {
			fd = 0xfffff;
			break;
		}
	}
	for (i = 0 ; i < NR_TASKS ; i++)
		if ((p = task[i]) && p->pid == pid)
			break;
	if (!pid || i >= NR_TASKS)
		return -ENOENT;
	if (!ino) {
		if (fd >= NR_OPEN || !p->filp[fd] || !p->filp[fd]->f_inode)
			return -ENOENT;
		ino = (pid << 16) + 0x100 + fd;
	} else {
		if (fd >= p->numlibraries)
			return -ENOENT;
		ino = (pid << 16) + 0x200 + fd;
	}
	if (!(*result = iget(sb,ino)))
		return -ENOENT;
	return 0;
}

static int proc_readfd(struct inode * inode, struct file * filp,
	struct dirent * dirent, int count)
{
	struct task_struct * p;
	unsigned int fd, pid, ino;
	int i,j;

	if (!inode || !S_ISDIR(inode->i_mode))
		return -EBADF;
	ino = inode->i_ino;
	pid = ino >> 16;
	ino &= 0x0000ffff;
	ino -= 7;
	if (ino > 1)
		return 0;
	while (1) {
		fd = filp->f_pos;
		filp->f_pos++;
		if (fd < 2) {
			i = j = fd+1;
			if (!fd)
				fd = inode->i_ino;
			else
				fd = (inode->i_ino & 0xffff0000) | 2;
			put_fs_long(fd, &dirent->d_ino);
			put_fs_word(i, &dirent->d_reclen);
			put_fs_byte(0, i+dirent->d_name);
			while (i--)
				put_fs_byte('.', i+dirent->d_name);
			return j;
		}
		fd -= 2;
		for (i = 1 ; i < NR_TASKS ; i++)
			if ((p = task[i]) && p->pid == pid)
				break;
		if (i >= NR_TASKS)
			return 0;
		if (!ino) {
			if (fd >= NR_OPEN)
				break;
			if (!p->filp[fd] || !p->filp[fd]->f_inode)
				continue;
		} else
			if (fd >= p->numlibraries)
				break;
		j = 10;
		i = 1;
		while (fd >= j) {
			j *= 10;
			i++;
		}
		j = i;
		if (!ino)
			ino = (pid << 16) + 0x100 + fd;
		else
			ino = (pid << 16) + 0x200 + fd;
		put_fs_long(ino, &dirent->d_ino);
		put_fs_word(i, &dirent->d_reclen);
		put_fs_byte(0, i+dirent->d_name);
		while (i--) {
			put_fs_byte('0'+(fd % 10), i+dirent->d_name);
			fd /= 10;
		}
		return j;
	}
	return 0;
}
