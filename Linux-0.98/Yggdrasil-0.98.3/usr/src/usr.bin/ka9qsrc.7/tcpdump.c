#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "internet.h"
#include "timer.h"
#include "tcp.h"
#include "trace.h"

extern FILE *trfp;

/* TCP segment header flags */
char *tcpflags[] = {
	"FIN",	/* 0x01 */
	"SYN",	/* 0x02 */
	"RST",	/* 0x04 */
	"PSH",	/* 0x08 */
	"ACK",	/* 0x10 */
	"URG"	/* 0x20 */
};

/* Dump a TCP segment header. Assumed to be in network byte order */
void
tcp_dump(bpp,source,dest,check)
struct mbuf **bpp;
int32 source,dest;	/* IP source and dest addresses */
int check;		/* 0 if checksum test is to be bypassed */
{
	int i;
	struct tcp seg;
	struct pseudo_header ph;
	int16 csum;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;

	/* Verify checksum */
	ph.source = source;
	ph.dest = dest;
	ph.protocol = TCP_PTCL;
	ph.length = len_mbuf(*bpp);
	csum = cksum(&ph,*bpp,ph.length);

	ntohtcp(&seg,bpp);

	fprintf(trfp,"TCP: %u->%u Seq x%lx",seg.source,seg.dest,seg.seq,seg.ack);
	if(seg.flags & ACK)
		fprintf(trfp," Ack x%lx",seg.ack);
	for(i=0;i<6;i++){
		if(seg.flags & 1 << i){
			fprintf(trfp," %s",tcpflags[i]);
		}
	}
	fprintf(trfp," Wnd %u",seg.wnd);
	if(seg.flags & URG)
		fprintf(trfp," UP x%x",seg.up);
	/* Print options, if any */
	if(seg.mss != 0)
		fprintf(trfp," MSS %u",seg.mss);
	if(check && csum != 0)
		fprintf(trfp," CHECKSUM ERROR (%u)",csum);
	fprintf(trfp,"\n");
}

