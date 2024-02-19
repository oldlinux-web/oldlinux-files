/*
 *  linux/lib/_exit.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define __LIBRARY__
#include <linux/unistd.h>

volatile void _exit(int exit_code)
{
fake_volatile:
	__asm__("movl %1,%%ebx\n\t"
		"int $0x80"::"a" (__NR_exit),"g" (exit_code));
	goto fake_volatile;
}
