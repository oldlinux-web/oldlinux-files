/*
 *  linux/fs/proc/base.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  proc base directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>

static int proc_readbase(struct inode *, struct file *, struct dirent *, int);
static int proc_lookupbase(struct inode *,const char *,int,struct inode **);

static struct file_operations proc_base_operations = {
	NULL,			/* lseek - default */
	NULL,			/* read - bad */
	NULL,			/* write - bad */
	proc_readbase,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

/*
 * proc directories can do almost nothing..
 */
struct inode_operations proc_base_inode_operations = {
	&proc_base_operations,	/* default base directory file-ops */
	NULL,			/* create */
	proc_lookupbase,	/* lookup */
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

struct proc_dir_entry {
	unsigned short low_ino;
	unsigned short namelen;
	char * name;
};

static struct proc_dir_entry base_dir[] = {
	{ 1,2,".." },
	{ 2,1,"." },
	{ 3,3,"mem" },
	{ 4,3,"cwd" },
	{ 5,4,"root" },
	{ 6,3,"exe" },
	{ 7,2,"fd" },
	{ 8,3,"lib" }
};

#define NR_BASE_DIRENTRY ((sizeof (base_dir))/(sizeof (base_dir[0])))

static int proc_match(int len,const char * name,struct proc_dir_entry * de)
{
	register int same __asm__("ax");

	if (!de || !de->low_ino)
		return 0;
	/* "" means "." ---> so paths like "/usr/lib//libc.a" work */
	if (!len && (de->name[0]=='.') && (de->name[1]=='\0'))
		return 1;
	if (de->namelen != len)
		return 0;
	__asm__("cld\n\t"
		"fs ; repe ; cmpsb\n\t"
		"setz %%al"
		:"=a" (same)
		:"0" (0),"S" ((long) name),"D" ((long) de->name),"c" (len)
		:"cx","di","si");
	return same;
}

static int proc_lookupbase(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	unsigned int pid;
	int i, ino;

	*result = NULL;
	if (!dir)
		return -ENOENT;
	if (!S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}
	ino = dir->i_ino;
	pid = ino >> 16;
	i = NR_BASE_DIRENTRY;
	while (i-- > 0 && !proc_match(len,name,base_dir+i))
		/* nothing */;
	if (i < 0) {
		iput(dir);
		return -ENOENT;
	}
	if (base_dir[i].low_ino == 1)
		ino = 1;
	else
		ino = (pid << 16) + base_dir[i].low_ino;
	for (i = 0 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid)
			break;
	if (!pid || i >= NR_TASKS) {
		iput(dir);
		return -ENOENT;
	}
	if (!(*result = iget(dir->i_sb,ino))) {
		iput(dir);
		return -ENOENT;
	}
	iput(dir);
	return 0;
}

static int proc_readbase(struct inode * inode, struct file * filp,
	struct dirent * dirent, int count)
{
	struct proc_dir_entry * de;
	unsigned int pid, ino;
	int i,j;

	if (!inode || !S_ISDIR(inode->i_mode))
		return -EBADF;
	ino = inode->i_ino;
	pid = ino >> 16;
	for (i = 0 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid)
			break;
	if (!pid || i >= NR_TASKS)
		return 0;
	if (((unsigned) filp->f_pos) < NR_BASE_DIRENTRY) {
		de = base_dir + filp->f_pos;
		filp->f_pos++;
		i = de->namelen;
		ino = de->low_ino;
		if (ino != 1)
			ino |= (pid << 16);
		put_fs_long(ino, &dirent->d_ino);
		put_fs_word(i,&dirent->d_reclen);
		put_fs_byte(0,i+dirent->d_name);
		j = i;
		while (i--)
			put_fs_byte(de->name[i], i+dirent->d_name);
		return j;
	}
	return 0;
}
