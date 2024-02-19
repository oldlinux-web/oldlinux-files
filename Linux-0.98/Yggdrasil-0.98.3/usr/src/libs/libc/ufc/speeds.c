/*
 * This fcrypt/crypt speed testing program
 * is derived from one floating around in
 * the net. It's distributed along with
 * UFC-crypt but is not.
 *
 * Michael Glad, email: glad@daimi.aau.dk
 *
 * @(#)speeds.c	1.5 01/23/92
 */

#include <sys/time.h>
#include <signal.h>
#include <stdio.h>

static int cnt;
#define ITIME	10		/* Number of seconds to run test. */

void
Stop ()
{
    printf ("Did %f %s()s per second.\n",
	    ((float) cnt) / ((float) ITIME),
#if defined(FCRYPT)
	    "fcrypt"
#else
	    "crypt"
#endif
    );
    exit (0);
}

/*
 * Silly rewrite of 'bzero'. I do so
 * because some machines don't have
 * bzero and some don't have memset.
 */

static void clearmem(start, cnt)
  char *start;
  int cnt;
  { while(cnt--)
      *start++ = '\0';
  }

main ()
{
    struct itimerval itv;

    clearmem (&itv, sizeof (itv));

    printf ("Running %s for %d seconds of virtual time ...\n",
#ifdef FCRYPT
    "UFC-crypt",
#else
    "crypt(libc)",
#endif
	    ITIME);

#ifdef FCRYPT
    init_des ();
#endif

    signal (SIGVTALRM, Stop);
    itv.it_value.tv_sec = ITIME;
    itv.it_value.tv_usec = 0;
    setitimer (ITIMER_VIRTUAL, &itv, NULL);

    for (cnt = 0;; cnt++)
    {
#ifdef FCRYPT
	fcrypt ("fredfred", "eek");
#else
	crypt ("fredfred", "eek");
#endif
    }
}






