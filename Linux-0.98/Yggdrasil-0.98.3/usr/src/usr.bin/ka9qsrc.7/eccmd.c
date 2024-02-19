#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "ec.h"
int
doetherstat(argc,argv)
int argc;
char *argv[];
{
	register struct ec *ecp;
	char buf[20];

	for(ecp = ec;ecp < &ec[nec]; ecp++){
		pether(buf,ecp->iface->hwaddr);
		printf("Controller %u, Ethernet address %s\n",ecp-ec,buf);

		printf("recv      bad       overf     drop      nomem     intrpt\n");
		printf("%-10lu%-10lu%-10lu%-10lu%-10lu%-10lu\n",
		 ecp->estats.recv,ecp->estats.bad,ecp->estats.over,
		 ecp->estats.drop,ecp->estats.nomem,ecp->estats.intrpt);

		printf("xmit      timeout   jam       jam16\n");
		printf("%-10lu%-10lu%-10lu%-10lu\n",
		 ecp->estats.xmit,ecp->estats.timeout,ecp->estats.jam,
		 ecp->estats.jam16);
	}
	return 0;
}

