/*
 *  linux/fs/msdos/inode.c
 *
 *  Written 1992 by Werner Almesberger
 */

#include <linux/msdos_fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/stat.h>
#include <linux/locks.h>

#include <asm/segment.h>

void msdos_put_inode(struct inode *inode)
{
	struct inode *depend;

	if (inode->i_nlink)
		return;
	inode->i_size = 0;
	msdos_truncate(inode);
	depend = MSDOS_I(inode)->i_depend;
	memset(inode,0,sizeof(struct inode));
	if (depend) {
		if (MSDOS_I(depend)->i_old != inode) {
			printk("Invalid link (0x%X): expected 0x%X, got "
			    "0x%X\n",(int) depend,(int) inode,(int)
			    MSDOS_I(depend)->i_old);
			panic("That's fatal");
		}
		MSDOS_I(depend)->i_old = NULL;
		iput(depend);
	}
}


void msdos_put_super(struct super_block *sb)
{
	cache_inval_dev(sb->s_dev);
	lock_super(sb);
	sb->s_dev = 0;
	unlock_super(sb);
	return;
}


static struct super_operations msdos_sops = { 
	msdos_read_inode,
	msdos_write_inode,
	msdos_put_inode,
	msdos_put_super,
	NULL, /* added in 0.96c */
	msdos_statfs
};


static int parse_options(char *options,char *check,char *conversion,uid_t *uid, gid_t *gid, int *umask)
{
	char *this,*value;

	*check = 'n';
	*conversion = 'b';
	*uid = current->uid;
	*gid = current->gid;
	*umask = current->umask;
	if (!options) return 1;
	for (this = strtok(options,","); this; this = strtok(NULL,",")) {
		if (value = strchr(this,'=')) *value++ = 0;
		if (!strcmp(this,"check") && value) {
			if (value[0] && !value[1] && strchr("rns",*value))
				*check = *value;
			else if (!strcmp(value,"relaxed")) *check = 'r';
			else if (!strcmp(value,"normal")) *check = 'n';
			else if (!strcmp(value,"strict")) *check = 's';
			else return 0;
		}
		else if (!strcmp(this,"conv") && value) {
			if (value[0] && !value[1] && strchr("bta",*value))
				*conversion = *value;
			else if (!strcmp(value,"binary")) *conversion = 'b';
			else if (!strcmp(value,"text")) *conversion = 't';
			else if (!strcmp(value,"auto")) *conversion = 'a';
			else return 0;
		}
		else if (!strcmp(this,"uid")) {
			if (!value || !*value)
				return 0;
			*uid = simple_strtoul(value,&value,0);
			if (*value)
				return 0;
		}
		else if (!strcmp(this,"gid")) {
			if (!value || !*value)
				return 0;
			*gid = simple_strtoul(value,&value,0);
			if (*value)
				return 0;
		}
		else if (!strcmp(this,"umask")) {
			if (!value || !*value)
				return 0;
			*umask = simple_strtoul(value,&value,8);
			if (*value)
				return 0;
		}
		else return 0;
	}
	return 1;
}


/* Read the super block of an MS-DOS FS. */

struct super_block *msdos_read_super(struct super_block *s,void *data)
{
	struct buffer_head *bh;
	struct msdos_boot_sector *b;
	int data_sectors;
	char check,conversion;
	uid_t uid;
	gid_t gid;
	int umask;

	if (!parse_options((char *) data,&check,&conversion,&uid,&gid,&umask)) {
		s->s_dev = 0;
		return NULL;
	}
	cache_init();
	lock_super(s);
	bh = bread(s->s_dev, 0, BLOCK_SIZE);
	unlock_super(s);
	if (bh == NULL) {
		s->s_dev = 0;
		printk("MSDOS bread failed\n");
		return NULL;
	}
	b = (struct msdos_boot_sector *) bh->b_data;
	s->s_blocksize = 1024;	/* we cannot handle anything else yet */
	MSDOS_SB(s)->cluster_size = b->cluster_size;
	MSDOS_SB(s)->fats = b->fats;
	MSDOS_SB(s)->fat_start = b->reserved;
	MSDOS_SB(s)->fat_length = b->fat_length;
	MSDOS_SB(s)->dir_start = b->reserved+b->fats*b->fat_length;
	MSDOS_SB(s)->dir_entries = *((unsigned short *) &b->dir_entries);
	MSDOS_SB(s)->data_start = MSDOS_SB(s)->dir_start+((MSDOS_SB(s)->
	    dir_entries << 5) >> 9);
	data_sectors = (*((unsigned short *) &b->sectors) ? *((unsigned short *)
	    &b->sectors) : b->total_sect)-MSDOS_SB(s)->data_start;
	MSDOS_SB(s)->clusters = b->cluster_size ? data_sectors/b->cluster_size :
	    0;
	MSDOS_SB(s)->fat_bits = MSDOS_SB(s)->clusters > MSDOS_FAT12 ? 16 : 12;
	brelse(bh);
printk("[MS-DOS FS Rel. alpha.8, FAT %d, check=%c, conv=%c, uid=%d, gid=%d, umask=%03o]\n",
  MSDOS_SB(s)->fat_bits,check,conversion,uid,gid,umask);
printk("[me=0x%x,cs=%d,#f=%d,fs=%d,fl=%d,ds=%d,de=%d,data=%d,se=%d,ts=%d]\n",
  b->media,MSDOS_SB(s)->cluster_size,MSDOS_SB(s)->fats,MSDOS_SB(s)->fat_start,
  MSDOS_SB(s)->fat_length,MSDOS_SB(s)->dir_start,MSDOS_SB(s)->dir_entries,
  MSDOS_SB(s)->data_start,*(unsigned short *) &b->sectors,b->total_sect);
	if (!MSDOS_SB(s)->fats || (MSDOS_SB(s)->dir_entries & (MSDOS_DPS-1))
	    || !b->cluster_size || MSDOS_SB(s)->clusters+2 > MSDOS_SB(s)->
		fat_length*SECTOR_SIZE*8/MSDOS_SB(s)->fat_bits) {
		s->s_dev = 0;
		printk("Unsupported FS parameters\n");
		return NULL;
	}
	if (!MSDOS_CAN_BMAP(MSDOS_SB(s))) printk("No bmap support\n");
	s->s_magic = MSDOS_SUPER_MAGIC;
	MSDOS_SB(s)->name_check = check;
	MSDOS_SB(s)->conversion = conversion;
	/* set up enough so that it can read an inode */
	s->s_op = &msdos_sops;
	MSDOS_SB(s)->fs_uid = uid;
	MSDOS_SB(s)->fs_gid = gid;
	MSDOS_SB(s)->fs_umask = umask;
	MSDOS_SB(s)->free_clusters = -1; /* don't know yet */
	MSDOS_SB(s)->fat_wait = NULL;
	MSDOS_SB(s)->fat_lock = 0;
	if (!(s->s_mounted = iget(s,MSDOS_ROOT_INO))) {
		s->s_dev = 0;
		printk("get root inode failed\n");
		return NULL;
	}
	return s;
}


void msdos_statfs(struct super_block *sb,struct statfs *buf)
{
	int free,this;

	put_fs_long(sb->s_magic,&buf->f_type);
	put_fs_long(MSDOS_SB(sb)->cluster_size*SECTOR_SIZE,&buf->f_bsize);
	put_fs_long(MSDOS_SB(sb)->clusters,&buf->f_blocks);
	lock_fat(sb);
	if (MSDOS_SB(sb)->free_clusters != -1)
		free = MSDOS_SB(sb)->free_clusters;
	else {
		free = 0;
		for (this = 2; this < MSDOS_SB(sb)->clusters+2; this++)
			if (!fat_access(sb,this,-1)) free++;
		MSDOS_SB(sb)->free_clusters = free;
	}
	unlock_fat(sb);
	put_fs_long(free,&buf->f_bfree);
	put_fs_long(free,&buf->f_bavail);
	put_fs_long(0,&buf->f_files);
	put_fs_long(0,&buf->f_ffree);
}


int msdos_bmap(struct inode *inode,int block)
{
	struct msdos_sb_info *sb;
	int cluster,offset;

	sb = MSDOS_SB(inode->i_sb);
	if ((sb->cluster_size & 1) || (sb->data_start & 1)) return 0;
	if (inode->i_ino == MSDOS_ROOT_INO) {
		if (sb->dir_start & 1) return 0;
		return (sb->dir_start >> 1)+block;
	}
	cluster = (block*2)/sb->cluster_size;
	offset = (block*2) % sb->cluster_size;
	if (!(cluster = get_cluster(inode,cluster))) return 0;
	return ((cluster-2)*sb->cluster_size+sb->data_start+offset) >> 1;
}


void msdos_read_inode(struct inode *inode)
{
	struct buffer_head *bh;
	struct msdos_dir_entry *raw_entry;
	int this;

/* printk("read inode %d\n",inode->i_ino); */
	MSDOS_I(inode)->i_busy = 0;
	MSDOS_I(inode)->i_depend = MSDOS_I(inode)->i_old = NULL;
	MSDOS_I(inode)->i_binary = 1;
	inode->i_uid = MSDOS_SB(inode->i_sb)->fs_uid;
	inode->i_gid = MSDOS_SB(inode->i_sb)->fs_gid;
	if (inode->i_ino == MSDOS_ROOT_INO) {
		inode->i_mode = (0777 & ~MSDOS_SB(inode->i_sb)->fs_umask) |
		    S_IFDIR;
		inode->i_op = &msdos_dir_inode_operations;
		inode->i_nlink = msdos_subdirs(inode)+2;
		    /* subdirs (neither . nor ..) plus . and "self" */
		inode->i_size = MSDOS_SB(inode->i_sb)->dir_entries*
		    sizeof(struct msdos_dir_entry);
		inode->i_blksize = MSDOS_SB(inode->i_sb)->cluster_size*
		    SECTOR_SIZE;
		inode->i_blocks = (inode->i_size+inode->i_blksize-1)/
		    inode->i_blksize;
		MSDOS_I(inode)->i_start = 0;
		MSDOS_I(inode)->i_attrs = 0;
		inode->i_mtime = inode->i_atime = inode->i_ctime = 0;
		return;
	}
	if (!(bh = bread(inode->i_dev,inode->i_ino >> MSDOS_DPB_BITS, BLOCK_SIZE)))
	    panic("unable to read i-node block");
	raw_entry = &((struct msdos_dir_entry *) (bh->b_data))
	    [inode->i_ino & (MSDOS_DPB-1)];
	if ((raw_entry->attr & ATTR_DIR) && *raw_entry->name && *(unsigned char *)
            raw_entry->name != DELETED_FLAG) {
		inode->i_mode = MSDOS_MKMODE(raw_entry->attr,0777 &
		    ~MSDOS_SB(inode->i_sb)->fs_umask) | S_IFDIR;
		inode->i_op = &msdos_dir_inode_operations;
		MSDOS_I(inode)->i_start = raw_entry->start;
		inode->i_nlink = msdos_subdirs(inode);
		    /* includes .., compensating for "self" */
#ifdef DEBUG
		if (!inode->i_nlink) {
			printk("directory %d: i_nlink == 0\n",inode->i_ino);
			inode->i_nlink = 1;
		}
#endif
		inode->i_size = 0;
		if (this = raw_entry->start)
			while (this != -1) {
				inode->i_size += SECTOR_SIZE*MSDOS_SB(inode->
				    i_sb)->cluster_size;
				if (!(this = fat_access(inode->i_sb,this,-1)))
					printk("Directory %d: bad FAT\n",
					    inode->i_ino);
			}
	}
	else {
		inode->i_mode = MSDOS_MKMODE(raw_entry->attr,0666 &
		    ~MSDOS_SB(inode->i_sb)->fs_umask) | S_IFREG;
		inode->i_op = MSDOS_CAN_BMAP(MSDOS_SB(inode->i_sb)) ? 
		    &msdos_file_inode_operations :
		    &msdos_file_inode_operations_no_bmap;
		MSDOS_I(inode)->i_start = raw_entry->start;
		inode->i_nlink = 1;
		inode->i_size = raw_entry->size;
	}
	MSDOS_I(inode)->i_binary = is_binary(MSDOS_SB(inode->i_sb)->conversion,
	    raw_entry->ext);
	MSDOS_I(inode)->i_attrs = raw_entry->attr & ATTR_UNUSED;
	/* this is as close to the truth as we can get ... */
	inode->i_blksize = MSDOS_SB(inode->i_sb)->cluster_size*SECTOR_SIZE;
	inode->i_blocks = (inode->i_size+inode->i_blksize-1)/
	    inode->i_blksize;
	inode->i_mtime = inode->i_atime = inode->i_ctime =
	    date_dos2unix(raw_entry->time,raw_entry->date);
	brelse(bh);
}


void msdos_write_inode(struct inode *inode)
{
	struct buffer_head *bh;
	struct msdos_dir_entry *raw_entry;

	inode->i_dirt = 0;
	if (inode->i_ino == MSDOS_ROOT_INO || !inode->i_nlink) return;
	if (!(bh = bread(inode->i_dev,inode->i_ino >> MSDOS_DPB_BITS, BLOCK_SIZE)))
	    panic("unable to read i-node block");
	raw_entry = &((struct msdos_dir_entry *) (bh->b_data))
	    [inode->i_ino & (MSDOS_DPB-1)];
	if (S_ISDIR(inode->i_mode)) {
		raw_entry->attr = ATTR_DIR;
		raw_entry->size = 0;
	}
	else {
		raw_entry->attr = ATTR_NONE;
		raw_entry->size = inode->i_size;
	}
	raw_entry->attr |= MSDOS_MKATTR(inode->i_mode) |
	    MSDOS_I(inode)->i_attrs;
	raw_entry->start = MSDOS_I(inode)->i_start;
	date_unix2dos(inode->i_mtime,&raw_entry->time,&raw_entry->date);
	bh->b_dirt = 1;
	brelse(bh);
}
