/*
 *  linux/fs/proc/mem.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>

#include <asm/segment.h>
#include <asm/io.h>

static int mem_read(struct inode * inode, struct file * file,char * buf, int count)
{
	unsigned long addr, pid, cr3;
	char *tmp;
	unsigned long pde, pte, page;
	int i;

	if (count < 0)
		return -EINVAL;
	pid = inode->i_ino;
	pid >>= 16;
	cr3 = 0;
	for (i = 1 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid) {
			cr3 = task[i]->tss.cr3;
			break;
		}
	if (!cr3)
		return -EACCES;
	addr = file->f_pos;
	tmp = buf;
	while (count > 0) {
		if (current->signal & ~current->blocked)
			break;
		pde = cr3 + (addr >> 20 & 0xffc);
		pte = *(unsigned long *) pde;
		if (!(pte & PAGE_PRESENT))
			break;
		pte &= 0xfffff000;
		pte += (addr >> 10) & 0xffc;
		page = *(unsigned long *) pte;
		if (!(page & 1))
			break;
		page &= 0xfffff000;
		page += addr & 0xfff;
		i = 4096-(addr & 0xfff);
		if (i > count)
			i = count;
		memcpy_tofs(tmp,(void *) page,i);
		addr += i;
		tmp += i;
		count -= i;
	}
	file->f_pos = addr;
	return tmp-buf;
}

static int mem_write(struct inode * inode, struct file * file,char * buf, int count)
{
	unsigned long addr, pid, cr3;
	char *tmp;
	unsigned long pde, pte, page;
	int i;

	if (count < 0)
		return -EINVAL;
	addr = file->f_pos;
	pid = inode->i_ino;
	pid >>= 16;
	cr3 = 0;
	for (i = 1 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid) {
			cr3 = task[i]->tss.cr3;
			break;
		}
	if (!cr3)
		return -EACCES;
	tmp = buf;
	while (count > 0) {
		if (current->signal & ~current->blocked)
			break;
		pde = cr3 + (addr >> 20 & 0xffc);
		pte = *(unsigned long *) pde;
		if (!(pte & PAGE_PRESENT))
			break;
		pte &= 0xfffff000;
		pte += (addr >> 10) & 0xffc;
		page = *(unsigned long *) pte;
		if (!(page & PAGE_PRESENT))
			break;
		if (!(page & 2)) {
			do_wp_page(0,addr,current,0);
			continue;
		}
		page &= 0xfffff000;
		page += addr & 0xfff;
		i = 4096-(addr & 0xfff);
		if (i > count)
			i = count;
		memcpy_fromfs((void *) page,tmp,i);
		addr += i;
		tmp += i;
		count -= i;
	}
	file->f_pos = addr;
	if (tmp != buf)
		return tmp-buf;
	if (current->signal & ~current->blocked)
		return -ERESTARTSYS;
	return 0;
}

static int mem_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
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
}

static struct file_operations proc_mem_operations = {
	mem_lseek,
	mem_read,
	mem_write,
	NULL,		/* mem_readdir */
	NULL,		/* mem_select */
	NULL,		/* mem_ioctl */
	NULL,		/* mmap */
	NULL,		/* no special open code */
	NULL		/* no special release code */
};

struct inode_operations proc_mem_inode_operations = {
	&proc_mem_operations,	/* default base directory file-ops */
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
	NULL			/* truncate */
};
