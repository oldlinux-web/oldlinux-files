#define __LIBRARY__
#include <unistd.h>

_syscall2(int,kill,pid_t,pid,int,signal)
