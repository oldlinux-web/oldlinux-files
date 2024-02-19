/*
 * This file contains the code for using the AppleTalk network.  This is a 
 * first cut and hopefully it will work.
 */


#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "ip.h"
#include "arp.h"
#include <MacTypes.h>
#include <Appletalk.h>
#include "mac_AT.h"
#include "trace.h"

struct at at[AT_MAX];		/* Per-controller info */

char appletalk_bdcst = 0xff;

unsigned	nat = 0;
	
/* Initialize interface */
at_init(interface,bufsize)
struct interface *interface;
unsigned bufsize;	/* Maximum size of receive queue in PACKETS */
{
	register struct at *atp;
	int16 dev;
	char *malloc();
	int err;
	ABRecPtr abrrdptr;
	ABRecPtr abrwrptr;
	int node, net;		/* vars to hold my node id and net id */
	
#ifdef DEBUG
	printf("at_init called\n");
#endif
	dev = interface->dev;
	if ( dev >= nat )
	{
		printf("problem with dev entry.  Number is  %d, max = %d\n", dev,nat);
		return(-1);
	}
	atp = &at[dev];
	atp->rcvmax = bufsize;
	/*
	 * see if the device is available for use with AppleTalk
	 */
	err = MPPOpen();
	if ( err != noErr )
	{
		printf("AppleTalk is not available on device  error %d\n", err);
		return(-1);
	}

	/*
	 * See if we can put our protocol type in and have the default handler use it
	 */
	err = LAPOpenProtocol(atp->ProtoType, NULL);
	if ( err != noErr)
	{
		/*
		 * In case we bombed out last time, try and remove the protocol type and try again
		 */
		if ( LAPCloseProtocol(atp->ProtoType) != 0 )
		{

			printf("Could not initialize AppleTalk with protocol type %d, error %d\n",
			 	atp->ProtoType, err);
			return(-1);
		}
		err = LAPOpenProtocol(atp->ProtoType, NULL);
		if ( err != noErr)
		{
			printf("Could not initialize AppleTalk with protocol type %d, error %d\n",
			 	atp->ProtoType, err);
			return(-1);
		}
	}
	/*
	 * Since everything is alright, lets allocate an ABusRecord
	 */
	
	atp->rdATptr = NewHandle(lapSize);
	if ( atp->rdATptr == NULL)
	{
		printf("Could not allocate handel for AppleBuss (0).\n");
		(void)LAPCloseProtocol(atp->ProtoType);
		return(-1);
	}

	atp->wrATptr = NewHandle(lapSize);
	
	if ( atp->wrATptr == NULL)
	{
		printf("Could not allocate handel for AppleBuss (1).\n");
		DisposHandle(atp->rdATptr);
		(void)LAPCloseProtocol(atp->ProtoType);
		return(-1);
	}
	
	/*
	 * now that everything is going well, let's issue an async read on the protocol
	 */
	HLock(atp->rdATptr);
	HLock(atp->wrATptr);
	abrrdptr = *atp->rdATptr;
	abrwrptr = *atp->wrATptr;
	
	abrwrptr->lapProto.lapAddress.lapProtType = atp->ProtoType;
	if ( at_startread(atp, abrrdptr) != 0 )
	{
		printf("Could not perform read on AppleTalk network.  Closing down device.\n");
		return(-1);
	}
	
	if ( GetNodeAddress( &node, &net) != 0)
	{
		printf("Could not get my own node address.  Something is wrong!!\n");
		return(-1);
	}
	if ( interface->hwaddr == NULLCHAR )
	{
		if ( (interface -> hwaddr = malloc(4)) == NULLCHAR)
		{
			printf("Could not allocate memory for hardware address.\n");
			return(-1);
		}
	}
		
	*interface->hwaddr = (unsigned char) node;
	printf("My AppleTalk node number is %d\n", node);
	return(0);
}

/* Send an IP datagram on AppleTalk */
at_send(bp,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *bp;		/* Buffer to send */
struct interface *interface;	/* Pointer to interface control block */
int32 gateway;			/* IP address of next hop */
char precedence;
char delay;
char throughput;
char reliability;
{
	char *egate,*res_arp();
	egate = res_arp(interface,ARP_APPLETALK,gateway,bp);
	if(egate != NULLCHAR)
	{
		(*interface->output)(interface,egate,interface->hwaddr,AIP_TYPE,bp);
	}
}

/* Send a packet with Ethernet header */
at_output(interface,dest,source,type,bp)
struct interface *interface;		/* Pointer to interface control block */
char dest[];		/* Destination Ethernet address */
char source[];		/* Source Appletalk address */
int16 type;		/* Type field */
struct mbuf *bp;	/* Data field */
{
	struct	appletalk ap;	/* AppleTalk struct for header info. */
	struct	mbuf	*hdr;
	struct	mbuf	*htonat();

	memcpy(&ap.source, source, APPLE_LEN);
	memcpy(&ap.dest, dest, APPLE_LEN);
	ap.type = type;
	hdr = htonat(&ap);
	hdr->next = bp;
	(*interface->raw)(interface,hdr);
}

/* Send raw packet (caller provides header) */
at_raw(interface,bp)
struct interface *interface;		/* Pointer to interface control block */
struct mbuf *bp;	/* Data field */
{
	register struct at *atp;	/* AppleTalk pointer */
	short size;					/* size of data in mbuf */
	int err;					/* error indicator from mac stuff */
	short	tmp=2;				/* there is an offset of 2 in the data buffer */
	short *mptr;				/* temporary pointer */
	ABRecPtr abrwrptr;			/* pointer to applebuss record */
	struct appletalk ap;		/* appletalk struct for front of record */
	struct mbuf	*hdr;			/* extra mbuf pointer */
	
	atp = &at[interface->dev];
	if ( interface->dev >= nat )
	{
		printf("problem with dev entry.  Number is  %d, max = %d\n", interface->dev,nat);
		return(-1);
	}
	dump(interface,IF_TRACE_OUT,TRACE_APPLETALK,bp);
	size = len_mbuf(bp);
	
	/*
		Set up the transmit structure
	 */
	
	/*
	 * take a part to get the address
	 */
	ntohat(&ap, &bp);
	abrwrptr = *atp->wrATptr;
	abrwrptr->lapProto.lapAddress.dstNodeID = ap.dest;
	abrwrptr->lapProto.lapDataPtr = atp->buffer;
	abrwrptr->lapProto.lapAddress.lapProtType = atp->ProtoType;
	
	/*
	 * now put header back on
	 */
	 
	hdr = htonat(&ap);
	hdr->next = bp;

	/*
		Copy all the data from the mbuf to the data packet holder.
		Max amount of data is 600 bytes
	 */
	while  ( hdr != NULLBUF)
	{
		bcopy(hdr->data, &abrwrptr->lapProto.lapDataPtr[tmp], hdr->cnt);
		tmp += hdr->cnt;
		if ( tmp >= 600)
			printf("sending: ERROR IN PACKET SIZE, size = %d\n", tmp);

		hdr = free_mbuf(hdr);
	}

	/* 
		set the packet length in the first two bytes of the LAP data buffer
		including these two bytes.
	 */
	 
	mptr = abrwrptr->lapProto.lapDataPtr;
	*mptr = tmp;
	
	abrwrptr->lapProto.lapReqCount = tmp;
	
	atp->astats.out++;
	
	/* send off a sync write and wait for return value. */
	
	err = LAPWrite(atp->wrATptr, FALSE);
	
	if ( err != noErr )
	{
		printf("at_raw: write failure to AppleTalk (%d)\n", err);
		return(0);
	}
}

/* Process any incoming AppleTalk packets on the receive queue */
int
at_recv(interface)
struct interface *interface;
{
	struct at *atp;			/* appletalk pointer */
	struct mbuf *bp;		/* place to store all the buffers */
	ABRecPtr abrrdptr;		/* Appletalk network storage */
	int	err;
	extern int32 ip_addr;
	struct appletalk ap;
	struct mbuf *htonat();
	
	/*
	 *  just to make sure it is for us
	 */

	atp = &at[interface->dev];
	
	if ( interface->dev >= nat )
	{
		printf("problem with dev entry.  Number is  %d, max = %d\n", interface->dev,nat);
		return(-1);
	}
	
	/*
	 * get a pointer to the read structure
	 */
	
	abrrdptr = *atp->rdATptr;
	
	/*
	 * since this was an async read, a 1 indicates it has not completed yet.
	 */

	if ( abrrdptr->lapProto.abResult == 1)
	{
		return;
	}
	else if  ( abrrdptr->lapProto.abResult == buf2SmallErr)
	{
		atp->astats.badsize++;
		if ( at_startread(atp, abrrdptr ) != 0)
		{
			printf("Error in starting async read on appletalk network.\n");
		}
		return(-1);
	}
	else if  ( abrrdptr->lapProto.abResult == readQErr)
	{
		atp->astats.drop++;
		if ( at_startread(atp, abrrdptr ) != 0)
		{
			printf("Error in starting async read on appletalk network.\n");
		}
		return(-1);
	}
	
	/*
	 * now set up the mbuf. count -2 because AT puts in the count sent in the first two]
	 * bytes of the data.
	 */
	
	if((bp = alloc_mbuf(abrrdptr->lapProto.lapActCount-2)) == NULLBUF)
	{
		atp->astats.nomem++;
		if ( at_startread(atp, abrrdptr ) != 0)
		{
			printf("Error in starting async read on appletalk network.\n");
		}
		return(-1);
	}
	
	/*
	 * move it over
	 */

	bcopy(&abrrdptr->lapProto.lapDataPtr[2], bp->data, abrrdptr->lapProto.lapActCount-2);
	
	bp->cnt = abrrdptr->lapProto.lapActCount - 2;
	
	/* not start another async read on this device. */
		
	if ( at_startread(at, abrrdptr) != 0 )
	{
		printf("Could not perform read on AppleTalk network.  Closing down device.\n");
		return(-1);
	}
	ntohat(&ap, &bp);
	
	switch (ap.type)
	{
	
		case AARP_TYPE:
			arp_input(interface, bp);
		 	atp->astats.any++;
			break;
		
		case AIP_TYPE:
			ip_route(bp, 0);
		 	atp->astats.any++;
			break;
			
		default:
		/*
			printf("at_recv: wrong type (%x)\n", ap.type);
		 */
		 	atp->astats.badtype++;

		 	free_p(bp);
			break;
	}
}
/* Shut down the Ethernet controller */
at_stop(interface)
struct interface *interface;
{
	int16 dev;
	struct at *atp;

#ifdef DEBUG
	printf("at_stop called. dev = %d\n", interface->dev);
#endif

	dev = interface->dev;
	if ( dev >= nat)
		return(-1);
	atp = &at[dev];
	
	/* need to unlock the frozen pointers and cancel any reads outstanding */

	(void)HUnlock(atp->rdATptr);
	(void)HUnlock(atp->wrATptr);
	(void)LAPRdCancel(atp->rdATptr);

	/* make sure to close the procotol down and dispose or any pointers */
	
	(void)LAPCloseProtocol(atp->ProtoType);
	(void)DisposHandle(atp->rdATptr);
	(void)DisposHandle(atp->wrATptr);

	return(0);
}
/* Attach AppleTalk to the system
 * argv[0]: hardware type, must be "0"
 * argv[1]: Protocol Type, e.g., "77"
 * argv[2]: device name,must be "B"
 * argv[3]: mode, must be "arpa"
 * argv[4]: interface label, e.g., "at0"
 * argv[5]: maximum number of packets allowed on receive queue, e.g., "5"
 * argv[6]: maximum transmission unit, bytes, e.g., "600" < appletalk limitation.
 */
at_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_at;
	extern struct interface *ifaces;
	unsigned dev;
	char *calloc(),*malloc();
	int at_init();
	int at_send();
	int at_recv();
	int at_stop();
	int err;
	int pat(),gat();

	if(nat >= AT_MAX){
		printf("Too many AppleTalk controllers\r\n");
		return -1;
	}
	dev = nat++;
	if_at = (struct interface *)calloc(1,sizeof(struct interface));

	if_at->name = malloc((unsigned)strlen(argv[4])+1);
	strcpy(if_at->name,argv[4]);
	if_at->mtu = atoi(argv[6]);
	if_at->send = at_send;
	if_at->output = at_output;
	if_at->raw = at_raw;
	if_at->recv = at_recv;
	if_at->stop = at_stop;
	if_at->dev = dev;
	if_at->flags = 0;
	at[dev].ProtoType = htoi(argv[1]);
	at[dev].net = malloc(strlen(argv[2])+1);
	strcpy(at[dev].net, argv[2]);

	if(strcmp(argv[3],"arpa") != 0){
		printf("Mode %s unknown for interface %s\r\n",
			argv[3],argv[4]);
		free((char *)if_at);
		return -1;
	}
	arp_init(ARP_APPLETALK,APPLE_LEN,AIP_TYPE,AARP_TYPE,appletalk_bdcst,pat,gat);
	if_at->next = ifaces;
	ifaces = if_at;
	/* Initialize device */
	if ( (err = at_init(if_at,(unsigned)atoi(argv[5]))) != 0 )
	{
		printf("AT_INIT failed. err = %d\n", err);
		return(-1);
	}
	else
	{
		return 0;
	}
}

/*
	this procedure will start up an async read on the appletalk network
 */

at_startread(atp, readptr)
struct at *atp;
ABRecPtr readptr;
{
	int err;
	
	/*
	 * set up the structure for a read
	 */
	 
	readptr->lapProto.lapAddress.lapProtType = atp->ProtoType;
	readptr->lapProto.lapReqCount = MAX_ATBUF;
	readptr->lapProto.lapDataPtr = atp->buffer;

	err = LAPRead( atp->rdATptr, TRUE);
	if ( err != noErr )
	{
		(void)LAPCloseProtocol(atp->ProtoType);
		(void)DisposHandle(atp->rdATptr);
		(void)DisposHandle(atp->wrATptr);
		return(-1);
	}			
	return(0);
}

bcopy(from, to, cnt)
char *to, *from;
int cnt;
{
	while ( cnt-- != 0 )
	{
		*to++ = *from++;
	}
}
/* Format an AppleTalk address into a printable ascii string */
pat(out,addr)
char *out,*addr;
{
	sprintf(out,"%02x", *addr & 0xff);
}

/* Convert an AppleTalk address from Hex/ASCII to binary */
gat(out,cp)
register char *out;
register char *cp;
{
	register int i;

	*out = htoi(cp);
}

/* Convert AppleTalk header in host form to network mbuf */
struct mbuf *
htonat(ap)
struct appletalk *ap;
{
	struct mbuf *bp;
	register char *cp;
	bp = alloc_mbuf(APPLEADDRLEN);
	bp->cnt = APPLEADDRLEN;
	cp = bp->data;
	memcpy(cp,&ap->dest,APPLE_LEN);
	cp += APPLE_LEN;
	memcpy(cp,&ap->source,APPLE_LEN);
	cp += APPLE_LEN;
	put16(cp,ap->type);

	return bp;
}
/* Extract AppleTalk header */
ntohat(ap,bpp)
struct appletalk *ap;
struct mbuf **bpp;
{
	pullup(bpp,&ap->dest,APPLE_LEN);
	pullup(bpp,&ap->source,APPLE_LEN);
	ap->type = pull16(bpp);
	return APPLEADDRLEN;
}



int
doatstat(argc,argv)
int argc;
char *argv[];
{
	struct at *atp;

	for(atp = at;atp < &at[nat]; atp++){
		
		printf("Controller %u:\n",atp-at);

		printf("any      output    badtype   nomem     drop     badsize\n");
		printf("%-10lu%-10lu%-10lu%-10lu%-10lu%-10lu\n",
	 	atp->astats.any, atp->astats.out,
	 	atp->astats.badtype, atp->astats.nomem,
	 	atp->astats.drop, atp->astats.badsize);
	}
	return 0;
}
