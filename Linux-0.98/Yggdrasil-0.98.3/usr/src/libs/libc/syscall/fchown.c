#define __LIBRARY__
#include <unistd.h>

_syscall3(int,fchown,int,fd,uid_t,owner,gid_t,group)
