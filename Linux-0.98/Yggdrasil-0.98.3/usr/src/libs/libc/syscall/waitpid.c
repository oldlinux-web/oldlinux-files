#define __LIBRARY__
#include <unistd.h>
#include <sys/wait.h>

_syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
