/*
 *  linux/fs/msdos/namei.c
 *
 *  Written 1992 by Werner Almesberger
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/msdos_fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>

/* MS-DOS "device special files" */

static char *reserved_names[] = {
    "CON     ","PRN     ","NUL     ","AUX     ",
    "LPT1    ","LPT2    ","LPT3    ","LPT4    ",
    "COM1    ","COM2    ","COM3    ","COM4    ",
    NULL };


/* Characters that are undesirable in an MS-DOS file name */
  
static char bad_chars[] = "*?<>|\" ";
static char bad_if_strict[] = "+=,;";


/* Formats an MS-DOS file name. Rejects invalid names. */

static int msdos_format_name(char conv,const char *name,int len,char *res)
{
	char *walk,**reserved;
	char c;
	int space;

	if (get_fs_byte(name) == DELETED_FLAG) return -EINVAL;
	if (get_fs_byte(name) == '.' && (len == 1 || (len == 2 &&
	    get_fs_byte(name+1) == '.'))) {
		memset(res+1,' ',10);
		while (len--) *res++ = '.';
		return 0;
	}
	space = 0; /* to make GCC happy */
	c = 0;
	for (walk = res; len && walk-res < 8; walk++) {
	    	c = get_fs_byte(name++);
		len--;
		if (conv != 'r' && strchr(bad_chars,c)) return -EINVAL;
		if (conv == 's' && strchr(bad_if_strict,c)) return -EINVAL;
  		if (c >= 'A' && c <= 'Z') {
			if (conv == 's') return -EINVAL;
			c += 32;
		}
		if (c < ' ' || c == ':' || c == '\\') return -EINVAL;
		if (c == '.') break;
		space = c == ' ';
		*walk = c >= 'a' && c <= 'z' ? c-32 : c;
	}
	if (space) return -EINVAL;
	if (conv == 's' && len && c != '.') {
		c = get_fs_byte(name++);
		len--;
		if (c != '.') return -EINVAL;
	}
	while (c != '.' && len--) c = get_fs_byte(name++);
	if (walk == res) return -EINVAL;
	if (c == '.') {
		while (walk-res < 8) *walk++ = ' ';
		while (len > 0 && walk-res < MSDOS_NAME) {
			c = get_fs_byte(name++);
			len--;
			if (conv != 'r' && strchr(bad_chars,c)) return -EINVAL;
			if (conv == 's' && strchr(bad_if_strict,c))
				return -EINVAL;
			if (c < ' ' || c == ':' || c == '\\' || c == '.')
				return -EINVAL;
			if (c >= 'A' && c <= 'Z') {
				if (conv == 's') return -EINVAL;
				c += 32;
			}
			space = c == ' ';
			*walk++ = c >= 'a' && c <= 'z' ? c-32 : c;
		}
		if (space) return -EINVAL;
		if (conv == 's' && len) return -EINVAL;
	}
	while (walk-res < MSDOS_NAME) *walk++ = ' ';
	for (reserved = reserved_names; *reserved; reserved++)
		if (!strncmp(res,*reserved,8)) return -EINVAL;
	return 0;
}


/* Locates a directory entry. */

static int msdos_find(struct inode *dir,const char *name,int len,
    struct buffer_head **bh,struct msdos_dir_entry **de,int *ino)
{
	char msdos_name[MSDOS_NAME];
	int res;

	if ((res = msdos_format_name(MSDOS_SB(dir->i_sb)->name_check,name,len,
	    msdos_name)) < 0) return res;
	return msdos_scan(dir,msdos_name,bh,de,ino);
}


int msdos_lookup(struct inode *dir,const char *name,int len,
    struct inode **result)
{
	int ino,res;
	struct msdos_dir_entry *de;
	struct buffer_head *bh;
	struct inode *next;

	*result = NULL;
	if (!dir) return -ENOENT;
	if (!S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOENT;
	}
	if (len == 1 && get_fs_byte(name) == '.') {
		*result = dir;
		return 0;
	}
	if (len == 2 && get_fs_byte(name) == '.' && get_fs_byte(name+1) == '.')
	    {
		ino = msdos_parent_ino(dir,0);
		iput(dir);
		if (ino < 0) return ino;
		if (!(*result = iget(dir->i_sb,ino))) return -EACCES;
		return 0;
	}
	if ((res = msdos_find(dir,name,len,&bh,&de,&ino)) < 0) {
		iput(dir);
		return res;
	}
	if (bh) brelse(bh);
/* printk("lookup: ino=%d\r\n",ino); */
	if (!(*result = iget(dir->i_sb,ino))) {
		iput(dir);
		return -EACCES;
	}
	if (MSDOS_I(*result)->i_busy) { /* mkdir in progress */
		iput(*result);
		iput(dir);
		return -ENOENT;
	}
	while (MSDOS_I(*result)->i_old) {
		next = MSDOS_I(*result)->i_old;
		iput(*result);
		if (!(*result = iget(next->i_sb,next->i_ino)))
			panic("msdos_lookup: Can't happen");
	}
	iput(dir);
	return 0;
}


/* Creates a directory entry (name is already formatted). */

static int msdos_create_entry(struct inode *dir,char *name,int is_dir,
    struct inode **result)
{
	struct buffer_head *bh;
	struct msdos_dir_entry *de;
	int res,ino;

	if ((res = msdos_scan(dir,NULL,&bh,&de,&ino)) < 0) {
		if (dir->i_ino == MSDOS_ROOT_INO) return -ENOSPC;
		if ((res = msdos_add_cluster(dir)) < 0) return res;
		if ((res = msdos_scan(dir,NULL,&bh,&de,&ino)) < 0) return res;
	}
	memcpy(de->name,name,MSDOS_NAME);
	de->attr = is_dir ? ATTR_DIR : ATTR_ARCH;
	de->start = 0;
	date_unix2dos(CURRENT_TIME,&de->time,&de->date);
	de->size = 0;
	bh->b_dirt = 1;
	if (*result = iget(dir->i_sb,ino)) msdos_read_inode(*result);
	brelse(bh);
	if (!*result) return -EIO;
	(*result)->i_mtime = (*result)->i_atime = (*result)->i_ctime =
	    CURRENT_TIME;
	(*result)->i_dirt = 1;
	return 0;
}


int msdos_create(struct inode *dir,const char *name,int len,int mode,
	struct inode **result)
{
	struct buffer_head *bh;
	struct msdos_dir_entry *de;
	char msdos_name[MSDOS_NAME];
	int ino,res;

	if (!dir) return -ENOENT;
	if ((res = msdos_format_name(MSDOS_SB(dir->i_sb)->name_check,name,len,
	    msdos_name)) < 0) {
		iput(dir);
		return res;
	}
	lock_creation();
	if (msdos_scan(dir,msdos_name,&bh,&de,&ino) >= 0) {
		unlock_creation();
		brelse(bh);
		iput(dir);
		return -EEXIST;
 	}
	res = msdos_create_entry(dir,msdos_name,S_ISDIR(mode),result);
	unlock_creation();
	iput(dir);
	return res;
}


#ifdef DEBUG

static void dump_fat(struct super_block *sb,int start)
{
	printk("[");
	while (start) {
		printk("%d ",start);
        	start = fat_access(sb,start,-1);
		if (!start) {
			printk("ERROR");
			break;
		}
		if (start == -1) break;
	}
	printk("]\n");
}

#endif


int msdos_mkdir(struct inode *dir,const char *name,int len,int mode)
{
	struct buffer_head *bh;
	struct msdos_dir_entry *de;
	struct inode *inode,*dot;
	char msdos_name[MSDOS_NAME];
	int ino,res;

	if ((res = msdos_format_name(MSDOS_SB(dir->i_sb)->name_check,name,len,
	    msdos_name)) < 0) {
		iput(dir);
		return res;
	}
	lock_creation();
	if (msdos_scan(dir,msdos_name,&bh,&de,&ino) >= 0) {
		unlock_creation();
		brelse(bh);
		iput(dir);
		return -EEXIST;
 	}
	if ((res = msdos_create_entry(dir,msdos_name,1,&inode)) < 0) {
		unlock_creation();
		iput(dir);
		return res;
	}
	dir->i_nlink++;
	inode->i_nlink = 2; /* no need to mark them dirty */
	MSDOS_I(inode)->i_busy = 1; /* prevent lookups */
	if ((res = msdos_add_cluster(inode)) < 0) goto mkdir_error;
	if ((res = msdos_create_entry(inode,MSDOS_DOT,1,&dot)) < 0)
		goto mkdir_error;
	dot->i_size = inode->i_size; /* doesn't grow in the 2nd create_entry */
	MSDOS_I(dot)->i_start = MSDOS_I(inode)->i_start;
	dot->i_nlink = inode->i_nlink;
	dot->i_dirt = 1;
	iput(dot);
	if ((res = msdos_create_entry(inode,MSDOS_DOTDOT,1,&dot)) < 0)
		goto mkdir_error;
	unlock_creation();
	dot->i_size = dir->i_size;
	MSDOS_I(dot)->i_start = MSDOS_I(dir)->i_start;
	dot->i_nlink = dir->i_nlink;
	dot->i_dirt = 1;
	MSDOS_I(inode)->i_busy = 0;
	iput(dot);
	iput(inode);
	iput(dir);
	return 0;
mkdir_error:
	iput(inode);
	if (msdos_rmdir(dir,name,len) < 0) panic("rmdir in mkdir failed");
	unlock_creation();
	return res;
}


int msdos_rmdir(struct inode *dir,const char *name,int len)
{
	int res,ino,pos;
	struct buffer_head *bh,*dbh;
	struct msdos_dir_entry *de,*dde;
	struct inode *inode;

	bh = NULL;
	inode = NULL;
	res = -EINVAL;
	if (get_fs_byte(name) == '.' && (len == 1 || (len == 2 &&
	    get_fs_byte(name+1) == '.'))) goto rmdir_done;
	if ((res = msdos_find(dir,name,len,&bh,&de,&ino)) < 0) goto rmdir_done;
	res = -ENOENT;
	if (!(inode = iget(dir->i_sb,ino))) goto rmdir_done;
	res = -ENOTDIR;
	if (!S_ISDIR(inode->i_mode)) goto rmdir_done;
	res = -EBUSY;
	if (dir->i_dev != inode->i_dev || dir == inode) goto rmdir_done;
	if (inode->i_count > 1) goto rmdir_done;
	if (MSDOS_I(inode)->i_start) { /* may be zero in mkdir */
		res = -ENOTEMPTY;
		pos = 0;
		dbh = NULL;
		while (msdos_get_entry(inode,&pos,&dbh,&dde) > -1)
			if (dde->name[0] && ((unsigned char *) dde->name)[0] !=
			    DELETED_FLAG && strncmp(dde->name,MSDOS_DOT,
			    MSDOS_NAME) && strncmp(dde->name,MSDOS_DOTDOT,
			    MSDOS_NAME)) goto rmdir_done;
		if (dbh) brelse(dbh);
	}
	inode->i_nlink = 0;
	dir->i_mtime = CURRENT_TIME;
	dir->i_nlink--;
	inode->i_dirt = dir->i_dirt = 1;
	de->name[0] = DELETED_FLAG;
	bh->b_dirt = 1;
	res = 0;
rmdir_done:
	brelse(bh);
	iput(dir);
	iput(inode);
	return res;
}


int msdos_unlink(struct inode *dir,const char *name,int len)
{
	int res,ino;
	struct buffer_head *bh;
	struct msdos_dir_entry *de;
	struct inode *inode;

	bh = NULL;
	inode = NULL;
	if ((res = msdos_find(dir,name,len,&bh,&de,&ino)) < 0)
		goto unlink_done;
	if (!(inode = iget(dir->i_sb,ino))) {
		res = -ENOENT;
		goto unlink_done;
	}
	if (!S_ISREG(inode->i_mode)) {
		res = -EPERM;
		goto unlink_done;
	}
	inode->i_nlink = 0;
	MSDOS_I(inode)->i_busy = 1;
	inode->i_dirt = 1;
	de->name[0] = DELETED_FLAG;
	bh->b_dirt = 1;
unlink_done:
	brelse(bh);
	iput(inode);
	iput(dir);
	return res;
}


static int rename_same_dir(struct inode *old_dir,char *old_name,
    struct inode *new_dir,char *new_name,struct buffer_head *old_bh,
    struct msdos_dir_entry *old_de,int old_ino)
{
	struct buffer_head *new_bh;
	struct msdos_dir_entry *new_de;
	struct inode *new_inode,*old_inode;
	int new_ino;
	int exists;

	if (!strncmp(old_name,new_name,MSDOS_NAME)) return 0;
	exists = msdos_scan(new_dir,new_name,&new_bh,&new_de,&new_ino) >= 0;
	if (*(unsigned char *) old_de->name == DELETED_FLAG) {
		if (exists) brelse(new_bh);
		return -ENOENT;
	}
	if (exists) {
		if (!(new_inode = iget(new_dir->i_sb,new_ino))) {
			brelse(new_bh);
			return -EIO;
		}
		if (S_ISDIR(new_inode->i_mode)) {
			iput(new_inode);
			brelse(new_bh);
			return -EPERM;
		}
		new_inode->i_nlink = 0;
		MSDOS_I(new_inode)->i_busy = 1;
		new_inode->i_dirt = 1;
		new_de->name[0] = DELETED_FLAG;
		new_bh->b_dirt = 1;
		iput(new_inode);
		brelse(new_bh);
	}
	memcpy(old_de->name,new_name,MSDOS_NAME);
	old_bh->b_dirt = 1;
	if (MSDOS_SB(old_dir->i_sb)->conversion == 'a') /* update binary info */
		if (old_inode = iget(old_dir->i_sb,old_ino)) {
			msdos_read_inode(old_inode);
			iput(old_inode);
		}
	return 0;
}


static int rename_diff_dir(struct inode *old_dir,char *old_name,
    struct inode *new_dir,char *new_name,struct buffer_head *old_bh,
    struct msdos_dir_entry *old_de,int old_ino)
{
	struct buffer_head *new_bh,*free_bh,*dotdot_bh;
	struct msdos_dir_entry *new_de,*free_de,*dotdot_de;
	struct inode *old_inode,*new_inode,*free_inode,*dotdot_inode,*walk;
	int new_ino,free_ino,dotdot_ino;
	int error,exists,ino;

	if (old_dir->i_dev != new_dir->i_dev) return -EINVAL;
	if (old_ino == new_dir->i_ino) return -EINVAL;
	if (!(walk = iget(new_dir->i_sb,new_dir->i_ino))) return -EIO;
	while (walk->i_ino != MSDOS_ROOT_INO) {
		ino = msdos_parent_ino(walk,1);
		iput(walk);
		if (ino < 0) return ino;
		if (ino == old_ino) return -EINVAL;
		if (!(walk = iget(new_dir->i_sb,ino))) return -EIO;
	}
	iput(walk);
	if ((error = msdos_scan(new_dir,NULL,&free_bh,&free_de,&free_ino)) < 0)
	    return error;
	exists = msdos_scan(new_dir,new_name,&new_bh,&new_de,&new_ino)
	    >= 0;
	if (!(old_inode = iget(old_dir->i_sb,old_ino))) {
		brelse(free_bh);
		if (exists) brelse(new_bh);
		return -EIO;
	}
	if (*(unsigned char *) old_de->name == DELETED_FLAG) {
		iput(old_inode);
		brelse(free_bh);
		if (exists) brelse(new_bh);
		return -ENOENT;
	}
	new_inode = NULL; /* to make GCC happy */
	if (exists) {
		if (!(new_inode = iget(new_dir->i_sb,new_ino))) {
			iput(old_inode);
			brelse(new_bh);
			return -EIO;
		}
		if (S_ISDIR(new_inode->i_mode)) {
			iput(new_inode);
			iput(old_inode);
			brelse(new_bh);
			return -EPERM;
		}
		new_inode->i_nlink = 0;
		MSDOS_I(new_inode)->i_busy = 1;
		new_inode->i_dirt = 1;
		new_de->name[0] = DELETED_FLAG;
		new_bh->b_dirt = 1;
	}
	memcpy(free_de,old_de,sizeof(struct msdos_dir_entry));
	memcpy(free_de->name,new_name,MSDOS_NAME);
	if (!(free_inode = iget(new_dir->i_sb,free_ino))) {
		free_de->name[0] = DELETED_FLAG;
/*  Don't mark free_bh as dirty. Both states are supposed to be equivalent. */
		brelse(free_bh);
		if (exists) {
			iput(new_inode);
			brelse(new_bh);
		}
		return -EIO;
	}
	msdos_read_inode(free_inode);
	MSDOS_I(old_inode)->i_busy = 1;
	cache_inval_inode(old_inode);
	old_inode->i_dirt = 1;
	old_de->name[0] = DELETED_FLAG;
	old_bh->b_dirt = 1;
	free_bh->b_dirt = 1;
	if (!exists) iput(free_inode);
	else {
		MSDOS_I(new_inode)->i_depend = free_inode;
		MSDOS_I(free_inode)->i_old = new_inode;
		/* free_inode is put when putting new_inode */
		iput(new_inode);
		brelse(new_bh);
	}
	if (S_ISDIR(old_inode->i_mode)) {
		if ((error = msdos_scan(old_inode,MSDOS_DOTDOT,&dotdot_bh,
		    &dotdot_de,&dotdot_ino)) < 0) goto rename_done;
		if (!(dotdot_inode = iget(old_inode->i_sb,dotdot_ino))) {
			brelse(dotdot_bh);
			error = -EIO;
			goto rename_done;
		}
		dotdot_de->start = MSDOS_I(dotdot_inode)->i_start =
		    MSDOS_I(new_dir)->i_start;
		dotdot_inode->i_dirt = 1;
		dotdot_bh->b_dirt = 1;
		old_dir->i_nlink--;
		new_dir->i_nlink++;
		/* no need to mark them dirty */
		dotdot_inode->i_nlink = new_dir->i_nlink;
		iput(dotdot_inode);
		brelse(dotdot_bh);
	}
	error = 0;
rename_done:
	brelse(free_bh);
	iput(old_inode);
	return error;
}


int msdos_rename(struct inode *old_dir,const char *old_name,int old_len,
	struct inode *new_dir,const char *new_name,int new_len)
{
	char old_msdos_name[MSDOS_NAME],new_msdos_name[MSDOS_NAME];
	struct buffer_head *old_bh;
	struct msdos_dir_entry *old_de;
	int old_ino,error;

	if ((error = msdos_format_name(MSDOS_SB(old_dir->i_sb)->name_check,
	    old_name,old_len,old_msdos_name)) < 0) goto rename_done;
	if ((error = msdos_format_name(MSDOS_SB(new_dir->i_sb)->name_check,
	    new_name,new_len,new_msdos_name)) < 0) goto rename_done;
	if ((error = msdos_scan(old_dir,old_msdos_name,&old_bh,&old_de,
	    &old_ino)) < 0) goto rename_done;
	lock_creation();
	if (old_dir == new_dir)
		error = rename_same_dir(old_dir,old_msdos_name,new_dir,
		    new_msdos_name,old_bh,old_de,old_ino);
	else error = rename_diff_dir(old_dir,old_msdos_name,new_dir,
		    new_msdos_name,old_bh,old_de,old_ino);
	unlock_creation();
	brelse(old_bh);
rename_done:
	iput(old_dir);
	iput(new_dir);
	return error;
}
