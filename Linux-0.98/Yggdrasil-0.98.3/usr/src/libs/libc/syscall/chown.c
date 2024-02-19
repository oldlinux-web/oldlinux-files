#define __LIBRARY__
#include <unistd.h>

_syscall3(int,chown,const char *,filename,uid_t,owned,gid_t,group)
