#define __LIBRARY__
#include <unistd.h>

_syscall2(int,fstat,int,fd,struct stat *,stat_buf)
