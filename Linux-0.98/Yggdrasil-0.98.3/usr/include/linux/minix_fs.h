#ifndef _LINUX_MINIX_FS_H
#define _LINUX_MINIX_FS_H

/*
 * The minix filesystem constants/structures
 */

#define MINIX_NAME_LEN 14
#define MINIX_ROOT_INO 1

#define MINIX_I_MAP_SLOTS 8
#define MINIX_Z_MAP_SLOTS 8
#define MINIX_SUPER_MAGIC 0x137F

#define MINIX_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix_inode)))
#define MINIX_DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct minix_dir_entry)))

struct minix_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
};

/*
 * minix super-block data on disk
 */
struct minix_super_block {
	unsigned short s_ninodes;
	unsigned short s_nzones;
	unsigned short s_imap_blocks;
	unsigned short s_zmap_blocks;
	unsigned short s_firstdatazone;
	unsigned short s_log_zone_size;
	unsigned long s_max_size;
	unsigned short s_magic;
};

struct minix_dir_entry {
	unsigned short inode;
	char name[MINIX_NAME_LEN];
};

extern int minix_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result);
extern int minix_create(struct inode * dir,const char * name, int len, int mode,
	struct inode ** result);
extern int minix_mkdir(struct inode * dir, const char * name, int len, int mode);
extern int minix_rmdir(struct inode * dir, const char * name, int len);
extern int minix_unlink(struct inode * dir, const char * name, int len);
extern int minix_symlink(struct inode * inode, const char * name, int len,
	const char * symname);
extern int minix_link(struct inode * oldinode, struct inode * dir, const char * name, int len);
extern int minix_mknod(struct inode * dir, const char * name, int len, int mode, int rdev);
extern int minix_rename(struct inode * old_dir, const char * old_name, int old_len,
	struct inode * new_dir, const char * new_name, int new_len);
extern struct inode * minix_new_inode(struct super_block * sb);
extern void minix_free_inode(struct inode * inode);
extern unsigned long minix_count_free_inodes(struct super_block *sb);
extern int minix_new_block(struct super_block * sb);
extern void minix_free_block(struct super_block * sb, int block);
extern unsigned long minix_count_free_blocks(struct super_block *sb);

extern int minix_bmap(struct inode *,int);

extern struct buffer_head * minix_getblk(struct inode *, int, int);
extern struct buffer_head * minix_bread(struct inode *, int, int);

extern void minix_truncate(struct inode *);
extern void minix_put_super(struct super_block *);
extern struct super_block *minix_read_super(struct super_block *,void *);
extern void minix_read_inode(struct inode *);
extern void minix_write_inode(struct inode *);
extern void minix_put_inode(struct inode *);
extern void minix_statfs(struct super_block *, struct statfs *);

extern struct inode_operations minix_file_inode_operations;
extern struct inode_operations minix_dir_inode_operations;
extern struct inode_operations minix_symlink_inode_operations;
extern struct inode_operations minix_chrdev_inode_operations;
extern struct inode_operations minix_blkdev_inode_operations;
extern struct inode_operations minix_fifo_inode_operations;

extern struct file_operations minix_file_operations;
extern struct file_operations minix_dir_operations;

#endif
