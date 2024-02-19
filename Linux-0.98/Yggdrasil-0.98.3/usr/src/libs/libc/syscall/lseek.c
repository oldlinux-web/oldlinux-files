#define __LIBRARY__
#include <unistd.h>

_syscall3(off_t,lseek,int,fildes,off_t,offset,int,origin)
