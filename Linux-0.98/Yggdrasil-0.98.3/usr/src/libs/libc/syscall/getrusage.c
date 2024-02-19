#define __LIBRARY__
#include <unistd.h>

_syscall2(int,getrusage,int,who,struct rusage *,rusage)
