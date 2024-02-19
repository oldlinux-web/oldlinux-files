/* Stuff generic to all Ethernet controllers */
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "arp.h"
#include "enet.h"

char ether_bdcst[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* Convert Ethernet header in host form to network mbuf */
struct mbuf *
htonether(ether,data)
struct ether *ether;
struct mbuf *data;
{
	struct mbuf *bp;
	register char *cp;

	if((bp = pushdown(data,ETHERLEN)) == NULLBUF)
		return NULLBUF;

	cp = bp->data;

	memcpy(cp,ether->dest,EADDR_LEN);
	cp += EADDR_LEN;
	memcpy(cp,ether->source,EADDR_LEN);
	cp += EADDR_LEN;
	put16(cp,ether->type);

	return bp;
}
/* Extract Ethernet header */
ntohether(ether,bpp)
struct ether *ether;
struct mbuf **bpp;
{
	pullup(bpp,ether->dest,EADDR_LEN);
	pullup(bpp,ether->source,EADDR_LEN);
	ether->type = pull16(bpp);
	return ETHERLEN;
}

/* Format an Ethernet address into a printable ascii string */
pether(out,addr)
char *out,*addr;
{
	sprintf(out,"%02x:%02x:%02x:%02x:%02x:%02x",
		uchar(addr[0]),uchar(addr[1]),
		uchar(addr[2]),uchar(addr[3]),
		uchar(addr[4]),uchar(addr[5]));
}

/* Convert an Ethernet address from Hex/ASCII to binary */
gether(out,cp)
register char *out;
register char *cp;
{
	register int i;

	for(i=6; i!=0; i--){
		*out++ = htoi(cp);
		if((cp = index(cp,':')) == NULLCHAR)	/* Find delimiter */
			break;
		cp++;			/* and skip over it */
	}
}
/* Version of gether for ARP, matches passed args */
gaether(out,in,cnt)
char *out;
char *in[];
int cnt;
{
	gether(out,in[0]);
}
/* Send an IP datagram on Ethernet */
int
enet_send(bp,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *bp;		/* Buffer to send */
struct interface *interface;	/* Pointer to interface control block */
int32 gateway;			/* IP address of next hop */
char precedence;
char delay;
char throughput;
char reliability;
{
	char *egate,*res_arp();

	egate = res_arp(interface,ARP_ETHER,gateway,bp);
	if(egate != NULLCHAR)
		return (*interface->output)(interface,egate,interface->hwaddr,IP_TYPE,bp);
	return 0;
}
/* Send a packet with Ethernet header */
int
enet_output(interface,dest,source,type,data)
struct interface *interface;		/* Pointer to interface control block */
char dest[];		/* Destination Ethernet address */
char source[];		/* Source Ethernet address */
int16 type;		/* Type field */
struct mbuf *data;	/* Data field */
{
	struct ether ep;
	struct mbuf *bp;

	memcpy(ep.dest,dest,EADDR_LEN);
	memcpy(ep.source,source,EADDR_LEN);
	ep.type = type;
	if((bp = htonether(&ep,data)) == NULLBUF){
		free_p(data);
		return -1;
	}
	return (*interface->raw)(interface,bp);
}
/* Process incoming Ethernet packets. Shared by all ethernet drivers. */
void
eproc(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	struct ether hdr;
	char multicast;
	void arp_input();

	/* Remove Ethernet header and kick packet upstairs */
	ntohether(&hdr,&bp);
	if(hdr.dest[0] & 1)
		multicast = 1;
	else
		multicast = 0;
	switch(hdr.type){
	case ARP_TYPE:
		arp_input(interface,bp);
		break;
	case IP_TYPE:
		ip_route(bp,multicast);
		break;
	default:
		free_p(bp);
		break;
	}
}

