#define __LIBRARY__
#include <unistd.h>

_syscall3(int,readlink,const char *,path,char *,buffer,int,bufsiz)
