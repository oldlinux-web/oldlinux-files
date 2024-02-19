/* Send and receive IP datagrams on serial lines. Compatible with SLIP
 * under Berkeley Unix.
 */
#include <stdio.h>
#include "config.h"
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "iface.h"
#include "ax25.h"
#include "ip.h"
#include "tcp.h"
#include "slhc.h"
#include "slip.h"
#include "asy.h"
#include "trace.h"
#if defined(MODEM_CALL)
#include <string.h>
#endif

extern unsigned restricted_dev;
int asy_ioctl();
int kiss_ioctl();
int slip_send();
void doslip();
int asy_output();

/* Slip level control structure */
struct slip slip[ASY_MAX];
void slip_recv();

/* Send routine for point-to-point slip
 * This is a trivial function since there is no slip link-level header
 */
/*ARGSUSED*/
int
slip_send(data,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *data;		/* Buffer to send */
struct interface *interface;	/* Pointer to interface control block */
int32 gateway;			/* Ignored (SLIP is point-to-point) */
char precedence;
char delay;
char throughput;
char reliability;
{
	register struct slip *sp;
	int type;

	if(interface == NULLIF){
		free_p(data);
		return -1;
	}
	dump(interface,IF_TRACE_OUT,TRACE_IP,data);
	sp = &slip[interface->dev];
	if (sp->vjcomp) {
		/* Attempt IP/ICP header compression */
		type = slhc_compress((struct slcompress *)sp->slcomp,&data,1);
		data->data[0] |= type;
	}
	return (*interface->raw)(interface,data);
}
/* Send a raw slip frame -- also trivial */
int
slip_raw(interface,data)
struct interface *interface;
struct mbuf *data;
{
	/* Queue a frame on the slip output queue and start transmitter */
	return slipq(interface->dev,data);
}
/* Encode a raw packet in slip framing, put on link output queue, and kick
 * transmitter
 */
static int
slipq(dev,data)
int16 dev;		/* Serial line number */
struct mbuf *data;	/* Buffer to be sent */
{
	register struct slip *sp;
	struct mbuf *slip_encode(),*bp;
	void asy_start();

	if((bp = slip_encode(data)) == NULLBUF)
		return -1;	

	sp = &slip[dev];
	enqueue(&sp->sndq,bp);
	sp->sndcnt++;
	if(sp->tbp == NULLBUF)
		asy_start(dev);
	return 0;
}
/* Start output, if possible, on asynch device dev */
static void
asy_start(dev)
int16 dev;
{
	register struct slip *sp;

	if(!stxrdy(dev))
		return;		/* Transmitter not ready */

	sp = &slip[dev];
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
/* Encode a packet in SLIP format */
static
struct mbuf *
slip_encode(bp)
struct mbuf *bp;
{
	struct mbuf *lbp;	/* Mbuf containing line-ready packet */
	register char *cp;
	char c;

	/* Allocate output mbuf that's twice as long as the packet.
	 * This is a worst-case guess (consider a packet full of FR_ENDs!)
	 */
	lbp = alloc_mbuf(2*len_mbuf(bp) + 2);
	if(lbp == NULLBUF){
		/* No space; drop */
		free_p(bp);
		return NULLBUF;
	}
	cp = lbp->data;

	/* Flush out any line garbage */
	*cp++ = FR_END;

	/* Copy input to output, escaping special characters */
	while(pullup(&bp,&c,1) == 1){
		switch(uchar(c)){
		case FR_ESC:
			*cp++ = FR_ESC;
			*cp++ = T_FR_ESC;
			break;
		case FR_END:
			*cp++ = FR_ESC;
			*cp++ = T_FR_END;
			break;
		default:
			*cp++ = c;
		}
	}
	*cp++ = FR_END;
	lbp->cnt = cp - lbp->data;
	return lbp;
}
/* Process incoming bytes in SLIP format
 * When a buffer is complete, return it; otherwise NULLBUF
 */
static
struct mbuf *
slip_decode(dev,c)
int16 dev;	/* Slip unit number */
char c;		/* Incoming character */
{
	struct mbuf *bp;
	register struct slip *sp;

	sp = &slip[dev];
	switch(uchar(c)){
	case FR_END:
		bp = sp->rbp;
		sp->rbp = NULLBUF;
		sp->rcnt = 0;
		return bp;	/* Will be NULLBUF if empty frame */
	case FR_ESC:
		sp->escaped = 1;
		return NULLBUF;
	}
	if(sp->escaped){
		/* Translate 2-char escape sequence back to original char */
		sp->escaped = 0;
		switch(uchar(c)){
		case T_FR_ESC:
			c = FR_ESC;
			break;
		case T_FR_END:
			c = FR_END;
			break;
		default:
			sp->errors++;
			break;
		}
	}
	/* We reach here with a character for the buffer;
	 * make sure there's space for it
	 */
	if(sp->rbp == NULLBUF){
		/* Allocate first mbuf for new packet */
		if((sp->rbp1 = sp->rbp = alloc_mbuf(SLIP_ALLOC)) == NULLBUF)
			return NULLBUF; /* No memory, drop */
		sp->rcp = sp->rbp->data;
	} else if(sp->rbp1->cnt == SLIP_ALLOC){
		/* Current mbuf is full; link in another */
		if((sp->rbp1->next = alloc_mbuf(SLIP_ALLOC)) == NULLBUF){
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
	*sp->rcp++ = c;
	sp->rbp1->cnt++;
	sp->rcnt++;
	return NULLBUF;
}
/* Process SLIP line I/O */
void
doslip(interface)
struct interface *interface;
{
	char c;
	struct mbuf *bp;
	int16 dev;
	int16 asy_recv();
	struct slip *sp;

	dev = interface->dev;
	if(dev == restricted_dev) return;
	sp = &slip[dev];

	/* Process any pending input */
	while(asy_recv(dev,&c,1) != 0) {
		if((bp = slip_decode(dev,c)) == NULLBUF)
			continue;

/*
		if (sp->iface->trace & IF_TRACE_RAW)
			raw_dump(sp->iface,IF_TRACE_IN,bp);
*/
		if (sp->vjcomp) {
			if ((c = bp->data[0]) & SL_TYPE_COMPRESSED_TCP) {
				if ( slhc_uncompress((struct slcompress *)sp->slcomp, &bp) <= 0 ) {
					free_p(bp);
					sp->errors++;
					continue;
				}
			} else if (c >= SL_TYPE_UNCOMPRESSED_TCP) {
				bp->data[0] &= 0x4f;
				if ( slhc_remember((struct slcompress *)sp->slcomp, &bp) <= 0 ) {
					free_p(bp);
					sp->errors++;
					continue;
				}
			}
		}
		(*slip[dev].recv)(interface,bp);
	}
	/* Kick the transmitter if it's idle */
	if(stxrdy(dev))
		asy_start(dev);
}
/* Unwrap incoming SLIP packets -- trivial operation since there's no
 * link level header
 */
void
slip_recv(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	int ip_route();

	/* By definition, all incoming packets are "addressed" to us */
	dump(interface,IF_TRACE_IN,TRACE_IP,bp);
	ip_route(bp,0);
}

#if defined(MODEM_CALL)

void check_time(),keep_things_going();

static char *ex="r\015t\011s n\012E\004b\010\\\\N\000";
static struct timer ar;
static int debug;

static
int m_send(dev, a)
unsigned dev;
char *a;
{
	unsigned l;
	char *ss, *cp, *pt;

	ss=pt=a;
	l=0;
	while(*a) {
		if(*a=='\\') {
			a++;
			if(*a=='d') {
				asy_output(dev,ss,l);
				l=0;
				a++;
				ss=pt=a;
				set_timer(&ar,1000);
				start_timer(&ar);
	    			while(ar.state == TIMER_RUN)
					keep_things_going();
				continue;
			}
			if((cp = strchr(ex,*a)) != NULL) {
				*pt++ = *(cp+1);
				l++;
				a++;
				continue;
			}
		} /* if(*a=='\\') */
		*pt++ = *a++;
		l++;
	}
	asy_output(dev,ss,l);
} /* m_send */

/******************************************************************************
 * returns the character position of the substring pat within the string
 *  src if the substring exists, 0 if the substring does not exist.
 *  the first character position is considered to be 1, not 0 which
 *  is the first position within the array.
 */
static
int xinstr(src,pat)
char *src,*pat;
{
	register char *s_src, *s_pat;
	int rtn;

	s_src=src;
	s_pat=pat;
	rtn=1;
	while(*s_src) {
		while((*s_pat) && (*s_src==*s_pat)) { s_src++; s_pat++; }
		if(!(*s_pat)) return(rtn);
		rtn++; s_pat=pat; s_src=(++src);
	}
	return(0);
} /* xinstr */

static
int m_expect(dev, a)
unsigned dev;
char *a;
{
	char *str, *pstr, c;
	int tot;

	if(*a=='\\') return 1;
/*
 * make room for the received string
 */
        if((str=malloc(5000)) == NULL) {
		printf("No room for malloc: m_expect\n");
		fflush(stdout);
		return -1;
	}
/*
 * a single backspace means don't wait for anything
 */
	pstr=str;
	*pstr='\0';
	tot=0;
	set_timer(&ar,30000);
	start_timer(&ar);
	while (!xinstr(str,a)) {
		while(asy_recv(dev,&c,1)) { 
/*
 * ignore incoming nulls
 */
			if(c) {
				*(pstr++)=c;
				*pstr='\0';
				if(++tot > 4995) {
					if(debug) printf("%s",str);fflush(stdout);
					free(str);
					return -1;
				}
			}
		}
		check_time();
		if(ar.state != TIMER_RUN) {
			free(str);
			if(debug) printf("%s",str);fflush(stdout);
			return -1;
		}
	}
	if(debug) printf("%s",str);fflush(stdout);
	free(str);
	return 1;
} /* m_expect */

int modem_init(dev, argc, argv)
unsigned dev;
int argc;
char **argv;
{
	int i;
	i=0;
	debug = 0;
	if(argv[0][0]=='-') {
		debug=1;
		i++;
		argc--;
	}
	while(argc) {
		if(debug) {
			printf("\nI'm sending  : '%s'\n",argv[i]);
			fflush(stdout);
		}
		if(debug && argc>1) {
			printf("I'm expecting: '%s'\n",argv[1+i]);
			fflush(stdout);
		}
		if(m_send(dev,argv[i++]) == -1) return -1;
		if(--argc>0) {
			if(m_expect(dev,argv[i++]) == -1) return -1;
			argc--;
		}
	}
	return 1;
} /* modem_init */
#endif /* MODEM_CALL */

doslipstat()
{
	int16 dev;
	int16 asy_recv();
	struct slip *sp;
	register struct interface *ifp;

	for(ifp=ifaces;ifp != NULLIF;ifp = ifp->next){
	  dev = ifp->dev;
	  if (dev >= ASY_MAX || slip[dev].recv == 0)
	    continue;
	  sp = &slip[dev];
	  printf("%s %d Errors\n",ifp->name, sp->errors);
	  if (sp->vjcomp) {
	    printf("  Input:"); slhc_i_status(sp->slcomp);
	    printf("  Output:"); slhc_o_status(sp->slcomp);
	  }
	}
}

