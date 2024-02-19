/*
 *  linux/lib/setsid.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define __LIBRARY__
#include <linux/types.h>
#include <linux/unistd.h>

_syscall0(pid_t,setsid)

