#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "lapb.h"
#include "netuser.h"
#include "ftp.h"
#include "telnet.h"
#include "iface.h"
#include "finger.h"
#include <ctype.h>
#ifdef	UNIX
#undef	toupper
#undef	tolower
#include <string.h>
#include <memory.h>
#endif

struct ax25_cb *ax25_cb[NHASH];

/* Default AX.25 parameters */
int16 t1init = 10000 / MSPTICK;	/* FRACK of 10 seconds */
int16 t2init = 1000 / MSPTICK;	/* 1 sec acknowledgment delay */
int16 t3init = 0 / MSPTICK;	/* No keep-alive polling */
int16 maxframe = 1;		/* Stop and wait */
int16 n2 = 10;			/* 10 retries */
int16 axwindow = 2048;		/* 2K incoming text before RNR'ing */
int16 paclen = 256;		/* 256-byte I fields */
int16 pthresh = 128;		/* Send polls for packets larger than this */

/* Address hash function. Exclusive-ORs each byte, ignoring
 * such insignificant, annoying things as E and H bits
 */
static
int16
ax25hash(s)
struct ax25_addr *s;
{
	register char x;
	register int i;
	register char *cp;

	x = 0;
	cp = s->call;
	for(i=ALEN; i!=0; i--)
		x ^= *cp++ & 0xfe;
	x ^= s->ssid & SSID;
	return uchar(x) % NHASH;
}
/* Look up entry in hash table */
struct ax25_cb *
find_ax25(addr)
register struct ax25_addr *addr;
{
	int16 hashval;
	register struct ax25_cb *axp;
	char i_state;

	/* Find appropriate hash chain */
	hashval = ax25hash(addr);

	/* Search hash chain */
	i_state = disable();
	for(axp = ax25_cb[hashval]; axp != NULLAX25; axp = axp->next){
		if(addreq(&axp->addr.dest,addr)){
			restore(i_state);
			return axp;
		}
	}
	restore(i_state);
	return NULLAX25;
}

/* Remove address entry from hash table */
del_ax25(axp)
register struct ax25_cb *axp;
{
	int16 hashval;
	char i_state;

	if(axp == NULLAX25)
		return;
	/* Remove from hash header list if first on chain */
	hashval = ax25hash(&axp->addr.dest);
	i_state = disable();

	/* Remove from chain list */
	if(ax25_cb[hashval] == axp)
		ax25_cb[hashval] = axp->next;
	if(axp->prev != NULLAX25)
		axp->prev->next = axp->next;
	if(axp->next != NULLAX25)
		axp->next->prev = axp->prev;

	/* Timers should already be stopped, but just in case... */
	stop_timer(&axp->t1);
	stop_timer(&axp->t2);
	stop_timer(&axp->t3);

	/* Free allocated resources */
	free_q(&axp->txq);
	free_q(&axp->rxasm);
	free_q(&axp->rxq);
	free((char *)axp);

	restore(i_state);
}

/* Create an ax25 control block. Allocate a new structure, if necessary,
 * and fill it with all the defaults. The caller
 * is still responsible for filling in the reply address
 */
struct ax25_cb *
cr_ax25(addr)
struct ax25_addr *addr;
{
	void recover(),send_ack(),pollthem(),ax_incom();
	register struct ax25_cb *axp;
	int16 hashval;
	char i_state;
#ifdef SID2
	void mbx_state();
#endif

	if(addr == NULLAXADDR)
		return NULLAX25;

	if((axp = find_ax25(addr)) == NULLAX25){
		/* Not already in table; create an entry
		 * and insert it at the head of the chain
		 */
		/* Find appropriate hash chain */
		hashval = ax25hash(addr);
		i_state = disable();
		axp = (struct ax25_cb *)calloc(1,sizeof(struct ax25_cb));
		if(axp == NULLAX25)
			return NULLAX25;
		/* Insert at beginning of chain */
		axp->prev = NULLAX25;
		axp->next = ax25_cb[hashval];
		if(axp->next != NULLAX25)
			axp->next->prev = axp;
		ax25_cb[hashval] = axp;
		restore(i_state);
	}
	axp->maxframe = maxframe;
	axp->window = axwindow;
	axp->paclen = paclen;
	axp->proto = V2;	/* Default, can be changed by other end */
	axp->pthresh = pthresh;
	axp->n2 = n2;
	axp->t1.start = t1init;
	axp->t1.func = recover;
	axp->t1.arg = (char *)axp;

	axp->t2.start = t2init;
	axp->t2.func = send_ack;
	axp->t2.arg = (char *)axp;

	axp->t3.start = t3init;
	axp->t3.func = pollthem;
	axp->t3.arg = (char *)axp;

#ifdef SID2
	axp->s_upcall = mbx_state;
#endif
	axp->r_upcall = ax_incom;
	return axp;
}

/*
 * setcall - convert callsign plus substation ID of the form
 * "KA9Q-0" to AX.25 (shifted) address format
 *   Address extension bit is left clear
 *   Return -1 on error, 0 if OK
 */
int
setcall(out,call)
struct ax25_addr *out;
char *call;
{
	int csize;
	unsigned ssid;
	register int i;
	register char *cp,*dp;
	char c;

	if(out == (struct ax25_addr *)NULL || call == NULLCHAR || *call == '\0'){
		return -1;
	}
	/* Find dash, if any, separating callsign from ssid
	 * Then compute length of callsign field and make sure
	 * it isn't excessive
	 */
	dp = index(call,'-');
	if(dp == NULLCHAR)
		csize = strlen(call);
	else
		csize = dp - call;
	if(csize > ALEN)
		return -1;
	/* Now find and convert ssid, if any */
	if(dp != NULLCHAR){
		dp++;	/* skip dash */
		ssid = atoi(dp);
		if(ssid > 15)
			return -1;
	} else
		ssid = 0;
	/* Copy upper-case callsign, left shifted one bit */
	cp = out->call;
	for(i=0;i<csize;i++){
		c = *call++;
		if(islower(c))
			c = toupper(c);
		*cp++ = c << 1;
	}
	/* Pad with shifted spaces if necessary */
	for(;i<ALEN;i++)
		*cp++ = ' ' << 1;
	
	/* Insert substation ID field and set reserved bits */
	out->ssid = 0x60 | (ssid << 1);
	return 0;
}
/* Set a digipeater string in an ARP table entry */
setpath(out,in,cnt)
char *out;	/* Target char array containing addresses in net form */
char *in[];	/* Input array of tokenized callsigns in ASCII */
int cnt;	/* Number of callsigns in array */
{
	struct ax25_addr addr;
	char *putaxaddr();

	if(cnt == 0)
		return;
	while(cnt-- != 0){
		setcall(&addr,*in++);
		addr.ssid &= ~E;
		out = putaxaddr(out,&addr);
	}
	out[-1] |= E;
}
addreq(a,b)
register struct ax25_addr *a,*b;
{
	if(memcmp(a->call,b->call,ALEN) != 0)
		return 0;
	if((a->ssid & SSID) != (b->ssid & SSID))
		return 0;
	return 1;
}
/* Convert encoded AX.25 address to printable string */
pax25(e,addr)
char *e;
struct ax25_addr *addr;
{
	register int i;
	char c,*cp;
#ifdef	BSD
	char *sprintf();
#endif

	cp = addr->call;
	for(i=ALEN;i != 0;i--){
		c = (*cp++ >> 1) & 0x7f;
		if(c == ' ')
			break;
		*e++ = c;
	}
	if ((addr->ssid & SSID) != 0)
		sprintf(e,"-%d",(addr->ssid >> 1) & 0xf);	/* ssid */
	else
		*e = 0;
}
/* Print a string of AX.25 addresses in the form
 * "KA9Q-0 [via N4HY-0,N2DSY-2]"
 * Designed for use by ARP - arg is a char string
 */
psax25(e,addr)
register char *e;
register char *addr;
{
	int i;
	struct ax25_addr axaddr;
	char tmp[16];
	char *getaxaddr();

	e[0] = '\0';	/* Give strcat a staritng point */
	for(i=0;;i++){
		/* Create local copy in host-format structure */
		addr = getaxaddr(&axaddr,addr);

		/* Create ASCII representation and append to output */
		pax25(tmp,&axaddr);
		strcat(e,tmp);

		if(axaddr.ssid & E)
			break;
		if(i == 0)
			strcat(e," via ");
		else
			strcat(e,",");
		/* Not really necessary, but speeds up subsequent strcats */
		e += strlen(e);
	}
}
char *
getaxaddr(ap,cp)
register struct ax25_addr *ap;
register char *cp;
{
	memcpy(ap->call,cp,ALEN);
	cp += ALEN;
	ap->ssid = *cp++;
	return cp;
}
char *
putaxaddr(cp,ap)
register char *cp;
register struct ax25_addr *ap;
{
	memcpy(cp,ap->call,ALEN);
	cp += ALEN;
	*cp++ = ap->ssid;
	return cp;
}

/* Convert a host-format AX.25 header into a mbuf ready for transmission */
struct mbuf *
htonax25(hdr,data)
register struct ax25 *hdr;
struct mbuf *data;
{
	struct mbuf *bp;
	register char *cp;
	register int16 i;

	if(hdr == (struct ax25 *)NULL || hdr->ndigis > MAXDIGIS)
		return NULLBUF;

	/* Allocate space for return buffer */
	i = AXALEN * (2 + hdr->ndigis);
	if((bp = pushdown(data,i)) == NULLBUF)
		return NULLBUF;

	/* Now convert */
	cp = bp->data;

	hdr->dest.ssid &= ~E;	/* Dest E-bit is always off */
	/* Encode command/response in C bits */
	switch(hdr->cmdrsp){
	case COMMAND:
		hdr->dest.ssid |= C;
		hdr->source.ssid &= ~C;
		break;
	case RESPONSE:
		hdr->dest.ssid &= ~C;
		hdr->source.ssid |= C;
		break;
	default:
		hdr->dest.ssid &= ~C;
		hdr->source.ssid &= ~C;
		break;
	}
	cp = putaxaddr(cp,&hdr->dest);

	/* Set E bit on source address if no digis */
	if(hdr->ndigis == 0){
		hdr->source.ssid |= E;
		putaxaddr(cp,&hdr->source);
		return bp;
	}
	hdr->source.ssid &= ~E;
	cp = putaxaddr(cp,&hdr->source);

	/* All but last digi get copied with E bit off */
	for(i=0; i < hdr->ndigis - 1; i++){
		hdr->digis[i].ssid &= ~E;
		cp = putaxaddr(cp,&hdr->digis[i]);
	}
	hdr->digis[i].ssid |= E;
	cp = putaxaddr(cp,&hdr->digis[i]);
	return bp;
}
/* Convert an AX.25 ARP table entry into a host format address structure
 * ready for use in transmitting a packet
 */
int
atohax25(hdr,hwaddr,source)
register struct ax25 *hdr;
register char *hwaddr;
struct ax25_addr *source;
{
	extern struct ax25_addr mycall;
	register struct ax25_addr *axp;

	hwaddr = getaxaddr(&hdr->dest,hwaddr);	/* Destination address */
	ASSIGN(hdr->source,*source);		/* Source address */
	if(hdr->dest.ssid & E){
		/* No digipeaters */
		hdr->ndigis = 0;
		hdr->dest.ssid &= ~E;
		hdr->source.ssid |= E;
		return 2;
	}
	hdr->source.ssid &= ~E;
	hdr->dest.ssid &= ~E;
	for(axp = hdr->digis; axp < &hdr->digis[MAXDIGIS]; axp++){
		hwaddr = getaxaddr(axp,hwaddr);
		if(axp->ssid & E){
			hdr->ndigis = axp - hdr->digis + 1;
			return hdr->ndigis;
		}
	}
	return -1;
}
/* Convert a network-format AX.25 header into a host format structure
 * Return -1 if error, number of addresses if OK
 */
int
ntohax25(hdr,bpp)
register struct ax25 *hdr;	/* Output structure */
struct mbuf **bpp;
{
	register struct ax25_addr *axp;
	char *getaxaddr();
	char buf[AXALEN];

	if(pullup(bpp,buf,AXALEN) < AXALEN)
		return -1;
	getaxaddr(&hdr->dest,buf);

	if(pullup(bpp,buf,AXALEN) < AXALEN)
		return -1;
	getaxaddr(&hdr->source,buf);

	/* Process C bits to get command/response indication */
	if((hdr->source.ssid & C) == (hdr->dest.ssid & C))
		hdr->cmdrsp = UNKNOWN;
	else if(hdr->source.ssid & C)
		hdr->cmdrsp = RESPONSE;
	else
		hdr->cmdrsp = COMMAND;

	hdr->ndigis = 0;
	if(hdr->source.ssid & E)
		return 2;	/* No digis */

	/* Process digipeaters */
	for(axp = hdr->digis;axp < &hdr->digis[MAXDIGIS]; axp++){
		if(pullup(bpp,buf,AXALEN) < AXALEN)
			return -1;
		getaxaddr(axp,buf);
		if(axp->ssid & E){	/* Last one */
			hdr->ndigis = axp - hdr->digis + 1;
			return hdr->ndigis + 2;			
		}
	}
	return -1;	/* Too many digis */
}

/* Figure out the frame type from the control field
 * This is done by masking out any sequence numbers and the
 * poll/final bit after determining the general class (I/S/U) of the frame
 */
int16
ftype(control)
register char control;
{
	if((control & 1) == 0)	/* An I-frame is an I-frame... */
		return I;
	if(control & 2)		/* U-frames use all except P/F bit for type */
		return(control & ~PF);
	else			/* S-frames use low order 4 bits for type */
		return(control & 0xf);
}

