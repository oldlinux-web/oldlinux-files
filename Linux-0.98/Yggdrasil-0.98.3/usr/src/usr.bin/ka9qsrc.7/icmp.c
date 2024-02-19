/* Internet Control Message Protocol */
#include "global.h"
#include "mbuf.h"
#include "internet.h"
#include "timer.h"
#include "iface.h"
#include "ip.h"
#include "icmp.h"

struct icmp_errors icmp_errors;
struct icmp_stats icmp_stats;

/* Process an incoming ICMP packet */
/*ARGSUSED*/
void
icmp_input(bp,protocol,source,dest,tos,length,rxbroadcast)
struct mbuf *bp;	/* Pointer to ICMP message */
char protocol;		/* Should always be ICMP_PTCL */
int32 source;		/* Sender of ICMP message */
int32 dest;		/* Us */
char tos;		/* Type of Service */
int16 length;		/* Length of ICMP message */
char rxbroadcast;
{
	struct mbuf *htonicmp(),*tbp;
	struct icmp icmp;	/* ICMP header */
	struct ip ip;		/* Offending datagram header */
	int16 type;		/* Type of ICMP message */

	if(rxbroadcast){
		/* Broadcast ICMP packets are to be IGNORED !! */
		icmp_errors.bdcsts++;
		free_p(bp);
		return;
	}
	if(cksum(NULLHEADER,bp,length) != 0){
		/* Bad ICMP checksum; discard */
		icmp_errors.checksum++;
		free_p(bp);
		return;
	}
	ntohicmp(&icmp,&bp);

	/* Process the message. Some messages are passed up to the protocol
	 * module for handling, others are handled here.
	 */
	type = icmp.type;
	if(type < ICMP_TYPES)
		icmp_stats.input[type]++;

	switch(uchar(type)){
	case TIME_EXCEED:	/* Time-to-live Exceeded */
	case DEST_UNREACH:	/* Destination Unreachable */
	case QUENCH:		/* Source Quench */
		ntohip(&ip,&bp);	/* Extract offending IP header */
		switch(uchar(ip.protocol)){
		case TCP_PTCL:
			tcp_icmp(ip.source,ip.dest,icmp.type,icmp.code,&bp);
			break;
		}
		break;
	case ECHO:		/* Echo Request */
		/* Change type to ECHO_REPLY, recompute checksum,
		 * and return datagram.
		 */
		icmp.type = ECHO_REPLY;
		if((tbp = htonicmp(&icmp,bp)) == NULLBUF){
			free_p(bp);
			return;
		}
		icmp_stats.output[ECHO_REPLY]++;
		ip_send(dest,source,ICMP_PTCL,tos,0,tbp,length,0,0);
		return;
	case REDIRECT:		/* Redirect */
	case PARAM_PROB:	/* Parameter Problem */
		break;
       case ECHO_REPLY:		/* Echo Reply */
		echo_proc(source,dest,&icmp);
		break;
	case TIMESTAMP:		/* Timestamp */
	case TIME_REPLY:	/* Timestamp Reply */
	case INFO_RQST:		/* Information Request */
	case INFO_REPLY:	/* Information Reply */
		break;
	}
	free_p(bp);
}
/* Return an ICMP response to the sender of a datagram.
 * Unlike most routines, the callER frees the mbuf.
 */
int
icmp_output(ip,data,type,code,args)
struct ip *ip;		/* Header of offending datagram */
struct mbuf *data;	/* Data portion of datagram */
char type,code;		/* Codes to send */
union icmp_args *args;
{
	struct mbuf *htonicmp();
	struct mbuf *htonip();
	struct mbuf *bp;
	struct icmp icmp;	/* ICMP protocol header */
	int16 dlen;		/* Length of data portion of offending pkt */
	int16 length;		/* Total length of reply */
	extern int32 ip_addr;	/* Our IP address */

	if(ip == NULLIP)
		return -1;
	if(type < ICMP_TYPES)
		icmp_stats.output[type]++;

	if(uchar(ip->protocol) == ICMP_PTCL){
		/* Never send an ICMP message about another ICMP message */
		icmp_errors.noloop++;
		return -1;
	}
	/* Compute amount of original datagram to return.
	 * We return the original IP header, and up to 8 bytes past that.
	 */
	dlen = min(8,len_mbuf(data));
	length = dlen + ICMPLEN + IPLEN + ip->optlen;
	/* Take excerpt from data portion */
	if(data != NULLBUF && (bp = copy_p(data,dlen)) == NULLBUF)
		return -1;	/* The caller will free data */

	/* Recreate and tack on offending IP header */
	if((data = htonip(ip,bp,0)) == NULLBUF){
		free_p(bp);
		return -1;
	}
	icmp.type = type;
	icmp.code = code;
	switch(uchar(icmp.type)){
	case PARAM_PROB:
		icmp.args.pointer = args->pointer;
		break;
	case REDIRECT:
		icmp.args.address = args->address;
		break;
	case ECHO:
	case ECHO_REPLY:
	case INFO_RQST:
	case INFO_REPLY:
	case TIMESTAMP:
	case TIME_REPLY:
		icmp.args.echo.id = args->echo.id;
		icmp.args.echo.seq = args->echo.seq;
		break;
	default:
		icmp.args.unused = 0;
		break;
	}
	/* Now stick on the ICMP header */
	if((bp = htonicmp(&icmp,data)) == NULLBUF){
		free_p(data);
		return -1;
	}
	return ip_send(ip_addr,ip->source,ICMP_PTCL,ip->tos,0,bp,length,0,0);
}
/* Generate ICMP header in network byte order, link data, compute checksum */
struct mbuf *
htonicmp(icmp,data)
struct icmp *icmp;
struct mbuf *data;
{
	struct mbuf *bp;
	register char *cp;
	int16 checksum;

	if((bp = pushdown(data,ICMPLEN)) == NULLBUF)
		return NULLBUF;
	cp = bp->data;

	*cp++ = icmp->type;
	*cp++ = icmp->code;
	cp = put16(cp,0);		/* Clear checksum */
	cp = put16(cp,icmp->args.echo.id);
	cp = put16(cp,icmp->args.echo.seq);

	/* Compute checksum, and stash result */
	checksum = cksum(NULLHEADER,bp,len_mbuf(bp));
	cp = &bp->data[2];
	cp = put16(cp,checksum);

	return bp;
}
/* Pull off ICMP header */
int
ntohicmp(icmp,bpp)
struct icmp *icmp;
struct mbuf **bpp;
{
	if(icmp == (struct icmp *)NULL)
		return -1;
	icmp->type = pullchar(bpp);
	icmp->code = pullchar(bpp);
	(void) pull16(bpp);		/* Toss checksum */
	icmp->args.echo.id = pull16(bpp);
	icmp->args.echo.seq = pull16(bpp);
	return 0;
}
