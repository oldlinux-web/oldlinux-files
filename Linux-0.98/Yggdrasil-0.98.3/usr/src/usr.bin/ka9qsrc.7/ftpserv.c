/* FTP Server state machine - see RFC 959 */

#define	LINELEN		128	/* Length of command buffer */

#include <stdio.h>
#include <ctype.h>
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "tcp.h"
#include "ftp.h"
#include "telnet.h"
#include "iface.h"
#include "ax25.h"
#include "lapb.h"
#include "finger.h"
#include "session.h"
#include "nr4.h"
#ifdef UNIX
#include "unix.h"
#undef	toupper
#undef	tolower
#include <string.h>
#include <sys/types.h>
time_t  time();
#endif /* UNIX */

/* Command table */
static char *commands[] = {
	"user",
#define	USER_CMD	0
	"acct",
#define	ACCT_CMD	1
	"pass",
#define	PASS_CMD	2
	"type",
#define	TYPE_CMD	3
	"list",
#define	LIST_CMD	4
	"cwd",
#define	CWD_CMD		5
	"dele",
#define	DELE_CMD	6
	"name",
#define	NAME_CMD	7
	"quit",
#define	QUIT_CMD	8
	"retr",
#define	RETR_CMD	9
	"stor",
#define	STOR_CMD	10
	"port",
#define	PORT_CMD	11
	"nlst",
#define	NLST_CMD	12
	"pwd",
#define	PWD_CMD		13
	"xpwd",			/* For compatibility with 4.2BSD */
#define	XPWD_CMD	14
	"mkd ",
#define	MKD_CMD		15
	"xmkd",			/* For compatibility with 4.2BSD */
#define	XMKD_CMD	16
	"xrmd",			/* For compatibility with 4.2BSD */
#define	XRMD_CMD	17
	"rmd ",
#define	RMD_CMD		18
	"stru",
#define	STRU_CMD	19
	"mode",
#define	MODE_CMD	20
	NULLCHAR
};

/* Response messages */
static char banner[] = "220 %s FTP version %s ready at %s\r\n";
static char badcmd[] = "500 Unknown command\r\n";
static char unsupp[] = "500 Unsupported command or option\r\n";
static char givepass[] = "331 Enter PASS command\r\n";
static char logged[] = "230 Logged in\r\n";
static char typeok[] = "200 Type OK\r\n";
static char only8[] = "501 Only logical bytesize 8 supported\r\n";
static char deleok[] = "250 File deleted\r\n";
static char mkdok[] = "200 MKD ok\r\n";
static char delefail[] = "550 Delete failed\r\n";
static char pwdmsg[] = "257 \"%s\" is current directory\r\n";
static char badtype[] = "501 Unknown type \"%s\"\r\n";
static char badport[] = "501 Bad port syntax\r\n";
static char unimp[] = "502 Command not yet implemented\r\n";
static char bye[] = "221 Goodbye!\r\n";
static char nodir[] = "553 Can't read directory \"%s\"\r\n";
static char cantopen[] = "550 Can't read file \"%s\"\r\n";
static char sending[] = "150 Opening data connection for %s %s\r\n";
static char cantmake[] = "553 Can't create \"%s\"\r\n";
static char portok[] = "200 Port command okay\r\n";
static char rxok[] = "226 File received OK\r\n";
static char txok[] = "226 File sent OK\r\n";
static char noperm[] = "550 Permission denied\r\n";
static char noconn[] = "425 Data connection reset\r\n";
static char notlog[] = "530 Please log in with USER and PASS\r\n";
static char okay[] = "200 Ok\r\n";

static struct tcb *ftp_tcb;

/* Start up FTP service */
ftp1(argc,argv)
int argc;
char *argv[];
{
	struct socket lsocket;
	void ftpscr(),ftpscs();

	lsocket.address = ip_addr;
	if(argc < 2)
		lsocket.port = FTP_PORT;
	else
		lsocket.port = atoi(argv[1]);

	ftp_tcb = open_tcp(&lsocket,NULLSOCK,TCP_SERVER,0,ftpscr,NULLVFP,ftpscs,0,(char *)NULL);
}
/* Shut down FTP server */
ftp0()
{
	if(ftp_tcb != NULLTCB)
		close_tcp(ftp_tcb);
}
/* FTP Server Control channel State change upcall handler */
/*ARGSUSED*/
static
void
ftpscs(tcb,old,new)
struct tcb *tcb;
char old,new;
{
	extern char hostname[],version[];
	struct ftp *ftp,*ftp_create();
	void ftp_delete();
	char *inet_ntoa();
	long t;
	char *cp,*cp1;

	switch(new){
/* Setting QUICKSTART piggybacks the server's banner on the SYN/ACK segment;
 * leaving it unset waits for the three-way handshake to complete before
 * sending the banner. Piggybacking unfortunately breaks some old TCPs,
 * so its use is not (yet) recommended.
*/
#ifdef	QUICKSTART
	case SYN_RECEIVED:
#else
	case ESTABLISHED:
#endif
		if((ftp = ftp_create(LINELEN)) == NULLFTP){
			/* No space, kill connection */
			close_tcp(tcb);
			return;
		}
		ftp->control = tcb;		/* Downward link */
		tcb->user = (char *)ftp;	/* Upward link */

		/* Set default data port */
		ftp->port.address = tcb->conn.remote.address;
		ftp->port.port = FTPD_PORT;

		/* Note current directory */
		log(tcb,"open FTP");
		time(&t);
		cp = ctime(&t);
		if((cp1 = index(cp,'\n')) != NULLCHAR)
			*cp1 = '\0';
		tprintf(ftp->control,banner,hostname,version,cp);
		break;		
	case CLOSE_WAIT:
		close_tcp(tcb);
		break;
	case CLOSED:
		log(tcb,"close FTP");
		if((ftp = (struct ftp *)tcb->user) != NULLFTP ||
		   !tcpval(ftp->control)) {   /* control session existing ? */
			if (ftp != NULLFTP)
				ftp_delete(ftp);
		}
		/* Check if server is being shut down */
		if(tcb == ftp_tcb)
			ftp_tcb = NULLTCB;
		del_tcp(tcb);
		break;
	}
}

/* FTP Server Control channel Receiver upcall handler */
/*ARGSUSED*/
static
void
ftpscr(tcb,cnt)
struct tcb *tcb;
int16 cnt;
{
	register struct ftp *ftp;
	char c;
	struct mbuf *bp;
	void ftpcommand();

	if((ftp = (struct ftp *)tcb->user) == NULLFTP){
		/* Unknown connection, just kill it */
		close_tcp(tcb);
		return;
	}
	switch(ftp->state){
	case COMMAND_STATE:
		/* Assemble an input line in the session buffer. Return if incomplete */
		recv_tcp(tcb,&bp,0);
		while(pullup(&bp,&c,1) == 1){
			switch(c){
			case '\r':	/* Strip cr's */
				continue;
			case '\n':	/* Complete line; process it */
				ftp->buf[ftp->cnt] = '\0';
				ftpcommand(ftp);
				ftp->cnt = 0;
				break;
			default:	/* Assemble line */
				if(ftp->cnt != LINELEN-1)
					ftp->buf[ftp->cnt++] = c;
				break;
			}
		}
		/* else no linefeed present yet to terminate command */
		break;
	case SENDING_STATE:
	case RECEIVING_STATE:
		/* Leave commands pending on receive queue until
		 * present command is done
		 */
		break;
	}
}

/* FTP server data channel connection state change upcall handler */
void
ftpsds(tcb,old,new)
struct tcb *tcb;
char old,new;
{
	register struct ftp *ftp;

	if((ftp = (struct ftp *)tcb->user) == NULLFTP){
		/* Unknown connection. Kill it */
		del_tcp(tcb);
	} else if((old == FINWAIT1 || old == CLOSING) && ftp->state == SENDING_STATE){
		/* We've received an ack of our FIN while sending; we're done */
		ftp->state = COMMAND_STATE;
		tprintf(ftp->control,txok);
		/* Kick command parser if something is waiting */
		if(ftp->control->rcvcnt != 0)
			ftpscr(ftp->control,ftp->control->rcvcnt);
	} else if(ftp->state == RECEIVING_STATE && new == CLOSE_WAIT){
		/* FIN received on incoming file */
#ifdef	CPM
		if(ftp->type == ASCII_TYPE)
			putc(CTLZ,ftp->fp);
#endif
		close_tcp(tcb);
		if(ftp->fp != stdout)
			fclose(ftp->fp);
		ftp->fp = NULLFILE;
		ftp->state = COMMAND_STATE;
		tprintf(ftp->control,rxok);
		/* Kick command parser if something is waiting */
		if(ftp->control->rcvcnt != 0)
			ftpscr(ftp->control,ftp->control->rcvcnt);
	} else if(new == CLOSED){
		if(tcb->reason != NORMAL){
			/* Data connection was reset, complain about it */
			tprintf(ftp->control,noconn);
			/* And clean up */
			if(ftp->fp != NULLFILE && ftp->fp != stdout)
				fclose(ftp->fp);
			ftp->fp = NULLFILE;
			ftp->state = COMMAND_STATE;
			/* Kick command parser if something is waiting */
			if(ftp->control->rcvcnt != 0)
				ftpscr(ftp->control,ftp->control->rcvcnt);
		}
		/* Clear only if another transfer hasn't already started */
		if(ftp->data == tcb)
			ftp->data = NULLTCB;
		del_tcp(tcb);
	}
}

/* Parse and execute ftp commands */
static
void
ftpcommand(ftp)
register struct ftp *ftp;
{
	void ftpdr(),ftpdt(),ftpsds();
	char *cmd,*arg,*cp,**cmdp,*file;
	char *pathname();
	char *mode;
	struct socket dport;
	int i;

#ifndef	CPM
	FILE *dir();
#endif

	cmd = ftp->buf;
	if(ftp->cnt == 0){
		/* Can't be a legal FTP command */
		tprintf(ftp->control,badcmd);
		return;
	}	
	cmd = ftp->buf;

#ifdef	UNIX
	/* Translate first word to lower case */
	for(cp = cmd;*cp != ' ' && *cp != '\0';cp++)
		*cp = tolower(*cp);
#else
	/* Translate entire buffer to lower case */
	for(cp = cmd;*cp != '\0';cp++)
		*cp = tolower(*cp);
#endif
	/* Find command in table; if not present, return syntax error */
	for(cmdp = commands;*cmdp != NULLCHAR;cmdp++)
		if(strncmp(*cmdp,cmd,strlen(*cmdp)) == 0)
			break;
	if(*cmdp == NULLCHAR){
		tprintf(ftp->control,badcmd);
		return;
	}
	/* Allow only USER, PASS and QUIT before logging in */
	if(ftp->cd == NULLCHAR || ftp->path[0] == NULLCHAR){
		switch(cmdp-commands){
		case USER_CMD:
		case PASS_CMD:
		case QUIT_CMD:
			break;
		default:
			tprintf(ftp->control,notlog);
			return;
		}
	}
	arg = &cmd[strlen(*cmdp)];
	while(*arg == ' ')
		arg++;

	/* Execute specific command */
	switch(cmdp-commands){
	case USER_CMD:
		if((ftp->username = malloc((unsigned)strlen(arg)+1)) == NULLCHAR){
			close_tcp(ftp->control);
			break;
		}
		strcpy(ftp->username,arg);
		tprintf(ftp->control,givepass);
 		/* erase all user info from possible previous session */
 		for(i = 0; i < MAXPATH; i++){
 			if(ftp->path[i] != NULLCHAR){
 				free(ftp->path[i]);
 				ftp->path[i] = NULLCHAR;
 			}
 			ftp->perms[i] = 0;
 		}
 		if(ftp->cd != NULLCHAR){
 			free(ftp->cd);
 			ftp->cd = NULLCHAR;
 		}
		break;
	case TYPE_CMD:
		switch(arg[0]){
		case 'A':
		case 'a':	/* Ascii */
			ftp->type = ASCII_TYPE;
			tprintf(ftp->control,typeok);
			break;
		case 'l':
		case 'L':
			while(*arg != ' ' && *arg != '\0')
				arg++;
			if(*arg == '\0' || *++arg != '8'){
				tprintf(ftp->control,only8);
				break;
			}	/* Note fall-thru */
		case 'B':
		case 'b':	/* Binary */
		case 'I':
		case 'i':	/* Image */
			ftp->type = IMAGE_TYPE;
			tprintf(ftp->control,typeok);
			break;
		default:	/* Invalid */
			tprintf(ftp->control,badtype,arg);
			break;
		}
		break;
	case QUIT_CMD:
		tprintf(ftp->control,bye);
		close_tcp(ftp->control);
		break;
	case RETR_CMD:
		/* Disk operation; return ACK now */
		tcp_output(ftp->control);
		file = pathname(ftp->cd,arg);
		if(ftp->type == IMAGE_TYPE)
			mode = binmode[READ_BINARY];
		else
			mode = "r";
		if(!permcheck(ftp,RETR_CMD,file)){
		 	tprintf(ftp->control,noperm);
		} else if((ftp->fp = fopen(file,mode)) == NULLFILE){
			tprintf(ftp->control,cantopen,file);
		} else {
			log(ftp->control,"RETR %s",file);
			dport.address = ip_addr;
			dport.port = FTPD_PORT;
			ftp->state = SENDING_STATE;
			tprintf(ftp->control,sending,"RETR",arg);
			ftp->data = open_tcp(&dport,&ftp->port,TCP_ACTIVE,
			 0,NULLVFP,ftpdt,ftpsds,ftp->control->tos,(char *)ftp);
		}
		free(file);
		break;
	case STOR_CMD:
		/* Disk operation; return ACK now */
		tcp_output(ftp->control);
		file = pathname(ftp->cd,arg);
		if(ftp->type == IMAGE_TYPE)
			mode = binmode[WRITE_BINARY];
		else
			mode = "w";
		if(!permcheck(ftp,STOR_CMD,file)){
		 	tprintf(ftp->control,noperm);
			free(file);
		 	break;
		} else if((ftp->fp = fopen(file,mode)) == NULLFILE){
			tprintf(ftp->control,cantmake,file);
		} else {
			log(ftp->control,"STOR %s",file);
			dport.address = ip_addr;
			dport.port = FTPD_PORT;
			ftp->state = RECEIVING_STATE;
			tprintf(ftp->control,sending,"STOR",arg);
			ftp->data = open_tcp(&dport,&ftp->port,TCP_ACTIVE,
			 0,ftpdr,NULLVFP,ftpsds,ftp->control->tos,(char *)ftp);
		}
		free(file);
		break;
	case PORT_CMD:
		if(pport(&ftp->port,arg) == -1){
			tprintf(ftp->control,badport);
		} else {
			tprintf(ftp->control,portok);
		}
		break;
#ifndef CPM
	case LIST_CMD:
		/* Disk operation; return ACK now */
		tcp_output(ftp->control);

		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,RETR_CMD,file)){
		 	tprintf(ftp->control,noperm);
		} else if((ftp->fp = dir(file,1)) == NULLFILE){
			tprintf(ftp->control,nodir,file);
		} else {
			dport.address = ip_addr;
			dport.port = FTPD_PORT;
			ftp->state = SENDING_STATE;
			tprintf(ftp->control,sending,"LIST",file);
			ftp->data = open_tcp(&dport,&ftp->port,TCP_ACTIVE,
			 0,NULLVFP,ftpdt,ftpsds,ftp->control->tos,(char *)ftp);
		}
		free(file);
		break;
	case NLST_CMD:
		/* Disk operation; return ACK now */
		tcp_output(ftp->control);

		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,RETR_CMD,file)){
		 	tprintf(ftp->control,noperm);
		} else if((ftp->fp = dir(file,0)) == NULLFILE){
			tprintf(ftp->control,nodir,file);
		} else {
			dport.address = ip_addr;
			dport.port = FTPD_PORT;
			ftp->state = SENDING_STATE;
			tprintf(ftp->control,sending,"NLST",file);
			ftp->data = open_tcp(&dport,&ftp->port,TCP_ACTIVE,
			 0,NULLVFP,ftpdt,ftpsds,ftp->control->tos,(char *)ftp);
		}
		free(file);
		break;
	case CWD_CMD:
		tcp_output(ftp->control);	/* Disk operation; return ACK now */

		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,RETR_CMD,file)){
		 	tprintf(ftp->control,noperm);
			free(file);
#if  	(defined(MSDOS) || defined(ATARI_ST))
		/* Don'tcha just LOVE %%$#@!! MS-DOS? */
		} else if(strcmp(file,"\\") == 0 || access(file,0) == 0){
#else
		} else if(access(file,0) == 0){	/* See if it exists */
#endif
			/* Succeeded, record in control block */
			free(ftp->cd);
			ftp->cd = file;
			tprintf(ftp->control,pwdmsg,file);
		} else {
			/* Failed, don't change anything */
			tprintf(ftp->control,nodir,file);
			free(file);
		}
		break;
	case XPWD_CMD:
	case PWD_CMD:
		tprintf(ftp->control,pwdmsg,ftp->cd);
		break;
#else
	case LIST_CMD:
	case NLST_CMD:
	case CWD_CMD:
	case XPWD_CMD:
	case PWD_CMD:
#endif
	case ACCT_CMD:		
		tprintf(ftp->control,unimp);
		break;
	case DELE_CMD:
		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,DELE_CMD,file)){
		 	tprintf(ftp->control,noperm);
		} else if(unlink(file) == 0){
			tprintf(ftp->control,deleok);
		} else {
			tprintf(ftp->control,delefail);
		}
		free(file);
		break;
	case PASS_CMD:
		tcp_output(ftp->control);	/* Send the ack now */
		ftplogin(ftp,arg);			
		break;
#ifndef	CPM
	case XMKD_CMD:
	case MKD_CMD:
		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,MKD_CMD,file)){
			tprintf(ftp->control,noperm);
		} else if(xmkdir(file,0777) == 0){
			tprintf(ftp->control,mkdok);
		} else {
			tprintf(ftp->control,cantmake);
		}
		free(file);
		break;
	case XRMD_CMD:
	case RMD_CMD:
		file = pathname(ftp->cd,arg);
		if(!permcheck(ftp,RMD_CMD,file)){
		 	tprintf(ftp->control,noperm);
		} else if(rmdir(file) == 0){
			tprintf(ftp->control,deleok);
		} else {
			tprintf(ftp->control,delefail);
		}
		free(file);
		break;
	case STRU_CMD:
		if(tolower(arg[0]) != 'f')
			tprintf(ftp->control,unsupp);
		else
			tprintf(ftp->control,okay);
		break;
	case MODE_CMD:
		if(tolower(arg[0]) != 's')
			tprintf(ftp->control,unsupp);
		else
			tprintf(ftp->control,okay);
		break;
	}
#endif
}
static
int
pport(sock,arg)
struct socket *sock;
char *arg;
{
	int32 n;
	int atoi(),i;

	n = 0;
	for(i=0;i<4;i++){
		n = atoi(arg) + (n << 8);
		if((arg = index(arg,',')) == NULLCHAR)
			return -1;
		arg++;
	}
	sock->address = n;
	n = atoi(arg);
	if((arg = index(arg,',')) == NULLCHAR)
		return -1;
	arg++;
	n = atoi(arg) + (n << 8);
	sock->port = n;
	return 0;
}
/* Attempt to log in the user whose name is in ftp->username and password
 * in pass
 */
static
ftplogin(ftp,pass)
struct ftp *ftp;
char *pass;
{
	char buf[80],*cp,*cp1,*getnenv();
	FILE *fp;
	int anony = 0;
	int i;

	if((fp = fopen(userfile,"r")) == NULLFILE){
		/* Userfile doesn't exist */
		tprintf(ftp->control,noperm);
		return;
	}

	while(fgets(buf,sizeof(buf),fp),!feof(fp)){
		if(buf[0] == '#')
			continue;	/* Comment */
		if((cp = index(buf,' ')) == NULLCHAR)
			/* Bogus entry */
			continue;
		*cp++ = '\0';		/* Now points to password */
		if(strcmp(ftp->username,buf) == 0)
			break;		/* Found user name */
	}
	if(feof(fp)){
		/* User name not found in file */
		fclose(fp);
		tprintf(ftp->control,noperm);
		return;
	}
	fclose(fp);
	/* Look for space after password field in file */
	if((cp1 = index(cp,' ')) == NULLCHAR){
		/* Invalid file entry */
		tprintf(ftp->control,noperm);
		return;
	}
	*cp1++ = '\0';	/* Now points to first path field */
	if(strcmp(cp,"*") == 0)
		anony = 1;	/* User ID is password-free */
	if(!anony && strcmp(cp,pass) != 0){
		/* Password required, but wrong one given */
		tprintf(ftp->control,noperm);
		return;
	}
	for(i = 0; i< MAXPATH; i++){
	  if((cp = index(cp1,' ')) == NULLCHAR){
	    /* Permission field missing, assume end of line */
	    break;
	  }
	  *cp++ = '\0';	/* now points to permission field */
	  ftp->path[i] = malloc((unsigned)strlen(cp1)+1);
	  strcpy(ftp->path[i],cp1);
	  /* set the permission bits */
	  ftp->perms[i] = atoi(cp);
	  if ((cp1 = index(cp,' ')) == NULLCHAR){
	    /* no next path field, so assume end of line */
	    break;
	  }
	  *cp1++ = '\0'; /* cp1 now points to the next path field */
	}

	/* Set up current directory and LAST specified path prefix */
	for (i= MAXPATH - 1; i>=0; i--)
	  if (ftp->perms[i])
	    break;

	ftp->cd = malloc((unsigned)strlen(ftp->path[i])+1);
	strcpy(ftp->cd,ftp->path[i]);
	
	tprintf(ftp->control,logged);
	if(!anony)
		log(ftp->control,"%s logged in",ftp->username);
	else
		log(ftp->control,"%s logged in, ID %s",ftp->username,pass);
}		

#if	(defined(MSDOS) || defined(ATARI_ST))
/* Illegal characters in a DOS filename */
char badchars[] = "\"[]|<>+=;,";
#endif

/* Return 1 if the file operation is allowed, 0 otherwise */
permcheck(ftp,op,file)
struct ftp *ftp;
int op;
char *file;
{
#if	(defined(MSDOS) || defined(ATARI_ST))
	char *cp;
#endif
	int i;

	if(file == NULLCHAR || ftp->path[0] == NULLCHAR)
		return 0;	/* Probably hasn't logged in yet */
#if	(defined(MSDOS) || defined(ATARI_ST))
	/* Check for characters illegal in MS-DOS file names */
	for(cp = badchars;*cp != '\0';cp++){
		if(index(file,*cp) != NULLCHAR)
			return 0;	
	}
#endif
#if	(!AMIGA && !MAC)
	/* The target file must be under the user's allowed search path */
 	for(i = 0; i < MAXPATH; i++)
 		if(ftp->path[i] != NULLCHAR &&
 		   strncmp(file,ftp->path[i],strlen(ftp->path[i])) == 0)
 			break;

	if(i == MAXPATH)
		return 0;
#endif

	switch(op){
	case RETR_CMD:
		/* User must have permission to read files */
		if(ftp->perms[i] & FTP_READ)
			return 1;
		return 0;
	case DELE_CMD:
	case RMD_CMD:
		/* User must have permission to (over)write files */
		if(ftp->perms[i] & FTP_WRITE)
			return 1;
		return 0;
	case STOR_CMD:
	case MKD_CMD:
		/* User must have permission to (over)write files, or permission
		 * to create them if the file doesn't already exist
		 */
		if(ftp->perms[i] & FTP_WRITE)
			return 1;
		if(access(file,2) == -1 && (ftp->perms[i] & FTP_CREATE))
			return 1;
		return 0;
	}
	return 0;	/* "can't happen" -- keep lint happy */
}
