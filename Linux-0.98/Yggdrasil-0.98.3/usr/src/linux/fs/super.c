/*
 *  linux/fs/super.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * super.c contains code to handle the super-block tables.
 */
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/proc_fs.h>
#include <linux/ext_fs.h>
#include <linux/msdos_fs.h>
#include <linux/iso_fs.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/locks.h>

#include <asm/system.h>
#include <asm/segment.h>

void wait_for_keypress(void);
void fcntl_init_locks(void);

/* set_bit uses setb, as gas doesn't recognize setc */
#define set_bit(bitnr,addr) ({ \
register int __res __asm__("ax"); \
__asm__("bt %2,%3;setb %%al":"=a" (__res):"a" (0),"r" (bitnr),"m" (*(addr))); \
__res; })

struct super_block super_block[NR_SUPER];
/* this is initialized in init/main.c */
dev_t ROOT_DEV = 0;

/* Move into include file later */

static struct file_system_type file_systems[] = {
	{minix_read_super,"minix"},
	{ext_read_super,"ext"},
	{msdos_read_super,"msdos"},
	{isofs_read_super,"iso9660"},
	{proc_read_super,"proc"},
	{NULL,NULL}
};

/* end of include file */

struct file_system_type *get_fs_type(char *name)
{
	int a;
	
	for(a = 0 ; file_systems[a].read_super ; a++)
		if (!strcmp(name,file_systems[a].name))
			return(&file_systems[a]);
	return(NULL);
}

void __wait_on_super(struct super_block * sb)
{
	add_wait_queue(&sb->s_wait,&current->wait);
repeat:
	current->state = TASK_UNINTERRUPTIBLE;
	if (sb->s_lock) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&sb->s_wait,&current->wait);
	current->state = TASK_RUNNING;
}

void sync_supers(dev_t dev)
{
	struct super_block * sb;

	for (sb = super_block + 0 ; sb < super_block + NR_SUPER ; sb++) {
		if (!sb->s_dev)
			continue;
		wait_on_super(sb);
		if (!sb->s_dev || !sb->s_dirt)
			continue;
		if (dev && (dev != sb->s_dev))
			continue;
		if (sb->s_op && sb->s_op->write_super)
			sb->s_op->write_super(sb);
	}
}

static struct super_block * get_super(dev_t dev)
{
	struct super_block * s;

	if (!dev)
		return NULL;
	s = 0+super_block;
	while (s < NR_SUPER+super_block)
		if (s->s_dev == dev) {
			wait_on_super(s);
			if (s->s_dev == dev)
				return s;
			s = 0+super_block;
		} else
			s++;
	return NULL;
}

void put_super(dev_t dev)
{
	struct super_block * sb;

	if (dev == ROOT_DEV) {
		printk("root diskette changed: prepare for armageddon\n\r");
		return;
	}
	if (!(sb = get_super(dev)))
		return;
	if (sb->s_covered) {
		printk("Mounted disk changed - tssk, tssk\n\r");
		return;
	}
	if (sb->s_op && sb->s_op->put_super)
		sb->s_op->put_super(sb);
}

static struct super_block * read_super(dev_t dev,char *name,int flags,void *data)
{
	struct super_block * s;
	struct file_system_type *type;

	if (!dev)
		return NULL;
	check_disk_change(dev);
	if (s = get_super(dev))
		return s;
	if (!(type = get_fs_type(name))) {
		printk("get fs type failed %s\n",name);
		return NULL;
	}
	for (s = 0+super_block ;; s++) {
		if (s >= NR_SUPER+super_block)
			return NULL;
		if (!s->s_dev)
			break;
	}
	s->s_dev = dev;
	s->s_flags = flags;
	if (!type->read_super(s,data)) {
		s->s_dev = 0;
		return NULL;
	}
	s->s_dev = dev;
	s->s_covered = NULL;
	s->s_rd_only = 0;
	s->s_dirt = 0;
	return s;
}

static int do_umount(dev_t dev)
{
	struct super_block * sb;

	if (dev==ROOT_DEV)
		return -EBUSY;
	if (!(sb=get_super(dev)) || !(sb->s_covered))
		return -ENOENT;
	if (!sb->s_covered->i_mount)
		printk("Mounted inode has i_mount=0\n");
	if (!fs_may_umount(dev, sb->s_mounted))
		return -EBUSY;
	sb->s_covered->i_mount=0;
	iput(sb->s_covered);
	sb->s_covered = NULL;
	iput(sb->s_mounted);
	sb->s_mounted = NULL;
	if (sb->s_op && sb->s_op->write_super && sb->s_dirt)
		sb->s_op->write_super (sb);
	put_super(dev);
	return 0;
}

int sys_umount(char * dev_name)
{
	struct inode * inode;
	int dev,retval;

	if (!suser())
		return -EPERM;
	retval = namei(dev_name,&inode);
	if (retval)
		return retval;
	dev = inode->i_rdev;
	if (!S_ISBLK(inode->i_mode)) {
		iput(inode);
		return -ENOTBLK;
	}
	if (IS_NODEV(inode)) {
		iput(inode);
		return -EACCES;
	}
	if (MAJOR(dev) >= MAX_BLKDEV) {
		iput(inode);
		return -ENODEV;
	}
	retval = do_umount(dev);
	if (!retval && blkdev_fops[MAJOR(dev)] && blkdev_fops[MAJOR(dev)]->release)
		blkdev_fops[MAJOR(dev)]->release(inode,NULL);
	iput(inode);
	if (retval)
		return retval;
	sync_dev(dev);
	return 0;
}

/*
 * do_mount() does the actual mounting after sys_mount has done the ugly
 * parameter parsing. When enough time has gone by, and everything uses the
 * new mount() parameters, sys_mount() can then be cleaned up.
 *
 * We cannot mount a filesystem if it has active, used, or dirty inodes.
 * We also have to flush all inode-data for this device, as the new mount
 * might need new info.
 */
static int do_mount(dev_t dev, const char * dir, char * type, int flags, void * data)
{
	struct inode * dir_i;
	struct super_block * sb;
	int error;

	error = namei(dir,&dir_i);
	if (error)
		return error;
	if (dir_i->i_count != 1 || dir_i->i_mount) {
		iput(dir_i);
		return -EBUSY;
	}
	if (!S_ISDIR(dir_i->i_mode)) {
		iput(dir_i);
		return -EPERM;
	}
	if (!fs_may_mount(dev)) {
		iput(dir_i);
		return -EBUSY;
	}
	sb = read_super(dev,type,flags,data);
	if (!sb || sb->s_covered) {
		iput(dir_i);
		return -EBUSY;
	}
	sb->s_flags = flags;
	sb->s_covered = dir_i;
	dir_i->i_mount = 1;
	return 0;		/* we don't iput(dir_i) - see umount */
}

/*
 * Flags is a 16-bit value that allows up to 16 non-fs dependent flags to
 * be given to the mount() call (ie: read-only, no-dev, no-suid etc).
 *
 * data is a (void *) that can point to any structure up to 4095 bytes, which
 * can contain arbitrary fs-dependent information (or be NULL).
 *
 * NOTE! As old versions of mount() didn't use this setup, the flags has to have
 * a special 16-bit magic number in the hight word: 0xC0ED. If this magic word
 * isn't present, the flags and data info isn't used, as the syscall assumes we
 * are talking to an older version that didn't understand them.
 */
int sys_mount(char * dev_name, char * dir_name, char * type,
	unsigned long new_flags, void *data)
{
	struct inode * inode;
	struct file_operations * fops;
	int dev;
	int retval;
	char tmp[100],*t;
	int i;
	unsigned long flags = 0;
	unsigned long page = 0;

	if (!suser())
		return -EPERM;
	if (retval = namei(dev_name,&inode))
		return retval;
	dev = inode->i_rdev;
	if (!S_ISBLK(inode->i_mode)) {
		iput(inode);
		return -ENOTBLK;
	}
	if (IS_NODEV(inode)) {
		iput(inode);
		return -EACCES;
	}
	if (MAJOR(dev) >= MAX_BLKDEV) {
		iput(inode);
		return -ENODEV;
	}
	fops = blkdev_fops[MAJOR(dev)];
	if (fops && fops->open) {
		if (retval = fops->open(inode,NULL)) {
			iput(inode);
			return retval;
		}
	}
	if ((new_flags & 0xffff0000) == 0xC0ED0000) {
		flags = new_flags & 0xffff;
		if (data && (unsigned long) data < TASK_SIZE)
			page = get_free_page(GFP_KERNEL);
	}
	if (page) {
		i = TASK_SIZE - (unsigned long) data;
		if (i < 0 || i > 4095)
			i = 4095;
		memcpy_fromfs((void *) page,data,i);
	}
	if (type) {
		for (i = 0 ; i < 100 ; i++)
			if (!(tmp[i] = get_fs_byte(type++)))
				break;
		t = tmp;
	} else
		t = "minix";
	retval = do_mount(dev,dir_name,t,flags,(void *) page);
	free_page(page);
	if (retval && fops && fops->release)
		fops->release(inode,NULL);
	iput(inode);
	return retval;
}

void mount_root(void)
{
	int i;
	struct file_system_type * fs_type = file_systems;
	struct super_block * p;
	struct inode * mi;

	if (32 != sizeof (struct minix_inode))
		panic("bad i-node size");
	for(i=0;i<NR_FILE;i++)
		file_table[i].f_count=0;
	fcntl_init_locks();
	if (MAJOR(ROOT_DEV) == 2) {
		printk("Insert root floppy and press ENTER");
		wait_for_keypress();
	}
	for(p = &super_block[0] ; p < &super_block[NR_SUPER] ; p++) {
		p->s_dev = 0;
		p->s_blocksize = 0;
		p->s_lock = 0;
		p->s_wait = NULL;
		p->s_mounted = p->s_covered = NULL;
	}
	while (fs_type->read_super && fs_type->name) {
		p = read_super(ROOT_DEV,fs_type->name,0,NULL);
		if (p) {
			mi = p->s_mounted;
			mi->i_count += 3 ;	/* NOTE! it is logically used 4 times, not 1 */
			p->s_covered = mi;
			p->s_flags = 0;
			current->pwd = mi;
			current->root = mi;
			return;
		}
		fs_type++;
	}
	panic("Unable to mount root");
}

/* XXX AJR added lock_super and free_super for isofs */
void lock_super(struct super_block * sb)
{
	cli();
	while (sb->s_lock)
		sleep_on(&(sb->s_wait));
	sb->s_lock = 1;
	sti();
}

void free_super(struct super_block * sb)
{
	sb->s_lock = 0;
	wake_up(&(sb->s_wait));
}

