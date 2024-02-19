#define __LIBRARY__
#include <unistd.h>

_syscall2(int,chmod,const char *,filename,mode_t,mode)
