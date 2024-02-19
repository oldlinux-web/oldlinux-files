#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "internet.h"
#include "udp.h"

extern FILE *trfp;

/* Dump a UDP header */
void
udp_dump(bpp,source,dest,check)
struct mbuf **bpp;
int32 source,dest;
int check;		/* If 0, bypass checksum verify */
{
	struct udp udp;
	struct pseudo_header ph;
	int16 csum;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;

	fprintf(trfp,"UDP:");

	/* Compute checksum */
	ph.source = source;
	ph.dest = dest;
	ph.protocol = UDP_PTCL;
	ph.length = len_mbuf(*bpp);
	if((csum = cksum(&ph,*bpp,ph.length)) == 0)
		check = 0;	/* No checksum error */

	ntohudp(&udp,bpp);

	fprintf(trfp," %u->%u",udp.source,udp.dest);
	fprintf(trfp," len %u",udp.length);
	if(udp.checksum == 0)
		check = 0;
	if(check)
		fprintf(trfp," CHECKSUM ERROR (%u)",csum);

	fprintf(trfp,"\n");
}

