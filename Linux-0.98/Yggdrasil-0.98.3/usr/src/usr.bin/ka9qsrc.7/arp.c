/* Address Resolution Protocol (ARP) functions. Sits between IP and
 * Level 2, mapping IP to Level 2 addresses for all outgoing datagrams.
 */
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "iface.h"
#include "enet.h"
#include "ax25.h"
#include "arp.h"
#ifdef	UNIX
#include <memory.h>
#endif

extern int32 ip_addr;		/* Our IP address */

/* ARP entries for particular subnetwork types. The table values
 * are filled in by calls to arp_init() at device attach time
 */
#define	NTYPES	9
struct arp_type arp_type[NTYPES];

/* Hash table headers */
struct arp_tab *arp_tab[ARPSIZE];

struct arp_stat arp_stat;

/* Initialize an entry in the ARP table
 * Called by the device driver at attach time
 */
arp_init(hwtype,hwalen,iptype,arptype,bdcst,format,scan)
unsigned int hwtype;	/* ARP Hardware type */
int hwalen;		/* Hardware address length */
int iptype;		/* Subnet's protocol ID for IP */
int arptype;		/* Subnet's protocol ID for ARP */
char *bdcst;		/* Subnet's broadcast address (if any) */
int (*format)();	/* Function to format hardware addresses */
int (*scan)();		/* Function to scan addresses in ascii */	
{
	register struct arp_type *at;

	if(hwtype >= NTYPES)
		return -1;	/* Table too small */

	at = &arp_type[hwtype];
	at->hwalen = (int16)hwalen;
	at->iptype = (int16)iptype;
	at->arptype = (int16)arptype;
	at->bdcst = bdcst;
	at->format = format;
	at->scan = scan;
	return 0;
}

/* Resolve an IP address to a hardware address; if not found,
 * initiate query and return NULLCHAR.  If an address is returned, the
 * interface driver may send the packet; if NULLCHAR is returned,
 * res_arp() will have saved the packet on its pending queue,
 * so no further action (like freeing the packet) is necessary.
 */
char *
res_arp(interface,hardware,target,bp)
struct interface *interface;	/* Pointer to interface block */
int16 hardware;		/* Hardware type */
int32 target;		/* Target IP address */
struct mbuf *bp;	/* IP datagram to be queued if unresolved */
{
	void arp_output();
	register struct arp_tab *arp;

	if((arp = arp_lookup(hardware,target)) != NULLARP && arp->state == ARP_VALID)
		return arp->hw_addr;
	/* Create an entry and put the datagram on the
	 * queue pending an answer
	 */
	arp = arp_add(target,hardware,NULLCHAR,0,0);
	enqueue(&arp->pending,bp);
	arp_output(interface,hardware,target);
	return NULLCHAR;
}
/* Handle incoming ARP packets. This is almost a direct implementation of
 * the algorithm on page 5 of RFC 826, except for:
 * 1. Outgoing datagrams to unresolved addresses are kept on a queue
 *    pending a reply to our ARP request.
 * 2. The names of the fields in the ARP packet were made more mnemonic.
 * 3. Requests for IP addresses listed in our table as "published" are
 *    responded to, even if the address is not our own.
 */
void
arp_input(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	struct arp arp;
	struct arp_tab *ap;
	struct arp_type *at;
	struct mbuf *htonarp();
	
	arp_stat.recv++;
	if(ntoharp(&arp,&bp) == -1)	/* Convert into host format */
		return;
	if(arp.hardware >= NTYPES){
		/* Unknown hardware type, ignore */
		arp_stat.badtype++;
		return;
	}
	at = &arp_type[arp.hardware];
	if(arp.protocol != at->iptype){
		/* Unsupported protocol type, ignore */
		arp_stat.badtype++;
		return;
	}
	if(uchar(arp.hwalen) > MAXHWALEN || uchar(arp.pralen) != sizeof(int32)){
		/* Incorrect protocol addr length (different hw addr lengths
		 * are OK since AX.25 addresses can be of variable length)
		 */
		arp_stat.badlen++;
		return;
	}
	if(memcmp(arp.shwaddr,at->bdcst,(int)at->hwalen) == 0){
		/* This guy is trying to say he's got the broadcast address! */
		arp_stat.badaddr++;
		return;
	}
	/* If this guy is already in the table, update its entry
	 * unless it's a manual entry (noted by the lack of a timer)
	 */
	ap = NULLARP;	/* ap plays the role of merge_flag in the spec */
	if((ap = arp_lookup(arp.hardware,arp.sprotaddr)) != NULLARP
	 && ap->timer.start != 0){
		ap = arp_add(arp.sprotaddr,arp.hardware,arp.shwaddr,uchar(arp.hwalen),0);
	}
	/* See if we're the address they're looking for */
	if(arp.tprotaddr == ip_addr){
		if(ap == NULLARP)	/* Only if not already in the table */
			arp_add(arp.sprotaddr,arp.hardware,arp.shwaddr,uchar(arp.hwalen),0);

		if(arp.opcode == ARP_REQUEST){
			/* Swap sender's and target's (us) hardware and protocol
			 * fields, and send the packet back as a reply
			 */
			memcpy(arp.thwaddr,arp.shwaddr,(int)uchar(arp.hwalen));
			/* Mark the end of the sender's AX.25 address
			 * in case he didn't
			 */
			if(arp.hardware == ARP_AX25)
				arp.thwaddr[uchar(arp.hwalen)-1] |= E;

			memcpy(arp.shwaddr,interface->hwaddr,(int)at->hwalen);
			arp.tprotaddr = arp.sprotaddr;
			arp.sprotaddr = ip_addr;
			arp.opcode = ARP_REPLY;
			if((bp = htonarp(&arp)) == NULLBUF)
				return;

			if(interface->forw != NULLIF)
				(*interface->forw->output)(interface->forw,
				 arp.thwaddr,interface->forw->hwaddr,at->arptype,bp);
			else 
				(*interface->output)(interface,arp.thwaddr,
				 interface->hwaddr,at->arptype,bp);
			arp_stat.inreq++;
		} else {
			arp_stat.replies++;
		}
	} else if(arp.opcode == ARP_REQUEST
		&& (ap = arp_lookup(arp.hardware,arp.tprotaddr)) != NULLARP
		&& ap->pub){
		/* Otherwise, respond if the guy he's looking for is
		 * published in our table.
		 */
		memcpy(arp.thwaddr,arp.shwaddr,(int)uchar(arp.hwalen));
		/* Mark the end of the sender's AX.25 address
		 * in case he didn't
		 */
		if(arp.hardware == ARP_AX25)
			arp.thwaddr[uchar(arp.hwalen)-1] |= E;
		memcpy(arp.shwaddr,ap->hw_addr,(int)at->hwalen);
		arp.tprotaddr = arp.sprotaddr;
		arp.sprotaddr = ap->ip_addr;
		arp.opcode = ARP_REPLY;
		if((bp = htonarp(&arp)) == NULLBUF)
			return;
		if(interface->forw != NULLIF)
			(*interface->forw->output)(interface->forw,
			 arp.thwaddr,interface->forw->hwaddr,at->arptype,bp);
		else 
			(*interface->output)(interface,arp.thwaddr,
			 interface->hwaddr,at->arptype,bp);
		arp_stat.inreq++;
	}
}
/* Add an IP-addr / hardware-addr pair to the ARP table */
struct arp_tab *
arp_add(ipaddr,hardware,hw_addr,hw_alen,pub)
int32 ipaddr;		/* IP address, host order */
int16 hardware;		/* Hardware type */
char *hw_addr;		/* Hardware address, if known; NULLCHAR otherwise */
int16 hw_alen;		/* Length of hardware address */
int pub;		/* Publish this entry? */
{
	void arp_drop();
	int ip_route();
	struct mbuf *bp,*dequeue();
	register struct arp_tab *ap;
	unsigned hashval,arp_hash();

	if((ap = arp_lookup(hardware,ipaddr)) == NULLARP){
		/* New entry */
		if((ap = (struct arp_tab *)calloc(1,sizeof(struct arp_tab))) == NULLARP)
			return NULLARP;
		ap->timer.func = arp_drop;
		ap->timer.arg = (char *)ap;
		ap->hardware = hardware;
		ap->ip_addr = ipaddr;

		/* Put on head of hash chain */
		hashval = arp_hash(hardware,ipaddr);
		ap->prev = NULLARP;
		ap->next = arp_tab[hashval];
		arp_tab[hashval] = ap;
		if(ap->next != NULLARP){
			ap->next->prev = ap;
		}
	}
	if(hw_addr == NULLCHAR){
		/* Await response */
		ap->state = ARP_PENDING;
		ap->timer.start = PENDTIME * (1000 / MSPTICK);
	} else {
		/* Response has come in, update entry and run through queue */
		ap->state = ARP_VALID;
		ap->timer.start = ARPLIFE * (1000 / MSPTICK);
		if(ap->hw_addr != NULLCHAR)
			free(ap->hw_addr);
		if((ap->hw_addr = malloc(hw_alen)) == NULLCHAR){
			free((char *)ap);
			return NULLARP;
		}
		memcpy(ap->hw_addr,hw_addr,(int)hw_alen);
		/* This kludge marks the end of an AX.25 address to allow
		 * for optional digipeaters (insert Joan Rivers salute here)
		 */
		if(hardware == ARP_AX25)
			ap->hw_addr[hw_alen-1] |= E;
		ap->pub = pub;
		while((bp = dequeue(&ap->pending)) != NULLBUF)
			ip_route(bp,0);
	}
	start_timer(&ap->timer);
	return ap;
}

/* Remove an entry from the ARP table */
void
arp_drop(ap)
register struct arp_tab *ap;
{
	unsigned arp_hash();

	if(ap == NULLARP)
		return;
	stop_timer(&ap->timer);	/* Shouldn't be necessary */
	if(ap->next != NULLARP)
		ap->next->prev = ap->prev;
	if(ap->prev != NULLARP)
		ap->prev->next = ap->next;
	else
		arp_tab[arp_hash(ap->hardware,ap->ip_addr)] = ap->next;
	if(ap->hw_addr != NULLCHAR)
		free(ap->hw_addr);
	free_q(&ap->pending);
	free((char *)ap);
}

/* Look up the given IP address in the ARP table */
struct arp_tab *
arp_lookup(hardware,ipaddr)
int16 hardware;
int32 ipaddr;
{
	unsigned arp_hash();
	register struct arp_tab *ap;

	for(ap = arp_tab[arp_hash(hardware,ipaddr)]; ap != NULLARP; ap = ap->next){
		if(ap->ip_addr == ipaddr && ap->hardware == hardware)
			break;
	}
	return ap;
}
/* Send an ARP request to resolve IP address target_ip */
static
void
arp_output(interface,hardware,target)
struct interface *interface;
int16 hardware;
int32 target;
{
	struct arp arp;
	struct mbuf *bp,*htonarp();
	struct arp_type *at;

	at = &arp_type[hardware];
	if(interface->output == NULLFP)
		return;
	
	arp.hardware = hardware;
	arp.protocol = at->iptype;
	arp.hwalen = at->hwalen;
	arp.pralen = sizeof(int32);
	arp.opcode = ARP_REQUEST;
	memcpy(arp.shwaddr,interface->hwaddr,(int)at->hwalen);
	arp.sprotaddr = ip_addr;
	memset(arp.thwaddr,0,(int)at->hwalen);
	arp.tprotaddr = target;
	if((bp = htonarp(&arp)) == NULLBUF)
		return;
	(*interface->output)(interface,at->bdcst,
		interface->hwaddr,at->arptype,bp);
	arp_stat.outreq++;
}

/* Hash a {hardware type, IP address} pair */
static
unsigned
arp_hash(hardware,ipaddr)
int16 hardware;
int32 ipaddr;
{
	register unsigned hashval;

	hashval = hardware;
	hashval ^= hiword(ipaddr);
	hashval ^= loword(ipaddr);
	hashval %= ARPSIZE;
	return hashval;
}		
/* Copy a host format arp structure into mbuf for transmission */
struct mbuf *
htonarp(arp)
register struct arp *arp;
{
	struct mbuf *bp;
	register char *buf;

	if(arp == (struct arp *)NULL)
		return NULLBUF;
	if((bp = alloc_mbuf(sizeof(struct arp))) == NULLBUF)
		return NULLBUF;

	buf = bp->data;

	buf = put16(buf,arp->hardware);
	buf = put16(buf,arp->protocol);
	*buf++ = arp->hwalen;
	*buf++ = arp->pralen;
	buf = put16(buf,arp->opcode);
	memcpy(buf,arp->shwaddr,(int)uchar(arp->hwalen));
	buf += arp->hwalen;
	buf = put32(buf,arp->sprotaddr);
	memcpy(buf,arp->thwaddr,(int)uchar(arp->hwalen));
	buf += arp->hwalen;
	buf = put32(buf,arp->tprotaddr);

	bp->cnt = buf - bp->data;
	return bp;
}
/* Convert an incoming ARP packet into a host-format structure */
int
ntoharp(arp,bpp)
register struct arp *arp;
struct mbuf **bpp;
{
	if(arp == (struct arp *)NULL || bpp == NULLBUFP)
		return -1;

	arp->hardware = pull16(bpp);
	arp->protocol = pull16(bpp);
	arp->hwalen = pullchar(bpp);
	arp->pralen = pullchar(bpp);
	arp->opcode = pull16(bpp);
	pullup(bpp,arp->shwaddr,(int16)uchar(arp->hwalen));
	arp->sprotaddr = pull32(bpp);
	pullup(bpp,arp->thwaddr,(int16)uchar(arp->hwalen));

	arp->tprotaddr = pull32(bpp);

	/* Get rid of anything left over */
	free_p(*bpp);
	*bpp = NULLBUF;
	return 0;
}
