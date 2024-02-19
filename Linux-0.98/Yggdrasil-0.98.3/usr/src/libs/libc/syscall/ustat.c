#define __LIBRARY__
#include <unistd.h>

_syscall2(int,ustat,dev_t,dev,struct ustat *,ubuf)
