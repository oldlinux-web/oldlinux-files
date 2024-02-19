/* Driver for 3COM Ethernet card */

#define	TIMER	20000	/* Timeout on transmissions */

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "enet.h"
#include "iface.h"
#include "ec.h"
#include "timer.h"
#include "arp.h"
#include "trace.h"

void ec0vec(),ec1vec(),ec2vec();
void (*ecvec[])() = {ec0vec,ec1vec,ec2vec};
struct ec ec[EC_MAX];		/* Per-controller info */
unsigned nec = 0;


/* Initialize interface */
int
ec_init(interface,bufsize)
struct interface *interface;
unsigned bufsize;	/* Maximum size of receive queue in PACKETS */
{
	register struct ec *ecp;
	register unsigned base;
	void eaudit();
	unsigned short getcs();
	int16 dev;

	dev = interface->dev;
	ecp = &ec[dev];
	base = ecp->base;
	ecp->rcvmax = bufsize;
	ecp->iface = interface;

	/* Pulse IE_RESET */
 	outportb(IE_CSR(base),IE_RESET);

	if(interface->hwaddr == NULLCHAR)
		interface->hwaddr = malloc(EADDR_LEN);
	getecaddr(base,interface->hwaddr);
	setecaddr(base,interface->hwaddr);
	if(memcmp(interface->hwaddr,ether_bdcst,EADDR_LEN) == 0){
		printf("EC address PROM contains broadcast address!!\n");
		return -1;
	}
	/* gain control of buffer */
	outportb(IE_CSR(base),IE_SYSBFR);

	/* Set transmit detect bits */
	outportb(EDLC_XMT(base),EDLC_16 | EDLC_JAM);

	/* Set up the receiver and flush status */
	outportb(EDLC_RCV(base),EDLC_MULTI|EDLC_GOOD|EDLC_ANY|EDLC_SHORT
	 |EDLC_DRIBBLE|EDLC_FCS|EDLC_OVER);
	inportb(EDLC_RCV(base));

	/* Start receiver */
	outportw(IE_RP(base),0);	/* Reset read pointer */
	outportb(IE_CSR(base),IE_RCVEDLC);
	return 0;
}

/* Send an IP datagram on Ethernet */
int
ec_send(bp,interface,gateway,precedence,delay,throughput,reliability)
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
ec_output(interface,dest,source,type,data)
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

/* Send raw packet (caller provides header) */
int
ec_raw(interface,bp)
struct interface *interface;	/* Pointer to interface control block */
struct mbuf *bp;		/* Data field */
{
	register struct ec *ecp;
	register unsigned base;
	register int i;
	short size;

	ecp = &ec[interface->dev];
	base = ecp->base;

	ecp->estats.xmit++;
	dump(interface,IF_TRACE_OUT,TRACE_ETHER,bp);

	size = len_mbuf(bp);
	/* Pad the size out to the minimum, if necessary,
	 * with junk from the last packet (nice security hole here)
	 */
	if(size < RUNT)
		size = RUNT;
	size = (size+1) & ~1;	/* round size up to next even number */

	/* Wait for transmitter ready, if necessary. IE_XMTBSY is valid
	 * only in the transmit mode, hence the initial check.
	 */
	if((inportb(IE_CSR(base)) & IE_BUFCTL) == IE_XMTEDLC){
		for(i=TIMER;(inportb(IE_CSR(base)) & IE_XMTBSY) && i != 0;i--)
			;
		if(i == 0){
			ecp->estats.timeout++;
			free_p(bp);
			return -1;
		}
	}
	ecp->size = size;
	/* Get control of the board buffer and disable receiver */
	outportb(IE_CSR(base),IE_SYSBFR);
	/* Point GP at beginning of packet */
	outportw(IE_GP(base),BFRSIZ-size);
	/* Actually load each piece with a fast assembler routine */
	while(bp != NULLBUF){
		outbuf(IE_BFR(base),bp->data,bp->cnt);
		bp = free_mbuf(bp);
	}
	/* Start transmitter */
	outportw(IE_GP(base),BFRSIZ-size);
	outportb(IE_CSR(base),IE_XMTEDLC);
	return 0;
}
ecint()
{
}

/* Service the Ethernet controller */
void
ec_recv(interface)
struct interface *interface;
{
	register struct ec *ecp;
	register unsigned base;
	struct mbuf *bp;
	int16 size;
	char stat;
	void arp_input();
	int ip_route();
	struct ether hdr;
	char multicast;

	ecp = &ec[interface->dev];
	base = ec[interface->dev].base;

	/* Check for transmit jam */
	if(!(inportb(IE_CSR(base)) & IE_XMTBSY)){
		stat = inportb(EDLC_XMT(base));
		if(stat & EDLC_16){
			ecp->estats.jam16++;
			rcv_fixup(base);
		} else if(stat & EDLC_JAM){
			/* Crank counter back to beginning and restart transmit */
			ecp->estats.jam++;
			outportb(IE_CSR(base),IE_SYSBFR);
			outportw(IE_GP(base),BFRSIZ - ecp->size);
			outportb(IE_CSR(base),IE_XMTEDLC);
		}
	}
#ifdef	foo
	if(inportb(IE_CSR(base)) & IE_RCVBSY)
		return;
#endif
	for(;;){
		stat = inportb(EDLC_RCV(base));
		if(stat & EDLC_STALE)
			break;

		if(stat & EDLC_OVER){
			ecp->estats.over++;
			rcv_fixup(base);
			continue;
		}
		if(stat & (EDLC_SHORT | EDLC_FCS | EDLC_DRIBBLE)){
			ecp->estats.bad++;
			rcv_fixup(base);
			continue;
		}
		if(stat & EDLC_ANY){
			/* Get control of the buffer */
			outportw(IE_GP(base),0);
			outportb(IE_CSR(base),IE_SYSBFR);
		
			/* Allocate mbuf and copy the packet into it */
			size = inportw(IE_RP(base));
			if(size < RUNT || size > GIANT)
				ecp->estats.bad++;
			else if((bp = alloc_mbuf(size)) == NULLBUF)
				ecp->estats.nomem++;
			else {
				ecp->estats.recv++;
				inbuf(IE_BFR(base),bp->data,size);
				bp->cnt = size;

				dump(interface,IF_TRACE_IN,TRACE_ETHER,bp);

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
			outportb(IE_CSR(base),IE_RCVEDLC);
			outportb(IE_RP(base),0);
		}
	}
}
static
rcv_fixup(base)
register unsigned base;
{
	outportb(IE_CSR(base),IE_SYSBFR);
	outportb(IE_CSR(base),IE_RCVEDLC);
	outportb(IE_RP(base),0);
}
/* Read Ethernet address from controller PROM */
static
getecaddr(base,cp)
register unsigned base;
register char *cp;
{
	register int i;

	for(i=0;i<EADDR_LEN;i++){
		outportw(IE_GP(base),i);
		*cp++ = inportb(IE_SAPROM(base));
	}
}
/* Set Ethernet address on controller */
static
setecaddr(base,cp)
register unsigned base;
register char *cp;
{
	register int i;

	for(i=0;i<EADDR_LEN;i++)
		outportb(EDLC_ADDR(base)+i,*cp++);
}
/* Shut down the Ethernet controller */
ec_stop(interface)
struct interface *interface;
{
	register unsigned base;
	int16 dev;

	dev = interface->dev;
	base = ec[dev].base;

	/* Pulse IE_RESET */
	outportb(IE_CSR(base),IE_RESET);
	outportb(IE_CSR(base),0);
}
/* Attach a 3-Com model 3C500 Ethernet controller to the system
 * argv[0]: hardware type, must be "3c500"
 * argv[1]: I/O address, e.g., "0x300"
 * argv[2]: vector, e.g., "3"
 * argv[3]: mode, must be "arpa"
 * argv[4]: interface label, e.g., "ec0"
 * argv[5]: maximum number of packets allowed on receive queue, e.g., "5"
 * argv[6]: maximum transmission unit, bytes, e.g., "1500"
 */
ec_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_ec;
	extern struct interface *ifaces;
	unsigned dev;
	int ec_init();
	int ec_send();
	void ec_recv();
	int ec_stop();
	int pether(),gaether();

	arp_init(ARP_ETHER,EADDR_LEN,IP_TYPE,ARP_TYPE,ether_bdcst,pether,gaether);
	if(nec >= EC_MAX){
		printf("Too many Ethernet controllers\n");
		return -1;
	}
	dev = nec++;
	if((if_ec = (struct interface *)calloc(1,sizeof(struct interface))) == NULLIF
	 ||(if_ec->name = malloc((unsigned)strlen(argv[4])+1)) == NULLCHAR){
		printf("ec_attach: no memory!\n");
		return -1;
	}
	strcpy(if_ec->name,argv[4]);
	if_ec->mtu = atoi(argv[6]);
	if_ec->send = ec_send;
	if_ec->output = ec_output;
	if_ec->raw = ec_raw;
	if_ec->recv = ec_recv;
	if_ec->stop = ec_stop;
	if_ec->dev = dev;

	ec[dev].base = htoi(argv[1]);
	ec[dev].vec = htoi(argv[2]);

	if(strcmp(argv[3],"arpa") != 0){
		printf("Mode %s unknown for interface %s\n",
			argv[3],argv[4]);
		free(if_ec->name);
		free((char *)if_ec);
		return -1;
	}
	/* Initialize device */
	if(ec_init(if_ec,(unsigned)atoi(argv[5])) != 0){
		free(if_ec->name);
		free((char *)if_ec);
		return -1;
	}
	if_ec->next = ifaces;
	ifaces = if_ec;

	return 0;
}
