#ifndef SIGNAL_H
#define SIGNAL_H

typedef unsigned int sigset_t;

#define NSIG	16

#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGILL		4
#define SIGTRAP		5
#define SIGABRT		6
#define SIGIOT		6
#define SIGUNUSED	7
#define SIGFPE		8
#define SIGKILL		9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOUT	22

/* sa_flags ??? */
/* 
#define SA_NOCLDSTOP
#define SA_RESTART
#define SA_ONSTACK
#define SA_NOCLDWAIT
#define SA_NODEFER
#define SA_RESETHAND
#define SA_SIGINFO 
*/

#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

typedef void (*__sighandler_t)(int);

#define SIG_DFL	((__sighandler_t)0)
#define SIG_IGN ((__sighandler_t)1)
#define SIG_ERR ((__sighandler_t)-1)

struct sigaction{
	__sighandler_t sa_handler;
	sigset_t sa_mask;
	int sa_flags;
};

#endif
