/*
 *  linux/fs/file_table.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/fs.h>
#include <linux/string.h>

struct file file_table[NR_FILE];

struct file * get_empty_filp(void)
{
	int i;
	struct file * f = file_table+0;

	for (i = 0; i++ < NR_FILE; f++)
		if (!f->f_count) {
			memset(f,0,sizeof(*f));
			f->f_count = 1;
			return f;
		}
	return NULL;
}
