#define __LIBRARY__
#include <unistd.h>

_syscall1(int,acct,const char *,filename)
