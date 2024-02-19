#define VERSION "1.26 08-21-87"
#define PUBDIR "/usr/spool/uucppublic"

/*% cc -M0 -Ox -K -i % -o rz; size rz;
<-xtx-*> cc386 -Ox rz.c -o $B/rz;  size $B/rz
 *
 * rz.c By Chuck Forsberg
 *
 *	cc -O rz.c -o rz		USG (3.0) Unix
 * 	cc -O -DV7  rz.c -o rz		Unix V7, BSD 2.8 - 4.3
 *
 *	ln rz rb;  ln rz rx			For either system
 *
 *	ln rz /usr/bin/rzrmail		For remote mail.  Make this the
 *					login shell. rzrmail then calls
 *					rmail(1) to deliver mail.
 *
 *
 *  Unix is a trademark of Western Electric Company
 *
 * A program for Unix to receive files and commands from computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting XMODEM.
 *  rz uses Unix buffered input to reduce wasted CPU time.
 *
 * Iff the program is invoked by rzCOMMAND, output is piped to 
 * "COMMAND filename"
 *
 *  Some systems (Venix, Coherent, Regulus) may not support tty raw mode
 *  read(2) the same way as Unix. ONEREAD must be defined to force one
 *  character reads for these systems. Added 7-01-84 CAF
 *
 *  Alarm signal handling changed to work with 4.2 BSD 7-15-84 CAF 
 *
 *  BIX added 6-30-87 to support BIX(TM) upload protocol used by the
 *  Byte Information Exchange.
 *
 *  NFGVMIN Updated 2-18-87 CAF for Xenix systems where c_cc[VMIN]
 *  doesn't work properly (even though it compiles without error!),
 *
 *  HOWMANY may be tuned for best performance
 *
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 */
#define LOGFILE "/tmp/rzlog"

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
FILE *popen();

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

/*
 * Max value for HOWMANY is 255.
 *   A larger value reduces system overhead but may evoke kernel bugs.
 *   133 corresponds to an XMODEM/CRC sector
 */
#ifndef HOWMANY
#define HOWMANY 133
#endif

int Zmodem=0;		/* ZMODEM protocol requested */
int Nozmodem = 0;	/* If invoked as "rb" */
unsigned Baudrate;
#include "rbsb.c"	/* most of the system dependent stuff here */

char *substr();
FILE *fout;

/*
 * Routine to calculate the free bytes on the current file system
 *  ~0 means many free bytes (unknown)
 */
long getfree()
{
	return(~0L);	/* many free bytes ... */
}

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103	/* send C not NAK to get crc not checksum */
#define TIMEOUT (-2)
#define RCDO (-3)
#define ERRORMAX 5
#define RETRYMAX 5
#define WCEOT (-10)
#define SECSIZ 128	/* cp/m's Magic Number record size */
#define PATHLEN 257	/* ready for 4.2 bsd ? */
#define KSIZE 1024	/* record size with k option */
#define UNIXFILE 0x8000	/* happens to the the S_IFREG file mask bit for stat */

int Lastrx;
int Crcflg;
int Firstsec;
int Eofseen;		/* indicates cpm eof (^Z) has been received */
int errors;
int Restricted=0;	/* restricted; no /.. or ../ in filenames */
#ifdef ONEREAD
/* Sorry, Regulus and some others don't work right in raw mode! */
int Readnum = 1;	/* Number of bytes to ask for in read() from modem */
#else
int Readnum = HOWMANY;	/* Number of bytes to ask for in read() from modem */
#endif

#define DEFBYTL 2000000000L	/* default rx file size */
long Bytesleft;		/* number of bytes of incoming file left */
long Modtime;		/* Unix style mod time for incoming file */
short Filemode;		/* Unix style mode for incoming file */
char Pathname[PATHLEN];
char *Progname;		/* the name by which we were called */

int Batch=0;
int Wcsmask=0377;
int Topipe=0;
int MakeLCPathname=TRUE;	/* make received pathname lower case */
int Verbose=0;
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Nflag = 0;		/* Don't really transfer files */
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Rxascii=FALSE;	/* receive files in ascii (translate) mode */
int Thisbinary;		/* current file is to be received in bin mode */
int Blklen;		/* record length of received packets */
char secbuf[KSIZE+1];
char linbuf[HOWMANY];
int Lleft=0;		/* number of characters in linbuf */
time_t timep[2];
char Lzmanag;		/* Local file management request */
char zconv;		/* ZMODEM file conversion request */
char zmanag;		/* ZMODEM file management request */
char ztrans;		/* ZMODEM file transport request */
int Zctlesc;		/* Encode control characters */
int Zrwindow = 1400;	/* RX window size (controls garbage count) */

jmp_buf tohere;		/* For the interrupt on RX timeout */

#include "zm.c"

int tryzhdrtype=ZRINIT;	/* Header type to send corresponding to Last rx close */

alrm()
{
	longjmp(tohere, -1);
}

/* called by signal interrupt or terminate to clean things up */
bibi(n)
{
	if (Zmodem)
		zmputs(Attn);
	canit(); mode(0);
	fprintf(stderr, "rz: caught signal %d; exiting", n);
	exit(128+n);
}

main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	char *virgin, **patts;
	char *getenv();
	int exitcode;

	Rxtimeout = 100;
	setbuf(stderr, NULL);
	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rksh")))
		Restricted=TRUE;

	chkinvok(virgin=argv[0]);	/* if called as [-]rzCOMMAND set flag */
	npats = 0;
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while( *++cp) {
				switch(*cp) {
				case '+':
					Lzmanag = ZMAPND; break;
				case '1':
					iofd = 1; break;
				case '7':
					Wcsmask = 0177;
				case 'a':
					Rxascii=TRUE;  break;
				case 'b':
					Rxbinary=TRUE; break;
				case 'c':
					Crcflg=TRUE; break;
				case 'D':
					Nflag = TRUE; break;
				case 'e':
					Zctlesc = 1; break;
				case 'p':
					Lzmanag = ZMPROT;  break;
				case 'q':
					Quiet=TRUE; Verbose=0; break;
				case 't':
					if (--argc < 1) {
						usage();
					}
					Rxtimeout = atoi(*++argv);
					if (Rxtimeout<10 || Rxtimeout>1000)
						usage();
					break;
				case 'w':
					if (--argc < 1) {
						usage();
					}
					Zrwindow = atoi(*++argv);
					break;
				case 'u':
					MakeLCPathname=FALSE; break;
				case 'v':
					++Verbose; break;
				default:
					usage();
				}
			}
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
			}
		}
	}
	if (npats > 1)
		usage();
	if (Verbose) {
		if (freopen(LOGFILE, "a", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(0200);
		}
		setbuf(stderr, NULL);
		fprintf(stderr, "argv[0]=%s Progname=%s\n", virgin, Progname);
	}
	if (fromcu() && !Quiet) {
		if (Verbose == 0)
			Verbose = 2;
	}
	mode(1);
	if (signal(SIGINT, bibi) == SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	}
	else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
	}
	signal(SIGTERM, bibi);
	if (wcreceive(npats, patts)==ERROR) {
		exitcode=0200;
		canit();
	}
	mode(0);
	if (exitcode && !Zmodem)	/* bellow again with all thy might. */
		canit();
	exit(exitcode);
}


usage()
{
	fprintf(stderr,"%s %s for %s by Chuck Forsberg\n",
	  Progname, VERSION, OS);
	fprintf(stderr,"Usage:	rz [-1abeuv]		(ZMODEM Batch)\n");
	fprintf(stderr,"or	rb [-1abuv]		(YMODEM Batch)\n");
	fprintf(stderr,"or	rx [-1abcv] file	(XMODEM or XMODEM-1k)\n");
	fprintf(stderr,"	  -1 For cu(1): Use fd 1 for input\n");
	fprintf(stderr,"	  -a ASCII transfer (strip CR)\n");
	fprintf(stderr,"	  -b Binary transfer for all files\n");
	fprintf(stderr,"	  -c Use 16 bit CRC	(XMODEM)\n");
	fprintf(stderr,"	  -e Ignore control characters	(ZMODEM)\n");
	fprintf(stderr,"	  -v Verbose more v's give more info\n");
	exit(1);
}
/*
 *  Debugging information output interface routine
 */
/* VARARGS1 */
vfile(f, a, b, c)
register char *f;
{
	if (Verbose > 2) {
		fprintf(stderr, f, a, b, c);
		fprintf(stderr, "\n");
	}
}

/*
 * Let's receive something already.
 */

char *rbmsg =
"%s ready. To begin transfer, type \"%s file ...\" to your modem program\r\n";

wcreceive(argc, argp)
char **argp;
{
	register c;

	if (Batch || argc==0) {
		Crcflg=(Wcsmask==0377);
		if ( !Quiet)
			fprintf(stderr, rbmsg, Progname, Nozmodem?"sb":"sz");
		if (c=tryz()) {
			if (c == ZCOMPL)
				return OK;
			if (c == ERROR)
				goto fubar;
			c = rzfiles();
			if (c)
				goto fubar;
		} else {
			for (;;) {
				if (wcrxpn(secbuf)== ERROR)
					goto fubar;
				if (secbuf[0]==0)
					return OK;
				if (procheader(secbuf) == ERROR)
					goto fubar;
				if (wcrx()==ERROR)
					goto fubar;
			}
		}
	} else {
		Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

		procheader(""); strcpy(Pathname, *argp); checkpath(Pathname);
		fprintf(stderr, "\nrz: ready to receive %s\r\n", Pathname);
		if ((fout=fopen(Pathname, "w")) == NULL)
			return ERROR;
		if (wcrx()==ERROR)
			goto fubar;
	}
	return OK;
fubar:
	canit();
	if (Topipe && fout) {
		pclose(fout);  return ERROR;
	}
	if (fout)
		fclose(fout);
	if (Restricted) {
		unlink(Pathname);
		fprintf(stderr, "\r\nrz: %s removed.\r\n", Pathname);
	}
	return ERROR;
}


/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * A null string represents no more files (YMODEM)
 */
wcrxpn(rpn)
char *rpn;	/* receive a pathname */
{
	register c;

#ifdef NFGVMIN
	readline(1);
#else
	purgeline();
#endif

et_tu:
	Firstsec=TRUE;  Eofseen=FALSE;
	sendline(Crcflg?WANTCRC:NAK);
	Lleft=0;	/* Do read next time ... */
	while ((c = wcgetsec(rpn, 100)) != 0) {
		if (c == WCEOT) {
			zperr( "Pathname fetch returned %d", c);
			sendline(ACK);
			Lleft=0;	/* Do read next time ... */
			readline(1);
			goto et_tu;
		}
		return ERROR;
	}
	sendline(ACK);
	return OK;
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

wcrx()
{
	register int sectnum, sectcurr;
	register char sendchar;
	register char *p;
	int cblklen;			/* bytes to dump this block */

	Firstsec=TRUE;sectnum=0; Eofseen=FALSE;
	sendchar=Crcflg?WANTCRC:NAK;

	for (;;) {
		sendline(sendchar);	/* send it now, we're ready! */
		Lleft=0;	/* Do read next time ... */
		sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);
		report(sectcurr);
		if (sectcurr==(sectnum+1 &Wcsmask)) {
			sectnum++;
			cblklen = Bytesleft>Blklen ? Blklen:Bytesleft;
			if (putsec(secbuf, cblklen)==ERROR)
				return ERROR;
			if ((Bytesleft-=cblklen) < 0)
				Bytesleft = 0;
			sendchar=ACK;
		}
		else if (sectcurr==(sectnum&Wcsmask)) {
			zperr( "Received dup Sector");
			sendchar=ACK;
		}
		else if (sectcurr==WCEOT) {
			if (closeit())
				return ERROR;
			sendline(ACK);
			Lleft=0;	/* Do read next time ... */
			return OK;
		}
		else if (sectcurr==ERROR)
			return ERROR;
		else {
			zperr( "Sync Error");
			return ERROR;
		}
	}
}

/*
 * Wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */

wcgetsec(rxbuf, maxtime)
char *rxbuf;
int maxtime;
{
	register checksum, wcj, firstch;
	register unsigned short oldcrc;
	register char *p;
	int sectcurr;

	for (Lastrx=errors=0; errors<RETRYMAX; errors++) {

		if ((firstch=readline(maxtime))==STX) {
			Blklen=KSIZE; goto get2;
		}
		if (firstch==SOH) {
			Blklen=SECSIZ;
get2:
			sectcurr=readline(1);
			if ((sectcurr+(oldcrc=readline(1)))==Wcsmask) {
				oldcrc=checksum=0;
				for (p=rxbuf,wcj=Blklen; --wcj>=0; ) {
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if ((firstch=readline(1)) < 0)
					goto bilge;
				if (Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					if (oldcrc & 0xFFFF)
						zperr( "CRC");
					else {
						Firstsec=FALSE;
						return sectcurr;
					}
				}
				else if (((checksum-firstch)&Wcsmask)==0) {
					Firstsec=FALSE;
					return sectcurr;
				}
				else
					zperr( "Checksum");
			}
			else
				zperr("Sector number garbled");
		}
		/* make sure eot really is eot and not just mixmash */
#ifdef NFGVMIN
		else if (firstch==EOT && readline(1)==TIMEOUT)
			return WCEOT;
#else
		else if (firstch==EOT && Lleft==0)
			return WCEOT;
#endif
		else if (firstch==CAN) {
			if (Lastrx==CAN) {
				zperr( "Sender CANcelled");
				return ERROR;
			} else {
				Lastrx=CAN;
				continue;
			}
		}
		else if (firstch==TIMEOUT) {
			if (Firstsec)
				goto humbug;
bilge:
			zperr( "TIMEOUT");
		}
		else
			zperr( "Got 0%o sector header", firstch);

humbug:
		Lastrx=0;
		while(readline(1)!=TIMEOUT)
			;
		if (Firstsec) {
			sendline(Crcflg?WANTCRC:NAK);
			Lleft=0;	/* Do read next time ... */
		} else {
			maxtime=40; sendline(NAK);
			Lleft=0;	/* Do read next time ... */
		}
	}
	/* try to stop the bubble machine. */
	canit();
	return ERROR;
}

/*
 * This version of readline is reasoably well suited for
 * reading many characters.
 *  (except, currently, for the Regulus version!)
 *
 * timeout is in tenths of seconds
 */
readline(timeout)
int timeout;
{
	register n;
	static char *cdq;	/* pointer for removing chars from linbuf */

	if (--Lleft >= 0) {
		if (Verbose > 8) {
			fprintf(stderr, "%02x ", *cdq&0377);
		}
		return (*cdq++ & Wcsmask);
	}
	n = timeout/10;
	if (n < 2)
		n = 3;
	if (Verbose > 5)
		fprintf(stderr, "Calling read: alarm=%d  Readnum=%d ",
		  n, Readnum);
	if (setjmp(tohere)) {
#ifdef TIOCFLUSH
/*		ioctl(iofd, TIOCFLUSH, 0); */
#endif
		Lleft = 0;
		if (Verbose>1)
			fprintf(stderr, "Readline:TIMEOUT\n");
		return TIMEOUT;
	}
	signal(SIGALRM, alrm); alarm(n);
	Lleft=read(iofd, cdq=linbuf, Readnum);
	alarm(0);
	if (Verbose > 5) {
		fprintf(stderr, "Read returned %d bytes\n", Lleft);
	}
	if (Lleft < 1)
		return TIMEOUT;
	--Lleft;
	if (Verbose > 8) {
		fprintf(stderr, "%02x ", *cdq&0377);
	}
	return (*cdq++ & Wcsmask);
}



/*
 * Purge the modem input queue of all characters
 */
purgeline()
{
	Lleft = 0;
#ifdef USG
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}


/*
 * Process incoming file information header
 */
procheader(name)
char *name;
{
	register char *openmode, *p, **pp;

	/* set default parameters and overrides */
	openmode = "w";
	Thisbinary = (!Rxascii) || Rxbinary;
	if (Lzmanag)
		zmanag = Lzmanag;

	/*
	 *  Process ZMODEM remote file management requests
	 */
	if (!Rxbinary && zconv == ZCNL)	/* Remote ASCII override */
		Thisbinary = 0;
	if (zconv == ZCBIN)	/* Remote Binary override */
		Thisbinary = TRUE;
	else if (zmanag == ZMAPND)
		openmode = "a";

#ifndef BIX
	/* ZMPROT check for existing file */
	if (zmanag == ZMPROT && (fout=fopen(name, "r"))) {
		fclose(fout);  return ERROR;
	}
#endif

	Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

	p = name + 1 + strlen(name);
	if (*p) {	/* file coming from Unix or DOS system */
		sscanf(p, "%ld%lo%o", &Bytesleft, &Modtime, &Filemode);
		if (Filemode & UNIXFILE)
			++Thisbinary;
		if (Verbose) {
			fprintf(stderr,  "Incoming: %s %ld %lo %o\n",
			  name, Bytesleft, Modtime, Filemode);
		}
	}

#ifdef BIX
	if ((fout=fopen("scratchpad", openmode)) == NULL)
		return ERROR;
	return OK;
#else

	else {		/* File coming from CP/M system */
		for (p=name; *p; ++p)		/* change / to _ */
			if ( *p == '/')
				*p = '_';

		if ( *--p == '.')		/* zap trailing period */
			*p = 0;
	}

	if (!Zmodem && MakeLCPathname && !IsAnyLower(name))
		uncaps(name);
	if (Topipe) {
		sprintf(Pathname, "%s %s", Progname+2, name);
		if (Verbose)
			fprintf(stderr,  "Topipe: %s %s\n",
			  Pathname, Thisbinary?"BIN":"ASCII");
		if ((fout=popen(Pathname, "w")) == NULL)
			return ERROR;
	} else {
		strcpy(Pathname, name);
		if (Verbose) {
			fprintf(stderr,  "Receiving %s %s %s\n",
			  name, Thisbinary?"BIN":"ASCII", openmode);
		}
		checkpath(name);
		if (Nflag)
			name = "/dev/null";
		if ((fout=fopen(name, openmode)) == NULL)
			return ERROR;
	}
	return OK;
#endif /* BIX */
}

/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
putsec(buf, n)
char *buf;
register n;
{
	register char *p;

	if (Thisbinary) {
		for (p=buf; --n>=0; )
			putc( *p++, fout);
	}
	else {
		if (Eofseen)
			return OK;
		for (p=buf; --n>=0; ++p ) {
			if ( *p == '\r')
				continue;
			if (*p == CPMEOF) {
				Eofseen=TRUE; return OK;
			}
			putc(*p ,fout);
		}
	}
	return OK;
}

/*
 *  Send a character to modem.  Small is beautiful.
 */
sendline(c)
{
	char d;

	d = c;
	if (Verbose>6)
		fprintf(stderr, "Sendline: %x\n", c);
	write(1, &d, 1);
}

xsendline(c)
{
	sendline(c);
}

flushmo() {}




/* make string s lower case */
uncaps(s)
register char *s;
{
	for ( ; *s; ++s)
		if (isupper(*s))
			*s = tolower(*s);
}
/*
 * IsAnyLower returns TRUE if string s has lower case letters.
 */
IsAnyLower(s)
register char *s;
{
	for ( ; *s; ++s)
		if (islower(*s))
			return TRUE;
	return FALSE;
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *
substr(s, t)
register char *s,*t;
{
	register char *ss,*tt;
	/* search for first char of token */
	for (ss=s; *s; s++)
		if (*s == *t)
			/* compare token with substring */
			for (ss=s,tt=t; ;) {
				if (*tt == 0)
					return s;
				if (*ss++ != *tt++)
					break;
			}
	return NULL;
}

/*
 * Log an error
 */
/*VARARGS1*/
zperr(s,p,u)
char *s, *p, *u;
{
	if (Verbose <= 0)
		return;
	fprintf(stderr, "Retry %d: ", errors);
	fprintf(stderr, s, p, u);
	fprintf(stderr, "\n");
}

/* send cancel string to get the other end to shut up */
canit()
{
	static char canistr[] = {
	 24,24,24,24,24,24,24,24,24,24,8,8,8,8,8,8,8,8,8,8,0
	};

	printf(canistr);
	Lleft=0;	/* Do read next time ... */
	fflush(stdout);
}


/*
 * Return 1 iff stdout and stderr are different devices
 *  indicating this program operating with a modem on a
 *  different line
 */
fromcu()
{
	struct stat a, b;
	fstat(1, &a); fstat(2, &b);
	return (a.st_rdev != b.st_rdev);
}

report(sct)
int sct;
{
	if (Verbose>1)
		fprintf(stderr,"%03d%c",sct,sct%10? ' ' : '\r');
}

/*
 * If called as [-][dir/../]vrzCOMMAND set Verbose to 1
 * If called as [-][dir/../]rzCOMMAND set the pipe flag
 * If called as rb use YMODEM protocol
 */
chkinvok(s)
char *s;
{
	register char *p;

	p = s;
	while (*p == '-')
		s = ++p;
	while (*p)
		if (*p++ == '/')
			s = p;
	if (*s == 'v') {
		Verbose=1; ++s;
	}
	Progname = s;
	if (s[0]=='r' && s[1]=='b')
		Nozmodem = TRUE;
	if (s[2] && s[0]=='r' && s[1]=='b')
		Topipe=TRUE;
	if (s[2] && s[0]=='r' && s[1]=='z')
		Topipe=TRUE;
}

/*
 * Totalitarian Communist pathname processing
 */
checkpath(name)
char *name;
{
	if (Restricted) {
		if (fopen(name, "r") != NULL) {
			canit();
			fprintf(stderr, "\r\nrz: %s exists\n", name);
			bibi(-1);
		}
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();
			fprintf(stderr,"\r\nrz:\tSecurity Violation\r\n");
			bibi(-1);
		}
	}
}

/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
tryz()
{
	register c, n;
	register cmdzack1flg;

	if (Nozmodem)		/* Check for "rb" program name */
		return 0;


	for (n=Zmodem?15:5; --n>=0; ) {
		/* Set buffer length (0) and capability flags */
		stohdr(0L);
#ifdef CANBREAK
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO|CANBRK;
#else
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO;
#endif
		if (Zctlesc)
			Txhdr[ZF0] |= TESCCTL;
		zshhdr(tryzhdrtype, Txhdr);
		if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
			tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */
again:
		switch (zgethdr(Rxhdr, 0)) {
		case ZRQINIT:
			continue;
		case ZEOF:
			continue;
		case TIMEOUT:
			continue;
		case ZFILE:
			zconv = Rxhdr[ZF0];
			zmanag = Rxhdr[ZF1];
			ztrans = Rxhdr[ZF2];
			tryzhdrtype = ZRINIT;
			c = zrdata(secbuf, KSIZE);
			mode(3);
			if (c == GOTCRCW)
				return ZFILE;
			zshhdr(ZNAK, Txhdr);
			goto again;
		case ZSINIT:
			Zctlesc = TESCCTL & Rxhdr[ZF0];
			if (zrdata(Attn, ZATTNLEN) == GOTCRCW) {
				zshhdr(ZACK, Txhdr);
				goto again;
			}
			zshhdr(ZNAK, Txhdr);
			goto again;
		case ZFREECNT:
			stohdr(getfree());
			zshhdr(ZACK, Txhdr);
			goto again;
		case ZCOMMAND:
			cmdzack1flg = Rxhdr[ZF0];
			if (zrdata(secbuf, KSIZE) == GOTCRCW) {
				if (cmdzack1flg & ZCACK1)
					stohdr(0L);
				else
					stohdr((long)sys2(secbuf));
				purgeline();	/* dump impatient questions */
				do {
					zshhdr(ZCOMPL, Txhdr);
				}
				while (++errors<20 && zgethdr(Rxhdr,1) != ZFIN);
				ackbibi();
				if (cmdzack1flg & ZCACK1)
					exec2(secbuf);
				return ZCOMPL;
			}
			zshhdr(ZNAK, Txhdr); goto again;
		case ZCOMPL:
			goto again;
		default:
			continue;
		case ZFIN:
			ackbibi(); return ZCOMPL;
		case ZCAN:
			return ERROR;
		}
	}
	return 0;
}

/*
 * Receive 1 or more files with ZMODEM protocol
 */
rzfiles()
{
	register c;

	for (;;) {
		switch (c = rzfile()) {
		case ZEOF:
		case ZSKIP:
			switch (tryz()) {
			case ZCOMPL:
				return OK;
			default:
				return ERROR;
			case ZFILE:
				break;
			}
			continue;
		default:
			return c;
		case ERROR:
			return ERROR;
		}
	}
}

/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */
rzfile()
{
	register c, n;
	long rxbytes;

	Eofseen=FALSE;
	if (procheader(secbuf) == ERROR) {
		return (tryzhdrtype = ZSKIP);
	}

	n = 20; rxbytes = 0l;

	for (;;) {
		stohdr(rxbytes);
		zshhdr(ZRPOS, Txhdr);
nxthdr:
		switch (c = zgethdr(Rxhdr, 0)) {
		default:
			vfile("rzfile: zgethdr returned %d", c);
			return ERROR;
		case ZNAK:
		case TIMEOUT:
			if ( --n < 0) {
				vfile("rzfile: zgethdr returned %d", c);
				return ERROR;
			}
		case ZFILE:
			zrdata(secbuf, KSIZE);
			continue;
		case ZEOF:
			if (rclhdr(Rxhdr) != rxbytes) {
				/*
				 * Ignore eof if it's at wrong place - force
				 *  a timeout because the eof might have gone
				 *  out before we sent our zrpos.
				 */
				errors = 0;  goto nxthdr;
			}
			if (closeit()) {
				tryzhdrtype = ZFERR;
				vfile("rzfile: closeit returned <> 0");
				return ERROR;
			}
			vfile("rzfile: normal EOF");
			return c;
		case ERROR:	/* Too much garbage in header search error */
			if ( --n < 0) {
				vfile("rzfile: zgethdr returned %d", c);
				return ERROR;
			}
			zmputs(Attn);
			continue;
		case ZDATA:
			if (rclhdr(Rxhdr) != rxbytes) {
				if ( --n < 0) {
					return ERROR;
				}
				zmputs(Attn);  continue;
			}
moredata:
			if (Verbose>1)
				fprintf(stderr, "\r%7ld ZMODEM%s    ",
				  rxbytes, Crc32?" CRC-32":"");
			switch (c = zrdata(secbuf, KSIZE)) {
			case ZCAN:
				vfile("rzfile: zgethdr returned %d", c);
				return ERROR;
			case ERROR:	/* CRC error */
				if ( --n < 0) {
					vfile("rzfile: zgethdr returned %d", c);
					return ERROR;
				}
				zmputs(Attn);
				continue;
			case TIMEOUT:
				if ( --n < 0) {
					vfile("rzfile: zgethdr returned %d", c);
					return ERROR;
				}
				continue;
			case GOTCRCW:
				n = 20;
				putsec(secbuf, Rxcount);
				rxbytes += Rxcount;
				stohdr(rxbytes);
				zshhdr(ZACK, Txhdr);
				sendline(XON);
				goto nxthdr;
			case GOTCRCQ:
				n = 20;
				putsec(secbuf, Rxcount);
				rxbytes += Rxcount;
				stohdr(rxbytes);
				zshhdr(ZACK, Txhdr);
				goto moredata;
			case GOTCRCG:
				n = 20;
				putsec(secbuf, Rxcount);
				rxbytes += Rxcount;
				goto moredata;
			case GOTCRCE:
				n = 20;
				putsec(secbuf, Rxcount);
				rxbytes += Rxcount;
				goto nxthdr;
			}
		}
	}
}

/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */
zmputs(s)
char *s;
{
	register c;

	while (*s) {
		switch (c = *s++) {
		case '\336':
			sleep(1); continue;
		case '\335':
			sendbrk(); continue;
		default:
			sendline(c);
		}
	}
}

/*
 * Close the receive dataset, return OK or ERROR
 */
closeit()
{
	if (Topipe) {
		if (pclose(fout)) {
			return ERROR;
		}
		return OK;
	}
	if (fclose(fout)==ERROR) {
		fprintf(stderr, "file close ERROR\n");
		return ERROR;
	}
	if (Modtime) {
		timep[0] = time(NULL);
		timep[1] = Modtime;
		utime(Pathname, timep);
	}
	if (Filemode)
		chmod(Pathname, (07777 & Filemode));
	return OK;
}

/*
 * Ack a ZFIN packet, let byegones be byegones
 */
ackbibi()
{
	register n;

	vfile("ackbibi:");
	Readnum = 1;
	stohdr(0L);
	for (n=3; --n>=0; ) {
		purgeline();
		zshhdr(ZFIN, Txhdr);
		switch (readline(100)) {
		case 'O':
			readline(1);	/* Discard 2nd 'O' */
			vfile("ackbibi complete");
			return;
		case RCDO:
			return;
		case TIMEOUT:
		default:
			break;
		}
	}
}



/*
 * Local console output simulation
 */
bttyout(c)
{
	if (Verbose || fromcu())
		putc(c, stderr);
}

/*
 * Strip leading ! if present, do shell escape. 
 */
sys2(s)
register char *s;
{
	if (*s == '!')
		++s;
	return system(s);
}
/*
 * Strip leading ! if present, do exec.
 */
exec2(s)
register char *s;
{
	if (*s == '!')
		++s;
	mode(0);
	execl("/bin/sh", "sh", "-c", s);
}
/* End of rz.c */
