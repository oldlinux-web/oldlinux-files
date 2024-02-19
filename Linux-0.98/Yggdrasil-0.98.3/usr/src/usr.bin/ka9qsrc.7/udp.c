/* Send and receive User Datagram Protocol packets */
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "udp.h"
#include "internet.h"

/* Hash table for UDP structures */
struct udp_cb *udps[NUDP] = { NULLUDP} ;
struct udp_stat udp_stat;	/* Statistics */

/* Create a UDP control block for lsocket, so that we can queue
 * incoming datagrams.
 */
int
open_udp(lsocket,r_upcall)
struct socket *lsocket;
void (*r_upcall)();
{
	register struct udp_cb *up;
	struct udp_cb *lookup_udp();
	int16 hval,hash_udp();

	if((up = lookup_udp(lsocket)) != NULLUDP)
		return 0;	/* Already exists */
	if((up = (struct udp_cb *)malloc(sizeof (struct udp_cb))) == NULLUDP){
		net_error = NO_SPACE;
		return -1;
	}
	up->rcvq = NULLBUF;
	up->rcvcnt = 0;
	up->socket.address = lsocket->address;
	up->socket.port = lsocket->port;
	up->r_upcall = r_upcall;

	hval = hash_udp(lsocket);
	up->next = udps[hval];
	up->prev = NULLUDP;
	if(up->next != NULLUDP)
		up->next->prev = up;
	udps[hval] = up;
	return 0;
}

/* Send a UDP datagram */
int
send_udp(lsocket,fsocket,tos,ttl,data,length,id,df)
struct socket *lsocket;		/* Source socket */
struct socket *fsocket;		/* Destination socket */
char tos;			/* Type-of-service for IP */
char ttl;			/* Time-to-live for IP */
struct mbuf *data;		/* Data field, if any */
int16 length;			/* Length of data field */
int16 id;			/* Optional ID field for IP */
char df;			/* Don't Fragment flag for IP */
{
	struct mbuf *htonudp(),*bp;
	struct pseudo_header ph;
	struct udp udp;

	length = UDPHDR;
	if(data != NULLBUF)
		length += len_mbuf(data);

	udp.source = lsocket->port;
	udp.dest = fsocket->port;
	udp.length = length;

	/* Create IP pseudo-header, compute checksum and send it */
	ph.length = length;
	ph.source = lsocket->address;
	ph.dest = fsocket->address;
	ph.protocol = UDP_PTCL;

	if((bp = htonudp(&udp,data,&ph)) == NULLBUF){
		net_error = NO_SPACE;
		free_p(data);
		return 0;
	}
	udp_stat.sent++;
	ip_send(lsocket->address,fsocket->address,UDP_PTCL,tos,ttl,bp,length,id,df);
	return length;
}

/* Accept a waiting datagram, if available. Returns length of datagram */
int
recv_udp(lsocket,fsocket,bp)
struct socket *lsocket;		/* Local socket to receive on */
struct socket *fsocket;		/* Place to stash incoming socket */
struct mbuf **bp;			/* Place to stash data packet */
{
	struct udp_cb *lookup_udp();
	register struct udp_cb *up;
	struct socket *sp;
	struct mbuf *buf;
	int16 length;

	up = lookup_udp(lsocket);
	if(up == NULLUDP){
		net_error = NO_CONN;
		return -1;
	}
	if(up->rcvcnt == 0){
		net_error = WOULDBLK;
		return -1;
	}
	buf = dequeue(&up->rcvq);
	up->rcvcnt--;

	sp = (struct socket *)buf->data;
	/* Fill in the user's foreign socket structure, if given */
	if(fsocket != NULLSOCK){
		fsocket->address = sp->address;
		fsocket->port = sp->port;
	}
	/* Strip socket header and hand data to user */
	pullup(&buf,NULLCHAR,sizeof(struct socket));
	length = len_mbuf(buf);
	if(bp != (struct mbuf **)NULL)
		*bp = buf;
	else
		free_p(buf);
	return length;
}
/* Delete a UDP control block */
int
del_udp(lsocket)
struct socket *lsocket;
{
	register struct udp_cb *up;
	struct udp_cb *lookup_udp();
	struct mbuf *bp;
	int16 hval,hash_udp();

	if((up = lookup_udp(lsocket)) == NULLUDP){
		net_error = INVALID;
		return -1;
	}		
	/* Get rid of any pending packets */
	while(up->rcvcnt != 0){
		bp = up->rcvq;
		up->rcvq = up->rcvq->anext;
		free_p(bp);
		up->rcvcnt--;
	}
	hval = hash_udp(&up->socket);
	if(udps[hval] == up){
		/* First on list */
		udps[hval] = up->next;
		if (up->next != NULLUDP)
			up->next->prev = NULLUDP;
	} else {
		up->prev->next = up->next;
		if (up->next != NULLUDP)
			up->next->prev = up->prev;
	}
	free((char *)up);
	return 0;
}
/* Process an incoming UDP datagram */
/*ARGSUSED*/
void
udp_input(bp,protocol,source,dest,tos,length,rxbroadcast)
struct mbuf *bp;	/* UDP header and data */
char protocol;		/* Should always be 17 */
int32 source;		/* Source IP address */
int32 dest;		/* Dest IP address */
char tos;
int16 length;
char rxbroadcast;	/* The only protocol that accepts 'em */
{
	struct pseudo_header ph;
	struct udp udp;
	struct udp_cb *up,*lookup_udp();
	struct socket lsocket;
	struct socket *fsocket;
	struct mbuf *packet;
	int ckfail = 0;

	if(bp == NULLBUF)
		return;

	udp_stat.rcvd++;

	/* Create pseudo-header and verify checksum */
	ph.source = source;
	ph.dest = dest;
	ph.protocol = protocol;
	ph.length = length;

	if(cksum(&ph,bp,length) != 0)
		/* Checksum apparently failed, note for later */
		ckfail++;

	/* Extract UDP header in host order */
	ntohudp(&udp,&bp);

	/* If the checksum field is zero, then ignore a checksum error.
	 * I think this is dangerously wrong, but it is in the spec.
	 */
	if(ckfail && udp.checksum != 0){
		udp_stat.cksum++;
		free_p(bp);
		return;
	}
	/* If this was a broadcast packet, pretend it was sent to us */
	if(rxbroadcast){
		lsocket.address = ip_addr;
		udp_stat.bdcsts++;
	} else
		lsocket.address = dest;

	lsocket.port = udp.dest;
	/* See if there's somebody around to read it */
	if((up = lookup_udp(&lsocket)) == NULLUDP){
		/* Nope, toss it on the floor */
		udp_stat.unknown++;
		free_p(bp);
		return;
	}
	/* Create space for the foreign socket info */
	if((packet = pushdown(bp,sizeof(struct socket))) == NULLBUF){
		/* No space, drop whole packet */
		free_p(bp);
		return;
	}
	fsocket = (struct socket *)packet->data;
	fsocket->address = source;
	fsocket->port = udp.source;

	/* Queue it */
	enqueue(&up->rcvq,packet);
	up->rcvcnt++;
	if(up->r_upcall)
		(*up->r_upcall)(&lsocket,up->rcvcnt);
}
/* Look up UDP socket, return control block pointer or NULLUDP if nonexistant */
static
struct udp_cb *
lookup_udp(socket)
struct socket *socket;
{
	register struct udp_cb *up;
	int16 hash_udp();

	up = udps[hash_udp(socket)];
	while(up != NULLUDP){
		if(socket->port == up->socket.port
		 && (socket->address == up->socket.address
		 || up->socket.address == 0))
			break;
		up = up->next;
	}
	return up;
}

/* Hash a UDP socket (address and port) structure */
static
int16
hash_udp(socket)
struct socket *socket;
{
	int16 hval;

	/* Compute hash function on socket structure */
/*	hval = hiword(socket->address);
	hval ^= loword(socket->address);
	hval ^= socket->port;
*/
	hval = socket->port;
	hval %= NUDP;
	return hval;
}
/* Convert UDP header in internal format to an mbuf in external format */
struct mbuf *
htonudp(udp,data,ph)
struct udp *udp;
struct mbuf *data;
struct pseudo_header *ph;
{
	struct mbuf *bp;
	register char *cp;
	int16 checksum;

	/* Allocate UDP protocol header and fill it in */
	if((bp = pushdown(data,UDPHDR)) == NULLBUF)
		return NULLBUF;

	cp = bp->data;
	cp = put16(cp,udp->source);	/* Source port */
	cp = put16(cp,udp->dest);	/* Destination port */
	cp = put16(cp,udp->length);	/* Length */
	*cp++ = 0;			/* Clear checksum */
	*cp-- = 0;

	/* All zeros and all ones is equivalent in one's complement arithmetic;
	 * the spec requires us to change zeros into ones to distinguish an
 	 * all-zero checksum from no checksum at all
	 */
	if((checksum = cksum(ph,bp,ph->length)) == 0)
		checksum = 0xffffffff;
	put16(cp,checksum);
	return bp;
}
/* Convert UDP header in mbuf to internal structure */
ntohudp(udp,bpp)
struct udp *udp;
struct mbuf **bpp;
{
	udp->source = pull16(bpp);
	udp->dest = pull16(bpp);
	udp->length = pull16(bpp);
	udp->checksum = pull16(bpp);
}
