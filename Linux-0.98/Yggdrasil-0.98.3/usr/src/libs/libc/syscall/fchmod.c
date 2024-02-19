#define __LIBRARY__
#include <unistd.h>

_syscall2(int,fchmod,int,fd,mode_t,mode)
