/*
 *  linux/fs/char_dev.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>
#include <sys/types.h>

#include <linux/sched.h>
#include <linux/kernel.h>

#include <asm/segment.h>
#include <asm/io.h>

extern int tty_read(unsigned minor,char * buf,int count,unsigned short flags);
extern int tty_write(unsigned minor,char * buf,int count);

typedef (*crw_ptr)(int,unsigned,char *,int,off_t *,unsigned short);

static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos, unsigned short flags)
{
	return ((rw==READ)?tty_read(minor,buf,count,flags):
		tty_write(minor,buf,count));
}

static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos, unsigned short flags)
{
	if (current->tty<0)
		return -EPERM;
	return rw_ttyx(rw,current->tty,buf,count,pos,flags);
}

static int rw_ram(int rw,char * buf, int count, off_t *pos)
{
	return -EIO;
}

static int rw_mem(int rw,char * buf, int count, off_t * pos)
{
	return -EIO;
}

static int rw_kmem(int rw,char * buf, int count, off_t * pos)
{
	/* kmem by Damiano */
	int i = *pos;	/* Current position where to read	*/

	/* i can go from 0 to LOW_MEM (See include/linux/mm.h	*/
	/* I am not shure about it but it doesn't mem fault :-)	*/
	while ( (count-- > 0)  && (i <LOW_MEM) ) {
		if (rw==READ)
			put_fs_byte( *(char *)i ,buf++);
		else
			return (-EIO);
		i++;
	}
	i -= *pos;		/* Count how many read or write		*/
	*pos += i;		/* Update position			*/
	return (i);		/* Return number read			*/
}

static int rw_port(int rw,char * buf, int count, off_t * pos)
{
	int i=*pos;

	while (count-->0 && i<65536) {
		if (rw==READ)
			put_fs_byte(inb(i),buf++);
		else
			outb(get_fs_byte(buf++),i);
		i++;
	}
	i -= *pos;
	*pos += i;
	return i;
}

static int rw_memory(int rw, unsigned minor, char * buf, int count,
	off_t * pos, unsigned short flags)
{
	switch(minor) {
		case 0:
			return rw_ram(rw,buf,count,pos);
		case 1:
			return rw_mem(rw,buf,count,pos);
		case 2:
			return rw_kmem(rw,buf,count,pos);
		case 3:
			return (rw==READ)?0:count;	/* rw_null */
		case 4:
			return rw_port(rw,buf,count,pos);
		default:
			return -EIO;
	}
}

#define NRDEVS ((sizeof (crw_table))/(sizeof (crw_ptr)))

static crw_ptr crw_table[]={
	NULL,		/* nodev */
	rw_memory,	/* /dev/mem etc */
	NULL,		/* /dev/fd */
	NULL,		/* /dev/hd */
	rw_ttyx,	/* /dev/ttyx */
	rw_tty,		/* /dev/tty */
	NULL,		/* /dev/lp */
	NULL};		/* unnamed pipes */

int char_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	unsigned int major,minor;
	crw_ptr call_addr;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	if (major >= NRDEVS)
		return -ENODEV;
	if (!(call_addr = crw_table[major]))
		return -ENODEV;
	return call_addr(READ,minor,buf,count,&filp->f_pos,filp->f_flags);
}

int char_write(struct inode * inode, struct file * filp, char * buf, int count)
{
	unsigned int major,minor;
	crw_ptr call_addr;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	if (major >= NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[major]))
		return -ENODEV;
	return call_addr(WRITE,minor,buf,count,&filp->f_pos,filp->f_flags);
}
