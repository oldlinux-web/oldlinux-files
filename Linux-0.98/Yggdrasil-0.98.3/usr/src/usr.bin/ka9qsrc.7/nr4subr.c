/*
 * nr4subr.c:  subroutines for net/rom transport layer.
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */
 
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "netrom.h"
#include "nr4.h"
#include "lapb.h"
#include <ctype.h>

/* Convert a net/rom transport header to host format structure.
 * Return -1 if error, 0 if OK.
 */

int
ntohnr4(hdr,bpp)
register struct nr4hdr *hdr ;
struct mbuf **bpp ;
{
	char *getaxaddr() ;
	char axbuf[AXALEN] ;
	unsigned char tbuf[NR4MINHDR] ;

	if (pullup(bpp, (char *)tbuf, NR4MINHDR) < NR4MINHDR)
		return -1 ;

	hdr->opcode = tbuf[4] ;

	switch (tbuf[4] & NR4OPCODE) {

		case NR4OPPID:			/* protocol ID extension */
			hdr->u.pid.family = tbuf[0] ;
			hdr->u.pid.proto = tbuf[1] ;
			break ;

		case NR4OPCONRQ:		/* connect request */
			hdr->u.conreq.myindex = tbuf[0] ;
			hdr->u.conreq.myid = tbuf[1] ;
			if (pullup(bpp,(char *)&(hdr->u.conreq.window), 1) < 1)
				return -1 ;
			if (pullup(bpp,axbuf,AXALEN) < AXALEN)
				return -1 ;
			(void)getaxaddr(&hdr->u.conreq.user,axbuf) ;
			if (pullup(bpp,axbuf,AXALEN) < AXALEN)
				return -1 ;
			(void)getaxaddr(&hdr->u.conreq.node,axbuf) ;
			break ;

		case NR4OPCONAK:		/* connect acknowledge */
			hdr->yourindex = tbuf[0] ;
			hdr->yourid = tbuf[1] ;
			hdr->u.conack.myindex = tbuf[2] ;
			hdr->u.conack.myid = tbuf[3] ;
			if (pullup(bpp,(char *)&(hdr->u.conack.window), 1) < 1)
				return -1 ;
			break ;

		case NR4OPDISRQ:		/* disconnect request */
			hdr->yourindex = tbuf[0] ;
			hdr->yourid = tbuf[1] ;
			break ;

		case NR4OPDISAK:		/* disconnect acknowledge */
			hdr->yourindex = tbuf[0] ;
			hdr->yourid = tbuf[1] ;
			break ;

		case NR4OPINFO:			/* information frame */
			hdr->yourindex = tbuf[0] ;
			hdr->yourid = tbuf[1] ;
			hdr->u.info.txseq = tbuf[2] ;
			hdr->u.info.rxseq = tbuf[3] ;
			break ;

		case NR4OPACK:			/* information acknowledge */
			hdr->yourindex = tbuf[0] ;
			hdr->yourid = tbuf[1] ;
			hdr->u.ack.rxseq = tbuf[3] ;
			break ;

		default:				/* what kind of frame is this? */
			return -1 ;
	}

	return 0 ;
}


struct mbuf *
htonnr4(hdr)
register struct nr4hdr *hdr ;
{
	static unsigned char hlen[NR4NUMOPS] = {5,20,6,5,5,5,5} ;
	struct mbuf *rbuf ;
	register char *cp ;
	char *putaxaddr() ;
	unsigned char opcode ;

	opcode = hdr->opcode & NR4OPCODE ;

	if (opcode >= NR4NUMOPS)
		return NULLBUF ;

	if (hdr == (struct nr4hdr *)NULL)
		return NULLBUF ;

	if ((rbuf = alloc_mbuf(hlen[opcode])) == NULLBUF)
		return NULLBUF ;

	rbuf->cnt = hlen[opcode] ;
	cp = rbuf->data ;

	cp[4] = hdr->opcode ;
	
	switch (opcode) {
	
		case NR4OPPID:
			*cp++ = hdr->u.pid.family ;
			*cp = hdr->u.pid.proto ;
			break ;

		case NR4OPCONRQ:
			*cp++ = hdr->u.conreq.myindex ;
			*cp++ = hdr->u.conreq.myid ;
			cp += 3 ;						/* skip to sixth byte */
			*cp++ = hdr->u.conreq.window ;
			cp = putaxaddr(cp, &hdr->u.conreq.user) ;
			(void) putaxaddr(cp, &hdr->u.conreq.node) ;
			break ;

		case NR4OPCONAK:
			*cp++ = hdr->yourindex ;
			*cp++ = hdr->yourid ;
			*cp++ = hdr->u.conack.myindex ;
			*cp++ = hdr->u.conack.myid ;
			cp++ ;							/* already loaded pid */
			*cp = hdr->u.conack.window ;
			break ;

		case NR4OPDISRQ:
			*cp++ = hdr->yourindex ;
			*cp = hdr->yourid ;
			break ;

		case NR4OPDISAK:
			*cp++ = hdr->yourindex ;
			*cp = hdr->yourid ;
			break ;

		case NR4OPINFO:
			*cp++ = hdr->yourindex ;
			*cp++ = hdr->yourid ;
			*cp++ = hdr->u.info.txseq ;
			*cp = hdr->u.info.rxseq ;
			break ;

		case NR4OPACK:
			*cp++ = hdr->yourindex ;
			*cp++ = hdr->yourid ;
			*++cp = hdr->u.ack.rxseq ;	/* skip third byte (tricky yuck) */
			break ;

	}

	return rbuf ;
}


/* Get a free circuit table entry, and allocate a circuit descriptor.
 * Initialize control block circuit number and ID fields.
 * Return a pointer to the circuit control block if successful,
 * NULLNR4CB if not.
 */

struct nr4cb *
new_n4circ()
{
	int i ;
	struct nr4cb *cb ;

	for (i = 0 ; i <  NR4MAXCIRC ; i++)		/* find a free circuit */
		if (Nr4circuits[i].ccb == NULLNR4CB)
			break ;

	if (i == NR4MAXCIRC)	/* no more circuits */
		return NULLNR4CB ;

	if ((cb = Nr4circuits[i].ccb =
		 (struct nr4cb *)calloc(1,sizeof(struct nr4cb))) == NULLNR4CB)
		return NULLNR4CB ;
	else {
		cb->mynum = i ;
		cb->myid = Nr4circuits[i].cid ;
		return cb ;
	}
}


/* Set the window size for a circuit and allocate the buffers for
 * the transmit and receive windows.  Set the control block window
 * parameter.  Return 0 if successful, -1 if not.
 */

int
init_nr4window(cb, window)
struct nr4cb *cb ;
unsigned window ;
{
	
	if (window == 0 || window > NR4MAXWIN) /* reject silly window sizes */
		return -1 ;
		
	if ((cb->txbufs =
		 (struct nr4txbuf *)calloc(window,sizeof(struct nr4txbuf)))
		 == (struct nr4txbuf *)0)
		return -1 ;

	if ((cb->rxbufs =
		 (struct nr4rxbuf *)calloc(window,sizeof(struct nr4rxbuf)))
		 == (struct nr4rxbuf *)0) {
		free((char *)cb->txbufs) ;
		cb->txbufs = (struct nr4txbuf *)0 ;
		return -1 ;
	}

	cb->window = window ;
	
	return 0 ;
}


/* Free a circuit.  Deallocate the control block and buffers, and
 * increment the circuit ID.  No return value.
 */

void
free_n4circ(cb)
struct nr4cb *cb ;
{
	unsigned circ ;

	if (cb == NULLNR4CB)
		return ;

	circ = cb->mynum ;
	
	if (cb->txbufs != (struct nr4txbuf *)0)
		free((char *)cb->txbufs) ;

	if (cb->rxbufs != (struct nr4rxbuf *)0)
		free((char *)cb->rxbufs) ;

	/* Better be safe than sorry: */

	free_q(&cb->txq) ;
	free_q(&cb->rxq) ;
	
	free((char *)cb) ;

	if (circ > NR4MAXCIRC)		/* Shouldn't happen. */
		return ;
		
	Nr4circuits[circ].ccb = NULLNR4CB ;

	Nr4circuits[circ].cid++ ;
}

/* See if any open circuit matches the given parameters.  This is used
 * to prevent opening multiple circuits on a duplicate connect request.
 * Returns the control block address if a match is found, or NULLNR4CB
 * otherwise.
 */

struct nr4cb *
match_n4circ(index, id, user, node)
int index ;					/* index of remote circuit */
int id ;					/* id of remote circuit */
struct ax25_addr *user ;	/* address of remote user */
struct ax25_addr *node ;	/* address of originating node */
{
	int i ;
	struct nr4cb *cb ;

	for (i = 0 ; i < NR4MAXCIRC ; i++) {
		if ((cb = Nr4circuits[i].ccb) == NULLNR4CB)
			continue ;		/* not an open circuit */
		if (cb->yournum == index && cb->yourid == id
			&& addreq(&cb->user,user) && addreq(&cb->node,node))
			return cb ;
	}

	/* if we get to here, we didn't find a match */

	return NULLNR4CB ;
}

/* Validate the index and id of a local circuit, returning the control
 * block if it is valid, or NULLNR4CB if it is not.
 */

struct nr4cb *
get_n4circ(index, id)
int index ;				/* local circuit index */
int id ;				/* local circuit id */
{
	struct nr4cb *cb ;

	if (index >= NR4MAXCIRC)
		return NULLNR4CB ;

	if ((cb = Nr4circuits[index].ccb) == NULLNR4CB)
		return NULLNR4CB ;

	if (cb->myid == id)
		return cb ;
	else
		return NULLNR4CB ;
}

/* Return 1 if b is "between" (modulo the size of an unsigned char)
 * a and c, 0 otherwise.
 */

int
nr4between(a, b, c)
unsigned a, b, c ;
{
	if ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a))
		return 1 ;
	else
		return 0 ;
}

/* Set up default timer values, etc., in newly connected control block.
 */

void
nr4defaults(cb)
struct nr4cb *cb ;
{
	int i ;
	struct timer *t ;

	if (cb == NULLNR4CB)
		return ;

	/* Set up the ACK and CHOKE timers */
	
	cb->tack.start = Nr4acktime / MSPTICK ;
	cb->tack.func = nr4ackit ;
	cb->tack.arg = (char *)cb ;

	cb->tchoke.start = Nr4choketime / MSPTICK ;
	cb->tchoke.func = nr4unchoke ;
	cb->tchoke.arg = (char *)cb ;

	cb->rxpastwin = cb->window ;

	/* Don't actually set the timers, since this is done */
	/* in nr4sbuf */
	
	for (i = 0 ; i < cb->window ; i++) {
		t = &cb->txbufs[i].tretry ;
		t->func = nr4txtimeout ;
		t->arg = (char *)cb ;
	}
}

/* See if this control block address is valid */

int
nr4valcb(cb)
struct nr4cb *cb ;
{
	int i ;

	if (cb == NULLNR4CB)
		return 0 ;
		
	for (i = 0 ; i < NR4MAXCIRC ; i++)
		if (Nr4circuits[i].ccb == cb)
			return 1 ;

	return 0 ;
}
