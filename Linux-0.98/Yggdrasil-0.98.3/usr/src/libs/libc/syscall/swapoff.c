#define __LIBRARY__
#include <unistd.h>

_syscall1(int,swapoff,const char *,specialfile)
