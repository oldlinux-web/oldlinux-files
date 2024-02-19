#define __LIBRARY__
#include <unistd.h>

_syscall2(int,sethostname,char *, name,int,len)

