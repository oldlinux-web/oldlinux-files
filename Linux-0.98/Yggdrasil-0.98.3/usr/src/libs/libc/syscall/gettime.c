#define __LIBRARY__
#include <unistd.h>

_syscall2(int,gettimeofday,struct timeval *,tv,struct timezone *,tz)

