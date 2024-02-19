/* Functions for level 3 net/rom support.
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */
 
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "netrom.h"
#include "lapb.h"
#include <ctype.h>
#ifdef	UNIX
#include <memory.h>
#endif

/* Convert a net/rom network header to host format structure
 * Return -1 if error, 0 if OK
 */

int
ntohnr3(hdr,bpp)
register struct nr3hdr *hdr ;	/* output structure */
struct mbuf **bpp ;
{
	char *getaxaddr() ;
	char buf[AXALEN] ;

	if (pullup(bpp,buf,AXALEN) < AXALEN)
		return -1 ;
	getaxaddr(&hdr->source,buf) ;

	if (pullup(bpp,buf,AXALEN) < AXALEN)
		return -1 ;
	getaxaddr(&hdr->dest,buf) ;

	if (pullup(bpp,(char *)&hdr->ttl,1) != 1)
		return -1 ;

	return 0 ;
}

/* Convert a host-format net/rom level 3 header into an mbuf ready
 * for transmission.
 */

struct mbuf *
htonnr3(hdr)
register struct nr3hdr *hdr;
{
	struct mbuf *rbuf ;
	register char *cp ;
	char *putaxaddr();

	if (hdr == (struct nr3hdr *) NULL)
		return NULLBUF ;

	/* Allocate space for return buffer */
	if ((rbuf = alloc_mbuf(NR3HLEN)) == NULLBUF)
		return NULLBUF ;

	rbuf->cnt = NR3HLEN ;

	/* Now convert */
	cp = rbuf->data ;

	hdr->source.ssid &= ~E ;	/* source E-bit is always off */
	hdr->dest.ssid |= E ;		/* destination E-bit always set */

	cp = putaxaddr(cp,&hdr->source) ;
	cp = putaxaddr(cp,&hdr->dest) ;
	*cp = hdr->ttl ;

	return rbuf ;
}

/* Convert a net/rom routing broadcast destination subpacket from
 * network format to a host format structure.  Return -1 if error,
 * 0 if OK.
 */
int ntohnrdest(ds,bpp)
register struct nr3dest *ds ;
struct mbuf **bpp ;
{
	char buf[AXALEN] ;
	char quality ;

	/* get destination callsign */
	if (pullup(bpp,buf,AXALEN) < AXALEN)
		return -1 ;
	memcpy(ds->dest.call,buf,ALEN) ;
	ds->dest.ssid = buf[ALEN] ;

	/* get destination alias */
	if (pullup(bpp,ds->alias,ALEN) < ALEN)
		return -1 ;
	ds->alias[ALEN] = '\0' ;

	/* get best neighbor callsign */
	if (pullup(bpp,buf,AXALEN) < AXALEN)
		return -1 ;
	memcpy(ds->neighbor.call,buf,ALEN) ;
	ds->neighbor.ssid = buf[ALEN] ;

	/* get route quality */
	if (pullup(bpp,&quality,1) < 1)
		return -1 ;
	ds->quality = uchar(quality) ;

	return 0 ;
}

/* Convert a host-format net/rom destination subpacket into an
 * mbuf ready for transmission as part of a route broadcast
 * packet.
 */
struct mbuf *
htonnrdest(ds)
register struct nr3dest *ds ;
{
	struct mbuf *rbuf ;
	register char *cp ;

	if (ds == (struct nr3dest *) NULL)
		return NULLBUF ;

	/* Allocate space for return buffer */
	if ((rbuf = alloc_mbuf(NRRTDESTLEN)) == NULLBUF)
		return NULLBUF ;

	rbuf->cnt = NRRTDESTLEN ;

	cp = rbuf->data ;

	memcpy(cp,ds->dest.call,ALEN) ;
	cp += ALEN ;
	*cp++ = ds->dest.ssid ;

	memcpy(cp,ds->alias,ALEN) ;
	cp += ALEN ;

	memcpy(cp,ds->neighbor.call,ALEN) ;
	cp += ALEN ;
	*cp++ = ds->neighbor.ssid ;

	*cp = uchar(ds->quality) ;

	return rbuf ;
}

