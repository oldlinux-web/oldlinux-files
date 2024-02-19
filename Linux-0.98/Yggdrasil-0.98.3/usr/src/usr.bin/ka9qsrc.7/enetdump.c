#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "enet.h"
#include "trace.h"

extern FILE *trfp;

ether_dump(bpp,check)
struct mbuf **bpp;
int check;	/* Not used */
{
	struct ether ehdr;
	char s[20],d[20];

	ntohether(&ehdr,bpp);
	pether(s,ehdr.source);
	pether(d,ehdr.dest);
	fprintf(trfp,"Ether: len %u %s->%s",ETHERLEN + len_mbuf(*bpp),s,d);

	switch(ehdr.type){
		case IP_TYPE:
			fprintf(trfp," type IP\n");
			ip_dump(bpp,1);
			break;
		case ARP_TYPE:
			fprintf(trfp," type ARP\n");
			arp_dump(bpp);
			break;
		default:
			fprintf(trfp," type 0x%x\n",ehdr.type);
			break;
	}
}
