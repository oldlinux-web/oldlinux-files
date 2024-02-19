/* This module implements the serial line framing method used by
 * net/rom nodes.  This allows the net/rom software to talk to
 * an actual net/rom over its serial interface, which is useful
 * if we want to do packet switching for multi-line wormholes.
 * Dan Frank, W9NK
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "ax25.h"
#include "nrs.h"
#include "asy.h"
#include "trace.h"

int asy_output();

/* control structures, sort of overlayed on async control blocks */
struct nrs nrs[ASY_MAX];

/* Send a raw net/rom serial frame */
nrs_raw(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	dump(interface,IF_TRACE_OUT,TRACE_AX25,bp) ;
	
	/* Queue a frame on the output queue and start transmitter */
	nrsq(interface->dev,bp);
}

/* Encode a raw packet in net/rom framing, put on link output queue, and kick
 * transmitter
 */
static
nrsq(dev,bp)
int16 dev;		/* Serial line number */
struct mbuf *bp;	/* Buffer to be sent */
{
	register struct nrs *sp;
	struct mbuf *nrs_encode();

	if((bp = nrs_encode(bp)) == NULLBUF)
		return;	

	sp = &nrs[dev];
	enqueue(&sp->sndq,bp);
	sp->sndcnt++;
	if(sp->tbp == NULLBUF)
		nrasy_start(dev);
}

/* Start output, if possible, on asynch device dev */
static
nrasy_start(dev)
int16 dev;
{
	register struct nrs *sp;

	if(!stxrdy(dev))
		return;		/* Transmitter not ready */

	sp = &nrs[dev];
	if(sp->tbp != NULLBUF){
		/* transmission just completed */
		free_p(sp->tbp);
		sp->tbp = NULLBUF;
	}
	if(sp->sndq == NULLBUF)
		return;	/* No work */

	sp->tbp = dequeue(&sp->sndq);
	sp->sndcnt--;
	asy_output(dev,sp->tbp->data,sp->tbp->cnt);
}

/* Encode a packet in net/rom serial format */
static
struct mbuf *
nrs_encode(bp)
struct mbuf *bp;
{
	struct mbuf *lbp;	/* Mbuf containing line-ready packet */
	register char *cp;
	char c;
	unsigned char csum = 0 ;

	/* Allocate output mbuf that's twice as long as the packet.
	 * This is a worst-case guess (consider a packet full of STX's!)
	 * Add five bytes for STX, ETX, checksum, and two nulls.
	 */
	lbp = alloc_mbuf(2*len_mbuf(bp) + 5);
	if(lbp == NULLBUF){
		/* No space; drop */
		free_p(bp);
		return NULLBUF;
	}
	cp = lbp->data;

	*cp++ = STX ;

	/* Copy input to output, escaping special characters */
	while(pullup(&bp,&c,1) == 1){
		switch(uchar(c)){
		case STX:
		case ETX:
		case DLE:
			*cp++ = DLE;
			/* notice drop through to default */
		default:
			*cp++ = c;
		}
		csum += uchar(c) ;
	}
	*cp++ = ETX;
	*cp++ = csum ;
	*cp++ = NUL ;
	*cp++ = NUL ;
	
	lbp->cnt = cp - lbp->data;
	return lbp;
}
/* Process incoming bytes in net/rom serial format
 * When a buffer is complete, return it; otherwise NULLBUF
 */
static
struct mbuf *
nrs_decode(dev,c)
int16 dev;	/* net/rom unit number */
char c;		/* Incoming character */
{
	struct mbuf *bp;
	register struct nrs *sp;

	sp = &nrs[dev];
	switch(sp->state) {
		case NRS_INTER:
			if (uchar(c) == STX) {	/* look for start of frame */
				sp->state = NRS_INPACK ;	/* we're in a packet */
				sp->csum = 0 ;				/* reset checksum */
			}
			return NULLBUF ;
			break ;	/* just for yucks */
		case NRS_CSUM:
			bp = sp->rbp ;
			sp->rbp = NULLBUF ;
			sp->rcnt = 0 ;
			sp->state = NRS_INTER ;	/* go back to inter-packet state */
			if (sp->csum == uchar(c)) {
				sp->packets++ ;
				return bp ;
			}
			else {
				free_p(bp) ;	/* drop packet with bad checksum */
				sp->errors++ ;	/* increment error count */
				return NULLBUF ;
			}
			break ;
		case NRS_ESCAPE:
			sp->state = NRS_INPACK ;	/* end of escape */
			break ;			/* this will drop through to char processing */
		case NRS_INPACK:
			switch (uchar(c)) {
				/* If we see an STX in a packet, assume that previous */
				/* packet was trashed, and start a new packet */
				case STX:
					free_p(sp->rbp) ;
					sp->rbp = NULLBUF ;
					sp->rcnt = 0 ;
					sp->csum = 0 ;
					sp->errors++ ;
					return NULLBUF ;
					break ;
				case ETX:
					sp->state = NRS_CSUM ;	/* look for checksum */
					return NULLBUF ;
					break ;
				case DLE:
					sp->state = NRS_ESCAPE ;
					return NULLBUF ;
					break ;
			}
	}
	/* If we get to here, it's with a character that's part of the packet.
	 * Make sure there's space for it.
	 */
	if(sp->rbp == NULLBUF){
		/* Allocate first mbuf for new packet */
		if((sp->rbp1 = sp->rbp = alloc_mbuf(NRS_ALLOC)) == NULLBUF) {
			sp->state = NRS_INTER ;
			return NULLBUF; /* No memory, drop */
		}
		sp->rcp = sp->rbp->data;
	} else if(sp->rbp1->cnt == NRS_ALLOC){
		/* Current mbuf is full; link in another */
		if((sp->rbp1->next = alloc_mbuf(NRS_ALLOC)) == NULLBUF){
			/* No memory, drop whole thing */
			free_p(sp->rbp);
			sp->rbp = NULLBUF;
			sp->rcnt = 0;
			sp->state = NRS_INTER ;
			return NULLBUF;
		}
		sp->rbp1 = sp->rbp1->next;
		sp->rcp = sp->rbp1->data;
	}
	/* Store the character, increment fragment and total
	 * byte counts
	 */
	*sp->rcp++ = c;
	sp->rbp1->cnt++;
	sp->rcnt++;
	sp->csum += uchar(c) ;	/* add to checksum */
	return NULLBUF;
}

/* Process net/rom serial line I/O */
void
nrs_recv(interface)
struct interface *interface;
{
	char c;
	struct mbuf *bp;
	int16 dev;
	int16 asy_recv();
	int ax_recv() ;

	dev = interface->dev;
	/* Process any pending input */
	while(asy_recv(dev,&c,1) != 0)
		if((bp = nrs_decode(dev,c)) != NULLBUF) {
			dump(interface,IF_TRACE_IN,TRACE_AX25,bp) ;
			ax_recv(interface,bp);
		}

	/* Kick the transmitter if it's idle */
	if(stxrdy(dev))
		nrasy_start(dev);
}

/* donrstat:  display status of active net/rom serial interfaces */
/*ARGSUSED*/
donrstat(argc,argv)
int argc ;
char *argv[] ;
{
	register struct nrs *np ;
	register int i ;

	printf("Interface  SndQ  RcvB  NumReceived  CSumErrors\n") ;

	for (i = 0, np = nrs ; i < ASY_MAX ; i++, np++)
		if (np->iface != NULLIF)
			printf(" %8s   %3d  %4d   %10lu  %10lu\n",
					np->iface->name, np->sndcnt, np->rcnt,
					np->packets, np->errors) ;

	return 0 ;
}
