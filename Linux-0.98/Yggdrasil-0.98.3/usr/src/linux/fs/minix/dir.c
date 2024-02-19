/*
 *  linux/fs/minix/dir.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  minix directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/minix_fs.h>
#include <linux/stat.h>

static int minix_dir_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	return -EISDIR;
}

static int minix_readdir(struct inode *, struct file *, struct dirent *, int);

static struct file_operations minix_dir_operations = {
	NULL,			/* lseek - default */
	minix_dir_read,		/* read */
	NULL,			/* write - bad */
	minix_readdir,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

/*
 * directories can handle most operations...
 */
struct inode_operations minix_dir_inode_operations = {
	&minix_dir_operations,	/* default directory file-ops */
	minix_create,		/* create */
	minix_lookup,		/* lookup */
	minix_link,		/* link */
	minix_unlink,		/* unlink */
	minix_symlink,		/* symlink */
	minix_mkdir,		/* mkdir */
	minix_rmdir,		/* rmdir */
	minix_mknod,		/* mknod */
	minix_rename,		/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* bmap */
	minix_truncate		/* truncate */
};

static int minix_readdir(struct inode * inode, struct file * filp,
	struct dirent * dirent, int count)
{
	unsigned int offset,i;
	char c;
	struct buffer_head * bh;
	struct minix_dir_entry * de;

	if (!inode || !S_ISDIR(inode->i_mode))
		return -EBADF;
	if (filp->f_pos & (sizeof (struct minix_dir_entry) - 1))
		return -EBADF;
	while (filp->f_pos < inode->i_size) {
		offset = filp->f_pos & 1023;
		bh = minix_bread(inode,(filp->f_pos)>>BLOCK_SIZE_BITS,0);
		if (!bh) {
			filp->f_pos += 1024-offset;
			continue;
		}
		de = (struct minix_dir_entry *) (offset + bh->b_data);
		while (offset < 1024 && filp->f_pos < inode->i_size) {
			offset += sizeof (struct minix_dir_entry);
			filp->f_pos += sizeof (struct minix_dir_entry);
			if (de->inode) {
				for (i = 0; i < MINIX_NAME_LEN; i++)
					if (c = de->name[i])
						put_fs_byte(c,i+dirent->d_name);
					else
						break;
				if (i) {
					put_fs_long(de->inode,&dirent->d_ino);
					put_fs_byte(0,i+dirent->d_name);
					put_fs_word(i,&dirent->d_reclen);
					brelse(bh);
					return i;
				}
			}
			de++;
		}
		brelse(bh);
	}
	return 0;
}
