#define __LIBRARY__
#include <unistd.h>

_syscall1(int,stime,time_t *,tptr)
