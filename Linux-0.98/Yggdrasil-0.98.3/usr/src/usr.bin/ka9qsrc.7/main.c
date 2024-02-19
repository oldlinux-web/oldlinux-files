/* Main network program - provides both client and server functions */


#define HOSTNAMELEN 64
unsigned restricted_dev=1000;
extern char *startup;	/* File to read startup commands from */
#include <stdio.h>
#include "config.h"
#include "global.h"
#include "mbuf.h"
#include "netuser.h"
#include "timer.h"
#include "icmp.h"
#include "iface.h"
#include "ip.h"
#include "tcp.h"
#include "ax25.h"
#include "netrom.h"
#include "ftp.h"
#include "telnet.h"
#include "remote.h"
#include "session.h"
#include "cmdparse.h"

#ifdef	ASY
#include "asy.h"
#include "slip.h"
#endif

#ifdef	NRS
#include "nrs.h"
#endif

#ifdef	SLFP
#include "slfp.h"
#endif

#ifdef UNIX		/* BSD or SYS5 */
#include "unix.h"
#include <memory.h>
#include <string.h>
#include <sys/types.h>
time_t time();
#endif

#ifdef AMIGA
#include "amiga.h"
#endif

#ifdef MAC
#include "mac.h"
#endif

#ifdef MSDOS
#include "asy.h"
#endif

#ifdef	ATARI_ST
#include "st.h"

#ifdef	LATTICE
long _MNEED = 100000L;		/* Reserve RAM for subshell... */
long _32K = 0x8000;		/* For GST Linker (Don't ask me! -- hyc) */
#endif

#ifdef	MWC
long	_stksize = 16384L;	/* Fixed stack size... -- hyc */
#endif
#endif	/* ATARI_ST */

#ifdef	SYS5
#include <signal.h>
int	background = 0;
#endif

#ifdef	TRACE
#include "trace.h"
/* Dummy structure for loopback tracing */
struct interface loopback = { NULLIF, "loopback" };
#endif

extern struct interface *ifaces;
extern char version[];
extern struct mbuf *loopq;
extern FILE *trfp;
extern char trname[];
extern int debug_options;

int mode;
FILE *logfp;
char badhost[] = "Unknown host %s\n";
char hostname[HOSTNAMELEN];	
unsigned nsessions = NSESSIONS;
int32 resolve();
int16 lport = 1001;
char prompt[] = "net> ";
char nospace[] = "No space!!\n";	/* Generic malloc fail message */
#ifdef	SYS5
int io_active = 0;
void daemon();
#endif

#if ((!defined(MSDOS) && !defined(ATARI_ST)) || defined(PC9801))	/* PC/ST uses F-10 key always */
unsigned char escape = 0x1d;	/* default escape character is ^] */
#endif

/* Command lookup and branch table */
int go(),doax25(),cmdmode(),doconnect(),dotelnet(),doexit(),doclose(),
	dohostname(),doreset(),dotcp(),dotrace(),doescape(),dohelp(),
	doroute(),doecho(),dolog(),doip(),dobootp(),dodomain(),dordate(),
	memstat(),doarp(),dosession(),doftp(),dostart(),dostop(),doattach(),
	dosmtp(),doudp(),doparam(),doeol(),dowait(),go_mode(),
	dodump(),dorecord(),doupload(),dokick(),domode(),doshell(),
	dodir(),docd(),doatstat(),doping(),doforward(),doremote(),donetrom(),
	donrstat(), dombox(), mulport(), xserv0(), xserv1();

#ifdef ETHER
int doetherstat();
#endif
#ifdef EAGLE
int doegstat();
#endif
#ifdef HAPN
int dohapnstat();
#endif
#ifdef _FINGER
int dofinger();
#endif

static struct cmds cmds[] = {
	/* The "go" command must be first */
	"",		go_mode,	0, NULLCHAR,	NULLCHAR,
	"!",		doshell,	0, NULLCHAR,	NULLCHAR,
#if	(MAC && APPLETALK)
	"applestat",	doatstat,	0,	NULLCHAR,	NULLCHAR,
#endif
#if	(AX25 || ETHER || APPLETALK)
	"arp",		doarp,		0, NULLCHAR,	NULLCHAR,
#endif
#ifdef	AX25
	"ax25",		doax25,		0, NULLCHAR,	NULLCHAR,
#endif	
	"attach",	doattach,	2,
		"attach <hardware> <hw specific options>", NULLCHAR,
	"bootp",	dobootp,	0, NULLCHAR,	NULLCHAR,
/* This one is out of alpabetical order to allow abbreviation to "c" */
#ifdef	AX25
	"connect",	doconnect,	3,"connect <interface> <callsign> [digipeaters]",
		NULLCHAR,
#endif
	"cd",		docd,		0, NULLCHAR,	NULLCHAR,
	"close",	doclose,	0, NULLCHAR,	NULLCHAR,
	"disconnect",	doclose,	0, NULLCHAR,	NULLCHAR,
	"dir",		dodir,		0, NULLCHAR,	NULLCHAR,
	"domain",	dodomain,	0, NULLCHAR,	NULLCHAR,
#ifdef	EAGLE
	"eaglestat",	doegstat,	0, NULLCHAR,	NULLCHAR,
#endif
	"echo",		doecho,		0, NULLCHAR,	"echo [refuse|accept]",
	"eol",		doeol,		0, NULLCHAR,
		"eol options: unix, standard",
#if	((!defined(MSDOS) && !defined(ATARI_ST)) || defined(PC9801))
	"escape",	doescape,	0, NULLCHAR,	NULLCHAR,   
#endif
#ifdef	PC_EC
	"etherstat",	doetherstat,	0, NULLCHAR,	NULLCHAR,
#endif  PC_EC
	"exit",		doexit,		0, NULLCHAR,	NULLCHAR,
#ifdef _FINGER
	"finger",	dofinger,	0, NULLCHAR, NULLCHAR,
#endif
	"forward",	doforward,	0, NULLCHAR,	NULLCHAR,
	"ftp",		doftp,		2, "ftp <address>",	NULLCHAR,
#ifdef HAPN
	"hapnstat",	dohapnstat,	0, NULLCHAR,	NULLCHAR,
#endif
	"help",		dohelp,		0, NULLCHAR,	NULLCHAR,
	"hostname",	dohostname,	0, NULLCHAR,	NULLCHAR,
	"kick",		dokick,		0, NULLCHAR,	NULLCHAR,
	"log",		dolog,		0, NULLCHAR,	NULLCHAR,
	"ip",		doip,		0, NULLCHAR,	NULLCHAR,
	"memstat",	memstat,	0, NULLCHAR,	NULLCHAR,
#ifdef	AX25
	"mbox",		dombox,		0, NULLCHAR,	NULLCHAR,
	"mode",		domode,		2, "mode <interface>",	NULLCHAR,
#endif
#ifdef	MULPORT
	"mulport",	mulport,	2, "mulport <on|off>",	NULLCHAR,
#endif
#ifdef	NETROM
	"netrom",	donetrom,	0, NULLCHAR,	NULLCHAR,
#ifdef	NRS
	"nrstat",	donrstat,	0, NULLCHAR,	NULLCHAR,
#endif
#endif
	"param",	doparam,	2, "param <interface>", NULLCHAR,
	"ping",		doping,		0, NULLCHAR,	NULLCHAR,
	"pwd",		docd,		0, NULLCHAR,	NULLCHAR,
	"rdate",	dordate,	0, NULLCHAR,	NULLCHAR,
	"record",	dorecord,	0, NULLCHAR,	NULLCHAR,
	"remote",	doremote,	4, "remote <address> <port> <command>",
							NULLCHAR,
	"reset",	doreset,	0, NULLCHAR,	NULLCHAR,
	"route",	doroute,	0, NULLCHAR,	NULLCHAR,
	"session",	dosession,	0, NULLCHAR,	NULLCHAR,
	"shell",	doshell,	0, NULLCHAR,	NULLCHAR,
	"smtp",		dosmtp,		0, NULLCHAR,	NULLCHAR,
#ifdef	SERVERS
	"start",	dostart,	2, "start <servername>",NULLCHAR,
	"stop",		dostop,		2, "stop <servername>",	NULLCHAR,
#endif
	"tcp",		dotcp,		0, NULLCHAR,	NULLCHAR,
	"telnet",	dotelnet,	2, "telnet <address>",	NULLCHAR,
#ifdef	TRACE
	"trace",	dotrace,	0, NULLCHAR,	NULLCHAR,
#endif
	"udp",		doudp,		0, NULLCHAR,	NULLCHAR,
	"upload",	doupload,	0, NULLCHAR,	NULLCHAR,
	"wait",		dowait,		0, NULLCHAR, 	NULLCHAR,
	"?",		dohelp,		0, NULLCHAR,	NULLCHAR,
	NULLCHAR,	NULLFP,		0,
		"Unknown command; type \"?\" for list",   NULLCHAR, 
};

#ifdef	SERVERS
/* "start" and "stop" subcommands */
int dis1(),echo1(),ftp1(),smtp1(),tn1(),rem1();

#ifdef UNIX
int tnix1();
#endif

#ifdef _FINGER
int finger1();
#endif

static struct cmds startcmds[] = {
	"discard",	dis1,		0, NULLCHAR, NULLCHAR,
	"echo",		echo1,		0, NULLCHAR, NULLCHAR,
#ifdef _FINGER
	"finger",	finger1,	0, NULLCHAR, NULLCHAR,
#endif
	"ftp",		ftp1,		0, NULLCHAR, NULLCHAR,
	"smtp",		smtp1,		0, NULLCHAR, NULLCHAR,
	"telnet",	tn1,		0, NULLCHAR, NULLCHAR,
#ifdef	UNIX
	"telunix",	tnix1,		0, NULLCHAR, NULLCHAR,
	"x",		xserv1,		0, NULLCHAR, NULLCHAR,
#endif
	"remote",	rem1,		0, NULLCHAR, NULLCHAR,
	NULLCHAR,	NULLFP,		0,
#ifdef UNIX
#ifdef _FINGER
		"start options: discard, echo, finger, ftp, remote, smtp, telnet, telunix, x", NULLCHAR,
#else
		"start options: discard, echo, ftp, remote, smtp, telnet, telunix, x", NULLCHAR,
#endif
#else /* UNIX */
#ifdef _FINGER
		"start options: discard, echo, finger, ftp, remote, smtp, telnet", NULLCHAR,
#else
		"start options: discard, echo, ftp, remote, smtp, telnet", NULLCHAR,
#endif
#endif /* UNIX */
};
int ftp_stop(),smtp_stop(),echo_stop(),dis_stop(),tn_stop();
int dis0(),echo0(),ftp0(),smtp0(),tn0(),rem0();

#ifdef UNIX
int tnix0();
#endif

#ifdef _FINGER
int finger0();
#endif

static struct cmds stopcmds[] = {
	"discard",	dis0,		0, NULLCHAR, NULLCHAR,
	"echo",		echo0,		0, NULLCHAR, NULLCHAR,
#ifdef _FINGER
	"finger",	finger0,	0, NULLCHAR, NULLCHAR,
#endif
	"ftp",		ftp0,		0, NULLCHAR, NULLCHAR,
	"smtp",		smtp0,		0, NULLCHAR, NULLCHAR,
	"telnet",	tn0,		0, NULLCHAR, NULLCHAR,
#ifdef UNIX
	"telunix",	tnix0,		0, NULLCHAR, NULLCHAR,
	"x",		xserv0,		0, NULLCHAR, NULLCHAR,
#endif
	"remote",	rem0,		0, NULLCHAR, NULLCHAR,
	NULLCHAR,	NULLFP,		0,
#ifdef UNIX
#ifdef _FINGER
		"stop options: discard, echo, finger, ftp, remote, smtp, telnet, telunix", NULLCHAR,
#else
		"stop options: discard, echo, ftp, remote, smtp, telnet, telunix", NULLCHAR,
#endif
#else /* UNIX */
#ifdef _FINGER
		"stop options: discard, echo, finger, ftp, remote, smtp, telnet", NULLCHAR,
#else
		"stop options: discard, echo, ftp, remote, smtp, telnet", NULLCHAR,
#endif
#endif /* UNIX */
};
#endif

void
keep_things_going()
{
	void check_time(), ip_recv();
	struct interface *ifp;
	struct mbuf *bp;

	/* Service the loopback queue */
	if((bp = dequeue(&loopq)) != NULLBUF){
		struct ip ip;
#ifdef	TRACE
		dump(&loopback,IF_TRACE_IN,TRACE_IP,bp);		
#endif
		/* Extract IP header */
		ntohip(&ip,&bp);
		ip_recv(&ip,bp,0);
	}
	/* Service the interfaces */
#ifdef	SYS5
	do {
	io_active = 0;
#endif
	for(ifp = ifaces; ifp != NULLIF; ifp = ifp->next){
		if(ifp->recv != NULLVFP)
			(*ifp->recv)(ifp);
	}
#ifdef	SYS5
	} while(io_active);
#endif

#ifdef	XOBBS
	/* service the W2XO PBBS code */
	axchk();
#endif



	/* Service the clock if it has ticked */
	check_time();

#ifdef	MSDOS
	/* Tell DoubleDos to let the other task run for awhile.
	 * If DoubleDos isn't active, this is a no-op
	 */
#ifndef PLUS
	giveup();
#endif
#else
	/* Wait until interrupt, then do it all over again */
	eihalt();
#endif
}

main(argc,argv)
int argc;
char *argv[];
{
	static char inbuf[BUFSIZ];	/* keep it off the stack */
	int c;
	char *ttybuf,*fgets();
	int16 cnt;
	int ttydriv();
	int cmdparse();
	void check_time(),ip_recv();
	FILE *fp;
#ifdef	FLOW
	extern int ttyflow;
#endif

#ifdef	UNIX
	fileinit(argv[0]);
#endif
#ifdef	SYS5
	if (signal(SIGINT, SIG_IGN) == SIG_IGN) {
		background++;
		daemon();
	} else
		ioinit();
#else
	ioinit();
#endif

#ifdef PLUS
/*
 * set: cursor to block, attributes off, keyboard to ALT mode, 
 *      transmit functions off, use HP fonts
 */
	printf(   "\033&d@"     /* display attributes off         */
                  "\033[11m");  /* use ALT fonts                  */
#endif
#ifdef	MSDOS
#ifndef PLUS
	chktasker();
#endif
#endif
#ifdef	MSDOS
	printf("KA9Q Internet Protocol Package, v%s DS = %x\n",version,
		getds());
#else
#ifdef	SYS5
	if (!background) {
#endif
		printf("KA9Q Internet Protocol Package, v%s\n",version);
#endif
		printf("Copyright 1988 by Phil Karn, KA9Q\n");
#ifdef NETROM
		printf("NET/ROM Support Copyright 1989 by Dan Frank, W9NK\n") ;
#endif
#ifdef	SYS5
	}
#endif
	fflush(stdout);
	sessions = (struct session *)calloc(nsessions,sizeof(struct session));
	if(argc > 1){
		/* Read startup file named on command line */
		fp = fopen(argv[1],"r");
	} else {
		fp = fopen(startup,"r");
	}
	if(fp != NULLFILE){
		while(fgets(inbuf,BUFSIZ,fp) != NULLCHAR){
			cmdparse(cmds,inbuf);
		}
		fclose(fp);
	}		
#ifdef XOBBS
	axinit();
#endif
	cmdmode();

	/* Main commutator loop */
	for(;;){
		/* Process any keyboard input */
#ifdef	SYS5
		while((background == 0) && ((c = kbread()) != -1)){
#else
		while((c = kbread()) != -1){
#endif
#if	(defined(MSDOS) || defined(ATARI_ST))
			/* c == -2 means the command escape key (F10) */
			if(c == -2){
				if(mode != CMD_MODE){
					printf("\n");
					cmdmode();
				}
				continue;
			}
#endif
#ifdef undef
/* #ifdef SYS5 */
			if(c == escape && escape != 0){
				if(mode != CMD_MODE){
					printf("\r\n");
					cmdmode();
				}
				continue;
			}
#endif	 /* SYS5 */

#ifndef	FLOW
			if ((cnt = ttydriv(c, &ttybuf)) == 0)
				continue;
#else
			cnt = ttydriv(c, &ttybuf);
			if (ttyflow && (mode != CMD_MODE))
				go();		/* display pending chars */
			if (cnt == 0)
				continue;
#endif	/* FLOW */
#ifdef undef
/* #if	(!defined(MSDOS) && !defined(ATARI_ST)) */
			if((ttybuf[0] == escape) && (escape != 0)) {
				if(mode != CMD_MODE){
					printf("\r\n");
					cmdmode();
				}
				continue;
			}
#endif
			switch(mode){
			case CMD_MODE:
				(void)cmdparse(cmds,ttybuf);
				fflush(stdout);
				break;
			case CONV_MODE:
#ifdef undef
/* #if	((!defined(MSDOS) && !defined(ATARI_ST)) || defined(PC9801)) */
				if(ttybuf[0] == escape && escape != 0){
					printf("\n");
					cmdmode();
				} else
#endif	/* MSDOS */
					if(current->parse != NULLFP) 
						(*current->parse)(ttybuf,cnt);
				break;
			}
			if(mode == CMD_MODE){
				printf(prompt);
				fflush(stdout);
			}
		}
		keep_things_going();
	}
}
/* Standard commands called from main */

/* Enter command mode */
int
cmdmode()
{
	if(mode != CMD_MODE){
		mode = CMD_MODE;
		cooked();
		flowdefault();
		printf(prompt);
		fflush(stdout);
	}
	return 0;
}
static
doexit()
{
	void iostop();

#if	defined(PLUS)
/*
 * set: cursor to block, attributes off, keyboard to HP mode,
 *      transmit functions off, use HP fonts
 */
	printf(/* "\033*dK"	   cursor to block		*/
	          "\033&d@"	/* display attributes off  	*/
	       /* "\033&k0\\"	   KBD to HP mode		*/
	       /* "\033&s0A"	   transmit functions off	*/
	          "\033[10m");	/* use HP fonts			*/
#endif
	if(logfp != NULLFILE)
		fclose(logfp);
#ifdef	SYS5
	if (!background)
		iostop();
#else
	iostop();
#endif
#ifdef TRACE
	if (trfp != stdout) 
	  fclose(trfp);
#endif
	exit(0);
}
static
dohostname(argc,argv)
int argc;
char *argv[];
{
	char *strncpy();

	if(argc < 2)
		printf("%s\n",hostname);
	else 
		strncpy(hostname,argv[1],HOSTNAMELEN);
	return 0;
}
static
int
dolog(argc,argv)
int argc;
char *argv[];
{
	char *strncpy();

#ifdef	UNIX
	static char logname[256];
#else
	static char logname[15];
#endif
	if(argc < 2){
		if(logfp)
			printf("Logging to %s\n",logname);
		else
			printf("Logging off\n");
		return 0;
	}
	if(logfp){
		fclose(logfp);
		logfp = NULLFILE;
	}
	if(strcmp(argv[1],"stop") != 0){
#ifdef	UNIX
		strncpy(logname,argv[1],sizeof(logname));
#else
		strncpy(logname,argv[1],15);
#endif
		logfp = fopen(logname,"a+");
	}
	return 0;
}
static
int
dohelp()
{
	register struct cmds *cmdp;
	int i,j;

	printf("Main commands:\n");
	for(i=0,cmdp = cmds;cmdp->name != NULL;cmdp++,i++){
		printf("%s",cmdp->name);
		if((i % 4) == 3)
			printf("\n");
		else {
			for(j=strlen(cmdp->name);j < 16; j++)
				putchar(' ');
		}
	}
	if((i % 4) != 0)
		printf("\n");
	return 0;
}

doecho(argc,argv)
int argc;
char *argv[];
{
	extern int refuse_echo;

	if(argc < 2){
		if(refuse_echo)
			printf("Refuse\n");
		else
			printf("Accept\n");
	} else {
		if(argv[1][0] == 'r')
			refuse_echo = 1;
		else if(argv[1][0] == 'a')
			refuse_echo = 0;
		else
			return -1;
	}
	return 0;
}
/* set for unix end of line for remote echo mode telnet */
doeol(argc,argv)
int argc;
char *argv[];
{
	extern int unix_line_mode;

	if(argc < 2){
		if(unix_line_mode)
			printf("Unix\n");
		else
			printf("Standard\n");
	} else {
		if(strcmp(argv[1],"unix") == 0)
			unix_line_mode = 1;
		else if(strcmp(argv[1],"standard") == 0)
			unix_line_mode = 0;
		else {
			return -1;
		}
	}
	return 0;
}
/* Attach an interface
 * Syntax: attach <hw type> <I/O address> <vector> <mode> <label> <bufsize> [<speed>]
 */
doattach(argc,argv)
int argc;
char *argv[];
{
	extern struct cmds attab[];

	return subcmd(attab,argc,argv);
}
/* Manipulate I/O device parameters */
doparam(argc,argv)
int argc;
char *argv[];
{
	register struct interface *ifp;

	for(ifp=ifaces;ifp != NULLIF;ifp = ifp->next){
		if(strcmp(argv[1],ifp->name) == 0)
			break;
	}
	if(ifp == NULLIF){
		printf("Interface \"%s\" unknown\n",argv[1]);
		return 1;
	}
	if(ifp->ioctl == NULLFP){
		printf("Not supported\n");
		return 1;
	}
	/* Pass rest of args to device-specific code */
	return (*ifp->ioctl)(ifp,argc-2,argv+2);
}
/* Log messages of the form
 * Tue Jan 31 00:00:00 1987 44.64.0.7:1003 open FTP
 */
/*VARARGS2*/
log(tcb,fmt,arg1,arg2,arg3,arg4)
struct tcb *tcb;
char *fmt;
int32 arg1,arg2,arg3,arg4;
{
	char *cp;
	long t;
#if	(defined(MSDOS) || defined(ATARI_ST))
	int fd;
#endif

	if(logfp == NULLFILE)
		return;
	time(&t);
	cp = ctime(&t);
	rip(cp);
	if (tcb)
		fprintf(logfp,"%s %s - ",cp,psocket(&tcb->conn.remote));
	else
		fprintf(logfp,"%s - ",cp);
	fprintf(logfp,fmt,arg1,arg2,arg3,arg4);
	fprintf(logfp,"\n");
	fflush(logfp);
#if	(defined(MSDOS) || defined(ATARI_ST))
	/* MS-DOS doesn't really flush files until they're closed */
	fd = fileno(logfp);
	if((fd = dup(fd)) != -1)
		close(fd);
#endif
}
/* Configuration-dependent code */

/* List of supported hardware devices */
int modem_init(),asy_attach(),pc_attach(),at_attach(),nr_attach();

#ifdef	EAGLE
int eg_attach();
#endif
#ifdef	HAPN
int hapn_attach();
#endif
#ifdef	PC_EC
int ec_attach();
#endif
#ifdef LINUX_ETH
int linux_attach();
#endif
#ifdef	PACKET
int pk_attach();
#endif

struct cmds attab[] = {
#ifdef	PC_EC
	/* 3-Com Ethernet interface */
	"3c500", ec_attach, 7, 
	"attach 3c500 <address> <vector> arpa <label> <buffers> <mtu>",
	"Could not attach 3c500",
#endif
#ifdef LINUX_ETH
	/* linux Ethernet interface */
	"linux", linux_attach, 5, 
	"attach linux arpa <label> <mtu> <hex ether address as 1.2.3.4.5.6>",
	"Could not attach linux ethernet",
#endif
#ifdef	ASY
	/* Ordinary PC asynchronous adaptor */
	"asy", asy_attach, 8, 
#ifdef	UNIX
#ifndef	SLFP
	"attach asy 0 <ttyname> slip|ax25|nrs <label> <buffers> <mtu> <speed>",
#else
	"attach asy 0 <ttyname> slip|ax25|nrs|slfp <label> <buffers> <mtu> <speed>",
#endif	/* SLFP */
#else
#ifndef	SLFP
	"attach asy <address> <vector> slip|ax25|nrs <label> <buffers> <mtu> <speed>",
#else
	"attach asy <address> <vector> slip|ax25|nrs|slfp <label> <buffers> <mtu> <speed>",
#endif	/* SLFP */
#endif
	"Could not attach asy",
#endif
#ifdef	PC100
	/* PACCOMM PC-100 8530 HDLC adaptor */
	"pc100", pc_attach, 8, 
	"attach pc100 <address> <vector> ax25 <label> <buffers> <mtu> <speed>",
	"Could not attach pc100",
#endif
#ifdef	EAGLE
	/* EAGLE RS-232C 8530 HDLC adaptor */
	"eagle", eg_attach, 8,
	"attach eagle <address> <vector> ax25 <label> <buffers> <mtu> <speed>",
	"Could not attach eagle",
#endif
#ifdef	HAPN
	/* Hamilton Area Packet Radio (HAPN) 8273 HDLC adaptor */
	"hapn", hapn_attach, 8,
	"attach hapn <address> <vector> ax25 <label> <rx bufsize> <mtu> csma|full",
	"Could not attach hapn",
#endif
#ifdef	APPLETALK
	/* Macintosh AppleTalk */
	"0", at_attach, 7,
	"attach 0 <protocol type> <device> arpa <label> <rx bufsize> <mtu>",
	"Could not attach Appletalk",
#endif
#ifdef NETROM
	/* fake netrom interface */
	"netrom", nr_attach, 1,
	"attach netrom",
	"Could not attach netrom",
#endif
#ifdef	PACKET
	/* FTP Software's packet driver spec */
	"packet", pk_attach, 4,
	"attach packet <int#> <label> <buffers> <mtu>",
	"Could not attach packet driver",
#endif
	NULLCHAR, NULLFP, 0,
	"Unknown device",
	NULLCHAR,
};

/* Protocol tracing function pointers */
#ifdef	TRACE
int ax25_dump(),ether_dump(),ip_dump(),at_dump(),slfp_dump();

int (*tracef[])() = {
#ifdef	AX25
	ax25_dump,
#else
	NULLFP,
#endif

#ifdef	ETHER
	ether_dump,
#else
	NULLFP,
#endif
	ip_dump,

#ifdef	APPLETALK
	at_dump,
#else
	NULLFP,
#endif

#ifdef	SLFP
	slfp_dump,
#else
	NULLFP,
#endif
};
#else
int (*tracef[])() = { NULLFP };	/* No tracing at all */
dump(interface,direction,type,bp)
struct interface *interface;
int direction;
unsigned type;
struct mbuf *bp;
{
}
#endif

#ifdef	ASY

/* Attach a serial interface to the system
 * argv[0]: hardware type, must be "asy"
 * argv[1]: I/O address, e.g., "0x3f8"
 * argv[2]: vector, e.g., "4"
 * argv[3]: mode, may be:
 *	    "slip" (point-to-point SLIP)
 *	    "ax25" (AX.25 frame format in SLIP for raw TNC)
 *	    "nrs" (NET/ROM format serial protocol)
 *	    "slfp" (point-to-point SLFP, as used by the Merit Network and MIT
 * argv[4]: interface label, e.g., "sl0"
 * argv[5]: receiver ring buffer size in bytes
 * argv[6]: maximum transmission unit, bytes
 * argv[7]: interface speed, e.g, "9600"
 * argv[8]: optional ax.25 callsign (NRS only)
 *	    optional command string for modem (SLFP only)
 *          optional modem L.sys style command (SLIP only)
 */
int
asy_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_asy;
	extern struct interface *ifaces;
	int16 dev;
	int mode;
	int asy_init();
	int asy_send();
	int asy_ioctl();
	void doslip();
	int asy_stop();
	int ax_send();
	int ax_output();
	void kiss_recv();
	int kiss_raw();
	int kiss_ioctl();
	int slip_send();
	void slip_recv();
	int slip_raw();

#ifdef	SLFP
	int doslfp();
	int slfp_raw();
	int slfp_send();
	int slfp_recv();
	int slfp_init();
#endif

#ifdef	AX25
	struct ax25_addr addr ;
#endif
	int ax_send(),ax_output(),nrs_raw(),asy_ioctl();
	void nrs_recv();

	if(nasy >= ASY_MAX){
		printf("Too many asynch controllers\n");
		return -1;
	}
	if(strcmp(argv[3],"slip") == 0 || strcmp(argv[3],"cslip") ==0)
		mode = SLIP_MODE;
#ifdef	AX25
	else if(strcmp(argv[3],"ax25") == 0)
		mode = AX25_MODE;
#endif
#ifdef	NRS
	else if(strcmp(argv[3],"nrs") == 0)
		mode = NRS_MODE;
#endif
#ifdef	SLFP
	else if(strcmp(argv[3],"slfp") == 0)
		mode = SLFP_MODE;
#endif
	else {
		printf("Mode %s unknown for interface %s\n",
			argv[3],argv[4]);
		return(-1);
	}

	dev = nasy++;

	/* Create interface structure and fill in details */
	if_asy = (struct interface *)calloc(1,sizeof(struct interface));
	if_asy->name = malloc((unsigned)strlen(argv[4])+1);
	strcpy(if_asy->name,argv[4]);
	if_asy->mtu = atoi(argv[6]);
	if_asy->dev = dev;
	if_asy->recv = doslip;
	if_asy->stop = asy_stop;

	switch(mode){
#ifdef	SLIP
	case SLIP_MODE:
		if_asy->ioctl = asy_ioctl;
		if_asy->send = slip_send;
		if_asy->output = NULLFP;	/* ARP isn't used */
		if_asy->raw = slip_raw;
		if_asy->flags = 0;
		slip[dev].recv = slip_recv;
		if(strcmp(argv[3],"cslip") == 0) {
		  if (! slip[dev].slcomp)
		    slip[dev].slcomp = (char *)slhc_init(16, 16);
		  if (slip[dev].slcomp)
		    slip[dev].vjcomp = 1;
		} else
		  slip[dev].vjcomp = 0;
		break;
#endif
#ifdef	AX25
	case AX25_MODE:  /* Set up a SLIP link to use AX.25 */
		axarp();
		if(mycall.call[0] == '\0'){
			printf("set mycall first\n");
			free(if_asy->name);
			free((char *)if_asy);
			nasy--;
			return -1;
		}
		if_asy->ioctl = kiss_ioctl;
		if_asy->send = ax_send;
		if_asy->output = ax_output;
		if_asy->raw = kiss_raw;
		if(if_asy->hwaddr == NULLCHAR)
			if_asy->hwaddr = malloc(sizeof(mycall));
		memcpy(if_asy->hwaddr,(char *)&mycall,sizeof(mycall));
		slip[dev].recv = kiss_recv;
		break;
#endif
#ifdef	NRS
	case NRS_MODE: /* Set up a net/rom serial interface */
		if(argc < 9){
			/* no call supplied? */
			if(mycall.call[0] == '\0'){
				/* try to use default */
				printf("set mycall first or specify in attach statement\n");
				return -1;
			} else
				addr = mycall;
		} else {
			/* callsign supplied on attach line */
			if(setcall(&addr,argv[8]) == -1){
				printf ("bad callsign on attach line\n");
				free(if_asy->name);
				free((char *)if_asy);
				nasy--;
				return -1;
			}
		}
		if_asy->recv = nrs_recv;
		if_asy->ioctl = asy_ioctl;
		if_asy->send = ax_send;
		if_asy->output = ax_output;
		if_asy->raw = nrs_raw;
		if(if_asy->hwaddr == NULLCHAR)
			if_asy->hwaddr = malloc(sizeof(addr));
		memcpy(if_asy->hwaddr,(char *)&addr,sizeof(addr));
		nrs[dev].iface = if_asy;
		break;
#endif
#ifdef	SLFP
	case SLFP_MODE:
		if_asy->ioctl = asy_ioctl;
		if_asy->send = slfp_send;
		if_asy->recv = doslfp;
		if_asy->output = NULLFP;	/* ARP isn't used */
		if_asy->raw = slfp_raw;
		if_asy->flags = 0;
		slfp[dev].recv = slfp_recv;
		break;
#endif
	}
	if_asy->next = ifaces;
	ifaces = if_asy;
	asy_init(dev,argv[1],argv[2],(unsigned)atoi(argv[5]));
	asy_speed(dev,atoi(argv[7]));
/*
 * optional SLIP modem command?
 */
#if defined(SLIP) && defined(MODEM_CALL)
	if((mode == SLIP_MODE) && (argc > 8)) {
	    restricted_dev=dev;
	    if((modem_init(dev,argc-8,argv+8)) == -1) {
		printf("\nModem command sequence failed.\n");
		asy_stop(if_asy);
		ifaces = if_asy->next;
		free(if_asy->name);
		free((char *)if_asy);
		nasy--;
		restricted_dev=1000;
		return -1;
	    }
	    restricted_dev=1000;
	    return 0;
	}
#endif
#ifdef	SLFP
	if(mode == SLFP_MODE)
	    if(slfp_init(if_asy, argc>7?argv[8]:NULLCHAR) == -1) {
		printf("Request for IP address failed.\n");
		asy_stop(if_asy);
		ifaces = if_asy->next;
		free(if_asy->name);
		free((char *)if_asy);
		nasy--;
		return -1;
	    }
#endif
	return 0;
}
#endif
#ifndef	NETROM
#ifdef	AX25
struct ax25_addr nr_nodebc;
#endif
nr_route(bp)
struct mbuf *bp;
{
	free_p(bp);
}
nr_nodercv(bp)
{
	free_p(bp);
}
#endif


/* Display or set IP interface control flags */
domode(argc,argv)
int argc;
char *argv[];
{
	register struct interface *ifp;

	for(ifp=ifaces;ifp != NULLIF;ifp = ifp->next){
		if(strcmp(argv[1],ifp->name) == 0)
			break;
	}
	if(ifp == NULLIF){
		printf("Interface \"%s\" unknown\n",argv[1]);
		return 1;
	}
	if(argc < 3){
		printf("%s: %s\n",ifp->name,
		 (ifp->flags & CONNECT_MODE) ? "VC mode" : "Datagram mode");
		return 0;
	}
	switch(argv[2][0]){
	case 'v':
	case 'c':
	case 'V':
	case 'C':
		ifp->flags = CONNECT_MODE;
		break;
	case 'd':
	case 'D':
		ifp->flags = DATAGRAM_MODE;
		break;
	default:
		printf("Usage: %s [vc | datagram]\n",argv[0]);
		return 1;
	}
	return 0;
}

#ifdef SERVERS
dostart(argc,argv)
int argc;
char *argv[];
{
	return subcmd(startcmds,argc,argv);
}
dostop(argc,argv)
int argc;
char *argv[];
{
	return subcmd(stopcmds,argc,argv);
}
#endif SERVERS

#ifdef	TRACE
static
int
dotrace(argc,argv)
int argc;
char *argv[];
{
        extern int notraceall;  /* trace only in command mode? */
	struct interface *ifp;

	if(argc < 2){
 		printf("trace mode is %s\n", (notraceall ? "cmdmode" : "allmode"));
	        printf("trace to %s\n",trfp == stdout? "console" : trname);
		showtrace(&loopback);
		for(ifp = ifaces; ifp != NULLIF; ifp = ifp->next)
			showtrace(ifp);
		return 0;
	}
	if(strcmp("to",argv[1]) == 0){
		if(argc >= 3){
			if(trfp != stdout)
				fclose(trfp);
			if(strncmp(argv[2],"con",3) == 0)
				trfp = stdout;
			else {
				if((trfp = fopen(argv[2],"a")) == NULLFILE){
					printf("%s: cannot open\n",argv[2]);
					trfp = stdout;
					return 1;
				}
			}
			strcpy(trname,argv[2]);
		} else {
			printf("trace to %s\n",trfp == stdout? "console" : trname);
		}
		return 0;
	}
	if(strcmp("loopback",argv[1]) == 0)
		ifp = &loopback;
 	else if (strcmp("cmdmode", argv[1]) == 0) {
 		notraceall = 1;
 		return 0;
 	} else if (strcmp("allmode", argv[1]) == 0) {
 		notraceall = 0;
 		return 0;
	} else if (strcmp("telnet", argv[1]) == 0) {
		if (argc >= 3) {
		  if (strcmp(argv[2], "on") == 0)
		    debug_options = 1;
		  else
		    debug_options = 0;
		}
		printf("telnet option trace is %s\n", 
		       debug_options ? "on" : "off");
		return 0;
 	} else
		for(ifp = ifaces; ifp != NULLIF; ifp = ifp->next)
			if(strcmp(ifp->name,argv[1]) == 0)
				break;

	if(ifp == NULLIF){
		printf("Interface %s unknown\n",argv[1]);
		return 1;
	}
	if(argc >= 3)
		ifp->trace = htoi(argv[2]);

	showtrace(ifp);
	return 0;
}
/* Display the trace flags for a particular interface */
static
showtrace(ifp)
register struct interface *ifp;
{
	if(ifp == NULLIF)
		return;
	printf("%s:",ifp->name);
	if(ifp->trace & (IF_TRACE_IN | IF_TRACE_OUT)){
		if(ifp->trace & IF_TRACE_IN)
			printf(" input");
		if(ifp->trace & IF_TRACE_OUT)
			printf(" output");

		if(ifp->trace & IF_TRACE_HEX)
			printf(" (Hex/ASCII dump)");
		else if(ifp->trace & IF_TRACE_ASCII)
			printf(" (ASCII dump)");
		else
			printf(" (headers only)");
		printf("\n");
	} else
		printf(" tracing off\n");
	fflush(stdout);
}
#endif

#if	((!defined(MSDOS) && !defined(ATARI_ST)) || defined(PC9801))
static
int
doescape(argc,argv)
int argc;
char *argv[];
{
	if(argc < 2)
		printf("0x%x\n",escape);
	else 
		escape = *argv[1];
	return 0;
}
#endif	/* MSDOS */
/*ARGSUSED*/
static
doremote(argc,argv)
int argc;
char *argv[];
{
	struct socket fsock,lsock;
	struct mbuf *bp;

	lsock.address = ip_addr;
	fsock.address = resolve(argv[1]);
	lsock.port = fsock.port = atoi(argv[2]);
	bp = alloc_mbuf(1);
	if(strcmp(argv[3],"reset") == 0){
		*bp->data = SYS_RESET;
	} else if(strcmp(argv[3],"exit") == 0){
		*bp->data = SYS_EXIT;
	} else {
		printf("Unknown command %s\n",argv[3]);
		return 1;
	}
	bp->cnt = 1;
	send_udp(&lsock,&fsock,0,0,bp,0,0,0);
	return 0;
}
