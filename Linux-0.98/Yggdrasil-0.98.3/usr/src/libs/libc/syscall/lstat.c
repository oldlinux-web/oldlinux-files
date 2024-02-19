#define __LIBRARY__
#include <unistd.h>

_syscall2(int,lstat,const char *,filename,struct stat *,stat_buf)
