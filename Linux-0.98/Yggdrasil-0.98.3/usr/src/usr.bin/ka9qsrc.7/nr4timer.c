/* net/rom level 4 (transport) protocol timer management.
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "lapb.h"
#include "netrom.h"
#include "nr4.h"
#include <ctype.h>

#undef NR4DEBUG

/* The ACK timer has expired without any data becoming available.
 * Go ahead and send an ACK.
 */

void
nr4ackit(p)
char *p ;
{
	struct nr4cb *cb  = (struct nr4cb *)p ;
	struct nr4hdr rhdr ;

#ifdef NR4DEBUG
	printf("ACKIT called.\n") ;
#endif
	if (cb->qfull)				/* Are we choked? */
		rhdr.opcode = NR4OPACK | NR4CHOKE ;
	else
		rhdr.opcode = NR4OPACK ;
	rhdr.yourindex = cb->yournum ;
	rhdr.yourid = cb->yourid ;
	rhdr.u.ack.rxseq = cb->rxpected ;

	nr4sframe(&cb->node, &rhdr, NULLBUF) ;
}

/* Called when one of the transmit timers has expired */

void
nr4txtimeout(p)
char *p ;
{
	struct nr4cb *cb = (struct nr4cb *)p ;
	unsigned seq ;
	struct nr4txbuf *t ;

	/* Sanity check */

	if (cb->state != NR4STCON)
		return ;

	/* Scan through the send window looking for expired timers */
	
	for (seq = cb->ackxpected ;
		 nr4between(cb->ackxpected, seq, cb->nextosend) ;
		 seq = (seq + 1) & NR4SEQMASK) {
		
		t = &cb->txbufs[seq % cb->window] ;

		if (t->tretry.state == TIMER_EXPIRE) {
			t->tretry.state = TIMER_STOP ;	/* So we don't do it again */

			if (t->retries == Nr4retries) {
				cb->dreason = NR4RTIMEOUT ;
				nr4state(cb, NR4STDISC) ;
			}

			t->retries++ ;
			
			/* We keep track of the highest retry count in the window. */
			/* If packet times out and its new retry count exceeds the *
			/* max, we update the max and bump the backoff level.  This */
			/* expedient is to avoid bumping the backoff level for every */
			/* expiration, since with more than one timer we would back */
			/* off way too fast (and at a rate dependent on the window */
			/* size! */

			if (t->retries > cb->txmax) {
				cb->blevel++ ;
				cb->txmax = t->retries ;	/* update the max */
			}
			
			nr4sbuf(cb,seq) ;	/* Resend buffer */
		}
	 }
	
}

/* Connect/disconnect acknowledgement timeout */

void
nr4cdtimeout(p)
char *p ;
{
	struct nr4cb *cb = (struct nr4cb *)p ;
	struct nr4hdr hdr ;

	switch(cb->state) {
	  case NR4STCPEND:
	  	if (cb->cdtries == Nr4retries) {	/* Have we tried long enough? */
			cb->dreason = NR4RTIMEOUT ;
			nr4state(cb, NR4STDISC) ;		/* Give it up */
		} else {
			/* Set up header */
			
			hdr.opcode = NR4OPCONRQ ;
			hdr.u.conreq.myindex = cb->mynum ;
			hdr.u.conreq.myid = cb->myid ;
			hdr.u.conreq.window = Nr4window ;
			hdr.u.conreq.user = cb->luser ;
			hdr.u.conreq.node = mycall ;

			/* Bump tries counter and backoff level, and restart timer */
			/* We use a binary exponential backoff. */
			
			cb->cdtries++ ;
			cb->blevel++ ;
			cb->tcd.start *= 2 ;
			start_timer(&cb->tcd) ;

			/* Send connect request packet */

			nr4sframe(&cb->node, &hdr, NULLBUF) ;
		}
		break ;
		
	  case NR4STDPEND:
	  	if (cb->cdtries == Nr4retries) {	/* Have we tried long enough? */
			cb->dreason = NR4RTIMEOUT ;
			nr4state(cb, NR4STDISC) ;		/* Give it up */
		} else {
			/* Format header */
			
			hdr.opcode = NR4OPDISRQ ;
			hdr.yourindex = cb->yournum ;
			hdr.yourid = cb->yourid ;

			/* Bump retry count and start timer */
			/* We don't really need to be fancy here, since we */
			/* should have a good idea of the round trip time by now. */
			
			cb->cdtries++ ;
			start_timer(&cb->tcd) ;

			/* Send disconnect request packet */

			nr4sframe(&cb->node, &hdr, NULLBUF) ;
		}
		break ;
	}
}

/* The choke timer has expired.  Unchoke and kick. */

void
nr4unchoke(p)
char *p ;
{
	struct nr4cb *cb = (struct nr4cb *)p ;

	cb->choked = 0 ;
	nr4output(cb) ;
}

