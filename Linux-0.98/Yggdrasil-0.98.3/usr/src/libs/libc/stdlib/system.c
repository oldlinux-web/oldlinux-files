#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Urgh. If this works I'm surprised. Linus
 *
 * Should be updated to use sigactions, I think.
 */

int system(const char * cmd)
{
	int ret, pid, waitstat;
	sigset_t set, oset;

	if ((pid = fork()) == 0) {
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		exit(127);
	}
	if (pid < 0) return(127 << 8);

	sigemptyset (&set);
	sigaddset (&set, SIGINT);
	sigaddset (&set, SIGQUIT);
	sigprocmask (SIG_BLOCK, &set, &oset);

	while ((waitstat = wait(&ret)) != pid && waitstat != -1);
	if (waitstat == -1) ret = -1;

	sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);

	return(ret);
}
