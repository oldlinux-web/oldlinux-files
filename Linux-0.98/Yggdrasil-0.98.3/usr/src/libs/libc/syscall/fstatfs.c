#include <sys/vfs.h>
#define __LIBRARY__
#include <unistd.h>

_syscall2(int,fstatfs,int,fd,struct statfs *,stat_buf)
