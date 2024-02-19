#include <sys/types.h>
#include <signal.h>
#include <errno.h>

int sigprocmask(int how, sigset_t * set, sigset_t * oset)
{
    sigset_t old;

    old = ___sgetmask();

    if (set)
      switch(how) {
      case SIG_BLOCK:
	old = ___ssetmask((*set) | old);
	break;
      case SIG_UNBLOCK:
	old = ___ssetmask((~(*set)) & old);
	break;
      case SIG_SETMASK:
	old = ___ssetmask(*set);
	break;
      default:
	errno = EINVAL;
	return -1;
      }

    if (oset)
	*oset = old;

    return 0;
}
