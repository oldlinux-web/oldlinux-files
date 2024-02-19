#define __LIBRARY__
#include <unistd.h>

_syscall3(int,mknod,const char *,nodename,mode_t,mode,dev_t,dev)
