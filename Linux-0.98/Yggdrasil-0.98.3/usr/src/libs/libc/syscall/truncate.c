#define __LIBRARY__
#include <unistd.h>

_syscall2(int,truncate,const char *,path,size_t,offset)
