#define __LIBRARY__
#include <unistd.h>

_syscall2(int,utime,const char *,filename,struct utimbuf *,utime_buf)
