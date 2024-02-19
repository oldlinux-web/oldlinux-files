#define __LIBRARY__
#include <unistd.h>

_syscall2(int,setrlimit,int,resource,const struct rlimit *,rlp)
