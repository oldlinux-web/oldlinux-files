#include <stdio.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "ax25.h"
#include "ax_mbx.h"
#include "timer.h"
#include "netuser.h"
#include "ftp.h"
#include "tcp.h"
#include "telnet.h"
#include "iface.h"
#include "lapb.h"
#include "finger.h"
#include "cmdparse.h"
#include "session.h"
#include "nr4.h"
#ifdef	UNIX
#include <string.h>
#include <sys/types.h>
time_t time();
#endif
/* heard stuff */
#include "heard.h"
/* heard stuff */

char *ax25states[] = {
	"Disconnected",
	"Conn pending",
	"Disc pending",
	"Connected",
	"Recovery",
	"Frame Reject",
};

int domycall(),dodigipeat(),doaxstat(),dot1(),dot2(),dot3(),domaxframe(),
	doaxwindow(),dopaclen(),don2(),doaxreset(),dopthresh(),doheard();
#ifdef SID2
int dobbscall();
extern struct ax25_addr bbscall;
#endif

static struct cmds axcmds[] = {
	"digipeat",	dodigipeat,	0, NULLCHAR,	NULLCHAR,
	"maxframe",	domaxframe,	0, NULLCHAR,	NULLCHAR,
	"mycall",	domycall,	0, NULLCHAR,	NULLCHAR,
#ifdef SID2
	"bbscall",	dobbscall,	0, NULLCHAR,	NULLCHAR,
#endif
	"paclen",	dopaclen,	0, NULLCHAR,	NULLCHAR,
	"pthresh",	dopthresh,	0, NULLCHAR,	NULLCHAR,
	"reset",	doaxreset,	2, "ax25 reset <axcb>", NULLCHAR,
	"retry",	don2,		0, NULLCHAR,	NULLCHAR,
	"status",	doaxstat,	0, NULLCHAR,	NULLCHAR,
	"t1",		dot1,		0, NULLCHAR,	NULLCHAR,
	"t2",		dot2,		0, NULLCHAR,	NULLCHAR,
	"t3",		dot3,		0, NULLCHAR,	NULLCHAR,
	"window",	doaxwindow,	0, NULLCHAR,	NULLCHAR,
/* heard stuff */
	"heard",	doheard,	0, NULLCHAR,	NULLCHAR,
/* heard stuff */
#ifdef SID2
	NULLCHAR,	NULLFP,		0, "ax25 subcommands: bbscall digipeat heard maxframe mycall paclen pthresh reset\n\t\tretry status t1 t2 t3 window",	NULLCHAR,
#else
	NULLCHAR,	NULLFP,		0, "ax25 subcommands: digipeat heard maxframe mycall paclen pthresh reset retry\n\t\tstatus t1 t2 t3 window",	NULLCHAR,
#endif
};

/* Multiplexer for top-level ax25 command */
doax25(argc,argv)
int argc;
char *argv[];
{
	return subcmd(axcmds,argc,argv);
}

/*ARGSUSED*/
static
doaxreset(argc,argv)
int argc;
char *argv[];
{
	struct ax25_cb *axp;
	extern char notval[];

	axp = (struct ax25_cb *)htol(argv[1]);
	if(!ax25val(axp)){
		printf(notval);
		return 1;
	}
	reset_ax25(axp);
	return 0;
}

/* Display AX.25 link level control blocks */
static
doaxstat(argc,argv)
int argc;
char *argv[];
{
	register int i;
	register struct ax25_cb *axp;
	char tmp[10];
	extern char notval[];

	if(argc < 2){
		printf("    &AXB IF   Snd-Q   Rcv-Q   Remote    State\n");
		for(i=0;i<NHASH;i++){
			for(axp = ax25_cb[i];axp != NULLAX25; axp = axp->next){
				pax25(tmp,&axp->addr.dest);
				printf("%8lx %-5s%-8d%-8d%-10s%s\n",
					(long)axp,axp->interface->name,
					len_q(axp->txq),len_mbuf(axp->rxq),
					tmp,ax25states[axp->state]);
			}
		}
		return 0;
	}
	axp = (struct ax25_cb *)htol(argv[1]);
	if(!ax25val(axp)){
		printf(notval);
		return 1;
	}
	dumpstat(axp);
	return 0;
}
/* Dump one control block */
static
dumpstat(axp)
register struct ax25_cb *axp;
{
	char tmp[10];
	int i;

	if(axp == NULLAX25 || axp->interface == NULLIF)
		return;
	printf("&AXB IF   Remote   RB V(S) V(R) Unack P Retry State\n");
	pax25(tmp,&axp->addr.dest);
	printf("%4x %-5s%-9s",(int)axp,axp->interface->name,tmp);
	putchar(axp->rejsent ? 'R' : ' ');
	putchar(axp->remotebusy ? 'B' : ' ');
	printf(" %4d %4d",axp->vs,axp->vr);
	printf(" %02u/%02u %u",axp->unack,axp->maxframe,axp->proto);
	printf(" %02u/%02u",axp->retries,axp->n2);
	printf(" %s\n",ax25states[axp->state]);

	printf("T1: ");
	if(run_timer(&axp->t1))
		printf("%lu",(axp->t1.start - axp->t1.count) * MSPTICK);
	else
		printf("stop");
	printf("/%lu ms; ",axp->t1.start * MSPTICK);

	printf("T2: ");
	if(run_timer(&axp->t2))
		printf("%lu",(axp->t2.start - axp->t2.count) * MSPTICK);
	else
		printf("stop");
	printf("/%lu ms; ",axp->t2.start * MSPTICK);

	printf("T3: ");
	if(run_timer(&axp->t3))
		printf("%lu",(axp->t3.start - axp->t3.count) * MSPTICK);
	else
		printf("stop");
	printf("/%lu ms\n",axp->t3.start * MSPTICK);

	if(axp->addr.ndigis == 0)
		return;
	printf("Digipeaters:");
	for(i=0;i<axp->addr.ndigis;i++){
		pax25(tmp,&axp->addr.digis[i]);
		printf(" %s",tmp);
	}
	printf("\n");
}

/* Display or change our AX.25 address */
static
domycall(argc,argv)
int argc;
char *argv[];
{
	char buf[15];

	if(argc < 2){
		pax25(buf,&mycall);
		printf("%s\n",buf);
		return 0;
	}
	if(setcall(&mycall,argv[1]) == -1)
		return -1;
	mycall.ssid |= E;
	return 0;
}

#ifdef SID2
/* Display or change our AX.25 BBS address */
static
dobbscall(argc,argv)
int argc;
char *argv[];
{
	char buf[15];

	if(argc < 2){
		pax25(buf,&bbscall);
		printf("%s\n",buf);
		return 0;
	}
	if(setcall(&bbscall,argv[1]) == -1)
		return -1;
	bbscall.ssid |= E;
	return 0;
}
#endif

/* Control AX.25 digipeating */
static
dodigipeat(argc,argv)
int argc;
char *argv[];
{
	extern int digipeat;

	if(argc == 1) {
		printf("digipeat %s\n",digipeat ? "on" : "off");
	} else {
		if(strcmp(argv[1],"on") == 0)
			digipeat = 1;
		else
			digipeat = 0;
	}
}

/* Set retransmission timer */
static
dot1(argc,argv)
int argc;
char *argv[];
{
	extern int16 t1init;
	long atol();

	if(argc == 1) {
		printf("T1 %lu ms\n",(long)t1init * MSPTICK);
	} else {
		t1init = (int16)atol(argv[1]) / MSPTICK;
	}
}

/* Set acknowledgement delay timer */
static
dot2(argc,argv)
int argc;
char *argv[];
{
	extern int16 t2init;
	long atol();

	if(argc == 1) {
		printf("T2 %lu ms\n",(long)t2init * MSPTICK);
	} else {
		t2init = (int16)atol(argv[1]) / MSPTICK;
	}
}

/* Set idle timer */
static
dot3(argc,argv)
int argc;
char *argv[];
{
	extern int16 t3init;
	long atol();

	if(argc == 1) {
		printf("T3 %lu ms\n",(long)t3init * MSPTICK);
	} else {
		t3init = (int16)atol(argv[1]) / MSPTICK;
	}
}

/* Set retry limit count */
static
don2(argc,argv)
int argc;
char *argv[];
{
	extern int16 n2;

	if(argc == 1) {
		printf("Retry %u\n",n2);
	} else {
		n2 = atoi(argv[1]);
	}
}

/* Set maximum number of frames that will be allowed in flight */
static
domaxframe(argc,argv)
int argc;
char *argv[];
{
	extern int16 maxframe;

	if(argc == 1) {
		printf("Maxframe %u\n",maxframe);
	} else {
		maxframe = atoi(argv[1]);
	}
}

/* Set maximum length of I-frame data field */
static
dopaclen(argc,argv)
int argc;
char *argv[];
{
	extern int16 paclen;

	if(argc == 1) {
		printf("Paclen %u\n",paclen);
	} else {
		paclen = atoi(argv[1]);
	}
}
/* Set size of I-frame above which polls will be sent after a timeout */
static
dopthresh(argc,argv)
int argc;
char *argv[];
{
	extern int16 pthresh;

	if(argc == 1) {
		printf("Pthresh %u\n",pthresh);
	} else {
		pthresh = atoi(argv[1]);
	}
}

/* Set high water mark on receive queue that triggers RNR */
static
doaxwindow(argc,argv)
int argc;
char *argv[];
{
	extern int16 axwindow;

	if(argc == 1) {
		printf("Axwindow %u\n",axwindow);
	} else {
		axwindow = atoi(argv[1]);
	}
}
/* End of ax25 subcommands */

/* Initiate interactive AX.25 connect to remote station */
doconnect(argc,argv)
int argc;
char *argv[];
 {
	void ax_rx(),ax_tx(),ax_state();
	int ax_parse();
	struct ax25_addr dest;
	struct ax25 addr;
	struct ax25_cb *open_ax25();
	struct interface *ifp;
	struct session *s;
	extern int16 axwindow;
	int i;

	if (strcmp(argv[1],"netrom") == 0) {
		printf("Connect on netrom interface not supported\n") ;
		return 1 ;
	}
	
	for(ifp = ifaces; ifp != NULLIF; ifp = ifp->next)
		if(strcmp(argv[1],ifp->name) == 0)
			break;

	if(ifp == NULLIF){
		printf("Interface %s unknown\n",argv[1]);
		return 1;
	}
	setcall(&dest,argv[2]);
	/* See if a session already exists */
	for(s = sessions; s < &sessions[nsessions]; s++){
		if(s->type == AX25TNC
		 && addreq(&s->cb.ax25_cb->addr.dest,&dest)){
#if	(defined(MAC) || defined(AMIGA))
			printf("Session %lu to %s already exists\n",
#else
			printf("Session %u to %s already exists\n",
#endif
				s - sessions,argv[2]);
			return 1;
		}
	}
	/* Allocate a session descriptor */
	if((s = newsession()) == NULLSESSION){
		printf("Too many sessions\n");
		return 1;
	}
	if((s->name = malloc((unsigned)strlen(argv[2])+1)) != NULLCHAR)
		strcpy(s->name,argv[2]);
	s->type = AX25TNC;
	s->parse = ax_parse;
	current = s;
	ASSIGN(addr.source,mycall);
	setcall(&addr.dest,argv[2]);
	for(i=3; i < argc; i++)
		setcall(&addr.digis[i-3],argv[i]);

	addr.ndigis = i - 3;
	s->cb.ax25_cb = open_ax25(&addr,axwindow,ax_rx,ax_tx,ax_state,ifp,(char *)s);
	go();
	return 0;
}


/* Display changes in AX.25 state */
void
ax_state(axp,old,new)
struct ax25_cb *axp;
int old,new;
{
	struct session *s;

	s = (struct session *)axp->user;

	if(current != NULLSESSION && current->type == AX25TNC && current == s){
		/* Don't print transitions between CONNECTED and RECOVERY */
		if(new != RECOVERY && !(old == RECOVERY && new == CONNECTED))
			printf("%s\n",ax25states[new]);
		if(new == DISCONNECTED)
			cmdmode();
		fflush(stdout);
	}
	if(new == DISCONNECTED){
		axp->user = NULLCHAR;
		freesession(s);
	}
}
/* Handle typed characters on AX.25 connection */
int
ax_parse(buf,cnt)
char *buf;
int16 cnt;
{
	struct mbuf *bp;
	register char *cp;
	char c;

	if(current == NULLSESSION || current->type != AX25TNC)
		return;	/* "can't happen" */

	/* If recording is on, record outgoing stuff too */
	if(current->record != NULLFILE)
		fwrite(buf,1,(int)cnt,current->record);

	/* Allocate buffer and start it with the PID */
	bp = alloc_mbuf(cnt+1);
	*bp->data = PID_FIRST | PID_LAST | PID_NO_L3;
	bp->cnt++;

	/* Copy keyboard buffer to output, stripping line feeds */
	cp = bp->data + 1;
	while(cnt-- != 0){
		c = *buf++;
		if(c != '\n'){
			*cp++ = c;
			bp->cnt++;
		}
	}
	send_ax25(current->cb.ax25_cb,bp);
}
/* Handle new incoming terminal sessions
 * This is the default receive upcall function, used when
 * someone else connects to us
 */
void
ax_incom(axp,cnt)
register struct ax25_cb *axp;
int16 cnt;
{
	void ax_session(), mbx_incom() ;
	
#ifdef	SID2
	ax_session(axp, cnt);
#else
	if (ax25mbox)
		mbx_incom(axp,cnt) ;
	else
		ax_session(axp,cnt) ;
#endif
	return ;

}

/* This function sets up an ax25 chat session */
/*ARGSUSED*/
void
ax_session(axp,cnt)
register struct ax25_cb *axp ;
int16 cnt ;
{
	struct session *s;
	char remote[10];
	void ax_rx(),ax_state(),ax_tx();

	pax25(remote,&axp->addr.dest);
	if((s = newsession()) == NULLSESSION){
		/* Out of sessions */
		disc_ax25(axp);
		return;
	}
	s->type = AX25TNC;
	s->name = malloc((int16)strlen(remote)+1);
	s->cb.ax25_cb = axp;
	s->parse = ax_parse;
	strcpy(s->name,remote);
	axp->r_upcall = ax_rx;
	axp->s_upcall = ax_state;
	axp->t_upcall = ax_tx;
	axp->user = (char *)s;
#if	(defined(MAC) || defined(AMIGA))
	printf("\007Incoming AX25 session %lu from %s\n",s - sessions,remote);
#else
	printf("\007Incoming AX25 session %u from %s\n",s - sessions,remote);
#endif
	fflush(stdout);
}

/* Handle incoming terminal traffic */
void
ax_rx(axp,cnt)
struct ax25_cb *axp;
int16 cnt;
{
	register struct mbuf *bp;
	struct mbuf *recv_ax25();
	char c;

	/* Hold output if we're not the current session */
	if(mode != CONV_MODE || current == NULLSESSION
	 || current->type != AX25TNC || current->cb.ax25_cb != axp)
		return;

	if((bp = recv_ax25(axp,cnt)) == NULLBUF)
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
ax_tx(axp,cnt)
struct ax25_cb *axp;
int16 cnt;
{
	register char *cp;
	struct session *s;
	register struct mbuf *bp;
	int16 size;
	int c;

	if((s = (struct session *)axp->user) == NULLSESSION
	 || s->upload == NULLFILE)
		return;
	while(cnt != 0){
		size = min(cnt,axp->paclen+1);
		if((bp = alloc_mbuf(size)) == NULLBUF)
			break;
		cp = bp->data;
		/* Start with the PID */
		*cp++ = PID_FIRST | PID_LAST | PID_NO_L3;
		bp->cnt++;

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
		if(bp->cnt > 1) {
			send_ax25(axp,bp);
		} else {
			/* Empty frame, don't bother sending */
			free_p(bp);
			break;
		}
		cnt -= bp->cnt;
	}
	if(cnt != 0){
		/* Error or end-of-file */
		fclose(s->upload);
		s->upload = NULLFILE;
		free(s->ufile);
		s->ufile = NULLCHAR;
	}
}

/* heard stuff */
struct ax25_heard heard = {0,0};
/* Display AX.25 heard list */
static
doheard(argc,argv)
int argc;
char *argv[];
{
	struct heard_stuff *hp;
	struct ax25_addr *dp;
	extern struct ax25_heard heard;
	char tmp[16];
	int curr;
	long t;

	if(argc < 2) {
		if (!heard.enabled) {
			printf("not enabled\n");
			return(0);
		}
		time(&t);
		printf("\nHeard list: %s\n",ctime(&t));
		curr = heard.first;
		while (curr != -1) {
			hp = &heard.list[curr];
			pax25(tmp,&hp->info.source);
			printf("%-10s",tmp);
			if(hp->info.ndigis > 0){
				printf(" (via");
				/* Print digi string until packet got to us */
				for(dp = &hp->info.digis[0]; dp < &hp->info.digis[hp->info.ndigis]; dp++){
					if (dp->ssid & REPEATED) {
						pax25(tmp,dp);
						printf(" %s",tmp);
					} else {
						break;
					}
				}
				printf(")");
			}
			if (hp->flags) {
				printf("%-5s%-8s%-4s",
								(hp->flags & HEARD_ARP) ? " ARP" : "",
								(hp->flags & HEARD_NETROM) ? " NETROM" : "",
								(hp->flags & HEARD_IP) ? " IP" : "");
			}
			printf("  %s", ctime((long *)&hp->htime));
			curr = hp->next;
		}
		return 0;
	}
	if (strcmp(argv[1],"clear") == 0) {
		heard.cnt = 0;
		heard.first = -1;
		return 0;
	} else if (strcmp(argv[1],"on") == 0) {
		if (!heard.enabled) {
			heard.enabled = 1;
			heard.cnt = 0;
			heard.first = -1;
		}
		return 0;
	} else if (strcmp(argv[1],"off") == 0) {
		heard.enabled = 0;
		return 0;
	} else {
		printf("Usage: ax25 heard [ on | off | clear ]\n");
	}
	return 0;
}
/* heard stuff */
