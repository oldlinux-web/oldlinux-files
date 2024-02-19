#define __LIBRARY__
#include <unistd.h>

_syscall3(int,ioperm,unsigned long,from,unsigned long,num,int,turn_on)
