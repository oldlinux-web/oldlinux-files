/*
 *  linux/kernel/printk.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <stdarg.h>

#include <asm/segment.h>
#include <asm/system.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);
extern void console_print(const char *);

static unsigned long log_page = 0;
static unsigned long log_start = 0;
static unsigned long log_size = 0;
static struct wait_queue * log_wait = NULL;

int sys_syslog(int type, char * buf, int len)
{
	unsigned long i;
	char c;

	if (!suser())
		return -EPERM;
	switch (type) {
		case 0:
			i = log_page;
			log_page = 0;
			free_page(i);
			wake_up(&log_wait);
			return 0;
		case 1:
			i = get_free_page(GFP_KERNEL);
			if (log_page) {
				free_page(i);
				return 0;
			} else if (log_page = i) {
				log_start = log_size = 0;
				return 0;
			}
			return -ENOMEM;
		case 2:
			if (!buf || len < 0)
				return -EINVAL;
			if (!len)
				return 0;
			verify_area(buf,len);
			while (!log_size) {
				if (!log_page)
					return -EIO;
				if (current->signal & ~current->blocked)
					return -ERESTARTSYS;
				cli();
				if (!log_size)
					interruptible_sleep_on(&log_wait);
				sti();
			}
			i = 0;
			while (log_size && len) {
				c = *((char *) log_page+log_start);
				log_start++;
				log_size--;
				log_start &= 4095;
				put_fs_byte(c,buf);
				buf++;
				i++;
			}
			return i;
	}
	return -EINVAL;
}
			

int printk(const char *fmt, ...)
{
	va_list args;
	int i,j;
	char * p;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	for (j = 0; j < i && log_page ; j++) {
		p = (char *) log_page + (4095 & (log_start+log_size));
		*p = buf[j];
		if (log_size < 4096)
			log_size++;
		else
			log_start++;
	}
	if (log_page)
		wake_up(&log_wait);
	console_print(buf);
	return i;
}
