/* SMTP Server state machine - see RFC 821
 *  enhanced 4/88 Dave Trulli nn2z
 */
#include <stdio.h>
#include <time.h>
#ifdef __TURBOC__
#include <fcntl.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <memory.h>
#include <string.h>
#endif
#include <ctype.h>
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "tcp.h"
#include "smtp.h"

#ifdef	UNIX
#undef	toupper
#undef	tolower
time_t time();
#endif
#ifdef	BSD
char *sprintf();
#endif
char *getname();
void mail_delete();
static int rqueuejob();
int queuejob();
int validate_address();
long get_msgid();
struct list *addlist();
struct list * expandalias();

/* Command table */
static char *commands[] = {
	"helo",
#define	HELO_CMD	0
	"noop",
#define	NOOP_CMD	1
	"mail from:",
#define	MAIL_CMD	2
	"quit",
#define	QUIT_CMD	3
	"rcpt to:",
#define	RCPT_CMD	4
	"help",
#define	HELP_CMD	5
	"data",
#define	DATA_CMD	6
	"rset",
#define	RSET_CMD	7
	NULLCHAR
};

/* Reply messages */
static char help[] = "214-Commands:\r\n214-HELO NOOP MAIL QUIT RCPT HELP DATA RSET\r\n214 End\r\n";
static char banner[] = "220 %s SMTP ready\r\n";
static char closing[] = "221 Closing\r\n";
static char ok[] = "250 Ok\r\n";
static char reset[] = "250 Reset state\r\n";
static char sent[] = "250 Sent\r\n";
static char ourname[] = "250 %s, Share and Enjoy!\r\n";
static char enter[] = "354 Enter mail, end with .\r\n";
static char ioerr[] = "452 Temp file write error\r\n";
static char mboxerr[] = "452 Mailbox write error\r\n";
static char badcmd[] = "500 Command unrecognized\r\n";
static char syntax[] = "501 Syntax error\r\n";
static char needrcpt[] = "503 Need RCPT (recipient)\r\n";
static char unknown[] = "550 <%s> address unknown\r\n";

static struct tcb *smtp_tcb;
/* Start up SMTP receiver service */
smtp1(argc,argv)
int argc;
char *argv[];
{
	struct socket lsocket;
	void r_mail(),s_mail();

	lsocket.address = ip_addr;
	if(argc < 2)
		lsocket.port = SMTP_PORT;
	else
		lsocket.port = atoi(argv[1]);

	smtp_tcb = open_tcp(&lsocket,NULLSOCK,
		TCP_SERVER,0,r_mail,NULLVFP,s_mail,0,(char *)NULL);
}

/* Shutdown SMTP service (existing connections are allowed to finish) */
smtp0()
{
	if(smtp_tcb != NULLTCB)
		close_tcp(smtp_tcb);
}

/* SMTP connection state change upcall handler */
/*ARGSUSED*/
static void
s_mail(tcb,old,new)
struct tcb *tcb;
char old,new;
{
	struct mail *mp,*mail_create();

	switch(new){
#ifdef	QUICKSTART
	case SYN_RECEIVED:
#else
	case ESTABLISHED:
#endif
		if((mp = mail_create(tcb)) == NULLMAIL){
			close_tcp(tcb);
			break;
		}
		(void) tprintf(mp->tcb,banner,hostname);
		log(tcb,"open SMTP");
		break;		
	case CLOSE_WAIT:
		close_tcp(tcb);
		break;
	case CLOSED:
		log(tcb,"close SMTP");
		mp = (struct mail *)tcb->user;
		mail_delete(mp);				
		del_tcp(tcb);
		/* Check if server is being shut down */
		if(tcb == smtp_tcb)
			smtp_tcb = NULLTCB;
		break;
	}
}

/* SMTP receiver upcall handler */
static void
r_mail(tcb,cnt)
struct tcb *tcb;
int16 cnt;
{
	register struct mail *mp;
	char c;
	struct mbuf *bp;
	char *inet_ntoa();
	void docommand(),doline();

	if((mp = (struct mail *)tcb->user) == NULLMAIL){
		/* Unknown session */
		close_tcp(tcb);
		return;
	}
	recv_tcp(tcb,&bp,cnt);
	/* Assemble an input line in the session buffer.
	 * Return if incomplete
	 */
	while(pullup(&bp,&c,1) == 1){
		switch(c){
		case '\r':	/* Strip cr's */
#ifdef MSDOS
		case '\032':    /* Strip ctrl/Z's */
#endif
			continue;
		case '\n':	/* Complete line; process it */
			mp->buf[mp->cnt] = '\0';
			doline(mp);
			break;
		default:	/* Assemble line */
			if(mp->cnt != LINELEN-1)
				mp->buf[mp->cnt++] = c;
			break;
		}
	}
}
/* Process a line read on an SMTP connection (any state) */
static void
doline(mp)
register struct mail *mp;
{
	void docommand(),deliver();

	switch(mp->state){
	case COMMAND_STATE:
		docommand(mp);
		break;
	case DATA_STATE:
		tcp_output(mp->tcb);	/* Send ACK; disk I/O is slow */
		if(mp->buf[0] == '.' && mp->buf[1] == '\0'){
			mp->state = COMMAND_STATE;
		/* Also sends appropriate response */
			deliver(mp);
			fclose(mp->data);
			mp->data = NULLFILE;
			del_list(mp->to);
			mp->to = NULLLIST;
			break;
		}
		/* for UNIX mail compatiblity */
		if (strncmp(mp->buf,"From ",5) == 0)
			(void) putc('>',mp->data);
		/* Append to data file */
		if(fprintf(mp->data,"%s\n",mp->buf) < 0){
			mp->state = COMMAND_STATE;
			(void) tprintf(mp->tcb,ioerr);
		}
		break;
	}
	mp->cnt = 0;
}
/* Create control block, initialize */
static struct mail *
mail_create(tcb)
register struct tcb *tcb;
{
	register struct mail *mp;

	if((mp = (struct mail *)calloc(1,sizeof (struct mail))) == NULLMAIL)
		return NULLMAIL;
	mp->tcb = tcb;		/* Downward pointer */
	tcb->user = (char *)mp;	/* Upward pointer */
	return mp;
}

/* Free resources, delete control block */
static void
mail_delete(mp)
register struct mail *mp;
{

	if (mp == NULLMAIL)
		return;
	if(mp->system != NULLCHAR)
		free(mp->system);
	if(mp->from != NULLCHAR)
		free(mp->from);
	if(mp->data != NULLFILE)
		fclose(mp->data);
	del_list(mp->to);
	free((char *)mp);
}

/* Parse and execute mail commands */
static void
docommand(mp)
register struct mail *mp;
{
	register char **cmdp,*arg,*cp,*cmd;
	FILE *tmpfile();
	long t;
	char address_type;

	cmd = mp->buf;
	if(mp->cnt < 4){
		/* Can't be a legal SMTP command */
		(void) tprintf(mp->tcb,badcmd);
		return;
	}	
	cmd = mp->buf;

	/* Translate entire buffer to lower case */
	for(cp = cmd;*cp != '\0';cp++)
		*cp = tolower(*cp);

	/* Find command in table; if not present, return syntax error */
	for(cmdp = commands;*cmdp != NULLCHAR;cmdp++)
		if(strncmp(*cmdp,cmd,strlen(*cmdp)) == 0)
			break;
	if(*cmdp == NULLCHAR){
		(void) tprintf(mp->tcb,badcmd);
		return;
	}
	arg = &cmd[strlen(*cmdp)];
	/* Skip spaces after command */
	while(*arg == ' ')
		arg++;
	/* Execute specific command */
	switch(cmdp-commands){
	case HELO_CMD:
		if(mp->system != NULLCHAR)
			free(mp->system);
		if((mp->system = malloc((unsigned)strlen(arg)+1)) == NULLCHAR){
			/* If the system is out of memory, just close */
			close_tcp(mp->tcb);
			break;			
		} else {
			strcpy(mp->system,arg);
			(void) tprintf(mp->tcb,ourname,hostname);
		}
		break;
	case NOOP_CMD:
		(void) tprintf(mp->tcb,ok);
		break;
	case MAIL_CMD:
		if(mp->from != NULLCHAR)
			free(mp->from);
		if((mp->from = malloc((unsigned)strlen(arg)+1)) == NULLCHAR){
			/* If the system is out of memory, just close */
			close_tcp(mp->tcb);
			break;			
		} else {
			if((cp = getname(arg)) == NULLCHAR){
				(void) tprintf(mp->tcb,syntax);
				break;
			}
			strcpy(mp->from,cp);
			(void) tprintf(mp->tcb,ok);
		}
		break;
	case QUIT_CMD:
		(void) tprintf(mp->tcb,closing);
		close_tcp(mp->tcb);
		break;
	case RCPT_CMD:	/* Specify recipient */
		if((cp = getname(arg)) == NULLCHAR){
			(void) tprintf(mp->tcb,syntax);
			break;
		}

		/* check if address is ok */
		if ((address_type = validate_address(cp)) == BADADDR) {
			(void) tprintf(mp->tcb,unknown,cp);
			break;
		}
		/* if a local address check for an alias */
		if (address_type == LOCAL)
			expandalias(&mp->to, cp);
		else
			/* a remote address is added to the list */
			addlist(&mp->to, cp, address_type);

		(void) tprintf(mp->tcb,ok);
		break;
	case HELP_CMD:
		(void) tprintf(mp->tcb,help);
		break;
	case DATA_CMD:
		if(mp->to == NULLLIST){
			(void) tprintf(mp->tcb,needrcpt);
			break;
		}
		tcp_output(mp->tcb);	/* Send ACK; disk I/O is slow */
		if((mp->data = tmpfile()) == NULLFILE){
			(void) tprintf(mp->tcb,ioerr);
			break;
		}
		/* Add timestamp; ptime adds newline */
		time(&t);
		fprintf(mp->data,"Received: ");
		if(mp->system != NULLCHAR)
			fprintf(mp->data,"from %s ",mp->system);
		fprintf(mp->data,"by %s with SMTP\n\tid AA%ld ; %s",
				hostname, get_msgid(), ptime(&t));
		if(ferror(mp->data)){
			(void) tprintf(mp->tcb,ioerr);
		} else {
			mp->state = DATA_STATE;
			(void) tprintf(mp->tcb,enter);
		}
		break;
	case RSET_CMD:
		del_list(mp->to);
		mp->to = NULLLIST;
		mp->state = COMMAND_STATE;
		(void) tprintf(mp->tcb,reset);
		break;
	}
}
/* Given a string of the form <user@host>, extract the part inside the
 * brackets and return a pointer to it.
 */
static
char *
getname(cp)
register char *cp;
{
	register char *cp1;

	if((cp = index(cp,'<')) == NULLCHAR)
		return NULLCHAR;
	cp++;	/* cp -> first char of name */
	if((cp1 = index(cp,'>')) == NULLCHAR)
		return NULLCHAR;
	*cp1 = '\0';
	return cp;
}

/* Deliver mail to the appropriate mail boxes and delete temp file */
static
void
deliver(mp)
register struct mail *mp;
{
	int ret;

	/* send to the rqueue */
	if ((smtpmode & QUEUE) != 0) {
		ret = router_queue(mp->tcb,mp->data,mp->from,mp->to);
		if (ret != 0)
			(void) tprintf(mp->tcb,ioerr);
	} else {
		ret = mailit(mp->tcb,mp->data,mp->from,mp->to);
		if (ret != 0)
			(void) tprintf(mp->tcb,mboxerr);
	}
	if (ret == 0)
		(void) tprintf(mp->tcb,sent);
		
}

/* used to save local mail or reroute remote mail */
mailit(tcb,data,from,to)
struct tcb *tcb;
FILE *data;
char *from;
struct list *to;
{
	register struct list *ap;
	register FILE *fp;
	int c;
	char	mailbox[50];
	char	*cp;
	char	*desthost;
	int	fail = 0;
	time_t	t;
	for(ap = to;ap != NULLLIST;ap = ap->next) {

		fseek(data,0L,0);	/* rewind */

		/* non local mail queue it */
		if (ap->type == DOMAIN) {
			if ((desthost = index(ap->val,'@')) != NULLCHAR);
				desthost++;
			fail = queuejob(tcb,data,desthost,ap->val,from);
		} else {
			/* strip off host name */
			if ((cp = index(ap->val,'@')) != NULLCHAR)
				*cp = '\0';

			/* truncate long user names */
			if (strlen(ap->val) > MBOXLEN)
				ap->val[MBOXLEN] = '\0';

			/* if mail file is busy save it in our smtp queue
			 * and let the smtp daemon try later.
			 */
			if (mlock(mailspool,ap->val))
				fail = queuejob(tcb,data,hostname,ap->val,from);
			else {
				sprintf(mailbox,"%s/%s.txt",mailspool,ap->val);
				if((fp = fopen(mailbox,"a+")) != NULLFILE) {
					time(&t);
					fprintf(fp,
					"From %s %s",from,ctime(&t));
					while((c = getc(data)) != EOF)
						if(putc(c,fp) == EOF)
							break;
					if(ferror(fp))
						fail = 1;
					else
						fprintf(fp,"\n");
					/* Leave a blank line between msgs */
					fclose(fp);
					printf("New mail arrived for %s\n",ap->val);
					fflush(stdout);
				} else 
					fail = 1;
				(void) rmlock(mailspool,ap->val);
				if (fail)
					break;
				log(tcb,
				"SMTP recv: To: %s From: %s",ap->val,from);
			}
		}
	}
	return(fail) ;
}

/* Return Date/Time in Arpanet format in passed string */
char *
ptime(t)
long *t;
{
	/* Print out the time and date field as
	 *		"DAY day MONTH year hh:mm:ss ZONE"
	 */
	register struct tm *ltm;
	static char tz[4];
	static char str[40];
	extern char *getenv();
	extern struct tm *localtime();
	char *p;
	static char *days[7] = {
    "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };

	static char *months[12] = {
		"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov","Dec" };

	/* Read the system time */
	ltm = localtime(t);

	if (*tz == '\0')
		if (((p = getenv("TZ")) == NULL) &&
		     (p = getenv("TIMEZONE")) == NULL)
			strcpy(tz,"GMT");
		else
			strncpy(tz,p,3);

	/* rfc 822 format */
	sprintf(str,"%s, %.2d %s %02d %02d:%02d:%02d %.3s\n",
		days[ltm->tm_wday],
		ltm->tm_mday,
		months[ltm->tm_mon],
		ltm->tm_year,
		ltm->tm_hour,
		ltm->tm_min,
		ltm->tm_sec,
		tz);
	return(str);
}

long 
get_msgid()
{
	char sfilename[LINELEN];
	char s[20];
	register long sequence = 0;
	FILE *sfile;
	long atol();

	sprintf(sfilename,"%s/sequence.seq",mailqdir);
	sfile = fopen(sfilename,"r");

	/* if sequence file exists, get the value, otherwise set it */
	if (sfile != NULL) {
		(void) fgets(s,sizeof(s),sfile);
		sequence = atol(s);
	/* Keep it in range of and 8 digit number to use for dos name prefix. */
		if (sequence < 0L || sequence > 99999999L )
			sequence = 0;
		fclose(sfile);
	}

	/* increment sequence number, and write to sequence file */
	sfile = fopen(sfilename,"w");
	fprintf(sfile,"%ld",++sequence);
	fclose(sfile);
	return sequence;
}

#ifdef	MSDOS
/* Illegal characters in a DOS filename */
static char baddoschars[] = "\"[]:|<>+=;,";
#endif

/* test if mail address is valid */
int
validate_address(s)
char *s;
{
	char *cp;
	int32 addr;
	int32 mailroute();



	/* if address has @ in it the check dest address */
	if ((cp = index(s,'@')) != NULLCHAR) {
		cp++;
		/* 1st check if its our hostname
		* if not then check the hosts file and see
		* if we can resolve ther address to a know site
		* or one of our aliases
		*/
		if (strcmp(cp,hostname) != 0) {
			if ((addr = mailroute(cp)) == 0
				&& (smtpmode & QUEUE) == 0)
				return BADADDR;
			if (addr != ip_addr)
				return DOMAIN;
		}
		
		/* on a local address remove the host name part */
		*--cp = '\0';
	}

	/* if using an external router leave address alone */
	if ((smtpmode & QUEUE) != 0)
		return LOCAL;


	/* check for the user%host hack */
	if ((cp = index(s,'%')) != NULLCHAR) {
		*cp = '@';
		cp++;
		/* reroute based on host name following the % seperator */
		if (mailroute(cp) == 0)
			return BADADDR;
		else
			return DOMAIN;
	}

#ifdef MSDOS	/* dos file name checks */
	/* Check for characters illegal in MS-DOS file names */
	for(cp = baddoschars;*cp != '\0';cp++){
		if(index(s,*cp) != NULLCHAR)
			return BADADDR;	
	}
#endif
	return LOCAL;
}

/* place a mail job in the outbound queue */
int
queuejob(tcb,dfile,host,to,from)
struct tcb *tcb;
FILE *dfile;
char *host,*to,*from;
{
	FILE *fp;
	char tmpstring[50];
	char prefix[9];
	register int c;

	sprintf(prefix,"%ld",get_msgid());
	log(tcb,"SMTP queue job %s To: %s From: %s",prefix,to,from);
	mlock(mailqdir,prefix);
	sprintf(tmpstring,"%s/%s.txt",mailqdir,prefix);
	if((fp = fopen(tmpstring,"w")) == NULLFILE) {
		(void) rmlock(mailqdir,prefix);
		return 1;
	}
	while((c = getc(dfile)) != EOF)
		if(putc(c,fp) == EOF)
			break;
	if(ferror(fp)){
		fclose(fp);
		(void) rmlock(mailqdir,prefix);
		return 1;
	}
	fclose(fp);
	sprintf(tmpstring,"%s/%s.wrk",mailqdir,prefix);
	if((fp = fopen(tmpstring,"w")) == NULLFILE) {
		(void) rmlock(mailqdir,prefix);
		return 1;
	}
	fprintf(fp,"%s\n%s\n%s\n",host,from,to);
	fclose(fp);
	(void) rmlock(mailqdir,prefix);
	return 0;
}

/* Deliver mail to the appropriate mail boxes */
int
router_queue(tcb,data,from,to)
struct tcb *tcb;
FILE *data;
char *from;
struct list *to;
{
	int c;
	register struct list *ap;
	FILE *fp;
	char tmpstring[50];
	char prefix[9];

	sprintf(prefix,"%ld",get_msgid());
	mlock(routeqdir,prefix);
	sprintf(tmpstring,"%s/%s.txt",routeqdir,prefix);
	if((fp = fopen(tmpstring,"w")) == NULLFILE) {
		(void) rmlock(routeqdir,prefix);
		return 1;
	}
	fseek(data,0L,0);	/* rewind */
	while((c = getc(data)) != EOF)
		if(putc(c,fp) == EOF)
			break;
	if(ferror(fp)){
		fclose(fp);
		(void) rmlock(routeqdir,prefix);
		return 1;
	}
	fclose(fp);
	sprintf(tmpstring,"%s/%s.wrk",routeqdir,prefix);
	if((fp = fopen(tmpstring,"w")) == NULLFILE) {
		(void) rmlock(routeqdir,prefix);
		return 1;
	}
	fprintf(fp,"From: %s\n",from);
	for(ap = to;ap != NULLLIST;ap = ap->next) {
		fprintf(fp,"To: %s\n",ap->val);
	}
	fclose(fp);
	(void) rmlock(routeqdir,prefix);
	log(tcb,"SMTP rqueue job %s From: %s",prefix,from);
	return 0;
}

/* add an element to the front of the list pointed to by head 
** return NULLLIST if out of memory.
*/
struct list *
addlist(head,val,type)
struct list **head;
char *val;
int type;
{
	register struct list *tp;

	tp = (struct list *)calloc(1,sizeof(struct list));
	if (tp == NULLLIST)
		return NULLLIST;

	tp->next = NULLLIST;

	/* allocate storage for the char string */
	if ((tp->val = malloc((unsigned)strlen(val)+1)) == NULLCHAR) {
		(void) free((char *)tp);
		return NULLLIST;
	}
	strcpy(tp->val,val);
	tp->type = type;

	/* add entry to front of existing list */
	if (*head == NULLLIST)
		*head = tp;
	else {
		tp->next = *head;
		*head = tp;
	}
	return tp;

}

#define SKIPWORD(X) while(*X && *X!=' ' && *X!='\t' && *X!='\n' && *X!= ',') X++;
#define SKIPSPACE(X) while(*X ==' ' || *X =='\t' || *X =='\n' || *X == ',') X++;

/* check for and alias and expand alias into a address list */
struct list *
expandalias(head, user)
struct list **head;
char *user;
{
	FILE *fp;
	register char *s,*p;
	int inalias;
	struct list *tp;
	char buf[LINELEN];
	
	
		/* no alias file found */
	if ((fp = fopen(alias, "r")) == NULLFILE)
		return addlist(head, user, LOCAL);

	inalias = 0;
	while (fgets(buf,LINELEN,fp) != NULLCHAR) {
		p = buf;
		if ( *p == '#' || *p == '\0')
			continue;
		rip(p);

		/* if not in an matching entry skip continuation lines */
		if (!inalias && isspace(*p))
			continue;

		/* when processing an active alias check for a continuation */
		if (inalias) {
			if (!isspace(*p)) 
				break;	/* done */
		} else {
			s = p;
			SKIPWORD(p);
			*p++ = '\0';	/* end the alias name */
			if (strcmp(s,user) != 0)
				continue;	/* no match go on */
			inalias = 1;
		}

		/* process the recipients on the alias line */
		SKIPSPACE(p);
		while(*p != '\0' && *p != '#') {
			s = p;
			SKIPWORD(p);
			if (*p != '\0')
				*p++ = '\0';

			/* find hostname */
			if (index(s,'@') != NULLCHAR)
				tp = addlist(head,s,DOMAIN);
			else
				tp = addlist(head,s,LOCAL);
			SKIPSPACE(p);
		}
	}
	(void) fclose(fp);

	if (inalias)	/* found and processed and alias. */
		return tp;

	/* no alias found treat as a local address */
	return addlist(head, user, LOCAL);
}
