#ifndef _LINUX_FS_H
#define _LINUX_FS_H

/*
 * This file has definitions for some important file table
 * structures etc.
 */

#include <linux/limits.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/dirent.h>
#include <linux/vfs.h>

/* devices are as follows: (same as minix, so we can use the minix
 * file system. These are major numbers.)
 *
 *  0 - unused (nodev)
 *  1 - /dev/mem
 *  2 - /dev/fd
 *  3 - /dev/hd
 *  4 - /dev/ttyx
 *  5 - /dev/tty
 *  6 - /dev/lp
 *  7 -
 *  8 - /dev/sd
 *  9 - /dev/st
 * 10 - mice
 * 11 - scsi cdrom
 */

#define MAY_EXEC 1
#define MAY_WRITE 2
#define MAY_READ 4

#define READ 0
#define WRITE 1
#define READA 2		/* read-ahead - don't pause */
#define WRITEA 3	/* "write-ahead" - silly, but somewhat useful */

extern void buffer_init(void);
extern void inode_init(void);

#define MAJOR(a) (((unsigned)(a))>>8)
#define MINOR(a) ((a)&0xff)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define NIL_FILP	((struct file *)0)
#define SEL_IN		1
#define SEL_OUT		2
#define SEL_EX		4

/*
 * These are the fs-independent mount-flags: up to 16 flags are supported
 */
#define MS_RDONLY    1 /* mount read-only */
#define MS_NOSUID    2 /* ignore suid and sgid bits */
#define MS_NODEV     4 /* disallow access to device special files */
#define MS_NOEXEC    8 /* disallow program execution */
#define MS_SYNC     16 /* writes are synced at once */

/*
 * Note that read-only etc flags are inode-specific: setting some file-system
 * flags just means all the inodes inherit those flags by default. It might be
 * possible to overrride it sevelctively if you really wanted to with some
 * ioctl() that is not currently implemented.
 */
#define IS_RDONLY(inode) ((inode)->i_flags & MS_RDONLY)
#define IS_NOSUID(inode) ((inode)->i_flags & MS_NOSUID)
#define IS_NODEV(inode) ((inode)->i_flags & MS_NODEV)
#define IS_NOEXEC(inode) ((inode)->i_flags & MS_NOEXEC)
#define IS_SYNC(inode) ((inode)->i_flags & MS_SYNC)

/* the read-only stuff doesn't really belong here, but any other place is
   probably as bad and I don't want to create yet another include file. */

#define BLKROSET 4701 /* set device read-only (0 = read-write) */
#define BLKROGET 4702 /* get read-only status (0 = read_write) */
#define BLKRRPART 4703 /* re-read partition table */

#define BMAP_IOCTL 1	/* obsolete - kept for compatibility */
#define FIBMAP	   1	/* bmap access */
#define FIGETBSZ   2	/* get the block size used for bmap */

typedef char buffer_block[BLOCK_SIZE];

struct buffer_head {
	char * b_data;			/* pointer to data block (1024 bytes) */
	unsigned long b_size;		/* block size */
	unsigned long b_blocknr;	/* block number */
	dev_t b_dev;			/* device (0 = free) */
	unsigned short b_count;		/* users using this block */
	unsigned char b_uptodate;
	unsigned char b_dirt;		/* 0-clean,1-dirty */
	unsigned char b_lock;		/* 0 - ok, 1 -locked */
	struct wait_queue * b_wait;
	struct buffer_head * b_prev;		/* doubly linked list of hash-queue */
	struct buffer_head * b_next;
	struct buffer_head * b_prev_free;	/* doubly linked list of buffers */
	struct buffer_head * b_next_free;
	struct buffer_head * b_this_page;	/* circular list of buffers in one page */
	struct buffer_head * b_reqnext;		/* request queue */
};

#include <linux/pipe_fs_i.h>
#include <linux/minix_fs_i.h>
#include <linux/ext_fs_i.h>
#include <linux/msdos_fs_i.h>
#include <linux/iso_fs_i.h>

struct inode {
	dev_t		i_dev;
	unsigned long	i_ino;
	umode_t		i_mode;
	nlink_t		i_nlink;
	uid_t		i_uid;
	gid_t		i_gid;
	dev_t		i_rdev;
	off_t		i_size;
	time_t		i_atime;
	time_t		i_mtime;
	time_t		i_ctime;
	unsigned long	i_blksize;
	unsigned long	i_blocks;
	struct inode_operations * i_op;
	struct super_block * i_sb;
	struct wait_queue * i_wait;
	struct file_lock * i_flock;
	struct vm_area_struct * i_mmap;
	unsigned short i_count;
	unsigned short i_flags;
	unsigned char i_lock;
	unsigned char i_dirt;
	unsigned char i_pipe;
	unsigned char i_mount;
	unsigned char i_seek;
	unsigned char i_update;
	union {
		struct pipe_inode_info pipe_i;
		struct minix_inode_info minix_i;
		struct ext_inode_info ext_i;
		struct msdos_inode_info msdos_i;
		struct iso_inode_info isofs_i;
	} u;
};

struct file {
	mode_t f_mode;
	dev_t f_rdev;			/* needed for /dev/tty */
	off_t f_pos;
	unsigned short f_flags;
	unsigned short f_count;
	unsigned short f_reada;
	struct inode * f_inode;
	struct file_operations * f_op;
};

struct file_lock {
	struct file_lock *fl_next;	/* singly linked list */
	struct task_struct *fl_owner;	/* NULL if on free list, for sanity checks */
	struct wait_queue *fl_wait;
	char fl_type;
	char fl_whence;
	off_t fl_start;
	off_t fl_end;
};

#include <linux/minix_fs_sb.h>
#include <linux/ext_fs_sb.h>
#include <linux/msdos_fs_sb.h>
#include <linux/iso_fs_sb.h>

struct super_block {
	dev_t s_dev;
	unsigned long s_blocksize;
	unsigned char s_lock;
	unsigned char s_rd_only;
	unsigned char s_dirt;
	struct super_operations *s_op;
	unsigned long s_flags;
	unsigned long s_magic;
	unsigned long s_time;
	struct inode * s_covered;
	struct inode * s_mounted;
	struct wait_queue * s_wait;
	union {
		struct minix_sb_info minix_sb;
		struct ext_sb_info ext_sb;
		struct msdos_sb_info msdos_sb;
		struct isofs_sb_info isofs_sb;
	} u;
};

struct file_operations {
	int (*lseek) (struct inode *, struct file *, off_t, int);
	int (*read) (struct inode *, struct file *, char *, int);
	int (*write) (struct inode *, struct file *, char *, int);
	int (*readdir) (struct inode *, struct file *, struct dirent *, int count);
	int (*select) (struct inode *, struct file *, int, select_table *);
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned int);
	int (*mmap) (void);
	int (*open) (struct inode *, struct file *);
	void (*release) (struct inode *, struct file *);
};

struct inode_operations {
	struct file_operations * default_file_ops;
	int (*create) (struct inode *,const char *,int,int,struct inode **);
	int (*lookup) (struct inode *,const char *,int,struct inode **);
	int (*link) (struct inode *,struct inode *,const char *,int);
	int (*unlink) (struct inode *,const char *,int);
	int (*symlink) (struct inode *,const char *,int,const char *);
	int (*mkdir) (struct inode *,const char *,int,int);
	int (*rmdir) (struct inode *,const char *,int);
	int (*mknod) (struct inode *,const char *,int,int,int);
	int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
	int (*readlink) (struct inode *,char *,int);
	int (*follow_link) (struct inode *, struct inode *, int flag, int mode, struct inode ** res_inode);
	int (*bmap) (struct inode *,int);
	void (*truncate) (struct inode *);
};

struct super_operations {
	void (*read_inode)(struct inode *inode);
	void (*write_inode) (struct inode *inode);
	void (*put_inode) (struct inode *inode);
	void (*put_super)(struct super_block *sb);
	void (*write_super) (struct super_block *sb);
	void (*statfs) (struct super_block *sb, struct statfs *buf);
};

struct file_system_type {
	struct super_block *(*read_super)(struct super_block *sb,void *mode);
	char *name;
};

extern struct file_operations * chrdev_fops[MAX_CHRDEV];
extern struct file_operations * blkdev_fops[MAX_BLKDEV];

extern struct file_system_type *get_fs_type(char *name);

extern int fs_may_mount(dev_t dev);
extern int fs_may_umount(dev_t dev, struct inode * mount_root);

extern struct file file_table[NR_FILE];
extern struct super_block super_block[NR_SUPER];

extern void grow_buffers(int size);
extern int shrink_buffers(unsigned int priority);

extern int nr_buffers;
extern int nr_buffer_heads;

extern void check_disk_change(dev_t dev);
extern void invalidate_inodes(dev_t dev);
extern void invalidate_buffers(dev_t dev);
extern int floppy_change(struct buffer_head * first_block);
extern int ticks_to_floppy_on(unsigned int dev);
extern void floppy_on(unsigned int dev);
extern void floppy_off(unsigned int dev);
extern void sync_inodes(dev_t dev);
extern void sync_dev(dev_t dev);
extern void sync_supers(dev_t dev);
extern int bmap(struct inode * inode,int block);
extern int namei(const char * pathname, struct inode ** res_inode);
extern int lnamei(const char * pathname, struct inode ** res_inode);
extern int permission(struct inode * inode,int mask);
extern int open_namei(const char * pathname, int flag, int mode,
	struct inode ** res_inode, struct inode * base);
extern int do_mknod(const char * filename, int mode, dev_t dev);
extern void iput(struct inode * inode);
extern struct inode * iget(struct super_block * sb,int nr);
extern struct inode * get_empty_inode(void);
extern struct inode * get_pipe_inode(void);
extern struct file * get_empty_filp(void);
extern struct buffer_head * get_hash_table(dev_t dev, int block, int size);
extern struct buffer_head * getblk(dev_t dev, int block, int size);
extern void ll_rw_block(int rw, struct buffer_head * bh);
extern void ll_rw_page(int rw, int dev, int nr, char * buffer);
extern void ll_rw_swap_file(int rw, int dev, unsigned int *b, int nb, char *buffer);
extern void brelse(struct buffer_head * buf);
extern struct buffer_head * bread(dev_t dev, int block, int size);
extern void bread_page(unsigned long addr,dev_t dev,int b[4]);
extern struct buffer_head * breada(dev_t dev,int block,...);
extern void put_super(dev_t dev);
extern dev_t ROOT_DEV;

extern void mount_root(void);

extern int char_read(struct inode *, struct file *, char *, int);
extern int block_read(struct inode *, struct file *, char *, int);

extern int char_write(struct inode *, struct file *, char *, int);
extern int block_write(struct inode *, struct file *, char *, int);

#endif
