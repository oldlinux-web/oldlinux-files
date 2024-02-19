/*
 * Center for Information Technology Integration
 *           The University of Michigan
 *                    Ann Arbor
 *
 * Dedicated to the public domain.
 * Send questions to info@citi.umich.edu
 *
 * BOOTP is documented in RFC 951 and RFC 1048
 * Delinted, ANSIfied and reformatted - 5/30/91 P. Karn
 */


#include <time.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "udp.h"
#include "iface.h"
#include "ip.h"
#include "internet.h"
#include "cmdparse.h"
#include "bootp.h"

static int bootp_rx __ARGS((struct interface *ifp,struct mbuf *bp));
static void ntoh_bootp __ARGS((struct mbuf **bpp,struct bootp *bootpp));
static int mask2width __ARGS((int32 mask));

#define BOOTP_TIMEOUT	30		/* Time limit for booting       */
#define BOOTP_RETRANS	5		/* The inteval between sendings */

int WantBootp = 0;
static int SilentStartup = 0;

int
dobootp(argc,argv,p)
int argc;
char *argv[];
void *p;
{
	struct interface *ifp = NULLIF,*tifp;
	struct socket lsock, fsock;
	struct mbuf *bp;
	struct udp_cb *bootp_cb;
	register char *cp;
	time_t        now,		/* The current time (seconds)   */
		      starttime,	/* The start time of sending BOOTP */
		      lastsendtime;	/* The last time of sending BOOTP  */
	int i;

	ifp = ifaces;			/* default to the first interface */
	if(argc > 1) {
		for(i = 1; i != argc; ++i){
			
			if((tifp = if_lookup(argv[i])) != NULLIF) 
				ifp = tifp;
			else if(strncmp(argv[i], "silent", strlen(argv[i])) == 0)
				SilentStartup = 1;
			else if(strncmp(argv[i], "noisy", strlen(argv[i])) == 0)
				SilentStartup = 0;
			else {
				printf("bootp [net_name] [silent] [noisy]\n");
				return 1;
			}
		}
	}

	if(ifp == NULLIF)
		return 0;
	lsock.address = 0;
	lsock.port = IPPORT_BOOTPC;

	open_udp(&lsock,NULLVFP);

	fsock.address = 0xffffffff;
	fsock.port = IPPORT_BOOTPS;

  	/* Get boot starting time */
  	time(&starttime);
  	lastsendtime = 0;

  	/* Send the bootp request packet until a response is received or time
	   out */
  	for(;;){

		/* Allow bootp packets should be passed through iproute. */
		WantBootp = 1;

		/* Get the current time */
		time(&now);

		/* Stop, if time out */
		if(now - starttime >= BOOTP_TIMEOUT){
			printf("bootp: timed out, values not set\n");
			break;
		}

		/* Don't flood the network, send in intervals */
		if(now - lastsendtime > BOOTP_RETRANS){
			if(!SilentStartup) printf("Requesting...\n");

			/* Allocate BOOTP packet and fill it in */
			if((bp = alloc_mbuf(sizeof(struct bootp))) == NULLBUF)
				break;
			cp = bp->data;		/* names per the RFC: */
			*cp++ = BOOTREQUEST;		/* op */
			*cp++ = 1; /* ifp->iftype->type;	/* htype */
			*cp++ = 6; /* ifp->iftype->hwalen;	/* hlen */
			*cp++ = 0;			/* hops */
			cp = put32(cp,(int32) now);	/* xid */
			cp = put16(cp, now - starttime);/* secs */
			cp = put16(cp, 0);		/* unused */
			cp = put32(cp, ip_addr);	/* ciaddr */
			cp = put32(cp, 0L);		/* yiaddr */
			cp = put32(cp, 0L);		/* siaddr */
			cp = put32(cp, 0L);		/* giaddr */
			if (ifp->hwaddr) {
				memcpy(cp, ifp->hwaddr, 6);
				cp += 16;			/* chaddr */
			} else {
				memset(cp, 0, 16);		/* chaddr */
				cp += 16;
			}
			memset(cp, 0, 64);		/* sname */
			cp += 64;
			memset(cp, 0, 128);		/* file */
			cp += 128;
			memset(cp, 0, 64);		/* vend */
			cp += 64;
			bp->cnt = cp - bp->data;
			/* assert(bp->cnt == BOOTP_LEN) */

			/* Send out one BOOTP Request packet as a broadcast */
			send_udp(&lsock, &fsock,0,0,bp,bp->cnt,0,0);

			lastsendtime = now;
		}

		/* Give other tasks a chance to run. */
/*		pwait(NULL);*/
		keep_things_going();

		/* Test for and process any replies */
		if(recv_udp(&lsock, &fsock, &bp) > -1){
			if(bootp_rx(ifp,bp))
				break;
		} else if(net_error != WOULDBLK){
			printf("bootp: Net_error %d, no values set\n",
				 net_error);
			break;
		}
  	}

	WantBootp = 0;
	del_udp(&lsock);
	return 0;
}

/* Process BOOTP input received from 'interface'. */
static int
bootp_rx(ifp,bp)
struct interface *ifp;
struct mbuf *bp;
{
	int 		ch;
	int    		count;
	int32 		gateway = 0;
	int32 		nameserver = 0;
	int32 		broadcast, netmask;
	struct route 	*rp;
	struct bootp	reply;
	char		*cp;

	if(len_mbuf(bp) != sizeof(struct bootp)){
		free_p(bp);
		return 0;
	}
	ntoh_bootp(&bp, &reply);
	free_p(bp);

	if(reply.op != BOOTREPLY) 
		return 0;

	if(!SilentStartup)
		printf("Network %s configured:\n", ifp->name);

	if(ip_addr == 0){
		ip_addr = (int) reply.yiaddr;	/* yiaddr */
		if(!SilentStartup)
			printf("     IP address: %s\n",  
		inet_ntoa(ip_addr));
	}

#ifdef undef
	/* now process the vendor-specific block, check for cookie first. */
	cp = reply.vend;
	if(get32(cp) != 0x63825363L){
		printf("Invalid magic cookie.\n");
		return(0);
	}

	cp += 4;
	while(((ch = *cp) != BOOTP_END) && (++cp < (reply.vend + 64))) 
		switch(ch){
		case BOOTP_PAD:		/* They're just padding */
			continue;
		case BOOTP_SUBNET:	/* fixed length, 4 octets */
			cp++;		/* moved past length */

			/* Set the netmask */
        		/* Remove old entry if it exists */
			netmask = get32(cp);
			cp += 4;

        		rp = rt_blookup(ifp->addr & ifp->netmask,mask2width(ifp->netmask));
        		if(rp != NULLROUTE)
                		rt_drop(rp->target,rp->bits);
        		ifp->netmask = netmask;
        		rt_add(ifp->addr,mask2width(ifp->netmask),0L,ifp,0L,0L,0);

			if(!SilentStartup)
				printf("     Subnet mask: %s\n", inet_ntoa(netmask));
			
			/* Set the broadcast */
			broadcast = ifp->addr | ~(ifp->netmask);
        		rp = rt_blookup(ifp->broadcast,32);
        		if(rp != NULLROUTE && rp->iface == ifp)
                		rt_drop(ifp->broadcast,32);
        		ifp->broadcast = broadcast;
        		rt_add(ifp->broadcast,32,0L,ifp,1L,0L,1);
			
			if(!SilentStartup)
				printf("     Broadcast: %s\n", inet_ntoa(broadcast));

			break;
		case BOOTP_HOSTNAME:
			count = (int) *cp;
			cp++;

			if(Hostname != NULLCHAR)
				free(Hostname);
			Hostname = mallocw(count);
			strncpy(Hostname, cp, count);
			cp += count;

			if(!SilentStartup)
				printf("     Hostname: %s\n", Hostname);
			break;
		case BOOTP_DNS:
			count = (int) *cp;
			cp++;

			while(count){
				nameserver = get32(cp);
				add_nameserver(nameserver);
				if(!SilentStartup)
					printf("     Nameserver: %s\n", inet_ntoa(nameserver));
				cp += 4;
				count -= 4;
			}
			break;
		case BOOTP_GATEWAY:
			count = (int) *cp;
			cp++;

			gateway = get32(cp);

			/* Add the gateway as the default */
			rt_add(0,0,gateway,ifp,1,0,0);

			if(!SilentStartup)
				printf("     Default gateway: %s\n", inet_ntoa(gateway));
			cp += count;
			break;
		default:		/* variable field we don't know about */
			count = (int) *cp;
			cp++;

			cp += count;
			break;
	}

	rt_add(ifp->addr,mask2width(ifp->netmask),0L,ifp,1,0,0);
#endif
	return(1);
}


static void
ntoh_bootp(bpp, bootpp)
struct mbuf **bpp;
struct bootp *bootpp;
{
	bootpp->op = pullchar(bpp);                 	/* op */
	bootpp->htype = pullchar(bpp);			/* htype */
	bootpp->hlen = pullchar(bpp);			/* hlen */
	bootpp->hops = pullchar(bpp);			/* hops */
	bootpp->xid = pull32(bpp);			/* xid */
	bootpp->secs = pull16(bpp);			/* secs */
	bootpp->unused = pull16(bpp);			/* unused */
	bootpp->ciaddr = pull32(bpp);		/* ciaddr */
	bootpp->yiaddr = pull32(bpp);		/* ciaddr */
	bootpp->siaddr = pull32(bpp);		/* siaddr */
	bootpp->giaddr = pull32(bpp);		/* giaddr */
	pullup(bpp, bootpp->chaddr, 16);		/* chaddr */
	pullup(bpp, bootpp->sname, 64);		/* sname */
	pullup(bpp, bootpp->file, 128);		/* file name */
	pullup(bpp, bootpp->vend, 64);			/* vendor */
}




int
bootp_validPacket(ip, bpp)
struct ip *ip;
struct mbuf **bpp;
{
	struct udp udp;
	struct pseudo_header ph;
	int status;


	/* Must be a udp packet */
	if(ip->protocol !=  UDP_PTCL) 
		return 0;

	/* Invalid if packet is not the right size */
	if(len_mbuf(*bpp) != (sizeof(struct udp) + sizeof(struct bootp)))
		return 0;

	/* Invalid if not a udp bootp packet */
	ntohudp(&udp, bpp);

	status = (udp.dest == IPPORT_BOOTPC) ? 1 : 0;
	
	/* Restore packet, data hasn't changed */
        /* Create pseudo-header and verify checksum */
        ph.source = ip->source;
        ph.dest = ip->dest;
        ph.protocol = ip->protocol;
        ph.length = ip->length - IPLEN - ip->optlen;

	*bpp = htonudp(&udp, *bpp, &ph);

	return status;
}

/* Given a network mask, return the number of contiguous 1-bits starting
 * from the most significant bit.
 */
static int
mask2width(mask)
int32 mask;
{
        int width,i;

        width = 0;
        for(i = 31;i >= 0;i--){
                if(!(mask & (1L << i)))
                        break;
                width++;
        }
        return width;
}

