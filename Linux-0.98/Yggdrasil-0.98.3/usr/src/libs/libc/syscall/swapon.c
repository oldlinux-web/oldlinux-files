#define __LIBRARY__
#include <unistd.h>

_syscall1(int,swapon,const char *,specialfile)
