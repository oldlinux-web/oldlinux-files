#define __LIBRARY__
#include <unistd.h>

_syscall1(int,umount,const char *,specialfile)
