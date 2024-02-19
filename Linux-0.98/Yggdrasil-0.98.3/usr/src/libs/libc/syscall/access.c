#define __LIBRARY__
#include <unistd.h>

_syscall2(int,access,const char *,filename,mode_t,mode)
