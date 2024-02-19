/*
 *  linux/fs/proc/inode.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/system.h>
#include <asm/segment.h>

void proc_put_inode(struct inode *inode)
{
	if (inode->i_nlink)
		return;
	inode->i_size = 0;
}

void proc_put_super(struct super_block *sb)
{
	lock_super(sb);
	sb->s_dev = 0;
	unlock_super(sb);
}

static struct super_operations proc_sops = { 
	proc_read_inode,
	proc_write_inode,
	proc_put_inode,
	proc_put_super,
	NULL,
	proc_statfs
};

struct super_block *proc_read_super(struct super_block *s,void *data)
{
	lock_super(s);
	s->s_blocksize = 1024;
	s->s_magic = PROC_SUPER_MAGIC;
	s->s_op = &proc_sops;
	unlock_super(s);
	if (!(s->s_mounted = iget(s,PROC_ROOT_INO))) {
		s->s_dev = 0;
		printk("get root inode failed\n");
		return NULL;
	}
	return s;
}

void proc_statfs(struct super_block *sb, struct statfs *buf)
{
	put_fs_long(PROC_SUPER_MAGIC, &buf->f_type);
	put_fs_long(1024, &buf->f_bsize);
	put_fs_long(0, &buf->f_blocks);
	put_fs_long(0, &buf->f_bfree);
	put_fs_long(0, &buf->f_bavail);
	put_fs_long(0, &buf->f_files);
	put_fs_long(0, &buf->f_ffree);
	/* Don't know what value to put in buf->f_fsid */
}

int proc_bmap(struct inode * inode,int block)
{
	return 0;
}

int proc_create_block(struct inode * inode, int block)
{
	return 0;
}

void proc_read_inode(struct inode * inode)
{
	unsigned long ino, pid;
	struct task_struct * p;
	int i;
	
	inode->i_op = NULL;
	inode->i_mode = 0;
	inode->i_uid = 0;
	inode->i_gid = 0;
	inode->i_nlink = 1;
	inode->i_size = 0;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_blocks = inode->i_blksize = 0;
	ino = inode->i_ino;
	pid = ino >> 16;
	p = task[0];
	for (i = 0; i < NR_TASKS ; i++)
		if ((p = task[i]) && (p->pid == pid))
			break;
	if (!p || i >= NR_TASKS)
		return;
	if (ino == PROC_ROOT_INO) {
		inode->i_mode = S_IFDIR | 0555;
		inode->i_op = &proc_root_inode_operations;
		return;
	}
	if (!pid)
		return;
	ino &= 0x0000ffff;
	inode->i_uid = p->euid;
	inode->i_gid = p->egid;
	switch (ino) {
		case 2:
			inode->i_nlink = 2;
			for (i = 1 ; i < NR_TASKS ; i++)
				if (task[i])
					inode->i_nlink++;
			inode->i_mode = S_IFDIR | 0500;
			inode->i_op = &proc_base_inode_operations;
			return;
		case 3:
			inode->i_op = &proc_mem_inode_operations;
			inode->i_mode = S_IFCHR | 0600;
			inode->i_rdev = 0x0101;
			return;
		case 4:
		case 5:
		case 6:
			inode->i_op = &proc_link_inode_operations;
			inode->i_size = 3;
			inode->i_mode = S_IFLNK | 0700;
			return;
		case 7:
		case 8:
			inode->i_mode = S_IFDIR | 0500;
			inode->i_op = &proc_fd_inode_operations;
			inode->i_nlink = 2;
			return;
	}
	switch (ino >> 8) {
		case 1:
			ino &= 0xff;
			if (ino >= NR_OPEN || !p->filp[ino])
				return;
			inode->i_op = &proc_link_inode_operations;
			inode->i_size = 3;
			inode->i_mode = S_IFLNK | 0700;
			return;
		case 2:
			ino &= 0xff;
			if (ino >= p->numlibraries)
				return;
			inode->i_op = &proc_link_inode_operations;
			inode->i_size = 3;
			inode->i_mode = S_IFLNK | 0700;
			return;
	}
	return;
}

void proc_write_inode(struct inode * inode)
{
	inode->i_dirt=0;
}
