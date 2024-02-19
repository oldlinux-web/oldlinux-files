#define __LIBRARY__
#define waitpid libc_waitpid
#include <unistd.h>
#include <sys/wait.h>
#undef waitpid

static inline
_syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)

pid_t wait(int * wait_stat)
{
	return waitpid(WAIT_ANY,wait_stat,0);
}
