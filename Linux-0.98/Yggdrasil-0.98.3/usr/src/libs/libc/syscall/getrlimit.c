#define __LIBRARY__
#include <unistd.h>

_syscall2(int,getrlimit,int,resource,struct rlimit *,rlp)
