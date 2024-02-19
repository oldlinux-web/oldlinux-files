/*
 *  linux/fs/minix/namei.c
 *
 *  (C) 1991  Linus Torvalds
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

extern int permission(struct inode * inode,int mask);
extern struct inode * _namei(const char * filename, struct inode * base,
	int follow_links);

/*
 * comment out this line if you want names > MINIX_NAME_LEN chars to be
 * truncated. Else they will be disallowed.
 */
/* #define NO_TRUNCATE */

/*
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use minix_match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, minix_match returns 1 for success, 0 for failure.
 */
static int minix_match(int len,const char * name,struct minix_dir_entry * de)
{
	register int same __asm__("ax");

	if (!de || !de->inode || len > MINIX_NAME_LEN)
		return 0;
	/* "" means "." ---> so paths like "/usr/lib//libc.a" work */
	if (!len && (de->name[0]=='.') && (de->name[1]=='\0'))
		return 1;
	if (len < MINIX_NAME_LEN && de->name[len])
		return 0;
	__asm__("cld\n\t"
		"fs ; repe ; cmpsb\n\t"
		"setz %%al"
		:"=a" (same)
		:"0" (0),"S" ((long) name),"D" ((long) de->name),"c" (len)
		:"cx","di","si");
	return same;
}

/*
 *	minix_find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the entry was found, and the entry
 * itself (as a parameter - res_dir). It does NOT read the inode of the
 * entry - you'll have to do that yourself if you want to.
 */
static struct buffer_head * minix_find_entry(struct inode * dir,
	const char * name, int namelen, struct minix_dir_entry ** res_dir)
{
	int entries;
	int block,i;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	*res_dir = NULL;
	if (!dir)
		return NULL;
#ifdef NO_TRUNCATE
	if (namelen > MINIX_NAME_LEN)
		return NULL;
#else
	if (namelen > MINIX_NAME_LEN)
		namelen = MINIX_NAME_LEN;
#endif
	entries = dir->i_size / (sizeof (struct minix_dir_entry));
	if (!(block = dir->i_data[0]))
		return NULL;
	if (!(bh = bread(dir->i_dev,block)))
		return NULL;
	i = 0;
	de = (struct minix_dir_entry *) bh->b_data;
	while (i < entries) {
		if ((char *)de >= BLOCK_SIZE+bh->b_data) {
			brelse(bh);
			bh = NULL;
			if (!(block = bmap(dir,i/MINIX_DIR_ENTRIES_PER_BLOCK)) ||
			    !(bh = bread(dir->i_dev,block))) {
				i += MINIX_DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			de = (struct minix_dir_entry *) bh->b_data;
		}
		if (minix_match(namelen,name,de)) {
			*res_dir = de;
			return bh;
		}
		de++;
		i++;
	}
	brelse(bh);
	return NULL;
}

struct inode * minix_follow_link(struct inode * dir, struct inode * inode)
{
	unsigned short fs;
	struct buffer_head * bh;

	if (!dir) {
		dir = current->root;
		dir->i_count++;
	}
	if (!inode) {
		iput(dir);
		return NULL;
	}
	if (!S_ISLNK(inode->i_mode)) {
		iput(dir);
		return inode;
	}
	__asm__("mov %%fs,%0":"=r" (fs));
	if ((current->link_count > 5) || !inode->i_data[0] ||
	   !(bh = bread(inode->i_dev, inode->i_data[0]))) {
		iput(dir);
		iput(inode);
		return NULL;
	}
	iput(inode);
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	current->link_count++;
	inode = _namei(bh->b_data,dir,1);
	current->link_count--;
	__asm__("mov %0,%%fs"::"r" (fs));
	brelse(bh);
	return inode;
}

int minix_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	int ino;
	struct minix_dir_entry * de;
	struct buffer_head * bh;

	*result = NULL;
	if (!dir)
		return -ENOENT;
	if (!S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}
	if (!(bh = minix_find_entry(dir,name,len,&de))) {
		iput(dir);
		return -ENOENT;
	}
	ino = de->inode;
	brelse(bh);
	if (!(*result = iget(dir->i_dev,ino))) {
		iput(dir);
		return -EACCES;
	}
	iput(dir);
	return 0;
}

/*
 *	minix_add_entry()
 *
 * adds a file entry to the specified directory, using the same
 * semantics as minix_find_entry(). It returns NULL if it failed.
 *
 * NOTE!! The inode part of 'de' is left at 0 - which means you
 * may not sleep between calling this and putting something into
 * the entry, as someone else might have used it while you slept.
 */
static struct buffer_head * minix_add_entry(struct inode * dir,
	const char * name, int namelen, struct minix_dir_entry ** res_dir)
{
	int block,i;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	*res_dir = NULL;
	if (!dir)
		return NULL;
#ifdef NO_TRUNCATE
	if (namelen > MINIX_NAME_LEN)
		return NULL;
#else
	if (namelen > MINIX_NAME_LEN)
		namelen = MINIX_NAME_LEN;
#endif
	if (!namelen)
		return NULL;
	if (!(block = dir->i_data[0]))
		return NULL;
	if (!(bh = bread(dir->i_dev,block)))
		return NULL;
	i = 0;
	de = (struct minix_dir_entry *) bh->b_data;
	while (1) {
		if ((char *)de >= BLOCK_SIZE+bh->b_data) {
			brelse(bh);
			bh = NULL;
			block = minix_create_block(dir,i/MINIX_DIR_ENTRIES_PER_BLOCK);
			if (!block)
				return NULL;
			if (!(bh = bread(dir->i_dev,block))) {
				i += MINIX_DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			de = (struct minix_dir_entry *) bh->b_data;
		}
		if (i*sizeof(struct minix_dir_entry) >= dir->i_size) {
			de->inode=0;
			dir->i_size = (i+1)*sizeof(struct minix_dir_entry);
			dir->i_dirt = 1;
			dir->i_ctime = CURRENT_TIME;
		}
		if (!de->inode) {
			dir->i_mtime = CURRENT_TIME;
			for (i=0; i < MINIX_NAME_LEN ; i++)
				de->name[i]=(i<namelen)?get_fs_byte(name+i):0;
			bh->b_dirt = 1;
			*res_dir = de;
			return bh;
		}
		de++;
		i++;
	}
	brelse(bh);
	return NULL;
}

int minix_create(struct inode * dir,const char * name, int len, int mode,
	struct inode ** result)
{
	struct inode * inode;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	*result = NULL;
	if (!dir)
		return -ENOENT;
	inode = minix_new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
	inode->i_mode = mode;
	inode->i_dirt = 1;
	bh = minix_add_entry(dir,name,len,&de);
	if (!bh) {
		inode->i_nlink--;
		iput(inode);
		iput(dir);
		return -ENOSPC;
	}
	de->inode = inode->i_ino;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
	*result = inode;
	return 0;
}

int minix_mknod(struct inode * dir, const char * name, int len, int mode, int rdev)
{
	struct inode * inode;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	if (!dir)
		return -ENOENT;
	bh = minix_find_entry(dir,name,len,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	inode = minix_new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
	inode->i_uid = current->euid;
	inode->i_mode = mode;
	if (S_ISBLK(mode) || S_ISCHR(mode))
		inode->i_rdev = rdev;
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
	inode->i_dirt = 1;
	bh = minix_add_entry(dir,name,len,&de);
	if (!bh) {
		inode->i_nlink--;
		iput(inode);
		iput(dir);
		return -ENOSPC;
	}
	de->inode = inode->i_ino;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
	iput(inode);
	return 0;
}

int minix_mkdir(struct inode * dir, const char * name, int len, int mode)
{
	struct inode * inode;
	struct buffer_head * bh, *dir_block;
	struct minix_dir_entry * de;
	
	bh = minix_find_entry(dir,name,len,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	inode = minix_new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
	inode->i_size = 32;
	inode->i_dirt = 1;
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
	if (!(inode->i_data[0] = minix_new_block(inode->i_dev))) {
		iput(dir);
		inode->i_nlink--;
		iput(inode);
		return -ENOSPC;
	}
	inode->i_dirt = 1;
	if (!(dir_block = bread(inode->i_dev,inode->i_data[0]))) {
		iput(dir);
		inode->i_nlink--;
		iput(inode);
		return -ERROR;
	}
	de = (struct minix_dir_entry *) dir_block->b_data;
	de->inode=inode->i_ino;
	strcpy(de->name,".");
	de++;
	de->inode = dir->i_ino;
	strcpy(de->name,"..");
	inode->i_nlink = 2;
	dir_block->b_dirt = 1;
	brelse(dir_block);
	inode->i_mode = I_DIRECTORY | (mode & 0777 & ~current->umask);
	inode->i_dirt = 1;
	bh = minix_add_entry(dir,name,len,&de);
	if (!bh) {
		iput(dir);
		inode->i_nlink=0;
		iput(inode);
		return -ENOSPC;
	}
	de->inode = inode->i_ino;
	bh->b_dirt = 1;
	dir->i_nlink++;
	dir->i_dirt = 1;
	iput(dir);
	iput(inode);
	brelse(bh);
	return 0;
}

/*
 * routine to check that the specified directory is empty (for rmdir)
 */
static int empty_dir(struct inode * inode)
{
	int nr,block;
	int len;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	len = inode->i_size / sizeof (struct minix_dir_entry);
	if (len<2 || !inode->i_data[0] ||
	    !(bh=bread(inode->i_dev,inode->i_data[0]))) {
	    	printk("warning - bad directory on dev %04x\n",inode->i_dev);
		return 0;
	}
	de = (struct minix_dir_entry *) bh->b_data;
	if (de[0].inode != inode->i_ino || !de[1].inode || 
	    strcmp(".",de[0].name) || strcmp("..",de[1].name)) {
	    	printk("warning - bad directory on dev %04x\n",inode->i_dev);
		return 0;
	}
	nr = 2;
	de += 2;
	while (nr<len) {
		if ((void *) de >= (void *) (bh->b_data+BLOCK_SIZE)) {
			brelse(bh);
			block=bmap(inode,nr/MINIX_DIR_ENTRIES_PER_BLOCK);
			if (!block) {
				nr += MINIX_DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			if (!(bh=bread(inode->i_dev,block)))
				return 0;
			de = (struct minix_dir_entry *) bh->b_data;
		}
		if (de->inode) {
			brelse(bh);
			return 0;
		}
		de++;
		nr++;
	}
	brelse(bh);
	return 1;
}

int minix_rmdir(struct inode * dir, const char * name, int len)
{
	int retval;
	struct inode * inode;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	inode = NULL;
	bh = minix_find_entry(dir,name,len,&de);
	retval = -ENOENT;
	if (!bh)
		goto end_rmdir;
	retval = -EPERM;
	if (!(inode = iget(dir->i_dev, de->inode)))
		goto end_rmdir;
	if ((dir->i_mode & S_ISVTX) && current->euid &&
	   inode->i_uid != current->euid)
		goto end_rmdir;
	if (inode->i_dev != dir->i_dev)
		goto end_rmdir;
	if (inode == dir)	/* we may not delete ".", but "../dir" is ok */
		goto end_rmdir;
	if (!S_ISDIR(inode->i_mode)) {
		retval = -ENOTDIR;
		goto end_rmdir;
	}
	if (!empty_dir(inode)) {
		retval = -ENOTEMPTY;
		goto end_rmdir;
	}
	if (inode->i_count > 1) {
		retval = -EBUSY;
		goto end_rmdir;
	}
	if (inode->i_nlink != 2)
		printk("empty directory has nlink!=2 (%d)\n",inode->i_nlink);
	de->inode = 0;
	bh->b_dirt = 1;
	inode->i_nlink=0;
	inode->i_dirt=1;
	dir->i_nlink--;
	dir->i_ctime = dir->i_mtime = CURRENT_TIME;
	dir->i_dirt=1;
	retval = 0;
end_rmdir:
	iput(dir);
	iput(inode);
	brelse(bh);
	return retval;
}

int minix_unlink(struct inode * dir, const char * name, int len)
{
	int retval;
	struct inode * inode;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	retval = -ENOENT;
	inode = NULL;
	bh = minix_find_entry(dir,name,len,&de);
	if (!bh)
		goto end_unlink;
	if (!(inode = iget(dir->i_dev, de->inode)))
		goto end_unlink;
	retval = -EPERM;
	if ((dir->i_mode & S_ISVTX) && !suser() &&
	    current->euid != inode->i_uid &&
	    current->euid != dir->i_uid)
		goto end_unlink;
	if (S_ISDIR(inode->i_mode))
		goto end_unlink;
	if (!inode->i_nlink) {
		printk("Deleting nonexistent file (%04x:%d), %d\n",
			inode->i_dev,inode->i_ino,inode->i_nlink);
		inode->i_nlink=1;
	}
	de->inode = 0;
	bh->b_dirt = 1;
	inode->i_nlink--;
	inode->i_dirt = 1;
	inode->i_ctime = CURRENT_TIME;
	retval = 0;
end_unlink:
	brelse(bh);
	iput(inode);
	iput(dir);
	return retval;
}

int minix_symlink(struct inode * dir, const char * name, int len, const char * symname)
{
	struct minix_dir_entry * de;
	struct inode * inode = NULL;
	struct buffer_head * bh = NULL, * name_block = NULL;
	int i;
	char c;

	if (!(inode = minix_new_inode(dir->i_dev))) {
		iput(dir);
		return -ENOSPC;
	}
	inode->i_mode = S_IFLNK | 0777;
	inode->i_dirt = 1;
	if (!(inode->i_data[0] = minix_new_block(inode->i_dev))) {
		iput(dir);
		inode->i_nlink--;
		iput(inode);
		return -ENOSPC;
	}
	inode->i_dirt = 1;
	if (!(name_block = bread(inode->i_dev,inode->i_data[0]))) {
		iput(dir);
		inode->i_nlink--;
		iput(inode);
		return -ERROR;
	}
	i = 0;
	while (i < 1023 && (c=get_fs_byte(symname++)))
		name_block->b_data[i++] = c;
	name_block->b_data[i] = 0;
	name_block->b_dirt = 1;
	brelse(name_block);
	inode->i_size = i;
	inode->i_dirt = 1;
	bh = minix_find_entry(dir,name,len,&de);
	if (bh) {
		inode->i_nlink--;
		iput(inode);
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
	bh = minix_add_entry(dir,name,len,&de);
	if (!bh) {
		inode->i_nlink--;
		iput(inode);
		iput(dir);
		return -ENOSPC;
	}
	de->inode = inode->i_ino;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
	iput(inode);
	return 0;
}

int minix_link(struct inode * oldinode, struct inode * dir, const char * name, int len)
{
	struct minix_dir_entry * de;
	struct buffer_head * bh;

	if (S_ISDIR(oldinode->i_mode)) {
		iput(oldinode);
		iput(dir);
		return -EPERM;
	}
	bh = minix_find_entry(dir,name,len,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		iput(oldinode);
		return -EEXIST;
	}
	bh = minix_add_entry(dir,name,len,&de);
	if (!bh) {
		iput(dir);
		iput(oldinode);
		return -ENOSPC;
	}
	de->inode = oldinode->i_ino;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
	oldinode->i_nlink++;
	oldinode->i_ctime = CURRENT_TIME;
	oldinode->i_dirt = 1;
	iput(oldinode);
	return 0;
}

static int subdir(struct inode * new, struct inode * old)
{
	unsigned short fs;
	int ino;
	int result;

	__asm__("mov %%fs,%0":"=r" (fs));
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	new->i_count++;
	result = 0;
	for (;;) {
		if (new == old) {
			result = 1;
			break;
		}
		if (new->i_dev != old->i_dev)
			break;
		ino = new->i_ino;
		if (minix_lookup(new,"..",2,&new))
			break;
		if (new->i_ino == ino)
			break;
	}
	iput(new);
	__asm__("mov %0,%%fs"::"r" (fs));
	return result;
}

#define PARENT_INO(buffer) \
(((struct minix_dir_entry *) (buffer))[1].inode)

#define PARENT_NAME(buffer) \
(((struct minix_dir_entry *) (buffer))[1].name)

/*
 * rename uses retrying to avoid race-conditions: at least they should be minimal.
 * it tries to allocate all the blocks, then sanity-checks, and if the sanity-
 * checks fail, it tries to restart itself again. Very practical - no changes
 * are done until we know everything works ok.. and then all the changes can be
 * done in one fell swoop when we have claimed all the buffers needed.
 *
 * Anybody can rename anything with this: the permission checks are left to the
 * higher-level routines.
 */
static int do_minix_rename(struct inode * old_dir, const char * old_name, int old_len,
	struct inode * new_dir, const char * new_name, int new_len)
{
	struct inode * old_inode, * new_inode;
	struct buffer_head * old_bh, * new_bh, * dir_bh;
	struct minix_dir_entry * old_de, * new_de;
	int retval;

	goto start_up;
try_again:
	brelse(old_bh);
	brelse(new_bh);
	brelse(dir_bh);
	iput(old_inode);
	iput(new_inode);
	current->counter = 0;
	schedule();
start_up:
	old_inode = new_inode = NULL;
	old_bh = new_bh = dir_bh = NULL;
	old_bh = minix_find_entry(old_dir,old_name,old_len,&old_de);
	retval = -ENOENT;
	if (!old_bh)
		goto end_rename;
	old_inode = iget(old_dir->i_dev, old_de->inode);
	if (!old_inode)
		goto end_rename;
	new_bh = minix_find_entry(new_dir,new_name,new_len,&new_de);
	if (new_bh) {
		new_inode = iget(new_dir->i_dev, new_de->inode);
		if (!new_inode) {
			brelse(new_bh);
			new_bh = NULL;
		}
	}
	if (new_inode == old_inode) {
		retval = 0;
		goto end_rename;
	}
	if (S_ISDIR(old_inode->i_mode)) {
		retval = -EEXIST;
		if (new_bh)
			goto end_rename;
		retval = -EACCES;
		if (!permission(old_inode, MAY_WRITE))
			goto end_rename;
		retval = -EINVAL;
		if (subdir(new_dir, old_inode))
			goto end_rename;
		retval = -EIO;
		if (!old_inode->i_data[0])
			goto end_rename;
		if (!(dir_bh = bread(old_inode->i_dev, old_inode->i_data[0])))
			goto end_rename;
		if (PARENT_INO(dir_bh->b_data) != old_dir->i_ino)
			goto end_rename;
	}
	if (!new_bh)
		new_bh = minix_add_entry(new_dir,new_name,new_len,&new_de);
	retval = -ENOSPC;
	if (!new_bh)
		goto end_rename;
/* sanity checking before doing the rename - avoid races */
	if (new_inode && (new_de->inode != new_inode->i_ino))
		goto try_again;
	if (new_de->inode && !new_inode)
		goto try_again;
	if (old_de->inode != old_inode->i_ino)
		goto try_again;
/* ok, that's it */
	old_de->inode = 0;
	new_de->inode = old_inode->i_ino;
	if (new_inode)
		new_inode->i_nlink--;
	old_bh->b_dirt = 1;
	new_bh->b_dirt = 1;
	if (dir_bh) {
		PARENT_INO(dir_bh->b_data) = new_dir->i_ino;
		dir_bh->b_dirt = 1;
		old_dir->i_nlink--;
		new_dir->i_nlink++;
		old_dir->i_dirt = 1;
		new_dir->i_dirt = 1;
	}
	retval = 0;
end_rename:
	brelse(dir_bh);
	brelse(old_bh);
	brelse(new_bh);
	iput(old_inode);
	iput(new_inode);
	iput(old_dir);
	iput(new_dir);
	return retval;
}

/*
 * Ok, rename also locks out other renames, as they can change the parent of
 * a directory, and we don't want any races. Other races are checked for by
 * "do_rename()", which restarts if there are inconsistencies.
 *
 * Note that there is no race between different filesystems: it's only within
 * the same device that races occur: many renames can happen at once, as long
 * as they are on different partitions.
 */
int minix_rename(struct inode * old_dir, const char * old_name, int old_len,
	struct inode * new_dir, const char * new_name, int new_len)
{
	static struct task_struct * wait = NULL;
	static int lock = 0;
	int result;

	while (lock)
		sleep_on(&wait);
	lock = 1;
	result = do_minix_rename(old_dir, old_name, old_len,
		new_dir, new_name, new_len);
	lock = 0;
	wake_up(&wait);
	return result;
}

int minix_readlink(struct inode * inode, char * buffer, int buflen)
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
	if (inode->i_data[0])
		bh = bread(inode->i_dev, inode->i_data[0]);
	else
		bh = NULL;
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
