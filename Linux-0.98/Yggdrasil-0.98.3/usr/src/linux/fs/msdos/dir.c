/*
 *  linux/fs/msdos/dir.c
 *
 *  Written 1992 by Werner Almesberger
 *
 *  MS-DOS directory handling functions
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/msdos_fs.h>
#include <linux/errno.h>
#include <linux/stat.h>

static int msdos_dir_read(struct inode * inode,struct file * filp, char * buf,int count)
{
	return -EISDIR;
}

static int msdos_readdir(struct inode *inode,struct file *filp,
    struct dirent *dirent,int count);

static struct file_operations msdos_dir_operations = {
	NULL,			/* lseek - default */
	msdos_dir_read,		/* read */
	NULL,			/* write - bad */
	msdos_readdir,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

struct inode_operations msdos_dir_inode_operations = {
	&msdos_dir_operations,	/* default directory file-ops */
	msdos_create,		/* create */
	msdos_lookup,		/* lookup */
	NULL,			/* link */
	msdos_unlink,		/* unlink */
	NULL,			/* symlink */
	msdos_mkdir,		/* mkdir */
	msdos_rmdir,		/* rmdir */
	NULL,			/* mknod */
	msdos_rename,		/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	msdos_bmap,		/* bmap */
	NULL			/* truncate */
};

static int msdos_readdir(struct inode *inode,struct file *filp,
    struct dirent *dirent,int count)
{
	int ino,i,i2,last;
	char c,*walk;
	struct buffer_head *bh;
	struct msdos_dir_entry *de;

	if (!inode || !S_ISDIR(inode->i_mode)) return -EBADF;
	if (inode->i_ino == MSDOS_ROOT_INO) {
/* Fake . and .. for the root directory. */
		if (filp->f_pos == 2) filp->f_pos = 0;
		else if (filp->f_pos < 2) {
				walk = filp->f_pos++ ? ".." : ".";
				for (i = 0; *walk; walk++)
					put_fs_byte(*walk,dirent->d_name+i++);
				put_fs_long(MSDOS_ROOT_INO,&dirent->d_ino);
				put_fs_byte(0,dirent->d_name+i);
				put_fs_word(i,&dirent->d_reclen);
				return i;
			}
	}
	if (filp->f_pos & (sizeof(struct msdos_dir_entry)-1)) return -ENOENT;
	bh = NULL;
	while ((ino = msdos_get_entry(inode,&filp->f_pos,&bh,&de)) > -1) {
		if (de->name[0] && ((unsigned char *) (de->name))[0] !=
		    DELETED_FLAG && !(de->attr & ATTR_VOLUME)) {
			for (i = last = 0; i < 8; i++) {
				if (!(c = de->name[i])) break;
				if (c >= 'A' && c <= 'Z') c += 32;
				if (c != ' ') last = i+1;
				put_fs_byte(c,i+dirent->d_name);
			}
			i = last;
			if (de->ext[0] && de->ext[0] != ' ') {
				put_fs_byte('.',i+dirent->d_name);
				i++;
				for (i2 = 0; i2 < 3; i2++) {
					if (!(c = de->ext[i2])) break;
					if (c >= 'A' && c <= 'Z') c += 32;
					put_fs_byte(c,i+dirent->d_name);
					i++;
					if (c != ' ') last = i;
				}
			}
			if (i = last) {
				if (!strcmp(de->name,MSDOS_DOT))
					ino = inode->i_ino;
				else if (!strcmp(de->name,MSDOS_DOTDOT))
						ino = msdos_parent_ino(inode,0);
				put_fs_long(ino,&dirent->d_ino);
				put_fs_byte(0,i+dirent->d_name);
				put_fs_word(i,&dirent->d_reclen);
				brelse(bh);
				return i;
			}
		}
	}
	if (bh) brelse(bh);
	return 0;
}
