
/*
 *	Client routines for Simple Mail Transfer Protocol ala RFC821
 *	A.D. Barksdale Garbee II, aka Bdale, N3EUA
 *	Copyright 1986 Bdale Garbee, All Rights Reserved.
 *	Permission granted for non-commercial copying and use, provided
 *	this notice is retained.
 * 	Modified 14 June 1987 by P. Karn for symbolic target addresses,
 *	also rebuilt locking mechanism
 *	Limit on max simultaneous sessions, reuse of connections - 12/87 NN2Z
 *	Added return of mail to sender as well as batching of commands 1/88 nn2z
 */
#include <stdio.h>
#if	(!ATARI_ST || LATTICE)
#include <fcntl.h>
#endif
#include <time.h>
#ifdef UNIX
#include <sys/types.h>
#include <memory.h>
#include <string.h>
#endif
#include "global.h"
#include "netuser.h"
#include "mbuf.h"
#include "timer.h"
#include "tcp.h"
#include "smtp.h"
#include "trace.h"
#include "cmdparse.h"

#ifdef	BSD
char *sprintf();
#endif

extern int16 lport;			/* local port placeholder */
extern int32 resolve();
static struct timer smtpcli_t;
int32 gateway;

#ifdef SMTPTRACE
int16	smtptrace = 0;			/* used for trace level */
int dosmtptrace();
#endif

int16	smtpmaxcli  = MAXSESSIONS;	/* the max client connections allowed */
int16	smtpsessions = 0;		/* number of client connections
					* currently open */
int16	smtpmode = 0;

static struct smtp_cb *cli_session[MAXSESSIONS]; /* queue of client sessions  */

static char quitcmd[] = "QUIT\r\n";
static char eom[] = "\r\n.\r\n";

int smtptick(),dogateway(),dosmtpmaxcli(),mlock(),dotimer(),nextjob();
int setsmtpmode(),sendwindow();
void quit(),abort_trans(),retmail(),sendit(),del_session(),del_job();
void execjobs(),smtp_transaction(),logerr();
struct smtp_cb *newcb(),*lookup();
struct smtp_job *setupjob();

struct cmds smtpcmds[] = {
	"gateway",	dogateway,	0,	NULLCHAR,	NULLCHAR,
	"mode",		setsmtpmode,	0,	NULLCHAR,	NULLCHAR,
	"kick",		smtptick,	0,	NULLCHAR,	NULLCHAR,
	"maxclients",	dosmtpmaxcli,	0,	NULLCHAR,	NULLCHAR,
	"timer",	dotimer,	0,	NULLCHAR,	NULLCHAR,
#ifdef SMTPTRACE
	"trace",	dosmtptrace,	0,	NULLCHAR,	NULLCHAR,
#endif
	NULLCHAR,	NULLFP,		0,	
	"subcommands: gateway kick maxclients timer trace",
		NULLCHAR,
};

dosmtp(argc,argv)
int argc;
char *argv[];
{
	return subcmd(smtpcmds,argc,argv);
}

static int
dosmtpmaxcli(argc,argv)
int argc;
char *argv[];
{
	int x;
	if (argc < 2)
		printf("%d\n",smtpmaxcli);
	else {
		x = atoi(argv[1]);
		if (x > MAXSESSIONS)
			printf("max clients must be <= %d\n",MAXSESSIONS);
		else
			smtpmaxcli = x;
	}
	return 0;
}

static int
setsmtpmode(argc,argv)
int argc;
char *argv[];
{
	if (argc < 2) {
		printf("smtp mode: %s\n",
			(smtpmode & QUEUE) ? "queue" : "route");
	} else {
		switch(*argv[1]) {
		case 'q':
			smtpmode |= QUEUE;
			break;
		case 'r':
			smtpmode &= ~QUEUE;
			break;
		default:
			printf("Usage: smtp mode [queue | route]\n");
			break;
		}
	}
	return 0;
}
static int
dogateway(argc,argv)
int argc;
char *argv[];
{
	char *inet_ntoa();
	int32 n;
	extern char badhost[];

	if(argc < 2){
		printf("%s\n",inet_ntoa(gateway));
	} else if((n = resolve(argv[1])) == 0){
		printf(badhost,argv[1]);
		return 1;
	} else
		gateway = n;
	return 0;
}

#ifdef SMTPTRACE
static int
dosmtptrace(argc,argv)
int argc;
char *argv[];
{
	if (argc < 2)
		printf("%d\n",smtptrace);
	else 
		smtptrace = atoi(argv[1]);
	return 0;
}
#endif

/* Set outbound spool poll interval */
static int
dotimer(argc,argv)
int argc;
char *argv[];
{
	int smtptick();

	if(argc < 2){
		printf("%lu/%lu\n",
		(smtpcli_t.start - smtpcli_t.count)/(1000/MSPTICK),
		smtpcli_t.start/(1000/MSPTICK));
		return 0;
	}
	smtpcli_t.func = (void (*)())smtptick;/* what to call on timeout */
	smtpcli_t.arg = NULLCHAR;		/* dummy value */
	smtpcli_t.start = atoi(argv[1])*(1000/MSPTICK);	/* set timer duration */
	start_timer(&smtpcli_t);		/* and fire it up */
	return 0;
}

/* this is the routine that gets called every so often to do outgoing mail
   processing */
int
smtptick()
{
	register struct smtp_cb *cb;
	struct smtp_job *jp;
	struct list *ap;
	char	tmpstring[LINELEN], wfilename[13], prefix[9];
	char	from[LINELEN], to[LINELEN];
	char *cp, *cp1;
	int32 destaddr;
	FILE *wfile;

#ifdef SMTPTRACE
	if (smtptrace > 5) {
		printf("smtp daemon entered\n");
		fflush(stdout);
	}
#endif
	for(filedir(mailqueue,0,wfilename);wfilename[0] != '\0';
		filedir(mailqueue,1,wfilename)){

		/* save the prefix of the file name which it job id */
		cp = wfilename;
		cp1 = prefix;
		while (*cp && *cp != '.')
			*cp1++ = *cp++;
		*cp1 = '\0';

		/* lock this file from the smtp daemon */
		if (mlock(mailqdir,prefix))
			continue;

		sprintf(tmpstring,"%s/%s",mailqdir,wfilename);
		if ((wfile = fopen(tmpstring,"r")) == NULLFILE) {
			/* probably too many open files */
			(void) rmlock(mailqdir,prefix);
			/* continue to next message. The failure
			* may be temporary */
			continue;
		}

		(void) fgets(tmpstring,LINELEN,wfile);	/* read target host */
		rip(tmpstring);

		if ((destaddr = mailroute(tmpstring)) == 0) {
			fclose(wfile);
			printf("** smtp: Unknown address %s\n",tmpstring);
			fflush(stdout);
			(void) rmlock(mailqdir,prefix);
			continue;
		}

		if ((cb = lookup(destaddr)) == NULLCB) {
			/* there are enough processes running already */
			if (smtpsessions >= smtpmaxcli) {
#ifdef SMTPTRACE
				if (smtptrace) {
					printf("smtp daemon: too many processes\n");
					fflush(stdout);
				}
#endif
				fclose(wfile);
				(void) rmlock(mailqdir,prefix);
				break;
			}
			if ((cb = newcb()) == NULLCB) {
				fclose(wfile);
				(void) rmlock(mailqdir,prefix);
				break;
			} 
			cb->ipdest = destaddr;
		} else {
			/* This system is already is sending mail lets not
			* interfere with its send queue.
			*/
			if (cb->state != CLI_INIT_STATE) {
				fclose(wfile);
				(void) rmlock(mailqdir,prefix);
				continue;
			}
		}

		(void) fgets(from,LINELEN,wfile);	/* read from */
		rip(from);
		if ((jp = setupjob(cb,prefix,from)) == NULLJOB) {
			fclose(wfile);
			(void) rmlock(mailqdir,prefix);
			del_session(cb);
			break;
		}
		while (fgets(to,LINELEN,wfile) != NULLCHAR) {
			rip(to);
			if (addlist(&jp->to,to,DOMAIN) == NULLLIST) {
				fclose(wfile);
				del_session(cb);
			}
		}
		fclose(wfile);
#ifdef SMTPTRACE
		if (smtptrace > 1) {
			printf("queue job %s From: %s To:",prefix,from);
			for (ap = jp->to; ap != NULLLIST; ap = ap->next)
				printf(" %s",ap->val);
			printf("\n");
			fflush(stdout);
		}
#endif
	}

	/* start sending that mail */
	execjobs();

	/* Restart timer */
	start_timer(&smtpcli_t);
}

/* this is the master state machine that handles a single SMTP transaction */
void
smtp_transaction(cb)
register struct smtp_cb *cb;
{
	void smtp_cts();
	register char reply;
	register struct list *tp;
	int16 cnt;
	struct mbuf *bp,*bpl;
	char tbuf[LINELEN];
	int rcode;

#ifdef SMTPTRACE
	if (smtptrace > 5) 
		printf("smtp_transaction() enter state=%u\n",cb->state);
	if (smtptrace) {
		printf("%s\n",cb->buf);
		fflush(stdout);
	}
#endif
	/* Another line follows; ignore this one */
	if(cb->buf[0] == '0' || cb->buf[3] == '-')
		return;

	reply = cb->buf[0];
	rcode = atoi(cb->buf);

	/* if service shuting down */
	if (rcode == 421) {
		quit(cb);
		return;
	}

	switch(cb->state) {
	case CLI_OPEN_STATE:
		if (reply != '2')
			quit(cb);
		else {
			cb->state = CLI_HELO_STATE;
			sendit(cb,"HELO %s\r\nMAIL FROM:<%s>\r\n",
			hostname,cb->jobq->from);
		}
		break;
	case CLI_HELO_STATE:
		if (reply != '2')
			quit(cb);
		else 
			cb->state = CLI_MAIL_STATE;
		break;			
	case CLI_MAIL_STATE:
		if (reply != '2')
			quit(cb);
		else {
			cb->state = CLI_RCPT_STATE;
			cb->rcpts = 0;
			bpl = NULLBUF;
			for (tp = cb->jobq->to; tp != NULLLIST; tp = tp->next){
				sprintf(tbuf,"RCPT TO:<%s>\r\n",tp->val);
				bp = qdata(tbuf,(int16)strlen(tbuf));
				if (bp == NULLBUF) {
					free_p(bpl);
					quit(cb);
					return;
				}
				append(&bpl,bp);
				cb->rcpts++;
#ifdef SMTPTRACE
				if (smtptrace) {
					printf(">>> %s",tbuf);
					fflush(stdout);
				}
#endif
			}
			send_tcp(cb->tcb,bpl);
		}
		break;
	case CLI_RCPT_STATE:
		if (reply == '5') {
			logerr(cb);
		} else if (reply == '2') {
			cb->goodrcpt =1;
		} else {
			/* some kind of temporary failure */
			abort_trans(cb);
			break;
		}
		/* if more rcpts stay in this state */
		if (--cb->rcpts != 0)
			break;

		/* check for no good rcpt on the list */
		if (cb->goodrcpt == 0) {
			if (cb->errlog != NULLLIST)
				retmail(cb);
			(void) unlink(cb->wname);	/* unlink workfile */
			(void) unlink(cb->tname);	/* unlink text */
			abort_trans(cb);
			break;
		}
		/* if this file open fails abort */
		if ((cb->tfile = fopen(cb->tname,"r")) == NULLFILE)
			abort_trans(cb);
		else {
			/* optimize for slow packet links by sending
			 * DATA cmd and the 1st window of text
			 */
			if (cb->tcb->window <= cb->tcb->sndcnt)
				cnt = 0;
			else
				cnt = cb->tcb->window - cb->tcb->sndcnt;
			bp = qdata("DATA\r\n",6);
			cb->cts = 1;
			cb->state = CLI_SEND_STATE;
			if (sendwindow(cb,bp,cnt) == EOF)
				cb->cts = 0;
		}
		break;
	case CLI_SEND_STATE:
		if (reply == '3') {
			cb->state = CLI_UNLK_STATE;
		} else {
			/* change cts to transmit upcall queueing more data */
			cb->cts = 0;
			quit(cb);
		}
		break;
	case CLI_UNLK_STATE:
		/* if a good transfer or permanent failure remove job */
		if (reply == '2' || reply == '5') {
			if (reply == '5')
				logerr(cb);
			/* close and unlink the textfile */
			if(cb->tfile != NULLFILE) {
				fclose(cb->tfile);
				cb->tfile = NULLFILE;
			}
			if (cb->errlog != NULLLIST)
				retmail(cb);
			(void) unlink(cb->tname);
			(void) unlink(cb->wname);	/* unlink workfile */
			log(cb->tcb,"SMTP sent job %s To: %s From: %s",
			cb->jobq->jobname,cb->jobq->to->val,cb->jobq->from);
		}
		if (nextjob(cb)) {
			cb->state = CLI_MAIL_STATE;
			sendit(cb,"MAIL FROM:<%s>\r\n",cb->jobq->from);
		} else 
			/* the quit sent already in smtp_cts */
			cb->state = CLI_QUIT_STATE;
		break;
	case CLI_IDLE_STATE:	/* used after a RSET and more mail to send */
		if (reply != '2')
			quit(cb);
		else {
			cb->state = CLI_MAIL_STATE;
			sendit(cb,"MAIL FROM:<%s>\r\n",cb->jobq->from);
		}
		break;			
	case CLI_QUIT_STATE:
		break;
	}
}

/* abort the currrent job.
 * If more work exists set up the next job if
 * not then shut down.
*/
static void
abort_trans(cb)
register struct smtp_cb *cb;
{
	if(cb->tfile != NULLFILE) {
		fclose(cb->tfile);
		cb->tfile = NULLFILE;
	}
	if (nextjob(cb)) {
		sendit(cb,"RSET\r\n");
		cb->state = CLI_IDLE_STATE;
	} else 
		quit(cb);
}

/* close down link after a failure */
static void
quit(cb)
struct smtp_cb *cb;
{
	cb->state = CLI_QUIT_STATE;
	sendit(cb,quitcmd);		/* issue a quit command */
	close_tcp(cb->tcb);		/* close up connection */
}

/* smtp receiver upcall routine.  fires up the state machine to parse input */
static
void
smtp_rec(tcb,cnt)
struct tcb *tcb;
int16 cnt;
{
	register struct smtp_cb *cb;
	char *inet_ntoa();
	char c;
	struct mbuf *bp;

#ifdef SMTPTRACE
	if (smtptrace > 7)  {
		printf("smtp_rec called\n");
		fflush(stdout);
	}
#endif
	cb = (struct smtp_cb *)tcb->user;	/* point to our struct */
	recv_tcp(tcb,&bp,cnt);	/* suck up chars from low level routine */

	/* Assemble input line in buffer, return if incomplete */
	while(pullup(&bp,&c,1) == 1) {
		switch(c) {
		case '\r':	/* strip cr's */
			continue;
		case '\n':	/* line is finished, go do it! */
			cb->buf[cb->cnt] = '\0';
			smtp_transaction(cb);
			cb->cnt = 0;
			break;
		default:	/* other chars get added to buffer */
			if(cb->cnt != LINELEN-1)
				cb->buf[cb->cnt++] = c;
			break;
		}
	}
}

/* smtp transmitter ready upcall routine.  twiddles cts flag */
static 
void
smtp_cts(tcb,cnt)
struct tcb *tcb;
int16 cnt;
{
	register struct smtp_cb *cb;

#ifdef SMTPTRACE
	if (smtptrace > 7) {
		printf("smtp_cts called avail %d\n",cnt);
		fflush(stdout);
	}
#endif
	cb = (struct smtp_cb *)tcb->user;	/* point to our struct */

	/* don't do anything until/unless we're supposed to be sending */
	if(cb->cts == 0)
		return;

	if (sendwindow(cb,NULLBUF,cnt) == EOF)
		cb->cts = 0;
}

/* fill the rest of the window with data and send out the eof commands.
* It is done this way to minimize the number of packets sent.
*/
static int
sendwindow(cb,ibp,cnt)
register struct smtp_cb *cb;
struct mbuf *ibp;
int16 cnt;
{
	struct mbuf *bpl;
	register struct mbuf *bp;
	char *cp;
	int c;
	
	bpl = ibp;
	if((bp = alloc_mbuf(cnt)) == NULLBUF){
		/* Hard to know what to do here */
		return EOF;
	}
	cp = bp->data;
	while(cnt > 1 && (c = getc(cb->tfile)) != EOF){
#if (UNIX || MAC || AMIGA || ATARI_ST)
		if(c == '\n'){
			*cp++ = '\r';
			bp->cnt++;
			cnt--;
		}
#endif
		*cp++ = c;
		bp->cnt++;
		cnt--;
	}
	append(&bpl,bp);
	if (cnt > 1) {	/* EOF seen */
		fclose(cb->tfile);
		cb->tfile = NULLFILE;
		/* send the end of data character. */
		if (cnt < sizeof(eom) - 1) {
			bp = qdata(eom,5);
			append(&bpl,bp);
			cnt = 0;	/* dont let anyone else in */
		} else {
			memcpy(&bp->data[bp->cnt],eom,sizeof(eom) - 1);
			bp->cnt += sizeof(eom) - 1;
			cnt -= sizeof(eom) - 1;
		}
		/* send the quit in this packet if last job */
		if (cb->jobq->next == NULLJOB) {
			if (cnt < sizeof(quitcmd) - 1) {
				bp = qdata(quitcmd,sizeof(quitcmd) - 1);
				append(&bpl,bp);
			} else {
				memcpy(&bp->data[bp->cnt],
				quitcmd,sizeof(quitcmd) - 1);
				bp->cnt += sizeof(quitcmd) - 1;
			}
		}
		send_tcp(cb->tcb,bpl);
		if (cb->jobq->next == NULLJOB)
			close_tcp(cb->tcb);	/* close up connection */
		return EOF;
	} else {
		send_tcp(cb->tcb,bpl);
		return 0;
	}
}

/* smtp state change upcall routine. */
/*ARGSUSED*/
static
void
smtp_state(tcb,old,new)
register struct tcb *tcb;
char old,new;
{
	register struct smtp_cb *cb;
	extern char *tcpstates[];

#ifdef SMTPTRACE
	if (smtptrace > 7) {
		printf("smtp_state called: %s\n",tcpstates[new]);
		fflush(stdout);
	}
#endif
	cb = (struct smtp_cb *)tcb->user;
	switch(new) {
	case ESTABLISHED:
		cb->state = CLI_OPEN_STATE;	/* shouldn't be needed */
		break;
	case CLOSE_WAIT:
		close_tcp(tcb);			/* shut things down */
		break;
	case CLOSED:
		/* if this close was not done by us ie. a RST */
		if(cb->tfile != NULLFILE)
			fclose(cb->tfile);
		del_session(cb);
		del_tcp(tcb);
		break;
	}
}

/* Send message back to server */
/*VARARGS*/
static void
sendit(cb,fmt,arg1,arg2)
struct smtp_cb *cb;
char *fmt,*arg1,*arg2;
{
	struct mbuf *bp;
	char tmpstring[256];

#ifdef SMTPTRACE
	if (smtptrace) {
		printf(">>> ");
		printf(fmt,arg1,arg2);
		fflush(stdout);
	}
#endif
	sprintf(tmpstring,fmt,arg1,arg2);
	bp = qdata(tmpstring,(int16)strlen(tmpstring));
	send_tcp(cb->tcb,bp);
}

/* create mail lockfile */
int
mlock(dir,id)
char *dir,*id;
{
	char lockname[LINELEN];
	int fd;
	/* Try to create the lock file in an atomic operation */
	sprintf(lockname,"%s/%s.lck",dir,id);
#if	(ATARI_ST && !LATTICE)
	if(!access(lockname,0) || (fd = creat(lockname, 0666)) == -1)
		return -1;
#else
	if((fd = open(lockname, O_WRONLY|O_EXCL|O_CREAT,0600)) == -1)
		return -1;
#endif
	close(fd);
	return 0;
}

/* remove mail lockfile */
int
rmlock(dir,id)
char *dir,*id;
{
	char lockname[LINELEN];
	sprintf(lockname,"%s/%s.lck",dir,id);
	return(unlink(lockname));
}

/* free the message struct and data */
static void
del_session(cb)
register struct smtp_cb *cb;
{
	register struct smtp_job *jp,*tp;
	register int i;

	if (cb == NULLCB)
		return;
	for(i=0; i<MAXSESSIONS; i++) 
		if(cli_session[i] == cb) {
			cli_session[i] = NULLCB;
			break;
		}

	if(cb->wname != NULLCHAR)
		free(cb->wname);
	if(cb->tname != NULLCHAR)
		free(cb->tname);
	for (jp = cb->jobq; jp != NULLJOB;jp = tp) {
			tp = jp->next;
			del_job(jp);
	}
	del_list(cb->errlog);
	free((char *)cb);
	smtpsessions--;	/* number of connections active */
}

void
del_job(jp)
register struct smtp_job *jp;
{
	if ( *jp->jobname != '\0')
		(void) rmlock(mailqdir,jp->jobname);
	if(jp->from != NULLCHAR)
		free(jp->from);
	del_list(jp->to);
	free((char *)jp);
}

/* delete a list of list structs */
void
del_list(lp)
struct list *lp;
{
	register struct list *tp, *tp1;
	for (tp = lp; tp != NULLLIST; tp = tp1) {
			tp1 = tp->next;
			if(tp->val != NULLCHAR)
				free(tp->val);
			free((char *)tp);
	}
}

/* return message to sender */
static void
retmail(cb)
struct smtp_cb *cb;
{
	register struct list *lp;
	register FILE *tfile;
	register int c;
	FILE *infile,*tmpfile();
	char *host,*to;
	time_t t,time();
#ifdef SMTPTRACE
	if (smtptrace > 5) {
		printf("smtp job %s returned to sender\n",cb->wname);
		fflush(stdout);
	}
#endif
	/* A null From<> so no looping replys to MAIL-DAEMONS */
	to = cb->jobq->from;
	if (*to == '\0')
		return;
	if ((host = index(to,'@')) == NULLCHAR)
		host = hostname;
	else
		host++;
	if ((infile = fopen(cb->tname,"r")) == NULLFILE)
		return;
	if ((tfile = tmpfile()) == NULLFILE) {
		fclose(infile);
		return;
	}
	time(&t);
	fprintf(tfile,"Date: %s",ptime(&t));
	fprintf(tfile,"Message-Id: <%ld@%s>\n",get_msgid(),hostname);
	fprintf(tfile,"From: MAILER-DAEMON@%s\n",hostname);
	fprintf(tfile,"To: %s\n",to);
	fprintf(tfile,"Subject: Failed mail\n\n");
	fprintf(tfile,"  ===== transcript follows =====\n\n");

	for (lp = cb->errlog; lp != NULLLIST; lp = lp->next)
		fprintf(tfile,"%s\n",lp->val);

	fprintf(tfile,"\n  ===== Unsent message follows ====\n");

	while((c = getc(infile)) != EOF)
		if (putc(c,tfile) == EOF)
			break;
	fclose(infile);
	fseek(tfile,0L,0);
	if ((smtpmode & QUEUE) != 0)
		router_queue(cb->tcb,tfile,"",(struct list *)to);
	else
		queuejob(cb->tcb,tfile,host,to,"");
	fclose(tfile);
}

/* look to see if a smtp control block exists for this ipdest */
static struct smtp_cb *
lookup(destaddr)
int32 destaddr;
{
	register int i;

	for(i=0; i<MAXSESSIONS; i++) {
		if (cli_session[i] == NULLCB)
			continue;
		if(cli_session[i]->ipdest == destaddr)
			return cli_session[i];
	}
	return NULLCB;
}

/* create a new  smtp control block */
static struct smtp_cb *
newcb()
{
	register int i;
	register struct smtp_cb *cb;

	for(i=0; i<MAXSESSIONS; i++) {
		if(cli_session[i] == NULLCB) {
			cb = (struct smtp_cb *)calloc(1,sizeof(struct smtp_cb));
			if (cb == NULLCB)
				return(NULLCB);
			cb->wname = malloc((unsigned)strlen(mailqdir)+JOBNAME);
			if (cb->wname == NULLCHAR) {
				free((char *)cb);
				return(NULLCB);
			}
			cb->tname = malloc((unsigned)strlen(mailqdir)+JOBNAME);
			if (cb->tname == NULLCHAR) {
				free(cb->wname);
				free((char *)cb);
				return(NULLCB);
			}
			cb->state = CLI_INIT_STATE;
			cli_session[i] = cb;
			smtpsessions++;	/* number of connections active */
			return(cb);
		}
	}
	return NULLCB;
}

static void
execjobs()
{
	struct socket lsocket, fsocket;
	void smtp_rec(), smtp_cts(), smtp_state();
	register struct smtp_cb *cb;
	register int i;

	for(i=0; i<MAXSESSIONS; i++) {
		cb = cli_session[i];
		if (cb == NULLCB) 
			continue;
		if(cb->state != CLI_INIT_STATE)
			continue;

		sprintf(cb->tname,"%s/%s.txt",mailqdir,cb->jobq->jobname);
		sprintf(cb->wname,"%s/%s.wrk",mailqdir,cb->jobq->jobname);

		/* setup the socket */
		fsocket.address = cb->ipdest;
		fsocket.port = SMTP_PORT;
		lsocket.address = ip_addr;	/* our ip address */
		lsocket.port = lport++;		/* next unused port */
#ifdef SMTPTRACE
		if (smtptrace) {
			printf("Trying Connection to %s\n",inet_ntoa(fsocket.address));
			fflush(stdout);
		}
#endif

		/* open smtp connection */
		cb->state = CLI_OPEN_STATE;	/* init state placeholder */
		cb->tcb = open_tcp(&lsocket,&fsocket,TCP_ACTIVE,tcp_window,
			smtp_rec,smtp_cts,smtp_state,0,(char *)cb);
		cb->tcb->user = (char *)cb;	/* Upward pointer */
	}
}
	
/* add this job to control block queue */
struct smtp_job *
setupjob(cb,id,from)
struct smtp_cb *cb;
char *id,*from;
{
	register struct smtp_job *p1,*p2;

	p1 = (struct smtp_job *)calloc(1,sizeof(struct smtp_job));
	if (p1 == NULLJOB)
		return NULLJOB;
	p1->from = malloc((unsigned)strlen(from) + 1);
	if (p1->from == NULLCHAR) {
		free((char *)p1);
		return NULLJOB;
	}
	strcpy(p1->from,from);
	strcpy(p1->jobname,id);
	/* now add to end of jobq */
	if ((p2 = cb->jobq) == NULLJOB)
		cb->jobq = p1;
	else {
		while(p2->next != NULLJOB)
			p2 = p2->next;
		p2->next = p1;
	}
	return p1;
}

/* called to advance to the next job */
static int
nextjob(cb)
register struct smtp_cb *cb;
{
	register struct smtp_job *jp;


	jp = cb->jobq->next;
	del_job(cb->jobq);
	if (jp == NULLJOB) {
		cb->jobq = NULLJOB;
		return 0;
	}
	/* remove the error log of previous message */
	del_list(cb->errlog);
	cb->errlog = NULLLIST;
	cb->goodrcpt = 0;
	cb->jobq = jp;
	sprintf(cb->tname,"%s/%s.txt",mailqdir,jp->jobname);
	sprintf(cb->wname,"%s/%s.wrk",mailqdir,jp->jobname);
#ifdef SMTPTRACE
	if (smtptrace > 5) {
		printf("sending job %s\n",jp->jobname);
		fflush(stdout);
	}
#endif
		return 1;

}


/* mail routing funtion. For now just used the hosts file */
int32
mailroute(dest)
char *dest;
{
	int32 destaddr;

	/* look up address or use the gateway */
	if ((destaddr = resolve(dest)) == 0)
		if (gateway != 0) 
			destaddr = gateway; /* Use the gateway  */
	return destaddr;
	
}

/* save error reply for in error list */
static void
logerr(cb)
struct smtp_cb *cb;
{
	register struct list *lp,*tp;
	if ((tp = (struct list *)calloc(1,sizeof(struct list))) == NULLLIST)
		return;
	if ((tp->val = malloc((unsigned)strlen(cb->buf)+1)) == NULLCHAR) {
		free((char *)tp);
		return;
	}
	/* find end of list */
	if ((lp = cb->errlog) == NULLLIST)
		cb->errlog = tp;
	else {
		while(lp->next != NULLLIST)
			lp = lp->next;
		lp->next = tp;
	}
	strcpy(tp->val,cb->buf);
}
