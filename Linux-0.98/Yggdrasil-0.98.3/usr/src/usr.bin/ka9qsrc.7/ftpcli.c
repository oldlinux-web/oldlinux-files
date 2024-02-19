/* FTP client (interactive user) code */
#define	LINELEN		128	/* Length of command buffer */
#define DIRBUF		256	/* length of director buffer */
#include <stdio.h>
#ifdef __TURBOC__
#include "fcntl.h"
#endif
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "netuser.h"
#include "icmp.h"
#include "timer.h"
#include "tcp.h"
#include "ftp.h"
#include "session.h"
#include "cmdparse.h"
#include "telnet.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include "finger.h"
#include "nr4.h"
#ifdef	BSD
char *sprintf();
#endif

/* #ifdef __TURBOC__	*/
/* #include <fcntl.h>	*/
/* #endif		*/

extern struct session *current;
extern char nospace[];
extern char badhost[];
static char notsess[] = "Not an FTP session!\n";
static char cantwrite[] = "Can't write %s\n";
static char cantread[] = "Can't read %s\n";

int donothing(),doftpcd(),dolist(),doget(),dols(),doput(),dotype(),
	doabort(), domkdir(),dormdir(), domget(), domput();

struct cmds ftpabort[] = {
	"",		donothing,	0,	NULLCHAR,		NULLCHAR,
	"abort",	doabort,	0,	NULLCHAR,		NULLCHAR,
	NULLCHAR,	NULLFP,		0,	"Only valid command is \"abort\"", NULLCHAR,
};

struct cmds ftpcmds[] = {
	"",		donothing,	0,	NULLCHAR,		NULLCHAR,
	"cd",		doftpcd,	2,	"cd <directory>",	NULLCHAR,
	"dir",		dolist,		0,	NULLCHAR,		NULLCHAR,
	"list",		dolist,		0,	NULLCHAR,		NULLCHAR,
	"get",		doget,		2,	"get remotefile <localfile>",	NULLCHAR,
	"ls",		dols,		0,	NULLCHAR,		NULLCHAR,
	"mkdir",	domkdir,	2,	"mkdir <directory>",	NULLCHAR,
	"mget",		domget,		2,	"mget <file> [<file> ...]",	NULLCHAR,
	"mput",		domput,		2,	"mput <file> [<file> ...]",	NULLCHAR,
	"nlst",		dols,		0,	NULLCHAR,		NULLCHAR,
	"rmdir",	dormdir,	2,	"rmdir <directory>",	NULLCHAR,
	"put",		doput,		2,	"put localfile <remotefile>",	NULLCHAR,
	"type",		dotype,		0,	NULLCHAR,		NULLCHAR,
	NULLCHAR,	NULLFP,		0,	 NULLCHAR,		NULLCHAR,
};

/* Handle top-level FTP command */
doftp(argc,argv)
int argc;
char *argv[];
{
	int32 resolve();
	int ftpparse();
	char *inet_ntoa();
	void ftpccr(),ftpccs();
	struct session *s;
	struct ftp *ftp,*ftp_create();
	struct tcb *tcb;
	struct socket lsocket,fsocket;

	lsocket.address = ip_addr;
	lsocket.port = lport++;
	if((fsocket.address = resolve(argv[1])) == 0){
		printf(badhost,argv[1]);
		return 1;
	}
	if(argc < 3)
		fsocket.port = FTP_PORT;
	else
		fsocket.port = atoi(argv[2]);

	/* Allocate a session control block */
	if((s = newsession()) == NULLSESSION){
		printf("Too many sessions\n");
		return 1;
	}
	current = s;
	if((s->name = malloc((unsigned)strlen(argv[1])+1)) != NULLCHAR)
		strcpy(s->name,argv[1]);
	s->type = FTP;
	s->parse = ftpparse;

	/* Allocate an FTP control block */
	if((ftp = ftp_create(LINELEN)) == NULLFTP){
		s->type = FREE;
		printf(nospace);
		return 1;
	}
	ftp->state = STARTUP_STATE;
	s->cb.ftp = ftp;	/* Downward link */
	ftp->session = s;	/* Upward link */

	/* Now open the control connection */
	tcb = open_tcp(&lsocket,&fsocket,TCP_ACTIVE,
		0,ftpccr,NULLVFP,ftpccs,0,(char *)ftp);
	ftp->control = tcb;
	go();
	return 0;
}
/* Parse user FTP commands */
int
ftpparse(line,len)
char *line;
int16 len;
{
	struct mbuf *bp;

	switch(current->cb.ftp->state){
	case RECEIVING_STATE:
	case SENDING_STATE:
		/* The only command allowed in data transfer state is ABORT */
		if(cmdparse(ftpabort,line) == -1){
			printf("Transfer in progress; only ABORT is acceptable\n");
		}
		fflush(stdout);
		break;
	case COMMAND_STATE:
		/* Save it now because cmdparse modifies the original */
		bp = qdata(line,len);

		if(cmdparse(ftpcmds,line) == -1){
			/* Send it direct */
			if(bp != NULLBUF)
				send_tcp(current->cb.ftp->control,bp);
			else
				printf(nospace);
		} else {
			free_p(bp);
		}
		fflush(stdout);
		break;
	case STARTUP_STATE:		/* Starting up autologin */
		printf("Not connected yet, ignoring %s\r\n",line);
		break;
	case USER_STATE:		/* Got the user name */
		line[len] = '\0';
		return sndftpmsg(current->cb.ftp,"USER %s",line);
	case PASS_STATE:		/* Got the password */
		cooked();
		line[len] = '\0';
		return sndftpmsg(current->cb.ftp,"PASS %s",line);
	}
	return 0;
}
/* Handle null line to avoid trapping on first command in table */
/*ARGSUSED*/
static
int
donothing(argc,argv)
int argc;
char *argv[];
{
}
/* Translate 'cd' to 'cwd' for convenience */
/*ARGSUSED*/
static
int
doftpcd(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	return sndftpmsg(ftp,"CWD %s\r\n",argv[1]);
}
/* Translate 'mkdir' to 'xmkd' for convenience */
/*ARGSUSED*/
static
int
domkdir(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	return sndftpmsg(ftp,"XMKD %s\r\n",argv[1]);
}
/* Translate 'rmdir' to 'xrmd' for convenience */
/*ARGSUSED*/
static
int
dormdir(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	return sndftpmsg(ftp,"XRMD %s\r\n",argv[1]);
}
/* Handle "type" command from user */
static
int
dotype(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	if(argc < 2){
		switch(ftp->type){
		case IMAGE_TYPE:
			printf("Image\n");
			break;
		case ASCII_TYPE:
			printf("Ascii\n");
			break;
		}
		return 0;
	}
	switch(*argv[1]){
	case 'i':
	case 'b':
		ftp->type = IMAGE_TYPE;
		sndftpmsg(ftp,"TYPE I\r\n");
		break;
	case 'a':
		ftp->type = ASCII_TYPE;
		sndftpmsg(ftp,"TYPE A\r\n");
		break;
	case 'l':
		ftp->type = IMAGE_TYPE;
		sndftpmsg(ftp,"TYPE L %s\r\n",argv[2]);
		break;
	default:
		printf("Invalid type %s\n",argv[1]);
		return 1;
	}
	return 0;
}
/* Start receive transfer. Syntax: get <remote name> [<local name>] */
static
doget(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	if(ftp == NULLFTP){
		printf(notsess);
		return 1;
	}
	return dogetx(argc, argv, ftp);
}


dogetx(argc,argv,ftp)
int argc;
char *argv[];
register struct ftp *ftp;
{
	void ftpdr(),ftpcds();
	char *remotename,*localname;
	char *mode;

	remotename = argv[1];
	if(argc < 3)
		localname = remotename;
	else
		localname = argv[2];

	if(ftp->fp != NULLFILE && ftp->fp != stdout)
		fclose(ftp->fp);
	ftp->fp = NULLFILE;

	if(ftp->type == IMAGE_TYPE) 
		mode = binmode[WRITE_BINARY];
	else
		mode = "w";

	if((ftp->fp = fopen(localname,mode)) == NULLFILE){
		printf(cantwrite,localname);
		return 1;
	}
	ftp->state = RECEIVING_STATE;
	ftpsetup(ftp,ftpdr,NULLVFP,ftpcds);

	/* Generate the command to start the transfer */
	return sndftpmsg(ftp,"RETR %s\r\n",remotename);
}

struct mbuf *mget_args;
char mget_tmpname[258];
char *mget_buf;
FILE *mget_files;

/* Get a collection of files */
static int
domget(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;
	int i;
	long r;
	struct mbuf *bp, *lastbp;

	if((ftp = current->cb.ftp) == NULLFTP){
	  printf(notsess);
	  return 1;
	}
	if (mget_args) {
	  free_p(mget_args);
	  mget_args = NULL;
	}
	tmpnam(mget_tmpname);
	if (! mget_buf)
	  mget_buf = malloc(DIRBUF);

	lastbp = NULL;
	for(i=1;i<argc;i++){
	  bp = qdata(argv[i], strlen(argv[i])+1);
	  if (! bp) {
	    printf(nospace);
	    domgetdone(ftp);
	    return 1;
	  }
	  if (lastbp)
	    lastbp->next = bp;
	  else
	    mget_args = bp;
	  lastbp = bp;
	}		
	return domget1(ftp);
}

domgetdone(ftp)
	register struct ftp *ftp;
{
	if (mget_buf) {
	  free(mget_buf);
	  mget_buf = NULL;
	}
	if (mget_args) {
	  free_p(mget_args);
	  mget_args = NULL;
	}
	if (mget_files) {
	  fclose(mget_files);
	  mget_files = NULL;
	}
	unlink(mget_tmpname);
	ftp->mstate = MST_NONE;
}

domget1(ftp) 
	register struct ftp *ftp;
{
	void ftpdr(),ftpcds();
	int r;
	struct mbuf *bp;

	if (mget_files) {
	  fclose(mget_files);
	  mget_files = NULL;
	}

	unlink(mget_tmpname);

	if (ftp->fp != NULLFILE && ftp->fp != stdout)
	  fclose(ftp->fp);

	if (mget_args) {
	  bp = mget_args;
	  mget_args = bp->next;
	} else {
	  printf("Multiple get finished.\n");
	  domgetdone();
	  return 0;
	}

	if ((ftp->fp = fopen(mget_tmpname, "w")) == NULLFILE) {
	  printf(cantwrite, mget_tmpname);
	  domgetdone(ftp);
	  return 1;
	}
	ftp->state = RECEIVING_STATE;
	ftpsetup(ftp,ftpdr,NULLVFP,ftpcds);
	r = sndftpmsg(ftp,"NLST %s\r\n",bp->data);
	ftp->mstate = MST_NLST;

	free_mbuf(bp);
	return r;
}

domget2(ftp) 
	register struct ftp *ftp;
{
	int r;
	char *argv[2];

	if (! mget_files) {
	  if((mget_files = fopen(mget_tmpname,"r")) == NULLFILE){
	    printf("Can't read %s\n",mget_tmpname);
	    return domget1(ftp);
	  }
	}

	/* The tmp file now contains a list of the remote files, so
	 * go get 'em. Break out if the user signals an abort.
	 */

	if(fgets(mget_buf,DIRBUF,mget_files) == NULLCHAR)
	    return domget1(ftp);

	rip(mget_buf);
	ftp->mstate = MST_RETR;
	argv[1] = mget_buf;
	return dogetx(2, argv, ftp);
}

/* Put a collection of files */
static int
domput(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;
	int i;
	long r;
	struct mbuf *bp, *lastbp;
	char buffer[258];

	if((ftp = current->cb.ftp) == NULLFTP){
	  printf(notsess);
	  return 1;
	}
	if (mget_args) {
	  free_p(mget_args);
	  mget_args = NULL;
	}

	lastbp = NULL;
	for(i=1;i<argc;i++){
	  for (filedir (argv[i], 0, buffer); buffer[0] != '\0';
	       filedir (argv[i], 1, buffer))  {
	    bp = qdata(buffer, strlen(buffer)+1);
	    if (! bp) {
	      printf(nospace);
	      domgetdone(ftp);
	      return 1;
	    }
	    if (lastbp)
	      lastbp->next = bp;
	    else
	      mget_args = bp;
	    lastbp = bp;
	  }
	}
	return domput1(ftp);
}

domput1(ftp) 
	register struct ftp *ftp;
{
	int r;
	struct mbuf *bp;
	char *argv[2];

	if (mget_args) {
	  bp = mget_args;
	  mget_args = bp->next;
	} else {
	  printf("Multiple put finished.\n");
	  domgetdone();
	  return 0;
	}

	ftp->mstate = MST_STOR;
	argv[1] = bp->data;
	r = doputx(2, argv, ftp);
	free_mbuf(bp);
	return r;
}

/* List remote directory. Syntax: dir <remote directory/file> [<local name>] */
static
dolist(argc,argv)
int argc;
char *argv[];
{
	void ftpdr(),ftpcds();
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	if(ftp == NULLFTP){
		printf(notsess);
		return 1;
	}
	if(ftp->fp != NULLFILE && ftp->fp != stdout)
		fclose(ftp->fp);
	ftp->fp = NULLFILE;

	if(argc < 3){
		ftp->fp = stdout;
	} else if((ftp->fp = fopen(argv[2],"w")) == NULLFILE){
		printf(cantwrite,argv[2]);
		return 1;
	}
	ftp->state = RECEIVING_STATE;
	ftpsetup(ftp,ftpdr,NULLVFP,ftpcds);
	/* Generate the command to start the transfer
	 * It's done this way to avoid confusing the 4.2 FTP server
	 * if there's no argument
	 */
	if(argc > 1)
		return sndftpmsg(ftp,"LIST %s\r\n",argv[1]);
	else
		return sndftpmsg(ftp,"LIST\r\n","");
}
/* Abbreviated (name only) list of remote directory.
 * Syntax: ls <remote directory/file> [<local name>]
 */
static
dols(argc,argv)
int argc;
char *argv[];
{
	void ftpdr(),ftpcds();
	register struct ftp *ftp;

	ftp = current->cb.ftp;
	if(ftp == NULLFTP){
		printf(notsess);
		return 1;
	}
	if(ftp->fp != NULLFILE && ftp->fp != stdout)
		fclose(ftp->fp);
	ftp->fp = NULLFILE;

	if(argc < 3){
		ftp->fp = stdout;
	} else if((ftp->fp = fopen(argv[2],"w")) == NULLFILE){
		printf(cantwrite,argv[2]);
		return 1;
	}
	ftp->state = RECEIVING_STATE;
	ftpsetup(ftp,ftpdr,NULLVFP,ftpcds);
	/* Generate the command to start the transfer */
	if(argc > 1)
		return sndftpmsg(ftp,"NLST %s\r\n",argv[1]);
	else
		return sndftpmsg(ftp,"NLST\r\n","");
}
/* Start transmit. Syntax: put <local name> [<remote name>] */
static
doput(argc,argv)
int argc;
char *argv[];
{
	struct ftp *ftp;

	if((ftp = current->cb.ftp) == NULLFTP){
		printf(notsess);
		return 1;
	}

	return doputx (argc, argv, ftp);
}

doputx(argc,argv,ftp)
int argc;
char *argv[];
struct ftp *ftp;
{
	void ftpdt(),ftpcds();
	char *remotename,*localname;
	char *mode;

	localname = argv[1];
	if(argc < 3)
		remotename = localname;
	else
		remotename = argv[2];

	if(ftp->fp != NULLFILE && ftp->fp != stdout)
		fclose(ftp->fp);

	if(ftp->type == IMAGE_TYPE) 
		mode = binmode[READ_BINARY];
	else
		mode = "r";

	if((ftp->fp = fopen(localname,mode)) == NULLFILE){
		printf(cantread,localname);
		return 1;
	}
	ftp->state = SENDING_STATE;
	ftpsetup(ftp,NULLVFP,ftpdt,ftpcds);

	/* Generate the command to start the transfer */
	return sndftpmsg(ftp,"STOR %s\r\n",remotename);
}
/* Abort a GET or PUT operation in progress. Note: this will leave
 * the partial file on the local or remote system
 */
/*ARGSUSED*/
doabort(argc,argv)
int argc;
char *argv[];
{
	register struct ftp *ftp;

	ftp = current->cb.ftp;

	/* Close the local file */
	if(ftp->fp != NULLFILE && ftp->fp != stdout)
		fclose(ftp->fp);
	ftp->fp = NULLFILE;

	switch(ftp->state){
	case SENDING_STATE:
		/* Send a premature EOF.
		 * Unfortunately we can't just reset the connection
		 * since the remote side might end up waiting forever
		 * for us to send something.
		 */
		close_tcp(ftp->data);
		printf("Put aborted\n");
		break;
	case RECEIVING_STATE:
		/* Just exterminate the data channel TCB; this will
		 * generate a RST on the next data packet which will
		 * abort the sender
		 */
		del_tcp(ftp->data);
		ftp->data = NULLTCB;
		printf("Get aborted\n");
		break;
	}
	ftp->state = COMMAND_STATE;
	if (ftp->mstate)
		domgetdone(ftp);
	fflush(stdout);
}
/* create data port, and send PORT message */
static
ftpsetup(ftp,recv,send,state)
struct ftp *ftp;
void (*send)();
void (*recv)();
void (*state)();
{
	struct socket lsocket;
	struct mbuf *bp;

	lsocket.address = ip_addr;
	lsocket.port = lport++;

	/* Compose and send PORT a,a,a,a,p,p message */

	if((bp = alloc_mbuf(35)) == NULLBUF){	/* 5 more than worst case */
		printf(nospace);
		return;
	}
	/* I know, this looks gross, but it works! */
	sprintf(bp->data,"PORT %u,%u,%u,%u,%u,%u\r\n",
		hibyte(hiword(lsocket.address)),
		lobyte(hiword(lsocket.address)),
		hibyte(loword(lsocket.address)),
		lobyte(loword(lsocket.address)),
		hibyte(lsocket.port),
		lobyte(lsocket.port));
	bp->cnt = strlen(bp->data);
	send_tcp(ftp->control,bp);

	/* Post a listen on the data connection */
	ftp->data = open_tcp(&lsocket,NULLSOCK,TCP_PASSIVE,0,
		recv,send,state,0,(char *)ftp);
}
/* FTP Client Control channel Receiver upcall routine */
void
ftpccr(tcb,cnt)
register struct tcb *tcb;
int16 cnt;
{
	struct mbuf *bp;
	struct ftp *ftp;
	void doreply();
	char c;

	if((ftp = (struct ftp *)tcb->user) == NULLFTP){
		/* Unknown connection; kill it */
		close_tcp(tcb);
		return;
	}
	/* Hold output if we're not the current session */
	if(mode != CONV_MODE || current == NULLSESSION || current->cb.ftp != ftp)
		return;

	if(recv_tcp(tcb,&bp,cnt) > 0){
		while(pullup(&bp,&c,1) == 1){
			switch(c){
			case '\r':	/* Strip cr's */
				continue;
			case '\n':	/* Complete line; process it */
				ftp->buf[ftp->cnt] = '\0';
				doreply(ftp);
				ftp->cnt = 0;
				break;
			default:	/* Assemble line */
				if(ftp->cnt != LINELEN-1)
					ftp->buf[ftp->cnt++] = c;
				break;
			}
		}
		fflush(stdout);
	}
}


/* Process replies from the server */
static
void
doreply(ftp)
register struct ftp *ftp;
{
	void echo(), noecho();
	static char crlf[]="\n";

	fwrite(ftp->buf,1,(unsigned)ftp->cnt,stdout);
	fputc('\n', stdout);
	if (ftp->cnt < 3) return;
	ftp->buf[3] = '\0';
	switch(ftp->state){
	case SENDING_STATE:
	case RECEIVING_STATE:
		if (ftp->buf[0] == '5') doabort(0,(char *)0);
		break;
	case STARTUP_STATE:
		if (!strcmp(ftp->buf, "220")){
			ftp->state = USER_STATE;
			printf("Enter user name: ");
			fflush(stdout);
		} else ftp->state = COMMAND_STATE;
		break;
	case COMMAND_STATE:
	case USER_STATE:
		if (!strcmp(ftp->buf, "331")) {
			ftp->state = PASS_STATE;
			noecho();
			printf("Password: ");
			fflush(stdout);
		} else ftp->state = COMMAND_STATE;
		break;
	case PASS_STATE:
		echo();
		ftp->state = COMMAND_STATE;
		break;
	}
}

/* FTP Client Control channel State change upcall routine */
/*ARGSUSED*/
static
void
ftpccs(tcb,old,new)
register struct tcb *tcb;
char old,new;
{
	void ftp_delete();
	struct ftp *ftp;
	char notify = 0;
	extern char *tcpstates[];
	extern char *reasons[];
	extern char *unreach[];
	extern char *exceed[];

	/* Can't add a check for unknown connection here, it would loop
	 * on a close upcall! We're just careful later on.
	 */
	ftp = (struct ftp *)tcb->user;

	if(current != NULLSESSION && current->cb.ftp == ftp)
		notify = 1;

	switch(new){
	case CLOSE_WAIT:
		if(notify)
			printf("%s\n",tcpstates[new]);
		close_tcp(tcb);
		break;
	case CLOSED:	/* heh heh */
		if(notify){
			printf("%s (%s",tcpstates[new],reasons[tcb->reason]);
			if(tcb->reason == NETWORK){
				switch(tcb->type){
				case DEST_UNREACH:
					printf(": %s unreachable",unreach[tcb->code]);
					break;
				case TIME_EXCEED:
					printf(": %s time exceeded",exceed[tcb->code]);
					break;
				}
			}
			printf(")\n");
			cmdmode();
		}
		del_tcp(tcb);
		if(ftp != NULLFTP)
			ftp_delete(ftp);
		break;
	default:
		if(notify)
			printf("%s\n",tcpstates[new]);
		break;
	}
	if(notify)
		fflush(stdout);
}
/* FTP Client Data channel State change upcall handler */
/*ARGSUSED*/
static
void
ftpcds(tcb,old,new)
struct tcb *tcb;
char old,new;
{
	struct ftp *ftp;

	if((ftp = (struct ftp *)tcb->user) == NULLFTP){
		/* Unknown connection, kill it */
		close_tcp(tcb);
		return;
	}
	switch(new){
	case FINWAIT2:
	case TIME_WAIT:
		if(ftp->state == SENDING_STATE){
			/* We've received an ack of our FIN, so
			 * return to command mode
			 */
			ftp->state = COMMAND_STATE;
			if(current != NULLSESSION && current->cb.ftp == ftp){
				printf("Put complete, %lu bytes sent\n",
					tcb->snd.una - tcb->iss - 2);
				fflush(stdout);
			}
			if (ftp->mstate) {
			  tcb->user = NULL;
			  domput1(ftp);
			}
		}
		break;		
	case CLOSE_WAIT:
		close_tcp(tcb);
		if(ftp->state == RECEIVING_STATE){
			/* End of file received on incoming file */
#ifdef	CPM
			if(ftp->type == ASCII_TYPE)
				putc(CTLZ,ftp->fp);
#endif
			if(ftp->fp != stdout)
				fclose(ftp->fp);
			ftp->fp = NULLFILE;
			ftp->state = COMMAND_STATE;
			if(current != NULLSESSION && current->cb.ftp == ftp){
				printf("Get complete, %lu bytes received\n",
					tcb->rcv.nxt - tcb->irs - 2);
				fflush(stdout);
			}
			if (ftp->mstate) {
				tcb->user = NULL;
				domget2(ftp);
			}
		}
		break;
	case CLOSED:
		ftp->data = NULLTCB;
		del_tcp(tcb);
		break;
	}
}
/* Send a message on the control channel */
/*VARARGS*/
static
int
sndftpmsg(ftp,fmt,arg)
struct ftp *ftp;
char *fmt;
char *arg;
{
	struct mbuf *bp;
	int16 len;

	len = strlen(fmt) + strlen(arg) + 10;	/* fudge factor */
	if((bp = alloc_mbuf(len)) == NULLBUF){
		printf(nospace);
		return 1;
	}
	sprintf(bp->data,fmt,arg);
	bp->cnt = strlen(bp->data);
	send_tcp(ftp->control,bp);
	return 0;
}
