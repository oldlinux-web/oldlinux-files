#define __LIBRARY__
#include <unistd.h>

_syscall2(int,dup2,int,fd1,int,fd2)
