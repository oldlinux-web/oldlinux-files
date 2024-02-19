#include <sys/vfs.h>
#define __LIBRARY__
#include <unistd.h>

_syscall2(int,statfs,const char *,filename,struct statfs *,stat_buf)
