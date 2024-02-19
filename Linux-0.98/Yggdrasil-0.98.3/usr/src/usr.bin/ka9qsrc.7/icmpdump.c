#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "internet.h"
#include "icmp.h"
#include "trace.h"

extern FILE *trfp;

/* Dump an ICMP header */
/*ARGSUSED*/
void
icmp_dump(bpp,source,dest,check)
struct mbuf **bpp;
int32 source,dest;
int check;		/* If 0, bypass checksum verify */
{
	struct icmp icmp;
	int16 csum;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;
	csum = cksum(NULLHEADER,*bpp,len_mbuf(*bpp));
	
	ntohicmp(&icmp,bpp);
	
	if(uchar(icmp.type) <= 16 && icmptypes[uchar(icmp.type)] != NULLCHAR)
		fprintf(trfp,"ICMP: %s",icmptypes[uchar(icmp.type)]);
	else
		fprintf(trfp,"ICMP: type %u",uchar(icmp.type));

	switch(uchar(icmp.type)){
	case DEST_UNREACH:
		if(uchar(icmp.code) <= 5)
			fprintf(trfp," %s",unreach[uchar(icmp.code)]);
		else
			fprintf(trfp," code %u",uchar(icmp.code));
		break;
	case REDIRECT:
		if(uchar(icmp.code) <= 3)
			fprintf(trfp," %s",redirect[uchar(icmp.code)]);
		else
			fprintf(trfp," code %u",uchar(icmp.code));
		break;
	case TIME_EXCEED:
		if(uchar(icmp.code) <= 1)
			fprintf(trfp," %s",exceed[uchar(icmp.code)]);
		else
			fprintf(trfp," code %u",uchar(icmp.code));
		break;
	case PARAM_PROB:
		fprintf(trfp," pointer = %u",icmp.args.pointer);
		break;
	case ECHO:
	case ECHO_REPLY:
	case INFO_RQST:
	case INFO_REPLY:
	case TIMESTAMP:
	case TIME_REPLY:
		fprintf(trfp," id %u seq %u",icmp.args.echo.id,icmp.args.echo.seq);
		break;
	}
	if(check && csum != 0){
		fprintf(trfp," CHECKSUM ERROR (%u)",csum);
	}
	fprintf(trfp,"\n");
	/* Dump the offending IP header, if any */
	switch(icmp.type){
	case DEST_UNREACH:
	case TIME_EXCEED:
	case PARAM_PROB:
	case QUENCH:
	case REDIRECT:
		fprintf(trfp,"Returned ");
		ip_dump(bpp,0);
	}
}

