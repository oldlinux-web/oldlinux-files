/* ICMP-related user commands */
#include <stdio.h>
#include "global.h"
#include "icmp.h"
#include "mbuf.h"
#include "netuser.h"
#include "internet.h"
#include "timer.h"
#include "ping.h"

int
doicmpstat()
{
	extern struct icmp_errors icmp_errors;
	extern struct icmp_stats icmp_stats;
	extern char *icmptypes[];
	register int i;

	printf("ICMP: chksum err %u no space %u icmp %u bdcsts %u\n",
	 icmp_errors.checksum,icmp_errors.nospace,icmp_errors.noloop,
	 icmp_errors.bdcsts);
	printf("type  rcvd  sent\n");
	for(i=0;i<ICMP_TYPES;i++){
		if(icmp_stats.input[i] == 0 && icmp_stats.output[i] == 0)
			continue;
		printf("%-6u%-6u%-6u",i,icmp_stats.input[i],
			icmp_stats.output[i]);
		if(icmptypes[i] != NULLCHAR)
			printf("  %s",icmptypes[i]);
		printf("\n");
	}
	return 0;
}

/* Hash table list heads */
struct ping *ping[PMOD];

/* Counter for generating seq numbers */
static int16 iclk;

/* Increment counter -- called by low level clock tick */
icmpclk()
{
	iclk++;
}

/* Send ICMP Echo Request packets */
doping(argc,argv)
int argc;
char *argv[];
{
	int32 dest;
	struct ping *add_ping(),*pp1;
	register struct ping *pp;
	void ptimeout();
	int16 hval,hash_ping();
	int i;
	char *inet_ntoa();

	if(argc < 2){
		printf("Host                Sent    Rcvd   %%   Avg RTT  Interval\n");
		for(i=0;i<PMOD;i++){
			for(pp = ping[i];pp != NULLPING;pp = pp->next){
				printf("%-16s",inet_ntoa(pp->remote));
				printf("%8lu%8lu",pp->count,pp->echoes);
				printf("%4lu%10lu%10lu\n",
				 (long)pp->echoes * 100 / pp->count,
				 pp->echoes != 0 ? 
				 (long)pp->ttotal * MSPTICK / pp->echoes : 0,
				 ((long)pp->timer.start * MSPTICK + 500) / 1000);
			}
		}
		return 0;
	}
	if(strcmp(argv[1],"clear") == 0){
		for(i=0;i<PMOD;i++){
			for(pp = ping[i];pp != NULLPING;pp = pp1){
				pp1 = pp->next;
				del_ping(pp);
			}
		}
		return 0;
	}
	if((dest = resolve(argv[1])) == 0){
		printf("Host %s unknown\n",argv[1]);
		return 1;
	}
	/* See if dest is already in table */
	hval = hash_ping(dest);
	for(pp = ping[hval]; pp != NULLPING; pp = pp->next){
		if(pp->remote == dest){
			break;
		}
	}
	if(argc > 2){
		/* Inter-ping time is specified; set up timer structure */
		if(pp == NULLPING)
			pp = add_ping(dest);
		pp->timer.start = atoi(argv[2]) * (1000.0 / MSPTICK);
		pp->timer.func = ptimeout;
		pp->timer.arg = (char *)pp;
		pp->remote = dest;
		start_timer(&pp->timer);
		pp->count++;
		(void)pingem(dest,iclk,REPEAT);
	} else
		(void)pingem(dest,iclk,ONESHOT);

	return 0;
}

/* Called by ping timeout */
void
ptimeout(p)
char *p;
{
	register struct ping *pp;

	/* Send another ping */
	pp = (struct ping *)p;
	pp->count++;
	(void)pingem(pp->remote,iclk,REPEAT);
	start_timer(&pp->timer);
}
/* Send ICMP Echo Request packet */
static
pingem(dest,seq,id)
int32 dest;
int16 seq;
int16 id;
{
	struct mbuf *bp,*htonicmp();
	struct icmp icmp;
	extern struct icmp_stats icmp_stats;

	icmp_stats.output[ECHO]++;
	icmp.type = ECHO;
	icmp.code = 0;
	icmp.args.echo.seq = seq;
	icmp.args.echo.id = id;
	if((bp = htonicmp(&icmp,NULLBUF)) == NULLBUF)
		return 0;
	return ip_send(ip_addr,dest,ICMP_PTCL,0,0,bp,len_mbuf(bp),0,0);
}

/* Called with incoming Echo Reply packet */
/*ARGSUSED*/
int
echo_proc(source,dest,icmp)
int32 source,dest;
struct icmp *icmp;
{
	register struct ping *pp;
	int16 hval,hash_ping();
	int16 rtt;
	char *inet_ntoa();

	rtt = iclk - icmp->args.echo.seq;
	hval = hash_ping(source);
	for(pp = ping[hval]; pp != NULLPING; pp = pp->next)
		if(pp->remote == source)
			break;
	if(pp == NULLPING || icmp->args.echo.id != 1){
		printf("%s: echo reply id %u seq %u, %lu ms\n",
		 inet_ntoa(source),
		 icmp->args.echo.id,icmp->args.echo.seq,
		 (long)rtt * MSPTICK);
		 fflush(stdout);
	} else {
		/* Repeated poll, just keep stats */
		pp->ttotal += rtt;
		pp->echoes++;
	}
}
static
int16
hash_ping(dest)
int32 dest;
{
	int16 hval;

	hval = (hiword(dest) ^ loword(dest)) % PMOD;
	return hval;
}
/* Add entry to ping table */
static
struct ping *
add_ping(dest)
int32 dest;
{
	struct ping *pp;
	int16 hval,hash_ping();

	pp = (struct ping *)calloc(1,sizeof(struct ping));
	if(pp == NULLPING)
		return NULLPING;

	hval = hash_ping(dest);
	pp->prev = NULLPING;
	pp->next = ping[hval];
	if(pp->next != NULLPING)
		pp->next->prev = pp;
	ping[hval] = pp;
	return pp;
}
/* Delete entry from ping table */
static
del_ping(pp)
struct ping *pp;
{
	int16 hval,hash_ping();

	stop_timer(&pp->timer);

	if(pp->next != NULLPING)
		pp->next->prev = pp->prev;
	if(pp->prev != NULLPING) {
		pp->prev->next = pp->next;
	} else {
		hval = hash_ping(pp->remote);
		ping[hval] = pp->next;
	}
	free((char *)pp);	
}

