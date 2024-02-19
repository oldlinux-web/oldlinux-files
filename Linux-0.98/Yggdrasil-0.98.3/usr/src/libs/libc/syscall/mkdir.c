#define __LIBRARY__
#include <unistd.h>

_syscall2(int,mkdir,const char *,pathname,mode_t,mode)
