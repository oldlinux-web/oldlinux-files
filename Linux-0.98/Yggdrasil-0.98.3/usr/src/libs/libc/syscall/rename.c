#define __LIBRARY__
#include <unistd.h>

_syscall2(int,rename,const char *,from,const char *,to)
