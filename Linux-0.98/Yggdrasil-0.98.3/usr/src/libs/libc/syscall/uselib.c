#define __LIBRARY__
#include <unistd.h>

#if 1
_syscall1(int,uselib,const char *,filename)
#else
_syscall2(int,uselib,const char *,filename,daddr_t,address)
#endif
