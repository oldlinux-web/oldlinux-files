/* Send and receive IP datagrams on serial lines. Compatible with SL/FP
 * as used with the Merit Network and MIT.
 */
#include "config.h"
#ifdef SLFP

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "ip.h"
#include "slfp.h"

#ifdef UNIX	/* BSD or SYS5 */
#include "unix.h"
#endif

#include "asy.h"
#ifdef MSDOS
#include "8250.h"
#endif
#include "trace.h"

int asy_ioctl();
int slfp_send();
int doslfp();
int asy_output();

/* SL/FP level control structure */
struct slfp slfp[ASY_MAX];
char slfp_ack[ACK_LEN] = { SLFP_ACK } ;
char slfp_req[REQ_LEN] = { SLFP_REQ } ;
char ip_hdr[HDR_LEN] = { 2, 1, 0, 0 } ;	/* IP Packet Header */
char ar_hdr[HDR_LEN] = { 2, 3, 0, 0 } ;	/* "Addr Req" Packet Header */
struct interface asy_interface =	/* Fake interface for "dump" proc */
	{ NULLIF, "asy" } ;		/* Name of "asy" interface */

/* Routine to Initialize the Async line for SL/FP processing.
 * Mostly involves requesting the IP Address for this host.
 */
int
slfp_init(interface,modem_cmd)
struct interface *interface ;
char *modem_cmd ;		/* optional command to Modem */
{
    register struct slfp *sp;
    register struct timer *ar ;
    char *modem_line ;
    int i ;

    sp = &slfp[interface->dev];
    ar = &sp->ar_timer ;
 
    slfp[interface->dev].req_pending = 0 ;

    /* If a Modem Command is present, send it and wait for Connection */
    if (modem_cmd != NULLCHAR) {
	char c; c='\r';
	modem_line = (char *)malloc(strlen(modem_cmd)+2) ;
	if (modem_line == NULLCHAR)
	    return -1 ;
	strcpy(modem_line, modem_cmd);
	strcat(modem_line, "\r") ;
	asy_output(interface->dev,&c,1);	/* Wake up modem */
	set_timer(ar,500);
	start_timer(ar);
	while(ar->state == TIMER_RUN)
	    keep_things_going();
	asy_output(interface->dev,modem_line,strlen(modem_line));
	free(modem_line);
	set_timer(ar, 30000) ;	/* Wait up to 30 seconds for Connection */
	start_timer(ar);
    } else
	set_timer(ar, 500) ;	/* Wait half a second before sending REQs */

    if (ip_addr == 0) {		/* Ask network for my IP address... */
	sp->ar_pending = 1 ;

    /* Request an Address up to 4 times (every 10 seconds) before giving up */
	for (i=0; i<4; i++) {
	    start_timer(ar) ;
	    while (sp->ar_pending && (ar->state == TIMER_RUN)) {
		keep_things_going() ;	/* Can't return until timeout or addr */
		if(kbread()==(-2))		/* Hit the Escape key */
		    return(-1);
	    }
	    if (!sp->ar_pending) {
		return 0 ;
	    }
	    slfp_send(NULLBUF, interface, 0L, 0, 0, 0, 0) ;
	    set_timer(ar, 10000) ;	/* Wait up to 10 seconds for IP Addr */
	}

	sp->ar_pending = 0 ;
	return -1 ;
    } else
	return 0;
}

/* Send routine for point-to-point slfp
 * This is a trivial function since slfp_encode adds the link-level header
 */
int
slfp_send(bp,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *bp;		/* Buffer to send */
struct interface *interface;	/* Pointer to interface control block */
int32 gateway;
char precedence;
char delay;
char throughput;
char reliability;
{
	if(interface == NULLIF){
		free_p(bp);
		return;
	}
	dump(interface,IF_TRACE_OUT,TRACE_IP,bp);
	(*interface->raw)(interface,bp);
}
/* Send a raw slfp frame -- also trivial */
slfp_raw(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	/* Make "asy" interface a shadow of the SLFP interface */
	asy_interface.trace = interface->trace ;

	/* Queue a frame on the slfp output queue and start transmitter */
	slfpq(interface->dev,bp);
}
/* Encode a raw packet in slfp framing, put on link output queue, and kick
 * transmitter
 */
static
slfpq(dev,bp)
int16 dev;		/* Serial line number */
struct mbuf *bp;	/* Buffer to be sent */
{
	register struct slfp *sp;
	struct mbuf *slfp_encode();

	if((bp = slfp_encode(dev,bp)) == NULLBUF)
		return;	

	sp = &slfp[dev];
	enqueue(&sp->sndq,bp);
	dump(&asy_interface,IF_TRACE_OUT,TRACE_SLFP,bp);
	sp->sndcnt++;
	if(sp->tbp == NULLBUF)
		slfp_asy_start(dev);
}

/* Handle REQ-ACK Timer expiration
 */
void
slfp_req_notify(dev)
int16 dev;
{
    register struct slfp *sp;
    register struct timer *rt ;		/* Timer for REQ-ACK negotiation */
    struct mbuf *bp ;

    sp = &slfp[dev];
    rt = &(sp->req_timer) ;
    if (sp->reqcnt++ >= 10) {
	sp->tbp = NULLBUF ;
	sp->req_pending = 0 ;
	bp = dequeue(&sp->sndq);
	sp->sndcnt--;
	free_p(bp) ;
    }
    else {
	set_timer(rt,2000) ;		/* 2-Second Timeout for ACK */
	start_timer(rt) ;
	asy_output(dev, slfp_req, REQ_LEN) ;
    }
}

/* Start output, if possible, on asynch device dev */
static
slfp_asy_start(dev)
int16 dev;
{
	register struct slfp *sp;
	register struct timer *rt ;	/* Timer for REQ-ACK negotiation */
	struct mbuf *bp ;

	if(!stxrdy(dev))
		return;		/* Transmitter not ready */

	sp = &slfp[dev];
	bp = sp->tbp ;
	if(bp != NULLBUF){
		/* transmission just completed */
		free_p(bp) ;
		sp->tbp = NULLBUF;
	}
	if(sp->sndq == NULLBUF)
		return;	/* No work */

	rt = &(sp->req_timer) ;
	if (sp->req_pending)
	    return ;
	sp->reqcnt = 0 ;
	sp->req_pending = 1 ;
	rt->func = slfp_req_notify ;
	rt->arg = (char *)dev ;
	set_timer(rt,2000) ;		/* 2-Second Timeout for ACK */
	start_timer(rt) ;
	asy_output(dev, slfp_req, REQ_LEN) ;
}
/* Encode a packet in SL/FP format */
static
struct mbuf *
slfp_encode(dev,bp)
int16 dev;		/* Serial line number */
struct mbuf *bp;
{
	struct mbuf *lbp;	/* Mbuf containing line-ready packet */
	register char *cp;
	char c;

	/* Allocate output mbuf that's twice as long as the packet.
	 * This is a worst-case guess (consider a packet full of SLFP_ENDs!)
	 */
	lbp = alloc_mbuf(HDR_LEN + 2*len_mbuf(bp) + 2);
	if(lbp == NULLBUF){
		/* No space; drop */
		free_p(bp);
		return NULLBUF;
	}
	cp = lbp->data;

	/* Prefix packet with the Correct Link-Level Header */
	if (slfp[dev].ar_pending)
	    memcpy(cp, ar_hdr, HDR_LEN) ;
	else
	    memcpy(cp, ip_hdr, HDR_LEN) ;
	cp += HDR_LEN ;

	/* Copy input to output, escaping special characters */
	while(pullup(&bp,&c,1) == 1){
		switch(c & 0xff){
		case SLFP_ESC:
			*cp++ = SLFP_ESC;
			*cp++ = SLFP_ESC - SLFP_ESC;
			break;
		case SLFP_END:
			*cp++ = SLFP_ESC;
			*cp++ = SLFP_END - SLFP_ESC;
			break;
		case SLFP_ACK:
			*cp++ = SLFP_ESC;
			*cp++ = SLFP_ACK - SLFP_ESC;
			break;
		case SLFP_REQ:
			*cp++ = SLFP_ESC;
			*cp++ = SLFP_REQ - SLFP_ESC;
			break;
		default:
			*cp++ = c;
		}
	}
	*cp++ = SLFP_END;
	lbp->cnt = cp - lbp->data;
	return lbp;
}

#ifdef	MSDOS
/* Invoked when SLFP_REQ is received during xmit of outgoing packet.
 * This allows immediate reception of packet from SCP, rather than
 * forcing it to buffer it until we finish sending the outgoing packet
 */
static
unsigned
slfp_urgent(dev)
int16 dev;	/* SL/FP unit number */
{
    register struct dma *dp ;

    dp = &asy[dev].dma ;
    if (dp->last_octet == SLFP_ESC)
	return 256 ;
    else {
	asy[dev].urgent = NULLCHAR ;
	return SLFP_ACK ;
    }
}
#endif

void
hndl_rcvd_req(dev, sp)
int16 dev;	/* SL/FP unit number */
register struct slfp *sp;
{
    char i_state ;

    if (sp->reqd) { /* REQ before rcv'g END of last Packet! */
	sp->missed_ends++ ;
	free_p(sp->rbp);	/* throw away current packet */
	sp->rbp = NULLBUF;
	sp->rcnt = 0;
    }

    sp->reqd = 1 ;
    i_state = disable() ;
#ifdef	MSDOS
    if (asy[dev].dma.flags)
	asy[dev].urgent = slfp_urgent ;
    else
#endif
	asy_output(dev, slfp_ack, ACK_LEN) ;
    restore(i_state) ;
}

void
hndl_rcvd_ack(dev, sp)
int16 dev;	/* SL/FP unit number */
register struct slfp *sp;
{
    char i_state ;

    i_state = disable() ;
    if (sp->req_pending == 0) {
        sp->false_acks++ ;
        restore(i_state) ;
        return ;
    }
    sp->req_pending = 0 ;
    stop_timer(&(sp->req_timer)) ;
    restore(i_state) ;
    sp->tbp = dequeue(&sp->sndq);
    sp->sndcnt--;
    asy_output(dev,sp->tbp->data,sp->tbp->cnt);
}

/* Process incoming bytes in SL/FP format
 * When a buffer is complete, return it; otherwise NULLBUF
 */
static
struct mbuf *
slfp_decode(dev,c)
int16 dev;	/* SL/FP unit number */
char c;		/* Incoming character */
{
	struct mbuf *bp;
	register struct slfp *sp;
	unsigned char uc ;

	sp = &slfp[dev];

	uc = c & 0xff;
	if (uc == SLFP_REQ) {
	    hndl_rcvd_req(dev, sp) ;
	    return NULLBUF ;
	}
	if (uc == SLFP_ACK) {
	    hndl_rcvd_ack(dev, sp) ;
	    return NULLBUF ;
	}
	if (sp->reqd == 0) {
	    return NULLBUF ;
	}

	switch(uc){
	case SLFP_END:
		sp->reqd = 0 ;
		/* Kick upstairs */
		bp = sp->rbp;
		sp->rbp = NULLBUF;
		sp->rcnt = 0;
		return bp;
		break ;

	case SLFP_ESC:
		sp->escaped = 1;
		return NULLBUF;
	}
	if(sp->escaped){
		sp->escaped = 0;
		uc += SLFP_ESC;
		switch(uc){
		case SLFP_ESC:
		case SLFP_REQ:
		case SLFP_ACK:
		case SLFP_END:
			break;
		default:
			uc -= SLFP_ESC;
			sp->bad_esc++;
			sp->errors++;
		}
	}
	/* We reach here with a character for the buffer;
	 * make sure there's space for it
	 */
	if(sp->rbp == NULLBUF){
		/* Allocate first mbuf for new packet */
		if((sp->rbp1 = sp->rbp = alloc_mbuf(SLFP_ALLOC)) == NULLBUF)
			return NULLBUF; /* No memory, drop */
		sp->rcp = sp->rbp->data;
	} else if(sp->rbp1->cnt == SLFP_ALLOC){
		/* Current mbuf is full; link in another */
		if((sp->rbp1->next = alloc_mbuf(SLFP_ALLOC)) == NULLBUF){
			/* No memory, drop whole thing */
			free_p(sp->rbp);
			sp->rbp = NULLBUF;
			sp->rcnt = 0;
			return NULLBUF;
		}
		sp->rbp1 = sp->rbp1->next;
		sp->rcp = sp->rbp1->data;
	}
	/* Store the character, increment fragment and total
	 * byte counts
	 */
	*sp->rcp++ = uc;
	sp->rbp1->cnt++;
	sp->rcnt++;
	return NULLBUF;
}
/* Process SL/FP line I/O */
int
doslfp(interface)
struct interface *interface;
{
	char c;
	struct mbuf *bp, *cp=NULLBUF;
	int16 dev;
	int16 asy_recv();

	dev = interface->dev;
	/* Process any pending input */
	while(asy_recv(dev,&c,1) != 0) {
		if((bp = slfp_decode(dev,c)) != NULLBUF) {
			(*slfp[dev].recv)(interface,bp);
		}
	}
	if (cp != NULLBUF)
		(*slfp[dev].recv)(interface,cp);
	
	/* Kick the transmitter if it's idle */
	if(stxrdy(dev))
		slfp_asy_start(dev);
}

/* Handle Address Reply packets
 */
void
addr_reply(dev,bp)
int16 dev;
struct mbuf *bp;
{
    if (len_mbuf(bp) != 4) { /* Invalid Address Response */
	free_p(bp) ;
	return ;
    }
    if (!slfp[dev].ar_pending) {
	free_p(bp) ;
	return ;
    }
    stop_timer(&slfp[dev].ar_timer) ;
    slfp[dev].ar_pending = 0 ;
    ip_addr = (unsigned char)(*bp->data++) ;
    ip_addr <<= 8 ;
    ip_addr |= (unsigned char)(*bp->data++) ;
    ip_addr <<= 8 ;
    ip_addr |= (unsigned char)(*bp->data++) ;
    ip_addr <<= 8 ;
    ip_addr |= (unsigned char)(*bp->data++) ;
    free_p(bp) ;
}

/* Unwrap incoming SL/FP packets -- use link level header to determine
 * how to handle packet
 */
slfp_recv(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	void ip_route();

	dump(interface,IF_TRACE_IN,TRACE_SLFP,bp);
	if (len_mbuf(bp) < HDR_LEN) {
	    free_p(bp) ;
	    return ;
	}
	if (memcmp(ip_hdr, bp->data, HDR_LEN) == 0) {
	    bp->data += HDR_LEN ;
	    bp->cnt -= HDR_LEN ;
	    dump(interface,IF_TRACE_IN,TRACE_IP,bp);
	    ip_route(bp,0); /* By def'n, all inbound packets are addr'd to us */
	}
	else if (memcmp(ar_hdr, bp->data, HDR_LEN) == 0) {
	    bp->data += HDR_LEN ;
	    bp->cnt -= HDR_LEN ;
	    dump(interface,IF_TRACE_IN,TRACE_IP,bp);
	    addr_reply(interface->dev,bp);
	} else {
	    dump(interface,IF_TRACE_IN,TRACE_IP,bp);
	    free_p(bp) ;
	}
}

int
slfp_dump(bpp, check)
struct mbuf **bpp;
int check ;
{
	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return 0;	

	printf("SLFP: ");
	/* Sneak peek at IP header and find length */
	if (len_mbuf(*bpp) < HDR_LEN) {
		printf("packet too short!\n");
		return 0;
	}

	if (memcmp(ip_hdr, (*bpp)->data, HDR_LEN) == 0)
	    printf("IP Packet\n") ;
	else if (memcmp(ar_hdr, (*bpp)->data, HDR_LEN) == 0)
	    printf("Addr Req Packet\n") ;
	else
	    printf("bad header\n") ;

	return 0 ;
}

#endif /* SLFP */
