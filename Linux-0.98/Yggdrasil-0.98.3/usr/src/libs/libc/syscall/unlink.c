#define __LIBRARY__
#include <unistd.h>

_syscall1(int,unlink,const char *,filename)
