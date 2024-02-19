/*
 *  linux/fs/namei.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * Some corrections by tytso.
 */

#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <const.h>
#include <sys/stat.h>

struct inode * _namei(const char * filename, struct inode * base,
	int follow_links);

#define ACC_MODE(x) ("\004\002\006\377"[(x)&O_ACCMODE])

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
 	if (!permission(dir,MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!len) {
		*result = dir;
		return 0;
	}
	if (!dir->i_op || !dir->i_op->lookup) {
		iput(dir);
		return -ENOENT;
	}
	return dir->i_op->lookup(dir,name,len,result);
}

struct inode * follow_link(struct inode * dir, struct inode * inode)
{
	if (!dir || !inode) {
		iput(dir);
		iput(inode);
		return NULL;
	}
	if (!inode->i_op || !inode->i_op->follow_link) {
		iput(dir);
		return inode;
	}
	return inode->i_op->follow_link(dir,inode);
}

/*
 *	dir_namei()
 *
 * dir_namei() returns the inode of the directory of the
 * specified name, and the name within that directory.
 */
static struct inode * dir_namei(const char * pathname,
	int * namelen, const char ** name, struct inode * base)
{
	char c;
	const char * thisname;
	int len,error;
	struct inode * inode;

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
			return NULL;
		}
		if (!(base = follow_link(base,inode)))
			return NULL;
	}
	*name = thisname;
	*namelen = len;
	return base;
}

struct inode * _namei(const char * pathname, struct inode * base,
	int follow_links)
{
	const char * basename;
	int namelen,error;
	struct inode * inode;

	if (!(base = dir_namei(pathname,&namelen,&basename,base)))
		return NULL;
	base->i_count++;	/* lookup uses up base */
	error = lookup(base,basename,namelen,&inode);
	if (error) {
		iput(base);
		return NULL;
	}
	if (follow_links)
		inode = follow_link(base,inode);
	else
		iput(base);
	if (inode) {
		inode->i_atime=CURRENT_TIME;
		inode->i_dirt=1;
	}
	return inode;
}

struct inode * lnamei(const char * pathname)
{
	return _namei(pathname, NULL, 0);
}

/*
 *	namei()
 *
 * is used by most simple commands to get the inode of a specified name.
 * Open, link etc use their own routines, but this is enough for things
 * like 'chmod' etc.
 */
struct inode * namei(const char * pathname)
{
	return _namei(pathname,NULL,1);
}

/*
 *	open_namei()
 *
 * namei for open - this is in fact almost the whole open-routine.
 */
int open_namei(const char * pathname, int flag, int mode,
	struct inode ** res_inode)
{
	const char * basename;
	int namelen,error;
	struct inode * dir, *inode;

	if ((flag & O_TRUNC) && !(flag & O_ACCMODE))
		flag |= O_WRONLY;
	mode &= 0777 & ~current->umask;
	mode |= I_REGULAR;
	if (!(dir = dir_namei(pathname,&namelen,&basename,NULL)))
		return -ENOENT;
	if (!namelen) {			/* special case: '/usr/' etc */
		if (!(flag & (O_ACCMODE|O_CREAT|O_TRUNC))) {
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
		return dir->i_op->create(dir,basename,namelen,mode,res_inode);
	}
	if (flag & O_EXCL) {
		iput(dir);
		iput(inode);
		return -EEXIST;
	}
	if (!(inode = follow_link(dir,inode)))
		return -ELOOP;
	if ((S_ISDIR(inode->i_mode) && (flag & O_ACCMODE)) ||
	    !permission(inode,ACC_MODE(flag))) {
		iput(inode);
		return -EPERM;
	}
	inode->i_atime = CURRENT_TIME;
	if (flag & O_TRUNC)
		minix_truncate(inode);
	*res_inode = inode;
	return 0;
}

int sys_mknod(const char * filename, int mode, int dev)
{
	const char * basename;
	int namelen;
	struct inode * dir;
	
	if (!suser())
		return -EPERM;
	if (!(dir = dir_namei(filename,&namelen,&basename, NULL)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
	if (!dir->i_op || !dir->i_op->mknod) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->mknod(dir,basename,namelen,mode,dev);
}

int sys_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen;
	struct inode * dir;

	if (!(dir = dir_namei(pathname,&namelen,&basename, NULL)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
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
	int namelen;
	struct inode * dir;

	if (!(dir = dir_namei(name,&namelen,&basename, NULL)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
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
	int namelen;
	struct inode * dir;

	if (!(dir = dir_namei(name,&namelen,&basename, NULL)))
		return -ENOENT;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
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
	int namelen;

	dir = dir_namei(newname,&namelen,&basename, NULL);
	if (!dir)
		return -EACCES;
	if (!namelen) {
		iput(dir);
		return -EPERM;
	}
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
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
	int namelen;

	oldinode = namei(oldname);
	if (!oldinode)
		return -ENOENT;
	dir = dir_namei(newname,&namelen,&basename, NULL);
	if (!dir) {
		iput(oldinode);
		return -EACCES;
	}
	if (!namelen) {
		iput(oldinode);
		iput(dir);
		return -EPERM;
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
	int old_len, new_len;

	old_dir = dir_namei(oldname,&old_len,&old_base, NULL);
	if (!old_dir)
		return -ENOENT;
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
	new_dir = dir_namei(newname,&new_len,&new_base, NULL);
	if (!new_dir) {
		iput(old_dir);
		return -ENOENT;
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
	if (!old_dir->i_op || !old_dir->i_op->rename) {
		iput(old_dir);
		iput(new_dir);
		return -EPERM;
	}
	return old_dir->i_op->rename(old_dir, old_base, old_len, 
		new_dir, new_base, new_len);
}
