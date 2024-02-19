#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "mac_AT.h"
#include "trace.h"

extern FILE *trfp;

at_dump(bpp)
struct mbuf **bpp;
{
	struct	appletalk ap;
	char s[20],d[20];

	ntohat(&ap,bpp);
	pat(s,ap.source);
	pat(d,ap.dest);
	fprintf(trfp,"AppleTalk: %s->%s type 0x%x len %u\r\n",s,d,ap.type,
		APPLEADDRLEN + len_mbuf(*bpp));
	switch(ap.type){
		case AIP_TYPE:
			ip_dump(bpp);
			break;
		case AARP_TYPE:
			arp_dump(bpp);
			break;
	}
}
