/*
   exec.c: exec() takes an argument list and does the appropriate thing
   (calls a builtin, calls a function, etc.)
*/

#include <signal.h>
#include <errno.h>
#include "rc.h"
#include "utils.h"
#include "exec.h"
#include "status.h"
#include "hash.h"
#include "builtins.h"
#include "footobar.h"
#include "jbwrap.h"
#include "except.h"
#include "redir.h"
#include "wait.h"

void exec(List *s, boolean parent) {
	char **av, **ev;
	int pid, stat;
	builtin_t *b;
	char *path = NULL;
	void (*handler)(int);
	boolean forked, saw_exec, saw_builtin;

	av = list2array(s, dashex);
	ev = makeenv();
	saw_builtin = saw_exec = FALSE;

	do {
		if (*av == NULL	|| isabsolute(*av))
			b = NULL;
		else if (!saw_builtin && fnlookup(*av) != NULL)
			b = funcall;
		else
			b = isbuiltin(*av);

		saw_builtin = FALSE; /* a builtin applies only to the immmediately following command, e.g., builtin exec echo hi */

		if (b == b_exec) {
			av++;
			saw_exec = TRUE;
			parent = FALSE;
		} else if (b == b_builtin) {
			av++;
			saw_builtin = TRUE;
		}
	} while (b == b_exec || b == b_builtin);

	if (*av == NULL && saw_exec) { /* do redirs and return on a null exec */
		doredirs();
		return;
	}

	if (b == NULL) {
		path = which(*av, TRUE);
		if (path == NULL && *av != NULL) { /* perform null commands for redirections */
			set(FALSE);
			redirq = NULL;
			empty_fifoq();
			if (parent)
				return;
			rc_exit(1);
		}
	}

	/* if parent & the redirq is nonnull, builtin or not it has to fork. */

	if (parent && (b == NULL || redirq != NULL)) {
		pid = rc_fork();
		forked = TRUE;
	} else {
		pid = 0;
		forked = FALSE;
	}

	switch (pid) {
	case -1:
		uerror("fork");
		rc_error(NULL);
		/* NOTREACHED */
	case 0:
		if (forked)
			setsigdefaults();
		doredirs();

		/* null commands performed for redirections */
		if (*av == NULL || b != NULL) {
			if (b != NULL)
				b(av);
			empty_fifoq();
			if (!forked && parent)
				return;
			rc_exit(getstatus());
		}
#ifdef NOEXECVE
		my_execve(path, (const char **) av, (const char **) ev); /* bogus, huh? */
#else
		execve(path, (const char **) av, (const char **) ev);
#endif
#ifdef DEFAULTINTERP
		if (errno == ENOEXEC) {
			*av = path;
			*--av = DEFAULTINTERP;
			execve(*av, (const char **) av, (const char **) ev);
		}
#endif
		uerror(*av);
		rc_exit(1);
		/* NOTREACHED */
	default:
		if ((handler = signal(SIGINT, SIG_IGN)) != sig)
			signal(SIGINT, handler); /* don't ignore interrupts in noninteractive mode */
		rc_wait4(pid, &stat);
		signal(SIGINT, handler);
		redirq = NULL;
		empty_fifoq();
		setstatus(stat);
		if (stat == SIGINT || stat == SIGQUIT) /* interrupted? let the handler deal with it. */
			rc_raise(ERROR);
	}
}
