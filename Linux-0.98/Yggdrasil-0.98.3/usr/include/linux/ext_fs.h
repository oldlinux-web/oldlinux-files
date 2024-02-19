#ifndef _LINUX_EXT_FS_H
#define _LINUX_EXT_FS_H

/*
 * The ext filesystem constants/structures
 */

#define EXT_NAME_LEN 255
#define EXT_ROOT_INO 1

#define EXT_SUPER_MAGIC 0x137D

#define EXT_INODES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct ext_inode)))

struct ext_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned short i_gid;
	unsigned short i_nlinks;
	unsigned long i_zone[12];
};

struct ext_free_inode {
	unsigned long count;
	unsigned long free[14];
	unsigned long next;
};

struct ext_free_block {
	unsigned long count;
	unsigned long free[254];
	unsigned long next;
};

struct ext_super_block {
	unsigned long s_ninodes;
	unsigned long s_nzones;
	unsigned long s_firstfreeblock;
	unsigned long s_freeblockscount;
	unsigned long s_firstfreeinode;
	unsigned long s_freeinodescount;
	unsigned long s_firstdatazone;
	unsigned long s_log_zone_size;
	unsigned long s_max_size;
	unsigned long s_reserved1;
	unsigned long s_reserved2;
	unsigned long s_reserved3;
	unsigned long s_reserved4;
	unsigned long s_reserved5;
	unsigned short s_magic;
};

struct ext_dir_entry {
	unsigned long inode;
	unsigned short rec_len;
	unsigned short name_len;
	char name[EXT_NAME_LEN];
};

extern int ext_open(struct inode * inode, struct file * filp);
extern void ext_release(struct inode * inode, struct file * filp);
extern int ext_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result);
extern int ext_create(struct inode * dir,const char * name, int len, int mode,
	struct inode ** result);
extern int ext_mkdir(struct inode * dir, const char * name, int len, int mode);
extern int ext_rmdir(struct inode * dir, const char * name, int len);
extern int ext_unlink(struct inode * dir, const char * name, int len);
extern int ext_symlink(struct inode * inode, const char * name, int len,
	const char * symname);
extern int ext_link(struct inode * oldinode, struct inode * dir, const char * name, int len);
extern int ext_mknod(struct inode * dir, const char * name, int len, int mode, int rdev);
extern int ext_rename(struct inode * old_dir, const char * old_name, int old_len,
	struct inode * new_dir, const char * new_name, int new_len);
extern struct inode * ext_new_inode(struct super_block * sb);
extern void ext_free_inode(struct inode * inode);
extern unsigned long ext_count_free_inodes(struct super_block *sb);
extern int ext_new_block(struct super_block * sb);
extern void ext_free_block(struct super_block * sb, int block);
extern unsigned long ext_count_free_blocks(struct super_block *sb);

extern int ext_bmap(struct inode *,int);

extern struct buffer_head * ext_getblk(struct inode *, int, int);
extern struct buffer_head * ext_bread(struct inode *, int, int);

extern void ext_truncate(struct inode *);
extern void ext_put_super(struct super_block *);
extern void ext_write_super(struct super_block *);
extern struct super_block *ext_read_super(struct super_block *,void *);
extern void ext_read_inode(struct inode *);
extern void ext_write_inode(struct inode *);
extern void ext_put_inode(struct inode *);
extern void ext_statfs(struct super_block *, struct statfs *);

extern int ext_lseek(struct inode *, struct file *, off_t, int);
extern int ext_read(struct inode *, struct file *, char *, int);
extern int ext_write(struct inode *, struct file *, char *, int);

extern struct inode_operations ext_file_inode_operations;
extern struct inode_operations ext_dir_inode_operations;
extern struct inode_operations ext_symlink_inode_operations;
extern struct inode_operations ext_chrdev_inode_operations;
extern struct inode_operations ext_blkdev_inode_operations;
extern struct inode_operations ext_fifo_inode_operations;

extern struct file_operations ext_file_operations;
extern struct file_operations ext_dir_operations;

#endif
