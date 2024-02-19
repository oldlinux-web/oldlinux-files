#define __LIBRARY__
#include <unistd.h>

_syscall1(int,chroot,const char *,pathname)
