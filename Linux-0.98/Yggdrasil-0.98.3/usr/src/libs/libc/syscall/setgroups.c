#define __LIBRARY__
#include <unistd.h>

_syscall2(int,setgroups,int,getsetlen,gid_t *,gidset)
