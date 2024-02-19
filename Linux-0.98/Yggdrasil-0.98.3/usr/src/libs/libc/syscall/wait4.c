#if 0
#define __NR_wait4   114
#endif
#define __LIBRARY__
#include <unistd.h>
#include <sys/wait.h>

_syscall4(pid_t,wait4,pid_t,pid,int *,status,int,options,struct rusage *,ru);
