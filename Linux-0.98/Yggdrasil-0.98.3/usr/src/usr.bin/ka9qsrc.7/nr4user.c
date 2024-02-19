/* net/rom level 4 (transport) protocol user level calls
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

/* Open a NET/ROM transport connection */
struct nr4cb *
open_nr4(node,user,r_upcall,t_upcall,s_upcall,puser)
struct ax25_addr *node ;	/* destination node address */
struct ax25_addr *user ;	/* local ax25 user callsign */
void (*r_upcall)() ;		/* received data upcall */
void (*t_upcall)() ;		/* transmit upcall */
void (*s_upcall)() ;		/* state change upcall */
char *puser ;				/* pointer to upper level control block */
{
	struct nr4cb *cb ;
	struct nr4hdr hdr ;

	if ((cb = new_n4circ()) == NULLNR4CB)
		return NULLNR4CB ;		/* No circuits available */

	/* Stuff what info we can into control block */

	cb->user = *node ;		/* When we initiate, user is remote node */
	cb->node = *node ;
	cb->luser = *user ;		/* Save local user for connect retries */
	cb->r_upcall = r_upcall ;
	cb->t_upcall = t_upcall ;
	cb->s_upcall = s_upcall ;
	cb->puser = puser ;

	/* Format connect request header */

	hdr.opcode = NR4OPCONRQ ;
	hdr.u.conreq.myindex = cb->mynum ;
	hdr.u.conreq.myid = cb->myid ;
	hdr.u.conreq.window = Nr4window ;
	hdr.u.conreq.user = *user ;

	/* The choice of mycall here is suspect.  If I have a unique */
	/* callsign per interface, then a layer violation will be */
	/* required to determine the "real" callsign for my (virtual) */
	/* node.  This suggests that callsign-per-interface is not */
	/* desirable, which answers *that* particular open question. */
	
	hdr.u.conreq.node = mycall ;

	/* Set and start connection retry timer */

	cb->cdtries = 1 ;
	cb->srtt = Nr4irtt ;
	cb->tcd.start = (2 * cb->srtt) / MSPTICK ;
	cb->tcd.func = nr4cdtimeout ;
	cb->tcd.arg = (char *)cb ;
	start_timer(&cb->tcd) ;
	
	/* Send connect request packet */

	nr4sframe(node, &hdr, NULLBUF) ;

	/* Set up initial state and signal state change */

	cb->state = NR4STDISC ;
	nr4state(cb, NR4STCPEND) ;

	/* Return control block address */

	return cb ;
}

/* Send a net/rom transport data packet */
int
send_nr4(cb,bp)
struct nr4cb *cb ;
struct mbuf *bp ;
{
	if (cb == NULLNR4CB || bp == NULLBUF)
		return -1 ;
	enqueue(&cb->txq,bp) ;
	return nr4output(cb) ;
}

/* Receive incoming net/rom transport data */
/*ARGSUSED*/
struct mbuf *
recv_nr4(cb,cnt)
struct nr4cb *cb ;
int16 cnt ;
{
	struct mbuf *bp ;

	if (cb->rxq == NULLBUF)
		return NULLBUF ;

	bp = cb->rxq ;			/* Just give `em everything */
	cb->rxq = NULLBUF ;

	/* Since we took everything, we always go unchoked.  If we */
	/* ever change this code to use cnt, or make it a real packet */
	/* delivery, there will have to be a check to see if the queue */
	/* has gotten short enough yet. */

	if (cb->qfull) {
		cb->qfull = 0 ;				/* Choke flag off */
		nr4ackit((char *)cb) ;		/* Get things rolling again */
	}

	return bp ;
}

/* Close a NET/ROM connection */
void
disc_nr4(cb)
struct nr4cb *cb ;
{
	struct nr4hdr hdr ;
	
	if (cb->state != NR4STCON)
		return ;

	/* Format disconnect request packet */
	
	hdr.opcode = NR4OPDISRQ ;
	hdr.yourindex = cb->yournum ;
	hdr.yourid = cb->yourid ;

	/* Set and start timer */
	
	cb->cdtries = 1 ;
	cb->tcd.start = (2 * cb->srtt) / MSPTICK ;
	cb->tcd.func = nr4cdtimeout ;
	cb->tcd.arg = (char *)cb ;
	start_timer(&cb->tcd) ;

	/* Send packet */

	nr4sframe(&cb->node, &hdr, NULLBUF) ;

	/* Signal state change.  nr4state will take care of stopping */
	/* the appropriate timers and resetting window pointers. */

	nr4state(cb, NR4STDPEND) ;
	
}

/* Abruptly terminate a NET/ROM transport connection */
void
reset_nr4(cb)
struct nr4cb *cb ;
{
	cb->dreason = NR4RRESET ;
	nr4state(cb,NR4STDISC) ;
}


/* Force retransmission on a NET/ROM transport connection */
int
kick_nr4(cb)
struct nr4cb *cb ;
{
	unsigned seq ;
	struct timer *t ;

	if(!nr4valcb(cb))
		return -1 ;

	switch (cb->state) {
	  case NR4STCPEND:
	  case NR4STDPEND:
	  	stop_timer(&cb->tcd) ;
		nr4cdtimeout((char *)cb) ;
		break ;

	  case NR4STCON:
	    if (cb->nextosend != cb->ackxpected) {	/* if send window is open: */
			for (seq = cb->ackxpected ;
				 nr4between(cb->ackxpected, seq, cb->nextosend) ;
				 seq = (seq + 1) & NR4SEQMASK) {
				t = &cb->txbufs[seq % cb->window].tretry ;
				stop_timer(t) ;
				t->state = TIMER_EXPIRE ;	/* fool retry routine */
			}
			nr4txtimeout((char *)cb) ;
		}
		break ;
	}

	return 0 ;
}
