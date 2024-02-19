#define __LIBRARY__
#include <unistd.h>

_syscall2(int,ulimit,int,cmd,long,limit)
