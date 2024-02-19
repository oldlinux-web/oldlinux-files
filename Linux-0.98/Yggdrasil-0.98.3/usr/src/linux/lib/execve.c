/*
 *  linux/lib/execve.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define __LIBRARY__
#include <linux/unistd.h>

_syscall3(int,execve,const char *,file,char **,argv,char **,envp)

