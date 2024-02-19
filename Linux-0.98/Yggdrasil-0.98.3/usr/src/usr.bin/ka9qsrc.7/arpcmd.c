#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "enet.h"
#include "ax25.h"
#include "arp.h"
#include "cmdparse.h"
#include <ctype.h>
#ifdef	UNIX
#undef	toupper
#undef	tolower
#endif

extern char badhost[];
extern char nospace[];

int doarpadd(),doarpdrop();
struct cmds arpcmds[] = {
	"add", doarpadd, 4,
	"arp add <hostid> ether|ax25|netrom <ether addr|callsign>",
	"arp add failed",

	"drop", doarpdrop, 3,
	"arp drop <hostid> ether|ax25|netrom",
	"not in table",

	"publish", doarpadd, 4,
	"arp publish <hostid> ether|ax25|netrom <ether addr|callsign>",
	"arp add failed",

	NULLCHAR, NULLFP, 0,
	"arp subcommands: add, drop, publish",
	NULLCHAR, 
};
char *arptypes[] = {
	"NET/ROM",
	"10 Mb Ethernet",
	"3 Mb Ethernet",
	"AX.25",
	"Pronet",
	"Chaos",
	"Arcnet",
	"Appletalk"
};

int
doarp(argc,argv)
int argc;
char *argv[];
{
	if(argc < 2){
		dumparp();
		return 0;
	}
	return subcmd(arpcmds,argc,argv);
}
static
doarpadd(argc,argv)
int argc;
char *argv[];
{
	int16 hwalen,hardware,naddr;
	int32 addr,resolve();
	char *hwaddr;
	struct arp_tab *ap;
	struct arp_type *at;
	void arp_drop();
	int pub = 0;

	if(argv[0][0] == 'p')	/* Is this entry published? */
		pub = 1;
	if((addr = resolve(argv[1])) == 0){
		printf(badhost,argv[1]);
		return 1;
	}
	/* This is a kludge. It really ought to be table driven */
	switch(tolower(argv[2][0])){
	case 'n':	/* Net/Rom pseudo-type */
		hardware = ARP_NETROM;
		naddr = argc - 3 ;
		if (naddr != 1) {
			printf("No digipeaters in NET/ROM arp entries - ") ;
			printf("use netrom route add\n") ;
			return 1 ;
		}
		break;
	case 'e':	/* "ether" */
		hardware = ARP_ETHER;
		naddr = 1;
		break;		
	case 'a':	/* "ax25" */
		hardware = ARP_AX25;
		naddr = argc - 3;
		break;
	case 'm':	/* "mac appletalk" */
		hardware = ARP_APPLETALK;
		naddr = 1;
		break;
	default:
		printf("unknown hardware type \"%s\"\n",argv[2]);
		return -1;
	}
	/* If an entry already exists, clear it */
	if((ap = arp_lookup(hardware,addr)) != NULLARP)
		arp_drop(ap);

	at = &arp_type[hardware];
	if(at->scan == NULLFP){
		printf("Attach device first\n");
		return 1;
	}
	/* Allocate buffer for hardware address and fill with remaining args */
	hwalen = at->hwalen * naddr;
	if((hwaddr = malloc(hwalen)) == NULLCHAR){
		printf(nospace);
		return 0;
	}
	/* Destination address */
	(*at->scan)(hwaddr,&argv[3],argc - 3);
	ap = arp_add(addr,hardware,hwaddr,hwalen,pub);	/* Put in table */
	free(hwaddr);					/* Clean up */
	stop_timer(&ap->timer);			/* Make entry permanent */
	ap->timer.count = ap->timer.start = 0;
	return 0;
}
/* Remove an ARP entry */
/*ARGSUSED*/
static
doarpdrop(argc,argv)
int argc;
char *argv[];
{
	int16 hardware;
	int32 addr,resolve();
	struct arp_tab *ap;
	void arp_drop();

	if((addr = resolve(argv[1])) == 0){
		printf(badhost,argv[1]);
		return 1;
	}
	/* This is a kludge. It really ought to be table driven */
	switch(tolower(argv[2][0])){
	case 'n':
		hardware = ARP_NETROM;
		break;
	case 'e':	/* "ether" */
		hardware = ARP_ETHER;
		break;		
	case 'a':	/* "ax25" */
		hardware = ARP_AX25;
		break;
	case 'm':	/* "mac appletalk" */
		hardware = ARP_APPLETALK;
		break;
	default:
		hardware = 0;
		break;
	}
	if((ap = arp_lookup(hardware,addr)) == NULLARP)
		return -1;
	arp_drop(ap);
	return 0;	
}
/* Dump ARP table */
static
dumparp()
{
	register int i;
	extern struct arp_stat arp_stat;
	register struct arp_tab *ap;
	char e[128];
	char *inet_ntoa();
	extern char *arptypes[];

	printf("received %u badtype %u bogus addr %u reqst in %u replies %u reqst out %u\n",
	 arp_stat.recv,arp_stat.badtype,arp_stat.badaddr,arp_stat.inreq,
	 arp_stat.replies,arp_stat.outreq);

	printf("IP addr         Type           Time Q Addr\n");
	for(i=0;i<ARPSIZE;i++){
		for(ap = arp_tab[i];ap != (struct arp_tab *)NULL;ap = ap->next){
			printf("%-16s",inet_ntoa(ap->ip_addr));
			printf("%-15s",arptypes[ap->hardware]);
			printf("%-5ld",ap->timer.count*(long)MSPTICK/1000);
			if(ap->state == ARP_PENDING)
				printf("%-2u",len_q(ap->pending));
			else
				printf("  ");
			if(ap->state == ARP_VALID){
				if(arp_type[ap->hardware].format != NULLFP){
					(*arp_type[ap->hardware].format)(e,ap->hw_addr);
				} else {
					e[0] = '\0';
				}
				printf("%s",e);
			} else {
				printf("[unknown]");
			}
			if(ap->pub)
				printf(" (published)");
			printf("\n");
		}
	}
	return 0;
}
