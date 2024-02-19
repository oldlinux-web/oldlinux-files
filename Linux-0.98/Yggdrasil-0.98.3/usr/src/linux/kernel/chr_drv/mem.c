/*
 *  linux/kernel/chr_drv/mem.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/mouse.h>

#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/string.h>

#include <asm/segment.h>
#include <asm/io.h>

static int read_ram(struct inode * inode, struct file * file,char * buf, int count)
{
	return -EIO;
}

static int write_ram(struct inode * inode, struct file * file,char * buf, int count)
{
	return -EIO;
}

static int read_core(struct inode * inode, struct file * file,char * buf, int count)
{
	unsigned long p = file->f_pos;
	int read;
	int count1;
	char * pnt;
	struct user dump;

	memset(&dump, 0, sizeof(struct user));
	dump.magic = CMAGIC;
	dump.u_dsize = high_memory >> 12;

	if (count < 0)
		return -EINVAL;
	if (p >= high_memory)
		return 0;
	if (count > high_memory - p)
		count = high_memory - p;
	read = 0;

	if (p < sizeof(struct user) && count > 0) {
		count1 = count;
		if (p + count1 > sizeof(struct user))
			count1 = sizeof(struct user)-p;
		pnt = (char *) &dump + p;
		memcpy_tofs(buf,(void *) pnt, count1);
		buf += count1;
		p += count1;
		count -= count1;
		read += count1;
	}

	while (p < (4096 + 4096) && count > 0) {
		put_fs_byte(0,buf);
		buf++;
		p++;
		count--;
		read++;
	}
	memcpy_tofs(buf,(void *) (p - 4096),count);
	read += count;
	file->f_pos += read;
	return read;
}

static int read_mem(struct inode * inode, struct file * file,char * buf, int count)
{
	unsigned long p = file->f_pos;
	int read;

	if (count < 0)
		return -EINVAL;
	if (p >= high_memory)
		return 0;
	if (count > high_memory - p)
		count = high_memory - p;
	read = 0;
	while (p < 4096 && count > 0) {
		put_fs_byte(0,buf);
		buf++;
		p++;
		count--;
		read++;
	}
	memcpy_tofs(buf,(void *) p,count);
	read += count;
	file->f_pos += read;
	return read;
}

static int write_mem(struct inode * inode, struct file * file,char * buf, int count)
{
	unsigned long p = file->f_pos;
	int written;

	if (count < 0)
		return -EINVAL;
	if (p >= high_memory)
		return 0;
	if (count > high_memory - p)
		count = high_memory - p;
	written = 0;
	while (p < 4096 && count > 0) {
		/* Hmm. Do something? */
		buf++;
		p++;
		count--;
		written++;
	}
	memcpy_fromfs((void *) p,buf,count);
	written += count;
	file->f_pos += written;
	return count;
}

static int read_port(struct inode * inode,struct file * file,char * buf, int count)
{
	unsigned int i = file->f_pos;
	char * tmp = buf;

	while (count-- > 0 && i < 65536) {
		put_fs_byte(inb(i),tmp);
		i++;
		tmp++;
	}
	file->f_pos = i;
	return tmp-buf;
}

static int write_port(struct inode * inode,struct file * file,char * buf, int count)
{
	unsigned int i = file->f_pos;
	char * tmp = buf;

	while (count-- > 0 && i < 65536) {
		outb(get_fs_byte(tmp),i);
		i++;
		tmp++;
	}
	file->f_pos = i;
	return tmp-buf;
}

static int read_null(struct inode * node,struct file * file,char * buf,int count)
{
	return 0;
}

static int write_null(struct inode * inode,struct file * file,char * buf, int count)
{
	return count;
}

static int read_zero(struct inode * node,struct file * file,char * buf,int count)
{
	int left;

	for (left = count; left > 0; left--) {
		put_fs_byte(0,buf);
		buf++;
	}
	return count;
}

static int write_zero(struct inode * inode,struct file * file,char * buf, int count)
{
	return count;
}

/*
 * The memory devices use the full 32 bits of the offset, and so we cannot
 * check against negative addresses: they are ok. The return value is weird,
 * though, in that case (0).
 *
 * also note that seeking relative to the "end of file" isn't supported:
 * it has no meaning, so it returns -EINVAL.
 */
static int memory_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
{
	switch (orig) {
		case 0:
			file->f_pos = offset;
			return file->f_pos;
		case 1:
			file->f_pos += offset;
			return file->f_pos;
		default:
			return -EINVAL;
	}
	if (file->f_pos < 0)
		return 0;
	return file->f_pos;
}

#define read_kmem read_mem
#define write_kmem write_mem

static struct file_operations ram_fops = {
	memory_lseek,
	read_ram,
	write_ram,
	NULL,		/* ram_readdir */
	NULL,		/* ram_select */
	NULL,		/* ram_ioctl */
	NULL,		/* ram_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations mem_fops = {
	memory_lseek,
	read_mem,
	write_mem,
	NULL,		/* mem_readdir */
	NULL,		/* mem_select */
	NULL,		/* mem_ioctl */
	NULL,		/* mem_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations kmem_fops = {
	memory_lseek,
	read_kmem,
	write_kmem,
	NULL,		/* kmem_readdir */
	NULL,		/* kmem_select */
	NULL,		/* kmem_ioctl */
	NULL,		/* kmem_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations null_fops = {
	memory_lseek,
	read_null,
	write_null,
	NULL,		/* null_readdir */
	NULL,		/* null_select */
	NULL,		/* null_ioctl */
	NULL,		/* null_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations port_fops = {
	memory_lseek,
	read_port,
	write_port,
	NULL,		/* port_readdir */
	NULL,		/* port_select */
	NULL,		/* port_ioctl */
	NULL,		/* port_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations zero_fops = {
	memory_lseek,
	read_zero,
	write_zero,
	NULL,		/* zero_readdir */
	NULL,		/* zero_select */
	NULL,		/* zero_ioctl */
	NULL,		/* zero_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static struct file_operations core_fops = {
	memory_lseek,
	read_core,
	NULL,
	NULL,		/* zero_readdir */
	NULL,		/* zero_select */
	NULL,		/* zero_ioctl */
	NULL,		/* zero_mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

static int memory_open(struct inode * inode, struct file * filp)
{
	switch (MINOR(inode->i_rdev)) {
		case 0:
			filp->f_op = &ram_fops;
			break;
		case 1:
			filp->f_op = &mem_fops;
			break;
		case 2:
			filp->f_op = &kmem_fops;
			break;
		case 3:
			filp->f_op = &null_fops;
			break;
		case 4:
			filp->f_op = &port_fops;
			break;
		case 5:
			filp->f_op = &zero_fops;
			break;
		case 6:
			filp->f_op = &core_fops;
			break;
		default:
			return -ENODEV;
	}
	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode,filp);
	return 0;
}

static struct file_operations memory_fops = {
	NULL,		/* lseek */
	NULL,		/* read */
	NULL,		/* write */
	NULL,		/* readdir */
	NULL,		/* select */
	NULL,		/* ioctl */
	NULL,		/* mmap */
	memory_open,	/* just a selector for the real open */
	NULL		/* release */
};

long chr_dev_init(long mem_start, long mem_end)
{
	chrdev_fops[1] = &memory_fops;
	mem_start = tty_init(mem_start);
	mem_start = lp_init(mem_start);
	mem_start = mouse_init(mem_start);
	return mem_start;
}
