/* UDP-related user commands */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "udp.h"
#include "internet.h"
#include "cmdparse.h"

int doudpstat();
struct cmds udpcmds[] = {
	"status",	doudpstat,	0,	NULLCHAR,	NULLCHAR,
	NULLCHAR,	NULLFP,		0,
		"udp subcommands: status",	NULLCHAR,
};

doudp(argc,argv)
int argc;
char *argv[];
{
	return subcmd(udpcmds,argc,argv);
}
/* Dump UDP statistics and control blocks */
doudpstat()
{
	extern struct udp_stat udp_stat;
	char *psocket();
	register struct udp_cb *udp;
	register int i;

	printf("sent %u rcvd %u bdcsts %u cksum err %u unknown socket %u\n",
	udp_stat.sent,udp_stat.rcvd,udp_stat.bdcsts,udp_stat.cksum,udp_stat.unknown);
	printf("    &UCB Rcv-Q  Local socket\n");
	for(i=0;i<NUDP;i++){
		for(udp = udps[i];udp != NULLUDP; udp = udp->next){
			printf("%8lx%6u  %s\n",(long)udp,udp->rcvcnt,
			 psocket(&udp->socket));
		}
	}
}

