/*
 *  linux/fs/namei.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * Some corrections by tytso.
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/stat.h>

#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])

/*
 * comment out this line if you want names > MINIX_NAME_LEN chars to be
 * truncated. Else they will be disallowed.
 */
/* #define NO_TRUNCATE */

/*
 *	permission()
 *
 * is used to check for read/write/execute permissions on a file.
 * I don't know if we should look at just the euid or both euid and
 * uid, but that should be easily changed.
 */
int permission(struct inode * inode,int mask)
{
	int mode = inode->i_mode;

/* special case: not even root can read/write a deleted file */
	if (inode->i_dev && !inode->i_nlink)
		return 0;
	else if (current->euid == inode->i_uid)
		mode >>= 6;
	else if (in_group_p(inode->i_gid))
		mode >>= 3;
	if (((mode & mask & 0007) == mask) || suser())
		return 1;
	return 0;
}

/*
 * lookup() looks up one part of a pathname, using the fs-dependent
 * routines (currently minix_lookup) for it. It also checks for
 * fathers (pseudo-roots, mount-points)
 */
int lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	struct super_block * sb;

	*result = NULL;
	if (len==2 && get_fs_byte(name)=='.' && get_fs_byte(name+1)=='.') {
		if (dir == current->root)
			len = 1;
		else if ((sb = dir->i_sb) && (dir == sb->s_mounted)) {
			sb = dir->i_sb;
			iput(dir);
			if (dir = sb->s_covered)
				dir->i_count++;
		}
	}
	if (!dir)
		return -ENOENT;
	if (!dir->i_op || !dir->i_op->lookup) {
		iput(dir);
		return -ENOTDIR;
	}
 	if (!permission(dir,MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!len) {
		*result = dir;
		return 0;
	}
	return dir->i_op->lookup(dir,name,len,result);
}

int follow_link(struct inode * dir, struct inode * inode,
	int flag, int mode, struct inode ** res_inode)
{
	if (!dir || !inode) {
		iput(dir);
		iput(inode);
		*res_inode = NULL;
		return -ENOENT;
	}
	if (!inode->i_op || !inode->i_op->follow_link) {
		iput(dir);
		*res_inode = inode;
		return 0;
	}
	return inode->i_op->follow_link(dir,inode,flag,mode,res_inode);
}

/*
 *	dir_namei()
 *
 * dir_namei() returns the inode of the directory of the
 * specified name, and the name within that directory.
 */
static int dir_namei(const char * pathname, int * namelen, const char ** name,
	struct inode * base, struct inode ** res_inode)
{
	char c;
	const char * thisname;
	int len,error;
	struct inode * inode;

	*res_inode = NULL;
	if (!base) {
		base = current->pwd;
		base->i_count++;
	}
	if ((c=get_fs_byte(pathname))=='/') {
		iput(base);
		base = current->root;
		pathname++;
		base->i_count++;
	}
	while (1) {
		thisname = pathname;
		for(len=0;(c=get_fs_byte(pathname++))&&(c!='/');len++)
			/* nothing */ ;
		if (!c)
			break;
		base->i_count++;
		error = lookup(base,thisname,len,&inode);
		if (error) {
			iput(base);
			return error;
		}
		error = follow_link(base,inode,0,0,&base);
		if (error)
			return error;
	}
	*name = thisname;
	*namelen = len;
	*res_inode = base;
	return 0;
}

static int _namei(const char * pathname, struct inode * base,
	int follow_links, struct inode ** res_inode)
{
	const char * basename;
	int namelen,error;
	struct inode * inode;

	*res_inode = NULL;
	error = dir_namei(pathname,&namelen,&basename,base,&base);
	if (error)
		return error;
	base->i_count++;	/* lookup uses up base */
	error = lookup(base,basename,namelen,&inode);
	if (error) {
		iput(base);
		return error;
	}
	if (follow_links) {
		error = follow_link(base,inode,0,0,&inode);
		if (error)
			return error;
	} else
		iput(base);
	*res_inode = inode;
	return 0;
}

int lnamei(const char * pathname, struct inode ** res_inode)
{
	return _namei(pathname,NULL,0,res_inode);
}

/*
 *	namei()
 *
 * is used by most simple commands to get the inode of a specified name.
 * Open, link etc use their own routines, but this is enough for things
 * like 'chmod' etc.
 */
int namei(const char * pathname, struct inode ** res_inode)
{
	return _namei(pathname,NULL,1,res_inode);
}

/*
 *	open_namei()
 *
 * namei for open - this is in fact almost the whole open-routine.
 *
 * Note that the low bits of "flag" aren't the same asin the open
 * system call - they are 00 - no permissions needed
 *			  01 - read permission needed
 *			  10 - write permission needed
 *			  11 - read/write permissions needed
 * which is a lot more logical, and also allows the "no perm" needed
 * for symlinks (where the permissions are checked later).
 */
int open_namei(const char * pathname, int flag, int mode,
	struct inode ** res_inode, struct inode * base)
{
	const char * basename;
	int namelen,error,i;
	struct inode * dir, *inode;
	struct task_struct ** p;

	mode &= 07777 & ~current->umask;
	mode |= S_IFREG;
	error = dir_namei(pathname,&namelen,&basename,base,&dir);
	if (error)
		return error;
	if (!namelen) {			/* special case: '/usr/' etc */
		if (!(flag & 2)) {
			*res_inode=dir;
			return 0;
		}
		iput(dir);
		return -EISDIR;
	}
	dir->i_count++;		/* lookup eats the dir */
	error = lookup(dir,basename,namelen,&inode);
	if (error) {
		if (!(flag & O_CREAT)) {
			iput(dir);
			return error;
		}
		if (!permission(dir,MAY_WRITE)) {
			iput(dir);
			return -EACCES;
		}
		if (!dir->i_op || !dir->i_op->create) {
			iput(dir);
			return -EACCES;
		}
		if (IS_RDONLY(dir)) {
			iput(dir);
			return -EROFS;
		}
		return dir->i_op->create(dir,basename,namelen,mode,res_inode);
	}
	if (flag & O_EXCL) {
		iput(dir);
		iput(inode);
		return -EEXIST;
	}
	if (error = follow_link(dir,inode,flag,mode,&inode))
		return error;
	if (S_ISDIR(inode->i_mode) && (flag & 2)) {
		iput(inode);
		return -EPERM;
	}
	if (!permission(inode,ACC_MODE(flag))) {
		iput(inode);
		return -EACCES;
	}
	if (S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode)) {
		if (IS_NODEV(inode)) {
			iput(inode);
			return -EACCES;
		}
	} else {
		if (IS_RDONLY(inode) && (flag & 2)) {
			iput(inode);
			return -EROFS;
		}
	}
 	if ((inode->i_count > 1) && (flag & 2))
 		for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
 			if (!*p)
 				continue;
 			if (inode == (*p)->executable) {
 				iput(inode);
 				return -ETXTBSY;
 			}
 			for (i=0; i < (*p)->numlibraries; i++)
 				if (inode == (*p)->libraries[i].library) {
 					iput(inode);
 					return -ETXTBSY;
 				}
 		}
	*res_inode = inode;
	return 0;
}

int do_mknod(const char * filename, int mode, dev_t dev)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(filename,&namelen,&basename, NULL, &dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->mknod) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->mknod(dir,basename,namelen,mode,dev);
}

int sys_mknod(const char * filename, int mode, dev_t dev)
{
	if (S_ISFIFO(mode) || suser())
		return do_mknod(filename,mode,dev);
	return -EPERM;
}

int sys_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(pathname,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->mkdir) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->mkdir(dir,basename,namelen,mode);
}

int sys_rmdir(const char * name)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(name,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->rmdir) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->rmdir(dir,basename,namelen);
}

int sys_unlink(const char * name)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(name,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -EPERM;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->unlink) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->unlink(dir,basename,namelen);
}

int sys_symlink(const char * oldname, const char * newname)
{
	struct inode * dir;
	const char * basename;
	int namelen, error;

	error = dir_namei(newname,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->symlink) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->symlink(dir,basename,namelen,oldname);
}

int sys_link(const char * oldname, const char * newname)
{
	struct inode * oldinode, * dir;
	const char * basename;
	int namelen, error;

	error = namei(oldname, &oldinode);
	if (error)
		return error;
	error = dir_namei(newname,&namelen,&basename,NULL,&dir);
	if (error) {
		iput(oldinode);
		return error;
	}
	if (!namelen) {
		iput(oldinode);
		iput(dir);
		return -EPERM;
	}
	if (IS_RDONLY(dir)) {
		iput(oldinode);
		iput(dir);
		return -EROFS;
	}
	if (dir->i_dev != oldinode->i_dev) {
		iput(dir);
		iput(oldinode);
		return -EXDEV;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		iput(oldinode);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->link) {
		iput(dir);
		iput(oldinode);
		return -EPERM;
	}
	return dir->i_op->link(oldinode, dir, basename, namelen);
}

int sys_rename(const char * oldname, const char * newname)
{
	struct inode * old_dir, * new_dir;
	const char * old_base, * new_base;
	int old_len, new_len, error;

	error = dir_namei(oldname,&old_len,&old_base,NULL,&old_dir);
	if (error)
		return error;
	if (!permission(old_dir,MAY_WRITE)) {
		iput(old_dir);
		return -EACCES;
	}
	if (!old_len || (get_fs_byte(old_base) == '.' &&
	    (old_len == 1 || (get_fs_byte(old_base+1) == '.' &&
	     old_len == 2)))) {
		iput(old_dir);
		return -EPERM;
	}
	error = dir_namei(newname,&new_len,&new_base,NULL,&new_dir);
	if (error) {
		iput(old_dir);
		return error;
	}
	if (!permission(new_dir,MAY_WRITE)) {
		iput(old_dir);
		iput(new_dir);
		return -EACCES;
	}
	if (!new_len || (get_fs_byte(new_base) == '.' &&
	    (new_len == 1 || (get_fs_byte(new_base+1) == '.' &&
	     new_len == 2)))) {
		iput(old_dir);
		iput(new_dir);
		return -EPERM;
	}
	if (new_dir->i_dev != old_dir->i_dev) {
		iput(old_dir);
		iput(new_dir);
		return -EXDEV;
	}
	if (IS_RDONLY(new_dir) || IS_RDONLY(old_dir)) {
		iput(old_dir);
		iput(new_dir);
		return -EROFS;
	}
	if (!old_dir->i_op || !old_dir->i_op->rename) {
		iput(old_dir);
		iput(new_dir);
		return -EPERM;
	}
	return old_dir->i_op->rename(old_dir, old_base, old_len, 
		new_dir, new_base, new_len);
}
