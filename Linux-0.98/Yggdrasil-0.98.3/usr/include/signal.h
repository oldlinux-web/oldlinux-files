#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <features.h>
#include <traditional.h>
#include <sys/types.h>
#include <linux/signal.h>

#ifndef SIGCLD
#define SIGCLD	SIGCHLD
#endif

typedef int sig_atomic_t;

typedef __sighandler_t	SignalHandler;

#define BAD_SIG		SIG_ERR

/* The Interviews version also has these ... */

#define SignalBad	((SignalHandler)-1)
#define SignalDefault	((SignalHandler)0)
#define SignalIgnore	((SignalHandler)1)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC__
extern const char* const sys_siglist[];
#else
extern char* sys_siglist[];
#endif

extern __sighandler_t
		signal _ARGS ((int __sig, __sighandler_t));
extern int	raise _ARGS ((int __sig));
extern int	kill _ARGS ((pid_t __pid, int __sig));
extern int	killpg _ARGS ((int __pgrp, int __sig));
extern int	sigaddset _ARGS ((sigset_t *__mask, int __sig));
extern int	sigdelset _ARGS ((sigset_t *__mask, int __sig));
extern int	sigemptyset _ARGS ((sigset_t *__mask));
extern int	sigfillset _ARGS ((sigset_t *__mask));
extern int	sigismember _ARGS ((const sigset_t *__mask, int __sig));
extern int	sigpending _ARGS ((sigset_t *__set));
extern int	sigprocmask _ARGS ((int __how, sigset_t *__set,
			sigset_t *__oldset));
extern int	sigsuspend _ARGS ((sigset_t *sigmask));
extern int	sigaction _ARGS ((int __sig, struct sigaction *__act,
			struct sigaction *__oldact));

#define __sigemptyset(set)	((*(set) = 0L), 0)
#define __sigfillset(set)       ((*(set) = -1L), 0)
#define __sigaddset(set, sig)   ((*(set) |= __sigmask (sig)), 0)
#define __sigdelset(set, sig)   ((*(set) &= ~__sigmask (sig)), 0)
#define __sigismember(set, sig) ((*(set) & __sigmask (sig)) ? 1 : 0)


#ifdef __OPTIMIZE__
#define sigemptyset	__sigemptyset
#define sigfillset	__sigfillset
#define sigaddset	__sigaddset
#define sigdelset	__sigdelset
#define sigismember	__sigismember
#endif


/* Return a mask that includes SIG only.  */
#define __sigmask(sig)  (1 << ((sig) - 1))

extern sigset_t	___ssetmask _ARGS ((sigset_t __mask));
extern sigset_t	___sgetmask _ARGS ((void));
extern int	__sigblock _ARGS ((int __mask));
extern int	__sigpause _ARGS ((int __mask));


/* BSD */
#ifdef __USE_BSD
#define sigmask		__sigmask

extern int	sigblock _ARGS ((int __mask));
extern int	sigpause _ARGS ((int __mask));
extern sigset_t	sigsetmask _ARGS ((int __mask));
extern sigset_t	siggetmask _ARGS ((void));
extern void	psignal _ARGS ((unsigned int __sig, const char *__str));

#endif  /* Use BSD.  */

#ifdef __USE_BSD_SIGNAL

static __inline__  __sighandler_t
__bsd_signal (int sig,  __sighandler_t handler)
{
  int ret;
  struct sigaction action, oaction;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  ret = sigaction (sig, &action, &oaction); 
  return (ret == -1) ? SIG_ERR : oaction.sa_handler;
}

#define signal	__bsd_signal

#endif	/* __USE_BSD_SIGNAL */

#ifdef __cplusplus
}
#endif

#endif /* _SIGNAL_H */
