/* TCP timeout routines */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "netuser.h"
#include "internet.h"
#include "tcp.h"

/* Timer timeout */
void
tcp_timeout(arg)
char *arg;
{
	register struct tcb *tcb;

	tcb = (struct tcb *)arg;

	if(tcb == NULLTCB)
		return;

	/* Make sure the timer has stopped (we might have been kicked) */
	stop_timer(&tcb->timer);

	switch(tcb->state){
	case TIME_WAIT:		/* 2MSL timer has expired */
		close_self(tcb,NORMAL);
		break;
	default:		/* Retransmission timer has expired */
		tcb->flags |= RETRAN;	/* Indicate > 1  transmission */
		tcb->backoff++;
		tcb->snd.ptr = tcb->snd.una;
		/* Reduce slowstart threshold to half current window */
		tcb->ssthresh = tcb->cwind / 2;
		tcb->ssthresh = max(tcb->ssthresh,tcb->mss);
		/* Shrink congestion window to 1 packet */
		tcb->cwind = tcb->mss;
		tcp_output(tcb);
	}
}
/* Backoff function - the subject of much research */
backoff(n)
int n;
{
	/* Use binary exponential up to retry #4, and quadratic after that
	 * This yields the sequence
	 * 1, 2, 4, 8, 16, 25, 36, 49, 64, 81, 100 ...
	 */

	if(n <= 4)
		return 1 << n;	/* Binary exponential back off */
	else
		return n * n;	/* Quadratic back off */
}

