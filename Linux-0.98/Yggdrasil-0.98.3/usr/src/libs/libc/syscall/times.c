#define __LIBRARY__
#include <unistd.h>

_syscall1(clock_t,times,struct tms *,tms_buf)
