#define __LIBRARY__
#include <unistd.h>

_syscall2(int,setreuid,uid_t,uid1,uid_t,uid2)
