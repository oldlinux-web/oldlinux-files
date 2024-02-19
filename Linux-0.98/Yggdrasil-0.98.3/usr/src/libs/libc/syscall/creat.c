#define __LIBRARY__
#include <unistd.h>

_syscall2(int,creat,const char *,filename,mode_t,mode)
