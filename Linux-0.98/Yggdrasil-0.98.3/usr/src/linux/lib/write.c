/*
 *  linux/lib/write.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define __LIBRARY__
#include <linux/unistd.h>
#include <linux/types.h>

_syscall3(int,write,int,fd,const char *,buf,off_t,count)

