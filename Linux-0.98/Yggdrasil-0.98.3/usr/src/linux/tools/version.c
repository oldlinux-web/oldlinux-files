/*
 *  linux/version.c
 *
 *  Copyright (C) 1992  Theodore Ts'o
 *
 *  May be freely distributed as part of Linux.
 */

#include <linux/config.h>
#include <linux/utsname.h>

#include "./version.h"

struct new_utsname system_utsname = {
	UTS_SYSNAME, UTS_NODENAME, UTS_RELEASE, UTS_VERSION, UTS_MACHINE
};

char *linux_banner = 
	"Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
	LINUX_COMPILE_HOST ") " UTS_VERSION " " LINUX_COMPILE_TIME " \n";
