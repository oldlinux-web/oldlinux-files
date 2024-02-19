/*
 *  linux/fs/ext/dir.c
 *
 *  Copyright (C) 1992 Remy Card (card@masi.ibp.fr)
 *
 *  from
 *
 *  linux/fs/minix/dir.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  ext directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ext_fs.h>
#include <linux/stat.h>

static int ext_dir_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	return -EISDIR;
}

static int ext_readdir(struct inode *, struct file *, struct dirent *, int);

static struct file_operations ext_dir_operations = {
	NULL,			/* lseek - default */
	ext_dir_read,		/* read */
	NULL,			/* write - bad */
	ext_readdir,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL			/* no special release code */
};

/*
 * directories can handle most operations...
 */
struct inode_operations ext_dir_inode_operations = {
	&ext_dir_operations,	/* default directory file-ops */
	ext_create,		/* create */
	ext_lookup,		/* lookup */
	ext_link,		/* link */
	ext_unlink,		/* unlink */
	ext_symlink,		/* symlink */
	ext_mkdir,		/* mkdir */
	ext_rmdir,		/* rmdir */
	ext_mknod,		/* mknod */
	ext_rename,		/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* bmap */
	ext_truncate		/* truncate */
};

static int ext_readdir(struct inode * inode, struct file * filp,
	struct dirent * dirent, int count)
{
	unsigned int offset,i;
	char c;
	struct buffer_head * bh;
	struct ext_dir_entry * de;

	if (!inode || !S_ISDIR(inode->i_mode))
		return -EBADF;
	while (filp->f_pos < inode->i_size) {
		offset = filp->f_pos & 1023;
		bh = ext_bread(inode,(filp->f_pos)>>BLOCK_SIZE_BITS,0);
		if (!bh) {
			filp->f_pos += 1024-offset;
			continue;
		}
		de = (struct ext_dir_entry *) (offset + bh->b_data);
		while (offset < 1024 && filp->f_pos < inode->i_size) {
			offset += de->rec_len;
			filp->f_pos += de->rec_len;
			if (de->rec_len < 8 || de->rec_len % 4 != 0 ||
			    de->rec_len < de->name_len + 8) {
				printk ("ext_readdir: bad directory entry\n");
				printk ("dev=%d, dir=%d, offset=%d, rec_len=%d, name_len=%d\n",
					inode->i_dev, inode->i_ino, offset, de->rec_len, de->name_len);
				return 0;
			}
			if (de->inode) {
				for (i = 0; i < de->name_len; i++)
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
			de = (struct ext_dir_entry *) ((char *) de 
				+ de->rec_len);
		}
		brelse(bh);
	}
	return 0;
}
