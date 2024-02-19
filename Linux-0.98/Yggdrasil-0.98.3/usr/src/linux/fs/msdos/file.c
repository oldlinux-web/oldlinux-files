/*
 *  linux/fs/msdos/file.c
 *
 *  Written 1992 by Werner Almesberger
 *
 *  MS-DOS regular file handling primitives
 */

#include <asm/segment.h>
#include <asm/system.h>

#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/msdos_fs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

static int msdos_file_read(struct inode *inode,struct file *filp,char *buf,
    int count);
static int msdos_file_write(struct inode *inode,struct file *filp,char *buf,
    int count);


static struct file_operations msdos_file_operations = {
	NULL,			/* lseek - default */
	msdos_file_read,	/* read */
	msdos_file_write,	/* write */
	NULL,			/* readdir - bad */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open is needed */
	NULL			/* release */
};

struct inode_operations msdos_file_inode_operations = {
	&msdos_file_operations,	/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	msdos_bmap,		/* bmap */
	msdos_truncate		/* truncate */
};

/* No bmap for MS-DOS FS' that don't align data at kByte boundaries. */

struct inode_operations msdos_file_inode_operations_no_bmap = {
	&msdos_file_operations,	/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
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
	msdos_truncate		/* truncate */
};


static int msdos_file_read(struct inode *inode,struct file *filp,char *buf,
    int count)
{
	char *start;
	int left,offset,size,sector,cnt;
	char ch;
	struct buffer_head *bh;
	void *data;

/* printk("msdos_file_read\n"); */
	if (!inode) {
		printk("msdos_file_read: inode = NULL\n");
		return -EINVAL;
	}
	if (!S_ISREG(inode->i_mode)) {
		printk("msdos_file_read: mode = %07o\n",inode->i_mode);
		return -EINVAL;
	}
	if (filp->f_pos >= inode->i_size || count <= 0) return 0;
	start = buf;
	while (left = MIN(inode->i_size-filp->f_pos,count-(buf-start))) {
		if (!(sector = msdos_smap(inode,filp->f_pos >> SECTOR_BITS)))
			break;
		offset = filp->f_pos & (SECTOR_SIZE-1);
		if (!(bh = msdos_sread(inode->i_dev,sector,&data))) break;
		filp->f_pos += (size = MIN(SECTOR_SIZE-offset,left));
		if (MSDOS_I(inode)->i_binary) {
			memcpy_tofs(buf,data+offset,size);
			buf += size;
		}
		else for (cnt = size; cnt; cnt--) {
				if ((ch = *((char *) data+offset++)) == '\r')
					size--;
				else {
					if (ch != 26) put_fs_byte(ch,buf++);
					else {
						filp->f_pos = inode->i_size;
						brelse(bh);
						return buf-start;
					}
				}
			}
		brelse(bh);
	}
	if (start == buf) return -EIO;
	return buf-start;
}


static int msdos_file_write(struct inode *inode,struct file *filp,char *buf,
    int count)
{
	int sector,offset,size,left,written;
	int error,carry;
	char *start,*to,ch;
	struct buffer_head *bh;
	void *data;

	if (!inode) {
		printk("msdos_file_write: inode = NULL\n");
		return -EINVAL;
	}
	if (!S_ISREG(inode->i_mode)) {
		printk("msdos_file_write: mode = %07o\n",inode->i_mode);
		return -EINVAL;
	}
/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
	if (filp->f_flags & O_APPEND) filp->f_pos = inode->i_size;
	if (count <= 0) return 0;
	error = carry = 0;
	for (start = buf; count || carry; count -= size) {
		while (!(sector = msdos_smap(inode,filp->f_pos >> SECTOR_BITS)))
			if ((error = msdos_add_cluster(inode)) < 0) break;
		if (error) {
			msdos_truncate(inode);
			break;
		}
		offset = filp->f_pos & (SECTOR_SIZE-1);
		size = MIN(SECTOR_SIZE-offset,MAX(carry,count));
		if (!(bh = msdos_sread(inode->i_dev,sector,&data))) {
			error = -EIO;
			break;
		}
		if (MSDOS_I(inode)->i_binary) {
			memcpy_fromfs(data+(filp->f_pos & (SECTOR_SIZE-1)),
			    buf,written = size);
			buf += size;
		}
		else {
			written = left = SECTOR_SIZE-offset;
			to = data+(filp->f_pos & (SECTOR_SIZE-1));
			if (carry) {
				*to++ = '\n';
				left--;
				carry = 0;
			}
			for (size = 0; size < count && left; size++) {
				if ((ch = get_fs_byte(buf++)) == '\n') {
					*to++ = '\r';
					left--;
				}
				if (!left) carry = 1;
				else {
					*to++ = ch;
					left--;
				}
			}
			written -= left;
		}
		filp->f_pos += written;
		if (filp->f_pos > inode->i_size) {
			inode->i_size = filp->f_pos;
			inode->i_dirt = 1;
		}
		bh->b_dirt = 1;
		brelse(bh);
	}
	inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	MSDOS_I(inode)->i_attrs |= ATTR_ARCH;
	inode->i_dirt = 1;
	return start == buf ? error : buf-start;
}


void msdos_truncate(struct inode *inode)
{
	int cluster;

	cluster = SECTOR_SIZE*MSDOS_SB(inode->i_sb)->cluster_size;
	(void) fat_free(inode,(inode->i_size+(cluster-1))/cluster);
	MSDOS_I(inode)->i_attrs |= ATTR_ARCH;
	inode->i_dirt = 1;
}
