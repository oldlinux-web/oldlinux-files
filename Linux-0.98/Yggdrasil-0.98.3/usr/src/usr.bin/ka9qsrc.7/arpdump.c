#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "arp.h"
#include "ax25.h"

extern FILE *trfp;

arp_dump(bpp)
struct mbuf **bpp;
{
	struct arp arp;
	char *inet_ntoa();
	struct arp_type *at;
	int is_ip = 0;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;
	fprintf(trfp,"ARP: len %d",len_mbuf(*bpp));
	if(ntoharp(&arp,bpp) == -1){
		fprintf(trfp," bad packet\n");
		return;
	}
	/* Print hardware type in Ascii if known, numerically if not */
	if(arp.hardware < NHWTYPES){
		at = &arp_type[arp.hardware];
		fprintf(trfp," hwtype %s",arptypes[arp.hardware]);
	} else {
		at = NULLATYPE;
		fprintf(trfp," hwtype %u",arp.hardware);
	}
	/* Print hardware length only if unknown type, or if it doesn't match
	 * the length in the known types table
	 */
	if(at == NULLATYPE || arp.hwalen != at->hwalen)
		fprintf(trfp," hwlen %u",arp.hwalen);

	/* Check for most common case -- upper level protocol is IP */
	if(at != NULLATYPE && arp.protocol == at->iptype){
		fprintf(trfp," prot IP");
		is_ip = 1;
	} else {
		fprintf(trfp," prot 0x%x prlen %u",arp.protocol,arp.pralen);
	}
	switch(arp.opcode){
	case ARP_REQUEST:
		fprintf(trfp," op REQUEST");
		break;
	case ARP_REPLY:
		fprintf(trfp," op REPLY");
		break;
	default:
		fprintf(trfp," op %u",arp.opcode);
		break;
	}
	if(is_ip)
		fprintf(trfp," target %s",inet_ntoa(arp.tprotaddr));
	fprintf(trfp,"\n");
}
