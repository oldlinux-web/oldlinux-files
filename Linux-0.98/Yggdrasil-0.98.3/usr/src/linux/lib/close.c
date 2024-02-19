/*
 *  linux/lib/close.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define __LIBRARY__
#include <linux/unistd.h>

_syscall1(int,close,int,fd)

