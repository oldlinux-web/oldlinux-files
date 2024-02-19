/* net/rom level 3 low level processing
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "arp.h"
#include "slip.h"
#include "ax25.h"
#include "netrom.h"
#include "nr4.h"
#include "lapb.h"
#include <ctype.h>
#ifdef	UNIX
#include <string.h>
#include <memory.h>
#endif

/* Nodes message broadcast address: "NODES" in shifted ASCII */
struct ax25_addr nr_nodebc = {
	'N'<<1, 'O'<<1, 'D'<<1, 'E'<<1, 'S'<<1, ' '<<1,
	('0'<<1) | E
} ;

struct nriface nrifaces[NRNUMIFACE] ;
unsigned nr_numiface ;
struct nrnbr_tab *nrnbr_tab[NRNUMCHAINS] ;
struct nrroute_tab *nrroute_tab[NRNUMCHAINS] ;
struct nrnf_tab *nrnf_tab[NRNUMCHAINS] ;
unsigned nr_nfmode = NRNF_NOFILTER ;
unsigned nr_ttl = 64 ;
unsigned obso_init = 6 ;
unsigned obso_minbc = 5 ;
unsigned nr_maxroutes = 5 ;
unsigned nr_autofloor = 1 ;
unsigned nr_verbose = 0 ;
struct interface *nr_interface ;

/* send a NET/ROM layer 3 datagram */
void nr3output(dest, data)
struct ax25_addr *dest ;
struct mbuf *data ;
{
	struct nr3hdr n3hdr ;
	struct mbuf *n3b ;

	n3hdr.dest = *dest ;	/* copy destination field */
	n3hdr.ttl = nr_ttl ;	/* time to live from initializer parm */

	if ((n3b = htonnr3(&n3hdr)) == NULLBUF) {
		free_p(data) ;
		return ;
	}

	append(&n3b, data) ;

	/* The null interface indicates that the packet needs to have */
	/* an appropriate source address inserted by nr_route */
	
	nr_route(n3b,NULLAX25) ;
}


/* send IP datagrams across a net/rom network connection */
/*ARGSUSED*/
int
nr_send(bp,interface,gateway,precedence,delay,throughput,reliability)
struct mbuf *bp ;
struct interface *interface ;
int32 gateway ;
char precedence ;
char delay ;
char throughput ;
char reliability ;
{
	struct ax25_addr dest ;
	struct mbuf *pbp ;
	struct nr4hdr n4hdr ;
	char *hwaddr ;
	struct arp_tab *arp ;

	if ((arp = arp_lookup(ARP_NETROM,gateway)) == NULLARP) {
		free_p(bp) ;	/* drop the packet if no route */
		return ;
	}
	hwaddr = arp->hw_addr ;				/* points to destination */
	memcpy(dest.call, hwaddr, ALEN) ;
	dest.ssid = hwaddr[ALEN] ;
		
	/* Create a "network extension" transport header */
	n4hdr.opcode = NR4OPPID ;
	n4hdr.u.pid.family = PID_IP ;
	n4hdr.u.pid.proto = PID_IP ;

	if ((pbp = htonnr4(&n4hdr)) == NULLBUF) {
		free_p(bp) ;
		return ;
	}

	append(&pbp,bp) ;		/* Append the data to that */
	nr3output(&dest, pbp) ; /* and pass off to level 3 code */

}

/* Figure out if a call is assigned to one of my net/rom
 * interfaces.
 */
static int
ismycall(addr)
struct ax25_addr *addr ;
{
	register int i ;
	int found = 0 ;
	
	for (i = 0 ; i < nr_numiface ; i++)
		if (addreq((struct ax25_addr *)(nrifaces[i].interface->hwaddr),
			addr)) {
			found = 1 ;
			break ;
		}

	return found ;
}


/* Route net/rom network layer packets.
 */
nr_route(bp, iaxp)
struct mbuf *bp ;			/* network packet */
struct ax25_cb *iaxp ;		/* incoming ax25 control block */
{
	struct nr3hdr n3hdr ;
	struct nr4hdr n4hdr ;
	struct ax25_cb *axp, *find_ax25(), *open_ax25() ;
	struct ax25 naxhdr ;
	struct ax25_addr neighbor, from ;
	struct mbuf *hbp, *pbp ;
	extern int16 axwindow ;
	void ax_incom(), nr4input() ;
	register struct nrnbr_tab *np ;
	register struct nrroute_tab *rp ;
	register struct nr_bind *bindp ;
	struct nr_bind *find_best() ;
	struct interface *interface ;
	unsigned ifnum ;
	
	if (ntohnr3(&n3hdr,&bp) == -1) {
		free_p(bp) ;
		return ;
	}

	/* If this isn't an internally generated network packet,
	 * give the router a chance to record a route back to the
	 * sender, in case they aren't in the local node's routing
	 * table yet.
	 */

	if (iaxp != NULLAX25) {
		/* find the interface number */
		for (ifnum = 0 ; ifnum < nr_numiface ; ifnum++)
			if (iaxp->interface == nrifaces[ifnum].interface)
				break ;

		if (ifnum == nr_numiface) {	/* shouldn't happen! */
			free_p(bp) ;
			return ;
		}

		from = iaxp->addr.dest ;
		from.ssid |= E ;

		/* Add (possibly) a zero-quality recorded route via */
		/* the neighbor from which this packet was received */
		/* Note that this doesn't work with digipeated neighbors, */
		/* at this point. */
		
		(void) nr_routeadd("      ",&n3hdr.source,ifnum,0,
									(char *)&from,0,1) ;
	}

	/* A packet from me, to me, can only be one thing: */
	/* a horrible routing loop.  This will probably result */
	/* from a bad manual ARP entry, but we should fix these */
	/* obscure errors as we find them. */
	
	if (ismycall(&n3hdr.dest)) {
		if (iaxp == NULLAX25) {		/* From me? */
			free_p(bp) ;
			return ;
		} else {					/* It's from somewhere else! */
			if (ntohnr4(&n4hdr,&bp) == -1) {
				free_p(bp) ;
				return ;
			}
			if ((n4hdr.opcode & NR4OPCODE) == 0) {
				if (n4hdr.u.pid.family == PID_IP
					&& n4hdr.u.pid.proto == PID_IP)
					ip_route(bp,0) ;
				else 					/* we don't do this proto */
					free_p(bp) ;

				return ;
			}
			
			/* Must be net/rom transport: */

			nr4input(&n4hdr,bp) ;

		}
		return ;
	}

	if ((rp = find_nrroute(&n3hdr.dest)) == NULLNRRTAB) {
		/* no route, drop the packet */
		free_p(bp) ;
		return ;
	}

	if ((bindp = find_best(rp->routes,1)) == NULLNRBIND) {
		/* This shouldn't happen yet, but might if we add */
		/* dead route detection */
		free_p(bp) ;
		return ;
	}

	np = bindp->via ;
	memcpy(neighbor.call,np->call,ALEN) ;
	neighbor.ssid = np->call[ALEN] ;
	interface = nrifaces[np->interface].interface ;

	/* Now check to see if iaxp is null.  That is */
	/* a signal that the packet originates here, */
	/* so we need to insert the callsign of the appropriate  */
	/* interface */
	if (iaxp == NULLAX25)
		memcpy((char *)&n3hdr.source,interface->hwaddr,AXALEN) ;
	
	/* Make sure there is a connection to the neighbor */
	if ((axp = find_ax25(&neighbor)) == NULLAX25 ||
		(axp->state != CONNECTED && axp->state != RECOVERY)) {
		/* Open a new connection or reinitialize old one */
		/* hwaddr has been advanced to point to neighbor + digis */
		atohax25(&naxhdr, np->call, (struct ax25_addr *)interface->hwaddr) ;
		axp = open_ax25(&naxhdr, axwindow, ax_incom, NULLVFP, NULLVFP,
						interface,(char *)0) ;
		if (axp == NULLAX25) {
			free_p(bp) ;
			return ;
		}
	}
		
	if (--n3hdr.ttl == 0) {	/* the packet's time to live is over! */
		free_p(bp) ;
		return ;
	}

	/* allocate and fill PID mbuf */
	if ((pbp = alloc_mbuf(1)) == NULLBUF) {
		free_p(bp) ;
		return ;
	}
	pbp->cnt = 1 ;
	*pbp->data = (PID_FIRST | PID_LAST | PID_NETROM) ;

	/* now format network header */
	if ((hbp = htonnr3(&n3hdr)) == NULLBUF) {
		free_p(pbp) ;
		free_p(bp) ;
		return ;
	}

	append(&pbp,hbp) ;		/* append header to pid */
	append(&pbp,bp) ;		/* append data to header */
	send_ax25(axp,pbp) ;	/* pass it off to ax25 code */
}
	

/* Perform a nodes broadcast on interface # ifno in the net/rom
 * interface table.
 */
 
nr_bcnodes(ifno)
unsigned ifno ;
{
	struct mbuf *hbp, *dbp, *savehdr ;
	struct nrroute_tab *rp ;
	struct nrnbr_tab *np ;
	struct nr_bind * bp ;
	struct nr3dest nrdest ;
	int i, didsend = 0, numdest = 0 ;
	register char *cp ;
	struct interface *axif = nrifaces[ifno].interface ;
	struct nr_bind *find_best() ;
	
	/* prepare the header */
	if ((hbp = alloc_mbuf(NR3NODEHL)) == NULLBUF)
		return ;
		
	hbp->cnt = NR3NODEHL ;	
	
	*hbp->data = NR3NODESIG ;
	memcpy(hbp->data+1,nrifaces[ifno].alias,ALEN) ;

	/* Some people don't want to advertise any routes; they
	 * just want to be a terminal node.  In that case we just
	 * want to send our call and alias and be done with it.
	 */

	if (!nr_verbose) {
		(*axif->output)(axif, (char *)&nr_nodebc, axif->hwaddr,
				 		(PID_FIRST | PID_LAST | PID_NETROM),
				 		hbp) ;	/* send it */
		return ;
	}
	
	/* make a copy of the header in case we need to send more than */
	/* one packet */
	savehdr = copy_p(hbp,NR3NODEHL) ;

	/* now scan through the routing table, finding the best routes */
	/* and their neighbors.  create destination subpackets and append */
	/* them to the header */
	for (i = 0 ; i < NRNUMCHAINS ; i++) {
		for (rp = nrroute_tab[i] ; rp != NULLNRRTAB ; rp = rp->next) {
			/* look for best, non-obsolescent route */
			if ((bp = find_best(rp->routes,0)) == NULLNRBIND)
				continue ;	/* no non-obsolescent routes found */
			if (bp->quality == 0)	/* this is a loopback route */
				continue ;			/* we never broadcast these */
			np = bp->via ;
			/* insert best neighbor */
			memcpy(nrdest.neighbor.call,np->call,ALEN) ;
			nrdest.neighbor.ssid = np->call[ALEN] ;
			/* insert destination from route table */
			nrdest.dest = rp->call ;
			/* insert alias from route table */
			strcpy(nrdest.alias,rp->alias) ;
			/* insert quality from binding */
			nrdest.quality = bp->quality ;
			/* create a network format destination subpacket */
			if ((dbp = htonnrdest(&nrdest)) == NULLBUF) {
				free_p(hbp) ;	/* drop the whole idea ... */
				free_p(savehdr) ;
				return ;
			}
			append(&hbp,dbp) ;	/* append to header and others */
			/* see if we have appended as many destinations */
			/* as we can fit into a single broadcast.  If we */
			/* have, go ahead and send them out. */
			if (++numdest == NRDESTPERPACK) {	/* filled it up */
				didsend = 1 ;	/* indicate that we did broadcast */
				numdest = 0 ;	/* reset the destination counter */
				(*axif->output)(axif, (char *)&nr_nodebc, axif->hwaddr,
						 		(PID_FIRST | PID_LAST | PID_NETROM),
						 		hbp) ;	/* send it */
				hbp = copy_p(savehdr,NR3NODEHL) ;	/* new header */
			}
		}
	}

	/* Now, here is something totally weird.  If our interfaces */
	/* have different callsigns than this one, advertise a very */
	/* high quality route to them.  Is this a good idea?  I don't */
	/* know.  However, it allows us to simulate a bunch of net/roms */
	/* hooked together with a diode matrix coupler. */
	for (i = 0 ; i < nr_numiface ; i++) {
		if (i == ifno)
			continue ;		/* don't bother with ours */
		cp = nrifaces[i].interface->hwaddr ;
		if (!addreq((struct ax25_addr *)axif->hwaddr,(struct ax25_addr *)cp)) {
			/* both destination and neighbor address */
			memcpy((char *)&nrdest.dest,cp,AXALEN) ;
			memcpy((char *)&nrdest.neighbor,cp,AXALEN) ;
			/* alias of the interface */
			strcpy(nrdest.alias,nrifaces[i].alias) ;
			/* and the very highest quality */
			nrdest.quality = 255 ;
			/* create a network format destination subpacket */
			if ((dbp = htonnrdest(&nrdest)) == NULLBUF) {
				free_p(hbp) ;	/* drop the whole idea ... */
				free_p(savehdr) ;
				return ;
			}
			append(&hbp,dbp) ;	/* append to header and others */
			if (++numdest == NRDESTPERPACK) {	/* filled it up */
				didsend = 1 ;	/* indicate that we did broadcast */
				numdest = 0 ;	/* reset the destination counter */
				(*axif->output)(axif, (char *)&nr_nodebc, axif->hwaddr,
						 		(PID_FIRST | PID_LAST | PID_NETROM),
						 		hbp) ;	/* send it */
				hbp = copy_p(savehdr,NR3NODEHL) ;	/* new header */
			}
		}
	}
			
	/* If we have a partly filled packet left over, or we never */
	/* sent one at all, we broadcast: */
	if (!didsend || numdest > 0)
		(*axif->output)(axif, (char *)&nr_nodebc, axif->hwaddr,
						(PID_FIRST | PID_LAST | PID_NETROM), hbp) ;

	free_p(savehdr) ;	/* free the header copy */
}


/* initialize fake arp entry for netrom */
nr3arp()
{
	int psax25(), setpath() ;

	arp_init(ARP_NETROM,AXALEN,0,0,NULLCHAR,psax25,setpath) ;
}

/* attach the net/rom interface.  no parms for now. */
/*ARGSUSED*/
nr_attach(argc,argv)
int argc ;
char *argv[] ;
{
	if (nr_interface != (struct interface *)0) {
		printf("netrom interface already attached\n") ;
		return -1 ;
	}

	nr3arp() ;
	
	nr_interface = (struct interface *)calloc(1,sizeof(struct interface)) ;
	nr_interface->name = "netrom" ;
	nr_interface->mtu = NR4MAXINFO ;
	nr_interface->send = nr_send ;
	nr_interface->next = ifaces ;
	ifaces = nr_interface ;
	return 0 ;
}

/* This function checks an ax.25 address and interface number against
 * the filter table and mode, and returns 1 if the address is to be
 * accepted, and 0 if it is to be filtered out.
 */
static int
accept_bc(addr,ifnum)
struct ax25_addr *addr ;
unsigned ifnum ;
{
	struct nrnf_tab *fp ;

	if (nr_nfmode == NRNF_NOFILTER)		/* no filtering in effect */
		return 1 ;

	fp = find_nrnf(addr,ifnum) ;		/* look it up */
	
	if ((fp != NULLNRNFTAB && nr_nfmode == NRNF_ACCEPT)
		|| (fp == NULLNRNFTAB && nr_nfmode == NRNF_REJECT))
		return 1 ;
	else
		return 0 ;
}


/* receive and process node broadcasts. */
nr_nodercv(interface,source,bp)
struct interface *interface ;
struct ax25_addr *source ;
struct mbuf *bp ;
{
	register int ifnum ;
	char bcalias[7] ;
	struct nr3dest ds ;
	char sbuf[AXALEN*3] ;
	
	/* First, see if this is even a net/rom interface: */
	for (ifnum = 0 ; ifnum < nr_numiface ; ifnum++)
		if (interface == nrifaces[ifnum].interface)
			break ;
			
	if (ifnum == nr_numiface) {	/* not in the interface table */
		free_p(bp) ;
		return ;
	}

	if (!accept_bc(source,(unsigned)ifnum)) {	/* check against filter */
		free_p(bp) ;
		return ;
	}
	
	/* See if it has a routing broadcast signature: */
	if (uchar(pullchar(&bp)) != NR3NODESIG) {
		free_p(bp) ;
		return ;
	}

	/* now try to get the alias */
	if (pullup(&bp,bcalias,ALEN) < ALEN) {
		free_p(bp) ;
		return ;
	}

	bcalias[ALEN] = '\0' ;		/* null terminate */

	/* copy source address and convert to arp format */
	memcpy(sbuf,source->call,ALEN) ;
	sbuf[ALEN] = (source->ssid | E) ;	/* terminate */
	
	/* enter the neighbor into our routing table */
	if (nr_routeadd(bcalias,source,(unsigned)ifnum,nrifaces[ifnum].quality,
					sbuf, 0, 0) == -1) {
		free_p(bp) ;
		return ;
	}
	
	/* we've digested the header; now digest the actual */
	/* routing information */
	while (ntohnrdest(&ds,&bp) != -1) {
		/* ignore routes to me! */
		if (ismycall(&ds.dest))
			continue ;
		/* ignore routes below the minimum quality threshhold */
		if (ds.quality < nr_autofloor)
			continue ;
		/* set loopback paths to 0 quality */
		if (ismycall(&ds.neighbor))
			ds.quality = 0 ;
		else
			ds.quality = ((ds.quality * nrifaces[ifnum].quality + 128)
						  / 256) & 0xff ;
		if (nr_routeadd(ds.alias,&ds.dest,(unsigned)ifnum,ds.quality,sbuf,0,0)
			== -1)
			break ;
	}
			
	free_p(bp) ;	/* This will free the mbuf if anything fails above */
}


/* The following are utilities for manipulating the routing table */

/* hash function for callsigns.  Look familiar? */
int16
nrhash(s)
struct ax25_addr *s ;
{
	register char x ;
	register int i ;
	register char *cp ;

	x = 0 ;
	cp = s->call ;
	for (i = ALEN ; i !=0 ; i--)
		x ^= *cp++ & 0xfe ;
	x ^= s->ssid & SSID ;
	return uchar(x) % NRNUMCHAINS ;
}

/* Find a neighbor table entry.  Neighbors are determined by
 * their callsign and the interface number.  This takes care
 * of the case where the same switch or hosts uses the same
 * callsign on two different channels.  This isn't done by
 * net/rom, but it might be done by stations running *our*
 * software.
 */
struct nrnbr_tab *
find_nrnbr(addr,ifnum)
register struct ax25_addr *addr ;
unsigned ifnum ;
{
	int16 hashval ;
	register struct nrnbr_tab *np ;
	char i_state ;
	struct ax25_addr ncall ;

	/* Find appropriate hash chain */
	hashval = nrhash(addr) ;

	/* search hash chain */
	i_state = disable() ;
	for (np = nrnbr_tab[hashval] ; np != NULLNTAB ; np = np->next) {
		memcpy(ncall.call,np->call,ALEN) ;	/* convert first in */
		ncall.ssid = np->call[ALEN] ; /* list to ax25 address format */
		if (addreq(&ncall,addr) && np->interface == ifnum) {
			restore(i_state) ;
			return np ;
		}
	}
	restore(i_state) ;
	return NULLNTAB ;
}


/* Find a route table entry */
struct nrroute_tab *
find_nrroute(addr)
register struct ax25_addr *addr ;
{
	int16 hashval ;
	register struct nrroute_tab *rp ;
	char i_state ;

	/* Find appropriate hash chain */
	hashval = nrhash(addr) ;

	/* search hash chain */
	i_state = disable() ;
	for (rp = nrroute_tab[hashval] ; rp != NULLNRRTAB ; rp = rp->next) {
		if (addreq(&rp->call,addr)) {
			restore(i_state) ;
			return rp ;
		}
	}
	restore(i_state) ;
	return NULLNRRTAB ;
}

/* Try to find the AX.25 address of a node with the given alias.  Return */
/* a pointer to the AX.25 address if found, otherwise NULLAXADDR.  The alias */
/* should be a six character, blank-padded, upper-case string. */

struct ax25_addr *
find_nralias(alias)
char *alias ;
{
	int i ;
	register struct nrroute_tab *rp ;

	/* Since the route entries are hashed by ax.25 address, we'll */
	/* have to search all the chains */
	
	for (i = 0 ; i < NRNUMCHAINS ; i++)
		for (rp = nrroute_tab[i] ; rp != NULLNRRTAB ; rp = rp->next)
			if (strncmp(alias, rp->alias, 6) == 0)
				return &rp->call ;

	/* If we get to here, we're out of luck */

	return NULLAXADDR ;
}

	
/* Find a binding in a list by its neighbor structure's address */
struct nr_bind *
find_binding(list,neighbor)
struct nr_bind *list ;
register struct nrnbr_tab *neighbor ;
{
	register struct nr_bind *bp ;

	for(bp = list ; bp != NULLNRBIND ; bp = bp->next)
		if (bp->via == neighbor)
			return bp ;

	return NULLNRBIND ;
}

/* Find the worst quality non-permanent binding in a list */
static
struct nr_bind *
find_worst(list)
struct nr_bind *list ;
{
	register struct nr_bind *bp ;
	struct nr_bind *worst = NULLNRBIND ;
	unsigned minqual = 1000 ; 	/* infinity */

	for (bp = list ; bp != NULLNRBIND ; bp = bp->next)
		if (!(bp->flags & NRB_PERMANENT) && bp->quality < minqual) {
			worst = bp ;
			minqual = bp->quality ;
		}

	return worst ;
}

/* Find the best binding of any sort in a list.  If obso is 1,
 * include entries below the obsolescence threshhold in the
 * search (used when this is called for routing broadcasts).
 * If it is 0, routes below the threshhold are treated as
 * though they don't exist.
 */
static
struct nr_bind *
find_best(list,obso)
struct nr_bind *list ;
unsigned obso ;
{
	register struct nr_bind *bp ;
	struct nr_bind *best = NULLNRBIND ;
	int maxqual = -1 ;	/* negative infinity */

	for (bp = list ; bp != NULLNRBIND ; bp = bp->next)
		if ((int)bp->quality > maxqual)
			if (obso || bp->obsocnt >= obso_minbc) {
				best = bp ;
				maxqual = bp->quality ;
			}

	return best ;
}

/* Add a route to the net/rom routing table */
nr_routeadd(alias,dest,ifnum,quality,neighbor,permanent,record)
char *alias ;				/* net/rom node alias, blank-padded and */
							/* null-terminated */
struct ax25_addr *dest ;	/* destination node callsign */
unsigned ifnum ;			/* net/rom interface number */
unsigned quality ;			/* route quality */
char *neighbor ;			/* neighbor node + 2 digis (max) in arp format */
unsigned permanent ;		/* 1 if route is permanent (hand-entered) */
unsigned record ;			/* 1 if route is a "record route" */
{
	struct nrroute_tab *rp ;
	struct nr_bind *bp ;
	struct nrnbr_tab *np ;
	int16 rhash, nhash ;
	struct ax25_addr ncall ;

	/* See if a routing table entry exists for this destination */
	if ((rp = find_nrroute(dest)) == NULLNRRTAB) {
		if ((rp =
			 (struct nrroute_tab *)calloc(1,sizeof(struct nrroute_tab)))
			== NULLNRRTAB)
			return -1 ;
		else {			/* create a new route table entry */
			strncpy(rp->alias,alias,6) ;
			rp->call = *dest ;
			rhash = nrhash(dest) ;
			rp->next = nrroute_tab[rhash] ;
			if (rp->next != NULLNRRTAB)
				rp->next->prev = rp ;
			nrroute_tab[rhash] = rp ;	/* link at head of hash chain */
		}
	} else if (!record) {
		strncpy(rp->alias,alias,6) ;	/* update the alias */
	}

	/* See if an entry exists for this neighbor */
	memcpy(ncall.call,neighbor,ALEN) ;	/* no digis included */
	ncall.ssid = neighbor[ALEN] ;
	if ((np = find_nrnbr(&ncall,ifnum)) == NULLNTAB) {
		if ((np =
			 (struct nrnbr_tab *)calloc(1,sizeof(struct nrnbr_tab)))
			 == NULLNTAB) {
			if (rp->num_routes == 0) {	/* we just added to table */
				nrroute_tab[rhash] = rp->next ;
				free((char *)rp) ;				/* so get rid of it */
			}
			return -1 ;
		}
		else {		/* create a new neighbor entry */
			memcpy(np->call,neighbor,3 * AXALEN) ;
			np->interface = ifnum ;
			nhash = nrhash(&ncall) ;
			np->next = nrnbr_tab[nhash] ;
			if (np->next != NULLNTAB)
				np->next->prev = np ;
			nrnbr_tab[nhash] = np ;
		}
	}
	else if (permanent) {		/* force this path to the neighbor */
		memcpy(np->call,neighbor,3 * AXALEN) ;
	}
		
	/* See if there is a binding between the dest and neighbor */
	if ((bp = find_binding(rp->routes,np)) == NULLNRBIND) {
		if ((bp =
			 (struct nr_bind *)calloc(1,sizeof(struct nr_bind)))
			== NULLNRBIND) {
			if (rp->num_routes == 0) {	/* we just added to table */
				nrroute_tab[rhash] = rp->next ;
				free((char *)rp) ;				/* so get rid of it */
			}
			if (np->refcnt == 0) {		/* we just added it */
				nrnbr_tab[nhash] = np->next ;
				free((char *)np) ;
			}
			return -1 ;
		}
		else {		/* create a new binding and link it in */
			bp->via = np ;	/* goes via this neighbor */
			bp->next = rp->routes ;	/* link into binding chain */
			if (bp->next != NULLNRBIND)
				bp->next->prev = bp ;
			rp->routes = bp ;
			rp->num_routes++ ;	/* bump route count */
			np->refcnt++ ;		/* bump neighbor ref count */
			bp->quality = quality ;
			bp->obsocnt = obso_init ;	/* use initial value */
			if (permanent)
				bp->flags |= NRB_PERMANENT ;
			else if (record)	/* notice permanent overrides record! */
				bp->flags |= NRB_RECORDED ;
		}
	} else {
		if (permanent) {	/* permanent request trumps all */
			bp->quality = quality ;
			bp->obsocnt = obso_init ;
			bp->flags |= NRB_PERMANENT ;
			bp->flags &= ~NRB_RECORDED ;	/* perm is not recorded */
		} else if (!(bp->flags & NRB_PERMANENT)) {	/* not permanent */
			if (record) {	/* came from nr_route */
				if (bp->flags & NRB_RECORDED) { /* no mod non-rec bindings */
					bp->quality = quality ;
					bp->obsocnt = obso_init ; /* freshen recorded routes */
				}
			} else {		/* came from a routing broadcast */
				bp->quality = quality ;
				bp->obsocnt = obso_init ;
				bp->flags &= ~NRB_RECORDED ; /* no longer a recorded route */
			}
		}
	}
		
	/* Now, check to see if we have too many bindings, and drop */
	/* the worst if we do */
	if (rp->num_routes > nr_maxroutes) {
		/* since find_worst never returns permanent entries, the */
		/* limitation on number of routes is circumvented for    */
		/* permanent routes */
		if ((bp = find_worst(rp->routes)) != NULLNRBIND) {
			memcpy(ncall.call,bp->via->call,ALEN) ;
			ncall.ssid = bp->via->call[ALEN] ;
			nr_routedrop(dest,&ncall,bp->via->interface) ;
		}
	}

	return 0 ;
}


/* Drop a route to dest via neighbor */
nr_routedrop(dest,neighbor,ifnum)
struct ax25_addr *dest, *neighbor ;
unsigned ifnum ;
{
	register struct nrroute_tab *rp ;
	register struct nrnbr_tab *np ;
	register struct nr_bind *bp ;

	if ((rp = find_nrroute(dest)) == NULLNRRTAB)
		return -1 ;

	if ((np = find_nrnbr(neighbor,ifnum)) == NULLNTAB)
		return -1 ;

	if ((bp = find_binding(rp->routes,np)) == NULLNRBIND)
		return -1 ;

	/* drop the binding first */
	if (bp->next != NULLNRBIND)
		bp->next->prev = bp->prev ;
	if (bp->prev != NULLNRBIND)
		bp->prev->next = bp->next ;
	else
		rp->routes = bp->next ;

	free((char *)bp) ;
	rp->num_routes-- ;		/* decrement the number of bindings */
	np->refcnt-- ;			/* and the number of neighbor references */
	
	/* now see if we should drop the route table entry */
	if (rp->num_routes == 0) {
		if (rp->next != NULLNRRTAB)
			rp->next->prev = rp->prev ;
		if (rp->prev != NULLNRRTAB)
			rp->prev->next = rp->next ;
		else
			nrroute_tab[nrhash(dest)] = rp->next ;

		free((char *)rp) ;
	}

	/* and check to see if this neighbor can be dropped */
	if (np->refcnt == 0) {
		if (np->next != NULLNTAB)
			np->next->prev = np->prev ;
		if (np->prev != NULLNTAB)
			np->prev->next = np->next ;
		else
			nrnbr_tab[nrhash(neighbor)] = np->next ;

		free((char *)np) ;
	}
	
	return 0 ;
}

/* Find the best neighbor for destination dest, in arp format */
char *
nr_getroute(dest)
struct ax25_addr *dest ;
{
	register struct nrroute_tab *rp ;
	register struct nr_bind *bp ;

	if ((rp = find_nrroute(dest)) == NULLNRRTAB)
		return NULLCHAR ;

	if ((bp = find_best(rp->routes,0)) == NULLNRBIND)	/* shouldn't happen! */
		return NULLCHAR ;

	return bp->via->call ;
}

/* Find an entry in the filter table */
struct nrnf_tab *
find_nrnf(addr,ifnum)
register struct ax25_addr *addr ;
unsigned ifnum ;
{
	int16 hashval ;
	register struct nrnf_tab *fp ;

	/* Find appropriate hash chain */
	hashval = nrhash(addr) ;

	/* search hash chain */
	for (fp = nrnf_tab[hashval] ; fp != NULLNRNFTAB ; fp = fp->next) {
		if (addreq(&fp->neighbor,addr) && fp->interface == ifnum) {
			return fp ;
		}
	}

	return NULLNRNFTAB ;
}

/* Add an entry to the filter table.  Return 0 on success,
 * -1 on failure
 */
int
nr_nfadd(addr,ifnum)
struct ax25_addr *addr ;
unsigned ifnum ;
{
	struct nrnf_tab *fp ;
	int16 hashval ;
	
	if (find_nrnf(addr,ifnum) != NULLNRNFTAB)
		return 0 ;	/* already there; it's a no-op */

	if ((fp = (struct nrnf_tab *)calloc(1,sizeof(struct nrnf_tab)))
		== NULLNRNFTAB)
		return -1 ;	/* no storage */

	hashval = nrhash(addr) ;
	fp->neighbor = *addr ;
	fp->interface = ifnum ;
	fp->next = nrnf_tab[hashval] ;
	if (fp->next != NULLNRNFTAB)
		fp->next->prev = fp ;
	nrnf_tab[hashval] = fp ;

	return 0 ;
}

/* Drop a neighbor from the filter table.  Returns 0 on success, -1
 * on failure.
 */
int
nr_nfdrop(addr,ifnum)
struct ax25_addr *addr ;
unsigned ifnum ;
{
	struct nrnf_tab *fp ;

	if ((fp = find_nrnf(addr,ifnum)) == NULLNRNFTAB)
		return -1 ;	/* not in the table */

	if (fp->next != NULLNRNFTAB)
		fp->next->prev = fp->prev ;
	if (fp->prev != NULLNRNFTAB)
		fp->prev->next = fp->next ;
	else
		nrnf_tab[nrhash(addr)] = fp->next ;

	free((char *)fp) ;

	return 0 ;
}
