/*
 *  linux/fs/open.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/vfs.h>
#include <linux/types.h>
#include <linux/utime.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/tty.h>
#include <asm/segment.h>

extern void fcntl_remove_locks(struct task_struct *, struct file *);

struct file_operations * chrdev_fops[MAX_CHRDEV] = {
	NULL,
};

struct file_operations * blkdev_fops[MAX_BLKDEV] = {
	NULL,
};

int sys_ustat(int dev, struct ustat * ubuf)
{
	return -ENOSYS;
}

int sys_statfs(const char * path, struct statfs * buf)
{
	struct inode * inode;
	int error;

	verify_area(buf, sizeof(struct statfs));
	error = namei(path,&inode);
	if (error)
		return error;
	if (!inode->i_sb->s_op->statfs) {
		iput(inode);
		return -ENOSYS;
	}
	inode->i_sb->s_op->statfs(inode->i_sb, buf);
	iput(inode);
	return 0;
}

int sys_fstatfs(unsigned int fd, struct statfs * buf)
{
	struct inode * inode;
	struct file * file;

	verify_area(buf, sizeof(struct statfs));
	if (fd >= NR_OPEN || !(file = current->filp[fd]))
		return -EBADF;
	if (!(inode = file->f_inode))
		return -ENOENT;
	if (!inode->i_sb->s_op->statfs)
		return -ENOSYS;
	inode->i_sb->s_op->statfs(inode->i_sb, buf);
	return 0;
}

int sys_truncate(const char * path, unsigned int length)
{
	struct inode * inode;
	int error;

	error = namei(path,&inode);
	if (error)
		return error;
	if (S_ISDIR(inode->i_mode) || !permission(inode,MAY_WRITE)) {
		iput(inode);
		return -EACCES;
	}
	if (IS_RDONLY(inode)) {
		iput(inode);
		return -EROFS;
	}
	inode->i_size = length;
	if (inode->i_op && inode->i_op->truncate)
		inode->i_op->truncate(inode);
	inode->i_atime = inode->i_mtime = CURRENT_TIME;
	inode->i_dirt = 1;
	iput(inode);
	return 0;
}

int sys_ftruncate(unsigned int fd, unsigned int length)
{
	struct inode * inode;
	struct file * file;

	if (fd >= NR_OPEN || !(file = current->filp[fd]))
		return -EBADF;
	if (!(inode = file->f_inode))
		return -ENOENT;
	if (S_ISDIR(inode->i_mode) || !(file->f_mode & 2))
		return -EACCES;
	inode->i_size = length;
	if (inode->i_op && inode->i_op->truncate)
		inode->i_op->truncate(inode);
	inode->i_atime = inode->i_mtime = CURRENT_TIME;
	inode->i_dirt = 1;
	return 0;
}

/* If times==NULL, set access and modification to current time,
 * must be owner or have write permission.
 * Else, update from *times, must be owner or super user.
 */
int sys_utime(char * filename, struct utimbuf * times)
{
	struct inode * inode;
	long actime,modtime;
	int error;

	error = namei(filename,&inode);
	if (error)
		return error;
	if (IS_RDONLY(inode)) {
		iput(inode);
		return -EROFS;
	}
	if (times) {
		if ((current->euid != inode->i_uid) && !suser()) {
			iput(inode);
			return -EPERM;
		}
		actime = get_fs_long((unsigned long *) &times->actime);
		modtime = get_fs_long((unsigned long *) &times->modtime);
	} else {
		if ((current->euid != inode->i_uid) &&
		    !permission(inode,MAY_WRITE)) {
			iput(inode);
			return -EACCES;
		}
		actime = modtime = CURRENT_TIME;
	}
	inode->i_atime = actime;
	inode->i_mtime = modtime;
	inode->i_dirt = 1;
	iput(inode);
	return 0;
}

/*
 * XXX should we use the real or effective uid?  BSD uses the real uid,
 * so as to make this call useful to setuid programs.
 */
int sys_access(const char * filename,int mode)
{
	struct inode * inode;
	int res, i_mode;

	mode &= 0007;
	res = namei(filename,&inode);
	if (res)
		return res;
	i_mode = res = inode->i_mode & 0777;
	iput(inode);
	if (current->uid == inode->i_uid)
		res >>= 6;
	else if (in_group_p(inode->i_gid))
		res >>= 3;
	if ((res & 0007 & mode) == mode)
		return 0;
	/*
	 * XXX we are doing this test last because we really should be
	 * swapping the effective with the real user id (temporarily),
	 * and then calling suser() routine.  If we do call the
	 * suser() routine, it needs to be called last. 
	 */
	if ((!current->uid) &&
	    (!(mode & 1) || (i_mode & 0111)))
		return 0;
	return -EACCES;
}

int sys_chdir(const char * filename)
{
	struct inode * inode;
	int error;

	error = namei(filename,&inode);
	if (error)
		return error;
	if (!S_ISDIR(inode->i_mode)) {
		iput(inode);
		return -ENOTDIR;
	}
	if (!permission(inode,MAY_EXEC)) {
		iput(inode);
		return -EACCES;
	}
	iput(current->pwd);
	current->pwd = inode;
	return (0);
}

int sys_chroot(const char * filename)
{
	struct inode * inode;
	int error;

	error = namei(filename,&inode);
	if (error)
		return error;
	if (!S_ISDIR(inode->i_mode)) {
		iput(inode);
		return -ENOTDIR;
	}
	if (!suser()) {
		iput(inode);
		return -EPERM;
	}
	iput(current->root);
	current->root = inode;
	return (0);
}

int sys_fchmod(unsigned int fd, mode_t mode)
{
	struct inode * inode;
	struct file * file;

	if (fd >= NR_OPEN || !(file = current->filp[fd]))
		return -EBADF;
	if (!(inode = file->f_inode))
		return -ENOENT;
	if ((current->euid != inode->i_uid) && !suser())
		return -EPERM;
	if (IS_RDONLY(inode))
		return -EROFS;
	inode->i_mode = (mode & 07777) | (inode->i_mode & ~07777);
	inode->i_dirt = 1;
	return 0;
}

int sys_chmod(const char * filename, mode_t mode)
{
	struct inode * inode;
	int error;

	error = namei(filename,&inode);
	if (error)
		return error;
	if ((current->euid != inode->i_uid) && !suser()) {
		iput(inode);
		return -EPERM;
	}
	if (IS_RDONLY(inode)) {
		iput(inode);
		return -EROFS;
	}
	inode->i_mode = (mode & 07777) | (inode->i_mode & ~07777);
	inode->i_dirt = 1;
	iput(inode);
	return 0;
}

int sys_fchown(unsigned int fd, uid_t user, gid_t group)
{
	struct inode * inode;
	struct file * file;

	if (fd >= NR_OPEN || !(file = current->filp[fd]))
		return -EBADF;
	if (!(inode = file->f_inode))
		return -ENOENT;
	if (IS_RDONLY(inode))
		return -EROFS;
	if ((current->euid == inode->i_uid && user == inode->i_uid &&
	     (in_group_p(group) || group == inode->i_gid)) ||
	    suser()) {
		inode->i_uid = user;
		inode->i_gid = group;
		inode->i_dirt=1;
		return 0;
	}
	return -EPERM;
}

int sys_chown(const char * filename, uid_t user, gid_t group)
{
	struct inode * inode;
	int error;

	error = lnamei(filename,&inode);
	if (error)
		return error;
	if (IS_RDONLY(inode)) {
		iput(inode);
		return -EROFS;
	}
	if ((current->euid == inode->i_uid && user == inode->i_uid &&
	     (in_group_p(group) || group == inode->i_gid)) ||
	    suser()) {
		inode->i_uid = user;
		inode->i_gid = group;
		inode->i_dirt=1;
		iput(inode);
		return 0;
	}
	iput(inode);
	return -EPERM;
}

/*
 * Note that while the flag value (low two bits) for sys_open means:
 *	00 - read-only
 *	01 - write-only
 *	10 - read-write
 *	11 - special
 * it is changed into
 *	00 - no permissions needed
 *	01 - read-permission
 *	10 - write-permission
 *	11 - read-write
 * for the internal routines (ie open_namei()/follow_link() etc). 00 is
 * used by symlinks.
 */
int sys_open(const char * filename,int flag,int mode)
{
	struct inode * inode;
	struct file * f;
	int i,fd;

	for(fd=0 ; fd<NR_OPEN ; fd++)
		if (!current->filp[fd])
			break;
	if (fd>=NR_OPEN)
		return -EMFILE;
	current->close_on_exec &= ~(1<<fd);
	f = get_empty_filp();
	if (!f)
		return -ENFILE;
	current->filp[fd] = f;
	f->f_flags = flag;
	if (f->f_mode = (flag+1) & O_ACCMODE)
		flag++;
	if (flag & (O_TRUNC | O_CREAT))
		flag |= 2;
	i = open_namei(filename,flag,mode,&inode,NULL);
	if (i) {
		current->filp[fd]=NULL;
		f->f_count--;
		return i;
	}
	if (flag & O_TRUNC)
		if (inode->i_op && inode->i_op->truncate) {
			inode->i_size = 0;
			inode->i_op->truncate(inode);
		}
	if (!IS_RDONLY(inode)) {
		inode->i_atime = CURRENT_TIME;
		inode->i_dirt = 1;
	}
	f->f_inode = inode;
	f->f_pos = 0;
	f->f_reada = 0;
	f->f_op = NULL;
	if (inode->i_op)
		f->f_op = inode->i_op->default_file_ops;
	if (f->f_op && f->f_op->open)
		if (i = f->f_op->open(inode,f)) {
			iput(inode);
			f->f_count--;
			current->filp[fd]=NULL;
			return i;
		}
	return (fd);
}

int sys_creat(const char * pathname, int mode)
{
	return sys_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

static int close_fp(struct file *filp)
{
	struct inode *inode;

	if (filp->f_count == 0) {
		printk("Close: file count is 0\n");
		return 0;
	}
	inode = filp->f_inode;
	if (S_ISREG(inode->i_mode))
		fcntl_remove_locks(current, filp);
	if (filp->f_count > 1) {
		filp->f_count--;
		return 0;
	}
	if (filp->f_op && filp->f_op->release)
		filp->f_op->release(inode,filp);
	filp->f_count--;
	filp->f_inode = NULL;
	iput(inode);
	return 0;
}

int sys_close(unsigned int fd)
{	
	struct file * filp;

	if (fd >= NR_OPEN)
		return -EINVAL;
	current->close_on_exec &= ~(1<<fd);
	if (!(filp = current->filp[fd]))
		return -EINVAL;
	current->filp[fd] = NULL;
	return (close_fp (filp));
}

/*
 * This routine is used by vhangup.  It send's sigkill to everything
 * waiting on a particular wait_queue.  It assumes root privledges.
 * We don't want to destroy the wait queue here, because the caller
 * should call wake_up immediately after calling kill_wait.
 */
static void kill_wait(struct wait_queue **q, int sig)
{
	struct wait_queue *next;
	struct wait_queue *tmp;
	struct task_struct *p;

	if (!q || !(next = *q))
		return;
	do { 
		tmp = next;
		next = tmp->next;
		if (p = tmp->task)
			send_sig (sig, p , 1);
	} while (next && next != *q);
}

/*
 * This routine looks through all the process's and closes any
 * references to the current processes tty.  To avoid problems with
 * process sleeping on an inode which has already been iput, anyprocess
 * which is sleeping on the tty is sent a sigkill (It's probably a rogue
 * process.)  Also no process should ever have /dev/console as it's
 * controlling tty, or have it open for reading.  So we don't have to
 * worry about messing with all the daemons abilities to write messages
 * to the console.  (Besides they should be using syslog.)
 */
int sys_vhangup(void)
{
	int j;
	struct task_struct ** process;
	struct file *filep;
	struct inode *inode;
	struct tty_struct *tty;
	extern int kill_pg (int pgrp, int sig, int priv);

	if (!suser())
		return -EPERM;
	/* send the SIGHUP signal. */
	kill_pg(current->pgrp, SIGHUP, 0);
	/* See if there is a controlling tty. */
	if (current->tty < 0)
		return 0;

	for (process = task + 0; process < task + NR_TASKS; process++) {
		for (j = 0; j < NR_OPEN; j++) {
			if (!*process)
				break;
			if (!(filep = (*process)->filp[j]))
				continue;
			if (!(inode = filep->f_inode))
				continue;
	 		if (!S_ISCHR(inode->i_mode))
				continue;
			if ((MAJOR(inode->i_rdev) == 5 ||
			     MAJOR(inode->i_rdev) == 4 ) &&
			    (MAJOR(filep->f_rdev) == 4 &&
			     MINOR(filep->f_rdev) == MINOR (current->tty))) {
		  /* so now we have found something to close.  We
		     need to kill every process waiting on the
		     inode. */
				(*process)->filp[j] = NULL;
				kill_wait (&inode->i_wait, SIGKILL);

		  /* now make sure they are awake before we close the
		     file. */

				wake_up (&inode->i_wait);

		  /* finally close the file. */

				(*process)->close_on_exec &= ~(1<<j);
				close_fp (filep);
			}
		}
	/* can't let them keep a reference to it around.
	   But we can't touch current->tty until after the
	   loop is complete. */

		if (*process && (*process)->tty == current->tty && *process != current) {
			(*process)->tty = -1;
		}
	}
   /* need to do tty->session = 0 */
	tty = TTY_TABLE(MINOR(current->tty));
	if (tty) {
		tty->session = 0;
		tty->pgrp = -1;
		current->tty = -1;
	}
	return 0;
}

