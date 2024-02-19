#define __LIBRARY__
#include <unistd.h>

_syscall2(int,link,const char *,filename1,const char *,filename2)
