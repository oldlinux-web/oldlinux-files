#if 0
#define __NR_wait4   114
#endif
#define __LIBRARY__
#define wait4 libc_wait4
#include <unistd.h>
#include <sys/wait.h>
#undef wait4

static inline
_syscall4(pid_t,wait4,pid_t,pid,int *,status,int,options,struct rusage *,ru);

pid_t wait3(int *wait_stat, int options, struct rusage *reserved)
{
	return wait4(WAIT_ANY, wait_stat, options, reserved);
}
