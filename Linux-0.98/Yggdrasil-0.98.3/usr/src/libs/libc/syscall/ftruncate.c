#define __LIBRARY__
#include <unistd.h>

_syscall2(int,ftruncate,int,fd,size_t,offset)
