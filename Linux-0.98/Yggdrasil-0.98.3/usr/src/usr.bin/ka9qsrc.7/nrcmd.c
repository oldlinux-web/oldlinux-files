/* net/rom user command processing
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */

#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "ax25.h"
#include "timer.h"
#include "iface.h"
#include "lapb.h"
#include "netrom.h"
#include "nr4.h"
#include "netuser.h"
#include "tcp.h"
#include "ftp.h"
#include "telnet.h"
#include "finger.h"
#include "ax_mbx.h"
#include "cmdparse.h"
#include "session.h"
#include <ctype.h>
#ifdef	UNIX
#undef	toupper
#undef	tolower
#include <memory.h>
#include <string.h>
#endif

#undef NRDEBUG

long atol();
extern struct session *current;

char *Nr4states[] = {
	"Disconnected",
	"Conn Pending",
	"Connected",
	"Disc Pending"
} ;

char *Nr4reasons[] = {
	"Normal",
	"By Peer",
	"Timeout",
	"Reset",
	"Refused"
} ;

static int dointerface(), dobcnodes(), donodetimer(), donrroute(),
		   donrttl(), doobsotimer(), donodefilter(), donrverbose(),
		   donrconnect(), donrreset(), donrwindow(), donrirtt(),
		   donracktime(), donrqlimit(), donrchoketime(), donrretries(),
		   donrstatus(), donrkick() ;

static struct cmds nrcmds[] = {
	"acktime",	donracktime,	0,	NULLCHAR,	NULLCHAR,
	"bcnodes",	dobcnodes,	2,	"netrom bcnodes <interface>", NULLCHAR,
	
/* Put connect before choketime to make it the default expansion of 'c' */

	"connect",	donrconnect,2,	"netrom connect <node>",	NULLCHAR,
	"choketime",	donrchoketime,	0,	NULLCHAR,	NULLCHAR,
	"interface",	dointerface,	4,
		"netrom interface <interface> <alias> <quality>",	NULLCHAR,
	"irtt",			donrirtt,		0,	NULLCHAR,	NULLCHAR,
	"kick",			donrkick,		2,	"netrom kick <&nrcb>",	NULLCHAR,
	"nodefilter",	donodefilter,	0,	NULLCHAR,	NULLCHAR,
	"nodetimer",	donodetimer,	0,	NULLCHAR,	NULLCHAR,
	"obsotimer",	doobsotimer,	0,	NULLCHAR,	NULLCHAR,
	"qlimit",	donrqlimit,	0,	NULLCHAR,	NULLCHAR,
	"reset",	donrreset,	2,	"netrom reset <&nrcb>",	NULLCHAR,
	"retries",	donrretries,0,	NULLCHAR,	NULLCHAR,
	"route",	donrroute,	0,	NULLCHAR,	NULLCHAR,
	"status",	donrstatus,	0,	NULLCHAR,	NULLCHAR,
	"ttl",		donrttl,	0,	NULLCHAR,	NULLCHAR,
	"verbose",	donrverbose,0,	NULLCHAR,	NULLCHAR,
	"window",	donrwindow,	0,	NULLCHAR,	NULLCHAR,
	NULLCHAR,	NULLFP,		0,
		"netrom subcommands: acktime bcnodes connect choketime interface irtt kick\n                    nodetimer nodefilter obsotimer qlimit reset retries route\n                    status ttl verbose window",
		NULLCHAR
} ;

static struct timer nodetimer ;	/* timer for nodes broadcasts */
static struct timer obsotimer ;	/* timer for aging routes */

/* Command multiplexer */
donetrom(argc,argv)
int argc ;
char *argv[] ;
{
	return subcmd(nrcmds,argc,argv) ;
}

static int dorouteadd(), doroutedrop(), doroutedump(), dorouteinfo() ;

static struct cmds routecmds[] = {
	"add",	dorouteadd,	6,
		"netrom route add <alias> <destination> <interface> <quality> <neighbor>",
		"add failed",
	"drop",	doroutedrop, 4,
		"netrom route drop <destination> <neighbor> <interface>",
		"drop failed",
	"info", dorouteinfo, 2,
		"netrom route info <destination>", NULLCHAR,
	NULLCHAR,	NULLFP,	0,
		"netrom route subcommands: add drop info",
		NULLCHAR
} ;

/* Route command multiplexer */
static
donrroute(argc, argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		doroutedump() ;
		return 0 ;
	}
	return subcmd(routecmds,argc,argv) ;
}

/* Dump a list of known routes */
static
doroutedump()
{
	register struct nrroute_tab *rp ;
	register int i, column ;
	char buf[32] ;
	char *cp ;
	int count = 0;
	
	column = 1 ;
	
	for (i = 0 ; i < NRNUMCHAINS ; i++)
		for (rp = nrroute_tab[i] ; rp != NULLNRRTAB ; rp = rp->next) {
			strcpy(buf,rp->alias) ;
			/* remove trailing spaces */
			if ((cp = index(buf,' ')) == NULLCHAR)
				cp = &buf[strlen(buf)] ;
			if (cp != buf)		/* don't include colon for null alias */
				*cp++ = ':' ;
			pax25(cp,&rp->call) ;
			printf("%-16s  ",buf) ;
			count++;
			if (column++ == 4) {
				printf("\n") ;
				column = 1 ;
			}
		}

	if (column != 1)
		printf("\n") ;
	printf("Total station count = %d\n",count);
		
	return 0 ;
}

/* print detailed information on an individual route */
/*ARGSUSED*/
static int
dorouteinfo(argc,argv)
int argc ;
char *argv[] ;
{
	register struct nrroute_tab *rp ;
	register struct nr_bind *bp ;
	register struct nrnbr_tab *np ;
	struct ax25_addr dest ;
	char neighbor[60] ;

	if (setcall(&dest,argv[1]) == -1) {
		printf ("bad destination name\n") ;
		return -1 ;
	}
		
	if ((rp = find_nrroute(&dest)) == NULLNRRTAB) {
		printf("no such route\n") ;
		return -1 ;
	}

	for (bp = rp->routes ; bp != NULLNRBIND ; bp = bp->next) {
		np = bp->via ;
		psax25(neighbor,np->call) ;
		printf("%1s %3d  %3d  %-8s  %s\n",
				(bp->flags & NRB_PERMANENT ? "P" :
				 bp->flags & NRB_RECORDED ? "R" : " "),
				bp->quality,bp->obsocnt,
				nrifaces[np->interface].interface->name,
				neighbor) ;
	}
	return 0 ;
}
		
/* convert a null-terminated alias name to a blank-filled, upcased */
/* version.  Return -1 on failure. */
static int
putalias(to,from,complain)
register char *to, *from ;
int complain ;
{
	int len, i ;
	
	if ((len = strlen(from)) > ALEN) {
		if (complain)
			printf ("alias too long - six characters max\n") ;
		return -1 ;
	}
	
	for (i = 0 ; i < ALEN ; i++) {
		if (i < len) {
			if (islower(*from))
				*to++ = toupper(*from++) ;
			else
				*to++ = *from++ ;
		}
		else
			*to++ = ' ' ;
	}
			
	*to = '\0' ;
	return 0 ;
}

/* Add a route */
static int
dorouteadd(argc, argv)
int argc ;
char *argv[] ;
{
	char alias[7] ;
	struct ax25_addr dest ;
	unsigned quality ;
	char neighbor[AXALEN * 3] ;
	register int i ;
	int naddr ;

	/* format alias (putalias prints error message if necessary) */
	if (putalias(alias,argv[1],1) == -1)
		return -1 ;

	/* format destination callsign */
	if (setcall(&dest,argv[2]) == -1) {
		printf("bad destination callsign\n") ;
		return -1 ;
	}

	/* find interface */
	for (i = 0 ; i < nr_numiface ; i++)
		if (!strcmp(nrifaces[i].interface->name,argv[3]))
			break ;
	if (i == nr_numiface) {
		printf("Interface \"%s\" not found\n",argv[3]) ;
		return -1 ;
	}
	
	/* get and check quality value */
	if ((quality = atoi(argv[4])) > 255) {
		printf("maximum route quality is 255\n") ;
		return -1 ;
	}

	/* make sure no more than 2 digis */
	naddr = argc - 5 ;
	if (naddr > 3) {
		printf("no more than 2 digipeaters for a net/rom neighbor\n") ;
		return -1 ;
	}
	
	/* format neighbor address string */
	setpath(neighbor,&argv[5],naddr) ;

	return nr_routeadd(alias,&dest,(unsigned)i,quality,neighbor,1,0) ;
}


/* drop a route */
/*ARGSUSED*/
static
doroutedrop(argc,argv)
int argc ;
char *argv[] ;
{
	struct ax25_addr dest, neighbor ;
	register int i ;

	/* format destination and neighbor callsigns */
	if (setcall(&dest,argv[1]) == -1) {
		printf("bad destination callsign\n") ;
		return -1 ;
	}
	if (setcall(&neighbor,argv[2]) == -1) {
		printf("bad neighbor callsign\n") ;
		return -1 ;
	}

	/* find interface */
	for (i = 0 ; i < nr_numiface ; i++)
		if (!strcmp(nrifaces[i].interface->name,argv[3]))
			break ;
	if (i == nr_numiface) {
		printf("Interface \"%s\" not found\n",argv[3]) ;
		return -1 ;
	}

	return nr_routedrop(&dest,&neighbor,(unsigned)i) ;
}
	
	
/* make an interface available to net/rom */
/*ARGSUSED*/
static int
dointerface(argc,argv)
int argc ;
char *argv[] ;
{
	int i ;
	register struct interface *ifp ;
	extern struct interface *ifaces ;

	if (nr_interface == NULLIF) {
		printf("Attach netrom interface first\n") ;
		return 1 ;
	}
	
	if (nr_numiface >= NRNUMIFACE) {
		printf("Only %d net/rom interfaces available\n",NRNUMIFACE) ;
		return 1 ;
	}
	
	for(ifp=ifaces;ifp != NULLIF;ifp = ifp->next){
		if(strcmp(argv[1],ifp->name) == 0)
			break;
	}
	if(ifp == NULLIF){
		printf("Interface \"%s\" unknown\n",argv[1]);
		return 1;
	}
	for (i = 0 ; i < nr_numiface ; i++)
		if (nrifaces[i].interface == ifp) {
			printf("Interface \"%s\" is already registered\n",argv[1]) ;
			return 1 ;
		}
		
	nrifaces[nr_numiface].interface = ifp ;

	if (putalias(nrifaces[nr_numiface].alias,argv[2],1) == -1)
		return 1 ;
		
	if ((nrifaces[nr_numiface].quality = atoi(argv[3])) > 255) {
		printf("Quality cannot be greater than 255\n") ;
		return 1 ;
	}
		
	nr_numiface++ ;			/* accept this interface */
	return 0 ;
}

/* Broadcast nodes list on named interface. */

/*ARGSUSED*/
static int
dobcnodes(argc,argv)
int argc ;
char *argv[] ;
{
	register int i ;
	for (i = 0 ; i < nr_numiface ; i++)
		if (!strcmp(nrifaces[i].interface->name,argv[1]))
			break ;
	if (i == nr_numiface) {
		printf("Interface \"%s\" not found\n",argv[1]) ;
		return 1 ;
	}
		
	nr_bcnodes((unsigned)i) ;
	return 0 ;
}

#define TICKSPERSEC	(1000L / MSPTICK)	/* Ticks per second */

/* Set outbound node broadcast interval */
static int
donodetimer(argc,argv)
int argc;
char *argv[];
{
	int donodetick();

	if(argc < 2){
		printf("%lu/%lu\n",
				(nodetimer.start - nodetimer.count)/TICKSPERSEC,
				nodetimer.start/TICKSPERSEC);
		return 0;
	}
	stop_timer(&nodetimer) ;	/* in case it's already running */
	nodetimer.func = (void (*)())donodetick;/* what to call on timeout */
	nodetimer.arg = NULLCHAR;		/* dummy value */
	nodetimer.start = atol(argv[1])*TICKSPERSEC;	/* set timer duration */
	start_timer(&nodetimer);		/* and fire it up */
	return 0;
}

static int
donodetick()
{
	register int i ;

	for (i = 0 ; i < nr_numiface ; i++)
		nr_bcnodes((unsigned)i) ;

	/* Restart timer */
	start_timer(&nodetimer) ;
}

/* Set timer for aging routes */
static int
doobsotimer(argc,argv)
int argc;
char *argv[];
{
	extern int doobsotick();

	if(argc < 2){
		printf("%lu/%lu\n",(obsotimer.start - obsotimer.count)/TICKSPERSEC,
		obsotimer.start/TICKSPERSEC);
		return 0;
	}
	stop_timer(&obsotimer) ;	/* just in case it's already running */
	obsotimer.func = (void (*)())doobsotick;/* what to call on timeout */
	obsotimer.arg = NULLCHAR;		/* dummy value */
	obsotimer.start = atol(argv[1])*TICKSPERSEC;	/* set timer duration */
	start_timer(&obsotimer);		/* and fire it up */
	return 0;
}


/* Go through the routing table, reducing the obsolescence count of
 * non-permanent routes, and purging them if the count reaches 0
 */
static int
doobsotick()
{
	register struct nrnbr_tab *np ;
	register struct nrroute_tab *rp, *rpnext ;
	register struct nr_bind *bp, *bpnext ;
	struct ax25_addr neighbor ;
	int16 nrhash();
	int i ;

	for (i = 0 ; i < NRNUMCHAINS ; i++) {
		for (rp = nrroute_tab[i] ; rp != NULLNRRTAB ; rp = rpnext) {
			rpnext = rp->next ; 	/* save in case we free this route */
			for (bp = rp->routes ; bp != NULLNRBIND ; bp = bpnext) {
				bpnext = bp->next ;	/* in case we free this binding */
				if (bp->flags & NRB_PERMANENT)	/* don't age these */
					continue ;
				if (--bp->obsocnt == 0) {		/* time's up! */
					if (bp->next != NULLNRBIND)
						bp->next->prev = bp->prev ;
					if (bp->prev != NULLNRBIND)
						bp->prev->next = bp->next ;
					else
						rp->routes = bp->next ;
					rp->num_routes-- ;			/* one less binding */
					np = bp->via ;				/* find the neighbor */
					free((char *)bp) ;					/* now we can free the bind */
					/* Check to see if we can free the neighbor */
					if (--np->refcnt == 0) {
						if (np->next != NULLNTAB)
							np->next->prev = np->prev ;
						if (np->prev != NULLNTAB)
							np->prev->next = np->next ;
						else {
							memcpy(neighbor.call,np->call,ALEN) ;
							neighbor.ssid = np->call[ALEN] ;
							nrnbr_tab[(int)nrhash(&neighbor)] = np->next ;
						}
						free((char *)np) ;	/* free the storage */
					}
				}
			}
			if (rp->num_routes == 0) {		/* did we free them all? */
				if (rp->next != NULLNRRTAB)
					rp->next->prev = rp->prev ;
				if (rp->prev != NULLNRRTAB)
					rp->prev->next = rp->next ;
				else
					nrroute_tab[i] = rp->next ;

				free((char *)rp) ;
			}
		}
	}

	start_timer(&obsotimer) ;
}


static int donfadd(), donfdrop(), donfmode() ;

static struct cmds nfcmds[] = {
	"add",	donfadd,	3,
		"netrom nodefilter add <neighbor> <interface>",
		"add failed",
	"drop",	donfdrop,	3,
		"netrom nodefilter drop <neighbor> <interface>",
		"drop failed",
	"mode",	donfmode,	0,	NULLCHAR,	NULLCHAR,
	NULLCHAR,	NULLFP,	0,
		"nodefilter subcommands: add drop mode",
		NULLCHAR
} ;

/* nodefilter command multiplexer */
static
donodefilter(argc,argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		donfdump() ;
		return 0 ;
	}
	return subcmd(nfcmds,argc,argv) ;
}

/* display a list of <callsign,interface> pairs from the filter
 * list.
 */
static
donfdump()
{
	int i, column = 1 ;
	struct nrnf_tab *fp ;
	char buf[16] ;

	for (i = 0 ; i < NRNUMCHAINS ; i++)
		for (fp = nrnf_tab[i] ; fp != NULLNRNFTAB ; fp = fp->next) {
			pax25(buf,&fp->neighbor) ;
			printf("%-7s %-8s  ",
					buf,nrifaces[fp->interface].interface->name) ;
			if (column++ == 4) {
				printf("\n") ;
				column = 1 ;
			}
		}

	if (column != 1)
		printf("\n") ;

	return 0 ;
}

/* add an entry to the filter table */
/*ARGSUSED*/
static
donfadd(argc,argv)
int argc ;
char *argv[] ;
{
	struct ax25_addr neighbor ;
	register int i ;

	/* format callsign */
	if (setcall(&neighbor,argv[1]) == -1) {
		printf("bad neighbor callsign\n") ;
		return -1 ;
	}

	/* find interface */
	for (i = 0 ; i < nr_numiface ; i++)
		if (!strcmp(nrifaces[i].interface->name,argv[2]))
			break ;
	if (i == nr_numiface) {
		printf("Interface \"%s\" not found\n",argv[2]) ;
		return -1 ;
	}

	return nr_nfadd(&neighbor,(unsigned)i) ;
}

/* drop an entry from the filter table */
/*ARGSUSED*/
static
donfdrop(argc,argv)
int argc ;
char *argv[] ;
{
	struct ax25_addr neighbor ;
	register int i ;

	/* format neighbor callsign */
	if (setcall(&neighbor,argv[1]) == -1) {
		printf("bad neighbor callsign\n") ;
		return -1 ;
	}

	/* find interface */
	for (i = 0 ; i < nr_numiface ; i++)
		if (!strcmp(nrifaces[i].interface->name,argv[2]))
			break ;
	if (i == nr_numiface) {
		printf("Interface \"%s\" not found\n",argv[2]) ;
		return -1 ;
	}

	return nr_nfdrop(&neighbor,(unsigned)i) ;
}

/* nodefilter mode subcommand */
static
donfmode(argc,argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		printf("filter mode is ") ;
		switch (nr_nfmode) {
			case NRNF_NOFILTER:
				printf("none\n") ;
				break ;
			case NRNF_ACCEPT:
				printf("accept\n") ;
				break ;
			case NRNF_REJECT:
				printf("reject\n") ;
				break ;
			default:
				printf("some strange, unknown value\n") ;
		}
		return 0 ;
	}
	
	switch (argv[1][0]) {
		case 'n':
		case 'N':
			nr_nfmode = NRNF_NOFILTER ;
			break ;
		case 'a':
		case 'A':
			nr_nfmode = NRNF_ACCEPT ;
			break ;
		case 'r':
		case 'R':
			nr_nfmode = NRNF_REJECT ;
			break ;
		default:
			printf("modes are: none accept reject\n") ;
			return -1 ;
	}

	return 0 ;
}


/* netrom network packet time-to-live initializer */
static
donrttl(argc, argv)
int argc ;
char *argv[] ;
{
	int val ;

	if (argc < 2) {
		printf("%d\n", nr_ttl) ;
		return 0 ;
	}

	val = atoi(argv[1]) ;

	if (val < 0 || val > 255) {
		printf("ttl must be between 0 and 255\n") ;
		return 1 ;
	}

	nr_ttl = val ;

	return 0 ;
}

/* verbose route broadcast */
static
donrverbose(argc,argv)
int argc ;
char *argv[] ;
{
	if (argc < 2) {
		printf("verbose is %s\n", nr_verbose ? "yes" : "no" ) ;
		return 0 ;
	}
	
	switch (argv[1][0]) {
		case 'n':
		case 'N':
			nr_verbose = 0 ;
			break ;
		case 'y':
		case 'Y':
			nr_verbose = 1 ;
			break ;
		default:
			printf("use: netrom verbose [yes|no]\n") ;
			return -1 ;
	}

	return 0 ;
}

/* Initiate a NET/ROM transport connection */
/*ARGSUSED*/
static int
donrconnect(argc,argv)
int argc ;
char *argv[] ;
{
	struct ax25_addr node, *np ;
	struct session *s ;
	char alias[7] ;

	/* See if the requested destination could be an alias, and */
	/* find and use it if it is.  Otherwise assume it is an ax.25 */
	/* address. */
	
	if (putalias(alias,argv[1],0) != -1 &&
		(np = find_nralias(alias)) != NULLAXADDR)
		  node = *np ;
	else
		setcall(&node,argv[1]) ;	/* parse ax25 callsign */

	/* Get a session descriptor */

	if ((s = newsession()) == NULLSESSION) {
		printf("Too many sessions\n") ;
		return 1 ;
	}

	if((s->name = malloc((unsigned)strlen(argv[1])+1)) != NULLCHAR)
		strcpy(s->name,argv[1]);
	s->type = NRSESSION ;
	s->parse = (int (*)())nr4_parse ;
	current = s;

	s->cb.nr4_cb = open_nr4(&node,&mycall,nr4_rx,nr4_tx,nr4_state,(char *)s) ;
	go() ;
	return 0 ;
}

/* Display changes in NET/ROM state */
/*ARGSUSED*/
void
nr4_state(cb,old,new)
struct nr4cb *cb ;
int old,new;
{
	struct session *s;

	s = (struct session *)cb->puser;

	if(current != NULLSESSION && current->type == NRSESSION && current == s){
		printf("%s",Nr4states[new]);
		if(new == NR4STDISC) {
			printf(" (%s)\n", Nr4reasons[cb->dreason]) ;
			cmdmode();
		} else
			printf("\n") ;
		fflush(stdout);
	}
	if(new == NR4STDISC){
		cb->puser = NULLCHAR;
		freesession(s);
	}
}

/* Handle typed characters on a NET/ROM connection */
void
nr4_parse(buf,cnt)
char *buf;
int16 cnt;
{
	struct mbuf *bp;
	register char *cp;
	int16 size, i ;
	char c;

	if(current == NULLSESSION || current->type != NRSESSION)
		return;	/* "can't happen" */

	/* If recording is on, record outgoing stuff too */
	if(current->record != NULLFILE)
		fwrite(buf,1,(int)cnt,current->record);

	/* Parse it out, splitting at transport frame boundaries */
	
	while (cnt != 0) {
#ifdef NRDEBUG
		printf("Once around the parse loop - cnt = %d\n", cnt) ;
#endif
		size = min(cnt, NR4MAXINFO) ;
		if ((bp = alloc_mbuf(size)) == NULLBUF)
			break ;
		/* Copy keyboard buffer to output, stripping line feeds */
		cp = bp->data ;
		for (i = 0 ; i < size ; i++){
			c = *buf++;
			if(c != '\n'){
				*cp++ = c;
				bp->cnt++;
			}
		}
		cnt -= size ;
		send_nr4(current->cb.nr4_cb,bp);
	}
}

/* Handle new incoming terminal sessions
 * This is the default state change upcall function, used when
 * someone else connects to us
 */
/*ARGSUSED*/
void
nr4_incom(cb,oldstate,newstate)
struct nr4cb *cb ;
int oldstate ;
int newstate ;
{
	void nr4_session(), mbx_nr4incom() ;

	if (newstate != NR4STCON)		/* why are you bothering us? */
		return ;					/* (shouldn't happen) */
		
	if (ax25mbox)
		mbx_nr4incom(cb) ;
	else
		nr4_session(cb) ;
	return ;

}

/* This function sets up a NET/ROM chat session */
void
nr4_session(cb)
struct nr4cb *cb ;
{
	struct session *s;
	char remote[10];

	pax25(remote,&cb->user);
	if((s = newsession()) == NULLSESSION){
		/* Out of sessions */
		disc_nr4(cb);
		return;
	}
	s->type = NRSESSION ;
	s->name = malloc((int16)strlen(remote)+1);
	s->cb.nr4_cb = cb ;
	s->parse = (int (*)())nr4_parse;
	strcpy(s->name,remote);
	cb->r_upcall = nr4_rx;
	cb->s_upcall = nr4_state;
	cb->t_upcall = nr4_tx;
	cb->puser = (char *)s;
#if	(defined(MAC) || defined(AMIGA))
	printf("\007Incoming NET/ROM session %lu from %s\n",s - sessions,remote);
#else
	printf("\007Incoming NET/ROM session %u from %s\n",s - sessions,remote);
#endif
	fflush(stdout);
}

/* Handle incoming terminal traffic */
void
nr4_rx(cb,cnt)
struct nr4cb *cb ;
int16 cnt;
{
	register struct mbuf *bp;
	char c;

	/* Hold output if we're not the current session */
	if(mode != CONV_MODE || current == NULLSESSION
	 || current->type != NRSESSION || current->cb.nr4_cb != cb)
		return;

	if((bp = recv_nr4(cb,cnt)) == NULLBUF)
		return;

	/* Display received characters, translating CR's to CR/LF */
	while(bp != NULLBUF){
		while(bp->cnt-- != 0){
			c = *bp->data++;
			if(c == '\r')
				c = '\n';
			putchar(c);
			if(current->record){
#ifndef UNIX
				if(c == '\n')
					fputc('\r',current->record);
#endif
				fputc(c,current->record);
			}
		}
		bp = free_mbuf(bp);
	}
	if(current->record)
		fflush(current->record);
	fflush(stdout);
}

/* Handle transmit upcalls. Used only for file uploading */
void
nr4_tx(cb,cnt)
struct nr4cb *cb ;
int16 cnt;
{
	register char *cp;
	struct session *s;
	register struct mbuf *bp;
	int16 size;
	int c;

	if((s = (struct session *)cb->puser) == NULLSESSION
	 || s->upload == NULLFILE)
		return;
	while(cnt != 0){
		size = min(cnt,NR4MAXINFO);
		if((bp = alloc_mbuf(size)) == NULLBUF)
			break;
		cp = bp->data;

		/* Now send data characters, translating between local
		 * keyboard end-of-line sequences and the (unwritten)
		 * AX.25 convention, which is carriage-return only
		 */
		 
		while(bp->cnt < size){
			if((c = getc(s->upload)) == EOF)
				break;
#ifdef	MSDOS
			/* MS-DOS gives cr-lf */
			if(c == '\n')
				continue;
#endif
#if	(defined(UNIX) || defined(MAC) || defined(AMIGA))
			/* These give lf only */
			if(c == '\n')
				c = '\r';
#endif
			*cp++ = c;
			bp->cnt++;
		}	
		cnt -= bp->cnt;
		
		if (bp->cnt != 0)	/* might happen with a newline at EOF */
			send_nr4(cb,bp);
		else
			free_p(bp) ;
			
		if (c == EOF)
			break ;
	}
	if(cnt != 0){
		/* Error or end-of-file */
		fclose(s->upload);
		s->upload = NULLFILE;
		free(s->ufile);
		s->ufile = NULLCHAR;
	}
}

/* Reset a net/rom connection abruptly */

/*ARGSUSED*/
static int
donrreset(argc,argv)
int argc;
char *argv[];
{
	struct nr4cb *cb ;
	extern char notval[];

	cb = (struct nr4cb *)htol(argv[1]);
	if(!nr4valcb(cb)){
		printf(notval);
		return 1;
	}
	reset_nr4(cb);
	return 0;
}

/* Force retransmission on a net/rom connection */

/*ARGSUSED*/
static int
donrkick(argc,argv)
int argc;
char *argv[];
{
	struct nr4cb *cb ;
	extern char notval[];

	cb = (struct nr4cb *)htol(argv[1]);

	if (kick_nr4(cb) == -1) {
		printf(notval);
		return 1;
	} else
		return 0;
}

/* netrom transport ACK delay timer */

static
donracktime(argc, argv)
int argc ;
char *argv[] ;
{
	long val ;

	if (argc < 2) {
		printf("%lu\n", Nr4acktime) ;
		return 0 ;
	}

	val = atol(argv[1]) ;

	Nr4acktime = val ;

	return 0 ;
}

/* netrom transport choke timeout */

static
donrchoketime(argc, argv)
int argc ;
char *argv[] ;
{
	long val ;

	if (argc < 2) {
		printf("%lu\n", Nr4choketime) ;
		return 0 ;
	}

	val = atol(argv[1]) ;

	Nr4choketime = val ;

	return 0 ;
}

/* netrom transport initial round trip time */

static
donrirtt(argc, argv)
int argc ;
char *argv[] ;
{
	long val ;

	if (argc < 2) {
		printf("%lu\n", Nr4irtt) ;
		return 0 ;
	}

	val = atol(argv[1]) ;

	Nr4irtt = val ;

	return 0 ;
}

/* netrom transport receive queue length limit.  This is the */
/* threshhold at which we will CHOKE the sender. */

static
donrqlimit(argc, argv)
int argc ;
char *argv[] ;
{
	unsigned val ;

	if (argc < 2) {
		printf("%u\n", Nr4qlimit) ;
		return 0 ;
	}

	val = atoi(argv[1]) ;

	if (val == 0) {
		printf("You cannot set the queue limit to 0\n") ;
		return 1 ;
	}
	
	Nr4qlimit = val ;

	return 0 ;
}

/* netrom transport maximum window.  This is the largest send and */
/* receive window we may negotiate */

static
donrwindow(argc, argv)
int argc ;
char *argv[] ;
{
	unsigned val ;

	if (argc < 2) {
		printf("%u\n", Nr4window) ;
		return 0 ;
	}

	val = atoi(argv[1]) ;

	if (val == 0 || val > NR4MAXWIN) {
		printf("Illegal NET/ROM window size.  Range is [1,%d]\n",
			   NR4MAXWIN) ;
		return 1 ;
	}
	
	Nr4window = val ;

	return 0 ;
}

/* netrom transport maximum retries.  This is used in connect and */
/* disconnect attempts; I haven't decided what to do about actual */
/* data retries yet. */

static
donrretries(argc, argv)
int argc ;
char *argv[] ;
{
	unsigned val ;

	if (argc < 2) {
		printf("%u\n", Nr4retries) ;
		return 0 ;
	}

	val = atoi(argv[1]) ;

	if (val == 0) {
		printf("Impatient, aren't we?  Zero retries not possible\n") ;
		return 1 ;
	}
	
	Nr4retries = val ;

	return 0 ;
}

/* Display the status of NET/ROM connections */

static
donrstatus(argc, argv)
int argc ;
char *argv[] ;
{
	int i ;
	struct nr4cb *cb ;
	char luser[10], ruser[10], node[10] ;
	extern char notval[] ;
	void donrdump() ;
	
	if (argc < 2) {
		printf("     &CB Snd-W Snd-Q Rcv-Q     LUser      RUser @Node     State\n");
		for (i = 0 ; i < NR4MAXCIRC ; i++) {
			if ((cb = Nr4circuits[i].ccb) == NULLNR4CB)
				continue ;
			pax25(luser,&cb->luser) ;
			pax25(ruser,&cb->user) ;
			pax25(node,&cb->node) ;
			printf("%8lx   %3d %5d %5d %9s  %9s %-9s %s\n",
				   (long)cb, cb->nbuffered, len_q(cb->txq),
				   len_mbuf(cb->rxq), luser, ruser, node,
				   Nr4states[cb->state]) ;
		}
		return 0 ;
	}

	cb = (struct nr4cb *)htol(argv[1]) ;
	if (!nr4valcb(cb)) {
		printf(notval) ;
		return 1 ;
	}

	donrdump(cb) ;
	return 0 ;
}

/* Dump one control block */

static void
donrdump(cb)
struct nr4cb *cb ;
{
	char luser[10], ruser[10], node[10] ;
	unsigned seq ;
	struct nr4txbuf *b ;
	struct timer *t ;

	pax25(luser,&cb->luser) ;
	pax25(ruser,&cb->user) ;
	pax25(node, &cb->node) ;

	printf("Local: %s %d/%d Remote: %s @ %s %d/%d State: %s\n",
		   luser, cb->mynum, cb->myid, ruser, node,
		   cb->yournum, cb->yourid, Nr4states[cb->state]) ;

	printf("Window: %-5u Rxpect: %-5u RxNext: %-5u RxQ: %-5d %s\n",
		   cb->window, uchar(cb->rxpected), uchar(cb->rxpastwin),
		   len_mbuf(cb->rxq), cb->qfull ? "RxCHOKED" : "") ;

	printf(" Unack: %-5u Txpect: %-5u TxNext: %-5u TxQ: %-5d %s\n",
		   cb->nbuffered, uchar(cb->ackxpected), uchar(cb->nextosend),
		   len_q(cb->txq), cb->choked ? "TxCHOKED" : "") ;

	printf("TACK: ") ;
	if (run_timer(&cb->tack))
		printf("%lu", (cb->tack.start - cb->tack.count) * MSPTICK) ;
	else
		printf("stop") ;
	printf("/%lu ms; ", cb->tack.start * MSPTICK) ;

	printf("TChoke: ") ;
	if (run_timer(&cb->tchoke))
		printf("%lu", (cb->tchoke.start - cb->tchoke.count) * MSPTICK) ;
	else
		printf("stop") ;
	printf("/%lu ms; ", cb->tchoke.start * MSPTICK) ;

	printf("TCD: ") ;
	if (run_timer(&cb->tcd))
		printf("%lu", (cb->tcd.start - cb->tcd.count) * MSPTICK) ;
	else
		printf("stop") ;
	printf("/%lu ms", cb->tcd.start * MSPTICK) ;

	if (run_timer(&cb->tcd))
		printf("; Tries: %u\n", cb->cdtries) ;
	else
		printf("\n") ;

	printf("Backoff Level %u SRTT %ld ms Mean dev %ld ms\n",
		   cb->blevel, cb->srtt, cb->mdev) ;

	/* If we are connected and the send window is open, display */
	/* the status of all the buffers and their timers */
	
	if (cb->state == NR4STCON && cb->nextosend != cb->ackxpected) {

		printf("TxBuffers:  Seq  Size  Tries  Timer\n") ;

		for (seq = cb->ackxpected ;
			 nr4between(cb->ackxpected, seq, cb->nextosend) ;
			 seq = (seq + 1) & NR4SEQMASK) {

			b = &cb->txbufs[seq % cb->window] ;
			t = &b->tretry ;

			printf("            %3u   %3d  %5d  %lu/%lu\n",
				   seq, len_mbuf(b->data), b->retries + 1,
				   (t->start - t->count) * MSPTICK, t->start * MSPTICK) ;
		}

	}

}
