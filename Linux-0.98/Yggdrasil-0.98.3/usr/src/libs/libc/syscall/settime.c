#define __LIBRARY__
#include <unistd.h>

_syscall2(int,settimeofday,const struct timeval *,tv, const struct timezone *,tz)
