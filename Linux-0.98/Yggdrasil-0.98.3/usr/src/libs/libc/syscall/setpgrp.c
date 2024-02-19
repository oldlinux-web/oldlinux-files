#define __LIBRARY__
#include <unistd.h>

_syscall2(int,setpgid,pid_t,pid,pid_t,pgid)

int setpgrp(void)
{
	return setpgid(0,0);
}
