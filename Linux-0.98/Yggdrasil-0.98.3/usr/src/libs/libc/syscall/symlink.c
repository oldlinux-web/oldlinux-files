#define __LIBRARY__
#include <unistd.h>

_syscall2(int,symlink,const char *,filename1,const char *,filename2)
