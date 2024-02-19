#define VERSION "sz 1.36 08-31-87"
#define PUBDIR "/usr/spool/uucppublic"

/*% cc -M0 -Ox -K -i -DNFGVMIN -DREADCHECK sz.c -lx -o sz; size sz
 *
 * sz.c By Chuck Forsberg
 *
 *	cc -O sz.c -o sz		USG (SYS III/V) Unix
 *	cc -O -DSVR2 sz.c -o sz		Sys V Release 2 with non-blocking input
 *					Define to allow reverse channel checking
 *	cc -O -DV7  sz.c -o sz		Unix Version 7, 2.8 - 4.3 BSD
 *
 *	cc -O -K -i -DNFGVMIN -DREADCHECK sz.c -lx -o sz	Xenix
 *
 *	ln sz sb			**** All versions ****
 *	ln sz sx			**** All versions ****
 *
 *
 *  ******* Some systems (Venix, Coherent, Regulus) do not *******
 *  ******* support tty raw mode read(2) identically to    *******
 *  ******* Unix. ONEREAD must be defined to force one     *******
 *  ******* character reads for these systems.		   *******
 *
 * A program for Unix to send files and commands to computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting Y/XMODEM.
 *
 *  Sz uses buffered I/O to greatly reduce CPU time compared to UMODEM.
 *
 *  USG UNIX (3.0) ioctl conventions courtesy Jeff Martin
 *
 * 1.34 implements tx backchannel garbage count and ZCRCW after ZRPOS
 * in accordance with the 7-31-87 ZMODEM Protocol Description
 */


char *substr(), *getenv();

#define LOGFILE "/tmp/szlog"

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

#define PATHLEN 256
#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

#define HOWMANY 2
int Zmodem=0;		/* ZMODEM protocol requested */
unsigned Baudrate;
unsigned Txwindow;	/* Control the size of the transmitted window */
unsigned Txwspac;	/* Spacing between zcrcq requests */
unsigned Txwcnt;	/* Counter used to space ack requests */
long Lrxpos;		/* Receiver's last reported offset */
int Fromcu = 0;		/* Were called from cu or yam */
int errors;
#include "rbsb.c"	/* most of the system dependent stuff here */

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 */
#ifdef READCHECK
char Myattn[] = { 0 };
#else
#ifdef USG
char Myattn[] = { 03, 0336, 0 };
#else
char Myattn[] = { 0 };
#endif
#endif

FILE *in;

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
#define WANTG 0107	/* Send G not NAK to get nonstop batch xmsn */
#define TIMEOUT (-2)
#define RCDO (-3)
#define RETRYMAX 10
#define SECSIZ 128	/* cp/m's Magic Number record size */
#define KSIZE 1024

char Lastrx;
char Crcflg;
int Wcsmask=0377;
int Verbose=0;
int Modem2=0;		/* XMODEM Protocol - don't send pathnames */
int Restricted=0;	/* restricted; no /.. or ../ in filenames */
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Ascii=0;		/* Add CR's for brain damaged programs */
int Fullname=0;		/* transmit full pathname */
int Unlinkafter=0;	/* Unlink file after it is sent */
int Dottoslash=0;	/* Change foo.bar.baz to foo/bar/baz */
int firstsec;
int errcnt=0;		/* number of files unreadable */
int blklen=SECSIZ;		/* length of transmitted records */
int Optiong;		/* Let it rip no wait for sector ACK's */
int Noeofseen;
int Totsecs;		/* total number of sectors this file */
char txbuf[KSIZE];
int Filcnt=0;		/* count of number of files opened */
int Lfseen=0;
unsigned Rxbuflen = 16384;	/* Receiver's max buffer length */
int Tframlen = 0;	/* Override for tx frame length */
int blkopt=0;		/* Override value for zmodem blklen */
int Rxflags = 0;
long bytcnt;
int Wantfcs32 = TRUE;	/* want to send 32 bit FCS */
char Lzconv;	/* Local ZMODEM file conversion request */
char Lzmanag;	/* Local ZMODEM file management request */
int Lskipnocor;
char Lztrans;
char zconv;		/* ZMODEM file conversion request */
char zmanag;		/* ZMODEM file management request */
char ztrans;		/* ZMODEM file transport request */
int Command;		/* Send a command, then exit. */
char *Cmdstr;		/* Pointer to the command string */
int Cmdtries = 11;
int Cmdack1;		/* Rx ACKs command, then do it */
int Exitcode;
int Testattn;		/* Force receiver to send Attn, etc with qbf. */
char *qbf="The quick brown fox jumped over the lazy dog's back 1234567890\r\n";
long Lastread;		/* Beginning offset of last buffer read */
int Lastn;		/* Count of last buffer read or -1 */
int Dontread;		/* Don't read the buffer, it's still there */
long Lastsync;		/* Last offset to which we got a ZRPOS */
int Beenhereb4;		/* How many times we've been ZRPOS'd same place */

jmp_buf tohere;		/* For the interrupt on RX timeout */
jmp_buf intrjmp;	/* For the interrupt on RX CAN */

/* called by signal interrupt or terminate to clean things up */
bibi(n)
{
	canit(); fflush(stdout); mode(0);
	fprintf(stderr, "sz: caught signal %d; exiting\n", n);
	if (n == SIGQUIT)
		abort();
	exit(128+n);
}
/* Called when Zmodem gets an interrupt (^X) */
onintr()
{
	signal(SIGINT, SIG_IGN);
	longjmp(intrjmp, -1);
}


#define sendline(c) putchar(c & Wcsmask)

#define xsendline(c) putchar(c)

flushmo()
{
	fflush(stdout);
}

int Zctlesc;	/* Encode control characters */
int Nozmodem = 0;	/* If invoked as "sb" */
char *Progname = "sz";
int Zrwindow = 1400;	/* RX window size (controls garbage count) */
#include "zm.c"


main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	int agcnt; char **agcv;
	char **patts;
	static char xXbuf[BUFSIZ];

	if ((cp = getenv("ZNULLS")) && *cp)
		Znulls = atoi(cp);
	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rksh")))
		Restricted=TRUE;
	chkinvok(argv[0]);

	Rxtimeout = 600;
	npats=0;
	if (argc<2)
		usage();
	setbuf(stdout, xXbuf);		
	while (--argc) {
		cp = *++argv;
		if (*cp++ == '-' && *cp) {
			while ( *cp) {
				switch(*cp++) {
				case '+':
					Lzmanag = ZMAPND; break;
				case '1':
					iofd = 1; break;
#ifdef CSTOPB
				case '2':
					Twostop = TRUE; break;
#endif
				case '7':
					Wcsmask=0177; break;
				case 'a':
					Lzconv = ZCNL;
					Ascii = TRUE; break;
				case 'b':
					Lzconv = ZCBIN; break;
				case 'C':
					if (--argc < 1) {
						usage();
					}
					Cmdtries = atoi(*++argv);
					break;
				case 'i':
					Cmdack1 = ZCACK1;
					/* **** FALL THROUGH TO **** */
				case 'c':
					if (--argc != 1) {
						usage();
					}
					Command = TRUE;
					Cmdstr = *++argv;
					break;
				case 'd':
					++Dottoslash;
					/* **** FALL THROUGH TO **** */
				case 'f':
					Fullname=TRUE; break;
				case 'e':
					Zctlesc = 1; break;
				case 'k':
					blklen=KSIZE; break;
				case 'L':
					if (--argc < 1) {
						usage();
					}
					blkopt = atoi(*++argv);
					if (blkopt<24 || blkopt>1024)
						usage();
					break;
				case 'l':
					if (--argc < 1) {
						usage();
					}
					Tframlen = atoi(*++argv);
					if (Tframlen<32 || Tframlen>1024)
						usage();
					break;
				case 'N':
					Lzmanag = ZMNEWL;  break;
				case 'n':
					Lzmanag = ZMNEW;  break;
				case 'o':
					Wantfcs32 = FALSE; break;
				case 'p':
					Lzmanag = ZMPROT;  break;
				case 'r':
					Lzconv = ZCRESUM;
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
				case 'T':
					Testattn = TRUE; break;
				case 'u':
					++Unlinkafter; break;
				case 'v':
					++Verbose; break;
				case 'w':
					if (--argc < 1) {
						usage();
					}
					Txwindow = atoi(*++argv);
					if (Txwindow < 256)
						Txwindow = 256;
					Txwindow = (Txwindow/64) * 64;
					Txwspac = Txwindow/4;
					if (blkopt > Txwspac
					 || (!blkopt && Txwspac < 1024))
						blkopt = Txwspac;
					break;
				case 'X':
					++Modem2; break;
				case 'Y':
					Lskipnocor = TRUE;
					/* **** FALLL THROUGH TO **** */
				case 'y':
					Lzmanag = ZMCLOB; break;
				default:
					usage();
				}
			}
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
				if ( !strcmp(*patts, "-"))
					iofd = 1;
			}
		}
	}
	if (npats < 1 && !Command) 
		usage();
	if (Verbose) {
		if (freopen(LOGFILE, "a", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(0200);
		}
		setbuf(stderr, NULL);
	}
	if ((Fromcu=from_cu()) && !Quiet) {
		if (Verbose == 0)
			Verbose = 2;
	}

	mode(1);

	if (signal(SIGINT, bibi) == SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	} else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
	}
	if ( !Fromcu)
		signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, bibi);

	if ( !Modem2) {
		if (!Nozmodem) {
			printf("rz\r");  fflush(stdout);
		}
		if (!Command && !Quiet && Verbose != 1) {
			fprintf(stderr, "%s: %d file%s requested:\r\n",
			 Progname, npats, npats>1?"s":"");
			for ( agcnt=npats, agcv=patts; --agcnt>=0; ) {
				fprintf(stderr, "%s ", *agcv++);
			}
			fprintf(stderr, "\r\n");
			printf("\r\n\bSending in Batch Mode\r\n");
		}
		if (!Nozmodem) {
			stohdr(0L);
			if (Command)
				Txhdr[ZF0] = ZCOMMAND;
			zshhdr(ZRQINIT, Txhdr);
		}
	}
	fflush(stdout);

	if (Command) {
		if (getzrxinit()) {
			Exitcode=0200; canit();
		}
		else if (zsendcmd(Cmdstr, 1+strlen(Cmdstr))) {
			Exitcode=0200; canit();
		}
	} else if (wcsend(npats, patts)==ERROR) {
		Exitcode=0200;
		canit();
	}
	fflush(stdout);
	mode(0);
	exit((errcnt != 0) | Exitcode);
	/*NOTREACHED*/
}

wcsend(argc, argp)
char *argp[];
{
	register n;

	Crcflg=FALSE;
	firstsec=TRUE;
	bytcnt = -1;
	for (n=0; n<argc; ++n) {
		Totsecs = 0;
		if (wcs(argp[n])==ERROR)
			return ERROR;
	}
	Totsecs = 0;
	if (Filcnt==0) {	/* bitch if we couldn't open ANY files */
		if (1) {
			Command = TRUE;
			Cmdstr = "echo \"sz: Can't open any requested files\"";
			if (getnak()) {
				Exitcode=0200; canit();
			}
			if (!Zmodem)
				canit();
			else if (zsendcmd(Cmdstr, 1+strlen(Cmdstr))) {
				Exitcode=0200; canit();
			}
			Exitcode = 1; return OK;
		}
		canit();
		fprintf(stderr,"\r\nCan't open any requested files.\r\n");
		return ERROR;
	}
	if (Zmodem)
		saybibi();
	else if ( !Modem2)
		wctxpn("");
	return OK;
}

wcs(oname)
char *oname;
{
	register c;
	register char *p;
	struct stat f;
	char name[PATHLEN];

	strcpy(name, oname);

	if (Restricted) {
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();
			fprintf(stderr,"\r\nsz:\tSecurity Violation\r\n");
			return ERROR;
		}
	}

	if ( !strcmp(oname, "-")) {
		if ((p = getenv("ONAME")) && *p)
			strcpy(name, p);
		else
			sprintf(name, "s%d.sz", getpid());
		in = stdin;
	}
	else if ((in=fopen(oname, "r"))==NULL) {
		++errcnt;
		return OK;	/* pass over it, there may be others */
	}
	++Noeofseen;  Lastread = 0;  Lastn = -1; Dontread = FALSE;
	/* Check for directory or block special files */
	fstat(fileno(in), &f);
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK) {
		fclose(in);
		return OK;
	}

	++Filcnt;
	switch (wctxpn(name)) {
	case ERROR:
		return ERROR;
	case ZSKIP:
		return OK;
	}
	if (!Zmodem && wctx(f.st_size)==ERROR)
		return ERROR;
	if (Unlinkafter)
		unlink(oname);
	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
wctxpn(name)
char *name;
{
	register char *p, *q;
	char name2[PATHLEN];
	struct stat f;

	if (Modem2) {
		if ((in!=stdin) && *name && fstat(fileno(in), &f)!= -1) {
			fprintf(stderr, "Sending %s, %ld blocks: ",
			  name, f.st_size>>7);
		}
		fprintf(stderr, "Give your local XMODEM receive command now.\r\n");
		return OK;
	}
	zperr("Awaiting pathname nak for %s", *name?name:"<END>");
	if ( !Zmodem)
		if (getnak())
			return ERROR;

	q = (char *) 0;
	if (Dottoslash) {		/* change . to . */
		for (p=name; *p; ++p) {
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q=p) = '/';
		}
		if (q && strlen(++q) > 8) {	/* If name>8 chars */
			q += 8;			/*   make it .ext */
			strcpy(name2, q);	/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}

	for (p=name, q=txbuf ; *p; )
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p=q;
	while (q < (txbuf + KSIZE))
		*q++ = 0;
	if (!Ascii && (in!=stdin) && *name && fstat(fileno(in), &f)!= -1)
		sprintf(p, "%lu %lo %o", f.st_size, f.st_mtime, f.st_mode);
	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen=KSIZE;
	else {		/* A little goodie for IMP/KMD */
		if (Zmodem)
			blklen = SECSIZ;
		txbuf[127] = (f.st_size + 127) >>7;
		txbuf[126] = (f.st_size + 127) >>15;
	}
	if (Zmodem)
		return zsendfile(txbuf, 1+strlen(p)+(p-txbuf));
	if (wcputsec(txbuf, 0, SECSIZ)==ERROR)
		return ERROR;
	return OK;
}

getnak()
{
	register firstch;

	Lastrx = 0;
	for (;;) {
		switch (firstch = readock(800,1)) {
		case ZPAD:
			if (getzrxinit())
				return ERROR;
			Ascii = 0;
			return FALSE;
		case TIMEOUT:
			zperr("Timeout on pathname");
			return TRUE;
		case WANTG:
#ifdef USG
			mode(2);	/* Set cbreak, XON/XOFF, etc. */
#endif
			Optiong = TRUE;
			blklen=KSIZE;
		case WANTCRC:
			Crcflg = TRUE;
		case NAK:
			return FALSE;
		case CAN:
			if ((firstch = readock(20,1)) == CAN && Lastrx == CAN)
				return TRUE;
		default:
			break;
		}
		Lastrx = firstch;
	}
}


wctx(flen)
long flen;
{
	register int thisblklen;
	register int sectnum, attempts, firstch;
	long charssent;

	charssent = 0;  firstsec=TRUE;  thisblklen = blklen;
	vfile("wctx:file length=%ld", flen);

	while ((firstch=readock(Rxtimeout, 2))!=NAK && firstch != WANTCRC
	  && firstch != WANTG && firstch!=TIMEOUT && firstch!=CAN)
		;
	if (firstch==CAN) {
		zperr("Receiver CANcelled");
		return ERROR;
	}
	if (firstch==WANTCRC)
		Crcflg=TRUE;
	if (firstch==WANTG)
		Crcflg=TRUE;
	sectnum=0;
	for (;;) {
		if (flen <= (charssent + 896L))
			thisblklen = 128;
		if ( !filbuf(txbuf, thisblklen))
			break;
		if (wcputsec(txbuf, ++sectnum, thisblklen)==ERROR)
			return ERROR;
		charssent += thisblklen;
	}
	fclose(in);
	attempts=0;
	do {
		purgeline();
		sendline(EOT);
		fflush(stdout);
		++attempts;
	}
		while ((firstch=(readock(Rxtimeout, 1)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX) {
		zperr("No ACK on EOT");
		return ERROR;
	}
	else
		return OK;
}

wcputsec(buf, sectnum, cseclen)
char *buf;
int sectnum;
int cseclen;	/* data length of this sector to send */
{
	register checksum, wcj;
	register char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch=0;	/* part of logic to detect CAN CAN */

	if (Verbose>2)
		fprintf(stderr, "Sector %3d %2dk\n", Totsecs, Totsecs/8 );
	else if (Verbose>1)
		fprintf(stderr, "\rSector %3d %2dk ", Totsecs, Totsecs/8 );
	for (attempts=0; attempts <= RETRYMAX; attempts++) {
		Lastrx= firstch;
		sendline(cseclen==KSIZE?STX:SOH);
		sendline(sectnum);
		sendline(-sectnum -1);
		oldcrc=checksum=0;
		for (wcj=cseclen,cp=buf; --wcj>=0; ) {
			sendline(*cp);
			oldcrc=updcrc((0377& *cp), oldcrc);
			checksum += *cp++;
		}
		if (Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline((int)oldcrc>>8);
			sendline((int)oldcrc);
		}
		else
			sendline(checksum);

		if (Optiong) {
			firstsec = FALSE; return OK;
		}
		firstch = readock(Rxtimeout, (Noeofseen&&sectnum) ? 2:1);
gotnak:
		switch (firstch) {
		case CAN:
			if(Lastrx == CAN) {
cancan:
				zperr("Cancelled");  return ERROR;
			}
			break;
		case TIMEOUT:
			zperr("Timeout on sector ACK"); continue;
		case WANTCRC:
			if (firstsec)
				Crcflg = TRUE;
		case NAK:
			zperr("NAK on sector"); continue;
		case ACK: 
			firstsec=FALSE;
			Totsecs += (cseclen>>7);
			return OK;
		case ERROR:
			zperr("Got burst for sector ACK"); break;
		default:
			zperr("Got %02x for sector ACK", firstch); break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = readock(Rxtimeout, 2)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	zperr("Retry Count Exceeded");
	return ERROR;
}

/* fill buf with count chars padding with ^Z for CPM */
filbuf(buf, count)
register char *buf;
{
	register c, m;

	if ( !Ascii) {
		m = read(fileno(in), buf, count);
		if (m <= 0)
			return 0;
		while (m < count)
			buf[m++] = 032;
		return count;
	}
	m=count;
	if (Lfseen) {
		*buf++ = 012; --m; Lfseen = 0;
	}
	while ((c=getc(in))!=EOF) {
		if (c == 012) {
			*buf++ = 015;
			if (--m == 0) {
				Lfseen = TRUE; break;
			}
		}
		*buf++ =c;
		if (--m == 0)
			break;
	}
	if (m==count)
		return 0;
	else
		while (--m>=0)
			*buf++ = CPMEOF;
	return count;
}
/* fill buf with count chars */
zfilbuf(buf, count)
register char *buf;
{
	register c, m;

	m=count;
	while ((c=getc(in))!=EOF) {
		*buf++ =c;
		if (--m == 0)
			break;
	}
	return (count - m);
}

/* VARARGS1 */
vfile(f, a, b, c)
register char *f;
{
	if (Verbose > 2) {
		fprintf(stderr, f, a, b, c);
		fprintf(stderr, "\n");
	}
}


alrm()
{
	longjmp(tohere, -1);
}


/*
 * readock(timeout, count) reads character(s) from file descriptor 0
 *  (1 <= count <= 3)
 * it attempts to read count characters. If it gets more than one,
 * it is an error unless all are CAN
 * (otherwise, only normal response is ACK, CAN, or C)
 *  Only looks for one if Optiong, which signifies cbreak, not raw input
 *
 * timeout is in tenths of seconds
 */
readock(timeout, count)
{
	register int c;
	static char byt[5];

	if (Optiong)
		count = 1;	/* Special hack for cbreak */

	fflush(stdout);
	if (setjmp(tohere)) {
		zperr("TIMEOUT");
		return TIMEOUT;
	}
	c = timeout/10;
	if (c<2)
		c=2;
	if (Verbose>5) {
		fprintf(stderr, "Timeout=%d Calling alarm(%d) ", timeout, c);
		byt[1] = 0;
	}
	signal(SIGALRM, alrm); alarm(c);
#ifdef ONEREAD
	c=read(iofd, byt, 1);		/* regulus raw read is unique */
#else
	c=read(iofd, byt, count);
#endif
	alarm(0);
	if (Verbose>5)
		fprintf(stderr, "ret cnt=%d %x %x\n", c, byt[0], byt[1]);
	if (c<1)
		return TIMEOUT;
	if (c==1)
		return (byt[0]&0377);
	else
		while (c)
			if (byt[--c] != CAN)
				return ERROR;
	return CAN;
}
readline(n)
{
	return (readock(n, 1));
}


purgeline()
{
#ifdef USG
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}


/* send cancel string to get the other end to shut up */
canit()
{
	static char canistr[] = {
	 24,24,24,24,24,24,24,24,24,24,8,8,8,8,8,8,8,8,8,8,0
	};

	printf(canistr);
	fflush(stdout);
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

/*
 * return 1 iff stdout and stderr are different devices
 *  indicating this program operating with a modem on a
 *  different line
 */
from_cu()
{
	struct stat a, b;
	fstat(1, &a); fstat(2, &b);
	return (a.st_rdev != b.st_rdev);
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

char *babble[] = {
	"Send file(s) with ZMODEM/YMODEM/XMODEM Protocol",
	"	(Y) = Option applies to YMODEM only",
	"	(Z) = Option applies to ZMODEM only",
	"Usage:	sz [-12+abdefkLlNnquvwYy] [-] file ...",
	"	sz [-12Ceqv] -c COMMAND",
	"	sb [-12adfkquv] [-] file ...",
	"	sx [-12akquv] [-] file",
	"	1 Use stdout for modem input",
#ifdef CSTOPB
	"	2 Use 2 stop bits",
#endif
	"	+ Append to existing destination file (Z)",
	"	a (ASCII) change NL to CR/LF",
	"	b Binary file transfer override",
	"	c send COMMAND (Z)",
	"	d Change '.' to '/' in pathnames (Y/Z)",
	"	e Escape all control characters (Z)",
	"	f send Full pathname (Y/Z)",
	"	i send COMMAND, ack Immediately (Z)",
	"	k Send 1024 byte packets (Y)",
	"	L N Limit subpacket length to N bytes (Z)",
	"	l N Limit frame length to N bytes (l>=L) (Z)",
	"	n send file if source newer (Z)",
	"	N send file if source newer or longer (Z)",
	"	o Use 16 bit CRC instead of 32 bit CRC (Z)",
	"	p Protect existing destination file (Z)",
	"	r Resume/Recover interrupted file transfer (Z)",
	"	q Quiet (no progress reports)",
	"	u Unlink file after transmission",
	"	v Verbose - provide debugging information",
	"	w N Window is N bytes (Z)",
	"	Y Yes, overwrite existing file, skip if not present at rx (Z)",
	"	y Yes, overwrite existing file (Z)",
	"- as pathname sends standard input as sPID.sz or environment ONAME",
	""
};

usage()
{
	char **pp;

	for (pp=babble; **pp; ++pp)
		fprintf(stderr, "%s\n", *pp);
	fprintf(stderr, "%s for %s by Chuck Forsberg  ", VERSION, OS);
	exit(1);
}

/*
 * Get the receiver's init parameters
 */
getzrxinit()
{
	register n;
	struct stat f;

	for (n=10; --n>=0; ) {
		
		switch (zgethdr(Rxhdr, 1)) {
		case ZCHALLENGE:	/* Echo receiver's challenge numbr */
			stohdr(Rxpos);
			zshhdr(ZACK, Txhdr);
			continue;
		case ZCOMMAND:		/* They didn't see out ZRQINIT */
			stohdr(0L);
			zshhdr(ZRQINIT, Txhdr);
			continue;
		case ZRINIT:
			Rxflags = 0377 & Rxhdr[ZF0];
			Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
			Zctlesc |= Rxflags & TESCCTL;
			Rxbuflen = (0377 & Rxhdr[ZP0])+((0377 & Rxhdr[ZP1])<<8);
			vfile("Rxbuflen=%d Tframlen=%d", Rxbuflen, Tframlen);
			if ( !Fromcu)
				signal(SIGINT, SIG_IGN);
#ifdef USG
			mode(2);	/* Set cbreak, XON/XOFF, etc. */
#endif
#ifndef READCHECK
#ifndef USG
			/* Use 1024 byte frames if no sample/interrupt */
			if (Rxbuflen < 32 || Rxbuflen > 1024) {
				Rxbuflen = 1024;
				vfile("Rxbuflen=%d", Rxbuflen);
			}
#endif
#endif
			/* Override to force shorter frame length */
			if (Rxbuflen && (Rxbuflen>Tframlen) && (Tframlen>=32))
				Rxbuflen = Tframlen;
			if ( !Rxbuflen && (Tframlen>=32) && (Tframlen<=1024))
				Rxbuflen = Tframlen;
			vfile("Rxbuflen=%d", Rxbuflen);

			/* If using a pipe for testing set lower buf len */
			fstat(iofd, &f);
			if ((f.st_mode & S_IFMT) != S_IFCHR
			  && (Rxbuflen == 0 || Rxbuflen > 4096))
				Rxbuflen = 4096;
			/*
			 * If input is not a regular file, force ACK's each 1024
			 *  (A smarter strategey could be used here ...)
			 */
			fstat(fileno(in), &f);
			if (((f.st_mode & S_IFMT) != S_IFREG)
			  && (Rxbuflen == 0 || Rxbuflen > 1024))
				Rxbuflen = 1024;
			vfile("Rxbuflen=%d", Rxbuflen);

			return (sendzsinit());
		case ZCAN:
		case TIMEOUT:
			return ERROR;
		case ZRQINIT:
			if (Rxhdr[ZF0] == ZCOMMAND)
				continue;
		default:
			zshhdr(ZNAK, Txhdr);
			continue;
		}
	}
	return ERROR;
}

/* Send send-init information */
sendzsinit()
{
	register c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return OK;
	errors = 0;
	for (;;) {
		stohdr(0L);
		if (Zctlesc) {
			Txhdr[ZF0] |= TESCCTL; zshhdr(ZSINIT, Txhdr);
		}
		else
			zsbhdr(ZSINIT, Txhdr);
		zsdata(Myattn, 1+strlen(Myattn), ZCRCW);
		c = zgethdr(Rxhdr, 1);
		switch (c) {
		case ZCAN:
			return ERROR;
		case ZACK:
			return OK;
		default:
			if (++errors > 19)
				return ERROR;
			continue;
		}
	}
}

/* Send file name and related info */
zsendfile(buf, blen)
char *buf;
{
	register c;

	for (;;) {
		Txhdr[ZF0] = Lzconv;	/* file conversion request */
		Txhdr[ZF1] = Lzmanag;	/* file management request */
		if (Lskipnocor)
			Txhdr[ZF1] |= ZMSKNOLOC;
		Txhdr[ZF2] = Lztrans;	/* file transport request */
		Txhdr[ZF3] = 0;
		zsbhdr(ZFILE, Txhdr);
		zsdata(buf, blen, ZCRCW);
again:
		c = zgethdr(Rxhdr, 1);
		switch (c) {
		case ZRINIT:
			while ((c = readline(50)) > 0)
				if (c == ZPAD) {
					goto again;
				}
			/* **** FALL THRU TO **** */
		default:
			continue;
		case ZCAN:
		case TIMEOUT:
		case ZABORT:
		case ZFIN:
			return ERROR;
		case ZSKIP:
			fclose(in); return c;
		case ZRPOS:
			/*
			 * Suppress zcrcw request otherwise triggered by
			 * lastyunc==bytcnt
			 */
			Lastsync = (bytcnt = Txpos = Rxpos) -1;
			fseek(in, Rxpos, 0);
			Dontread = FALSE;
			return zsendfdata();
		}
	}
}

/* Send the data in the file */
zsendfdata()
{
	register c, e, n;
	register newcnt;
	register long tcount = 0;
	int junkcount;		/* Counts garbage chars received by TX */
	static int tleft = 6;	/* Counter for test mode */

	if (Baudrate > 300)
		blklen = 256;
	if (Baudrate > 1200)
		blklen = 512;
	if (Baudrate > 2400)
		blklen = KSIZE;
	if (Rxbuflen && blklen>Rxbuflen)
		blklen = Rxbuflen;
	if (blkopt && blklen > blkopt)
		blklen = blkopt;
	vfile("Rxbuflen=%d blklen=%d", Rxbuflen, blklen);
	vfile("Txwindow = %u Txwspac = %d", Txwindow, Txwspac);
	Lrxpos = 0;
	junkcount = 0;
	Beenhereb4 = FALSE;
somemore:
	if (setjmp(intrjmp)) {
waitack:
		junkcount = 0;
		c = getinsync(0);
gotack:
		switch (c) {
		default:
		case ZCAN:
			fclose(in);
			return ERROR;
		case ZSKIP:
			fclose(in);
			return c;
		case ZACK:
		case ZRPOS:
			break;
		case ZRINIT:
			return OK;
		}
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		while (rdchk(iofd)) {
#ifdef SVR2
			switch (checked)
#else
			switch (readline(1))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync(1);
				goto gotack;
			case XOFF:		/* Wait a while for an XON */
			case XOFF|0200:
				readline(100);
			}
		}
#endif
	}

	if ( !Fromcu)
		signal(SIGINT, onintr);
	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr(Txpos);
	zsbhdr(ZDATA, Txhdr);

	/*
	 * Special testing mode.  This should force receiver to Attn,ZRPOS
	 *  many times.  Each time the signal should be caught, causing the
	 *  file to be started over from the beginning.
	 */
	if (Testattn) {
		if ( --tleft)
			while (tcount < 20000) {
				printf(qbf); fflush(stdout);
				tcount += strlen(qbf);
#ifdef READCHECK
				while (rdchk(iofd)) {
#ifdef SVR2
					switch (checked)
#else
					switch (readline(1))
#endif
					{
					case CAN:
					case ZPAD:
#ifdef TCFLSH
						ioctl(iofd, TCFLSH, 1);
#endif
						goto waitack;
					case XOFF:	/* Wait for XON */
					case XOFF|0200:
						readline(100);
					}
				}
#endif
			}
		signal(SIGINT, SIG_IGN); canit();
		sleep(3); purgeline(); mode(0);
		printf("\nsz: Tcount = %ld\n", tcount);
		if (tleft) {
			printf("ERROR: Interrupts Not Caught\n");
			exit(1);
		}
		exit(0);
	}

	do {
		if (Dontread) {
			n = Lastn;
		} else {
			n = zfilbuf(txbuf, blklen);
			Lastread = Txpos;  Lastn = n;
		}
		Dontread = FALSE;
		if (n < blklen)
			e = ZCRCE;
		else if (junkcount > 3)
			e = ZCRCW;
		else if (bytcnt == Lastsync)
			e = ZCRCW;
		else if (Rxbuflen && (newcnt -= n) <= 0)
			e = ZCRCW;
		else if (Txwindow && (Txwcnt += n) >= Txwspac) {
			Txwcnt = 0;  e = ZCRCQ;
		}
		else
			e = ZCRCG;
		if (Verbose>1)
			fprintf(stderr, "\r%7ld ZMODEM%s    ",
			  Txpos, Crc32t?" CRC-32":"");
		zsdata(txbuf, n, e);
		bytcnt = Txpos += n;
		if (e == ZCRCW)
			goto waitack;
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		fflush(stdout);
		while (rdchk(iofd)) {
#ifdef SVR2
			switch (checked)
#else
			switch (readline(1))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync(1);
				if (c == ZACK)
					break;
#ifdef TCFLSH
				ioctl(iofd, TCFLSH, 1);
#endif
				/* zcrce - dinna wanna starta ping-pong game */
				zsdata(txbuf, 0, ZCRCE);
				goto gotack;
			case XOFF:		/* Wait a while for an XON */
			case XOFF|0200:
				readline(100);
			default:
				++junkcount;
			}
		}
#endif	/* READCHECK */
		if (Txwindow) {
			while ((tcount = Txpos - Lrxpos) >= Txwindow) {
				vfile("%ld window >= %u", tcount, Txwindow);
				if (e != ZCRCQ)
					zsdata(txbuf, 0, e = ZCRCQ);
				c = getinsync(1);
				if (c != ZACK) {
#ifdef TCFLSH
					ioctl(iofd, TCFLSH, 1);
#endif
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;
				}
			}
			vfile("window = %ld", tcount);
		}
	} while (n == blklen);
	if ( !Fromcu)
		signal(SIGINT, SIG_IGN);

	for (;;) {
		stohdr(Txpos);
		zsbhdr(ZEOF, Txhdr);
		switch (getinsync(0)) {
		case ZACK:
			continue;
		case ZRPOS:
			goto somemore;
		case ZRINIT:
			return OK;
		case ZSKIP:
			fclose(in);
			return c;
		default:
			fclose(in);
			return ERROR;
		}
	}
}

/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
getinsync(flag)
{
	register c;

	for (;;) {
		if (Testattn) {
			printf("\r\n\n\n***** Signal Caught *****\r\n");
			Rxpos = 0; c = ZRPOS;
		} else
			c = zgethdr(Rxhdr, 0);
		switch (c) {
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case TIMEOUT:
			return ERROR;
		case ZRPOS:
			/* ************************************* */
			/*  If sending to a modem beuufer, you   */
			/*   might send a break at this point to */
			/*   dump the modem's buffer.		 */
			if (Lastn >= 0 && Lastread == Rxpos) {
				Dontread = TRUE;
			} else {
				clearerr(in);	/* In case file EOF seen */
				fseek(in, Rxpos, 0);
			}
			bytcnt = Lrxpos = Txpos = Rxpos;
			if (Lastsync == Rxpos) {
				if (++Beenhereb4 > 4)
					if (blklen > 256)
						blklen /= 2;
			}
			Lastsync = Rxpos;
			return c;
		case ZACK:
			Lrxpos = Rxpos;
			if (flag || Txpos == Rxpos)
				return ZACK;
			continue;
		case ZRINIT:
		case ZSKIP:
			fclose(in);
			return c;
		case ERROR:
		default:
			zsbhdr(ZNAK, Txhdr);
			continue;
		}
	}
}


/* Say "bibi" to the receiver, try to do it cleanly */
saybibi()
{
	for (;;) {
		stohdr(0L);		/* CAF Was zsbhdr - minor change */
		zshhdr(ZFIN, Txhdr);	/*  to make debugging easier */
		switch (zgethdr(Rxhdr, 0)) {
		case ZFIN:
			sendline('O'); sendline('O'); flushmo();
		case ZCAN:
		case TIMEOUT:
			return;
		}
	}
}

/* Local screen character display function */
bttyout(c)
{
	if (Verbose)
		putc(c, stderr);
}

/* Send command and related info */
zsendcmd(buf, blen)
char *buf;
{
	register c;
	long cmdnum;

	cmdnum = getpid();
	errors = 0;
	for (;;) {
		stohdr(cmdnum);
		Txhdr[ZF0] = Cmdack1;
		zsbhdr(ZCOMMAND, Txhdr);
		zsdata(buf, blen, ZCRCW);
listen:
		Rxtimeout = 100;		/* Ten second wait for resp. */
		c = zgethdr(Rxhdr, 1);

		switch (c) {
		case ZRINIT:
			goto listen;	/* CAF 8-21-87 */
		case ERROR:
		case TIMEOUT:
			if (++errors > Cmdtries)
				return ERROR;
			continue;
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case ZSKIP:
		case ZRPOS:
			return ERROR;
		default:
			if (++errors > 20)
				return ERROR;
			continue;
		case ZCOMPL:
			Exitcode = Rxpos;
			saybibi();
			return OK;
		case ZRQINIT:
			vfile("******** RZ *******");
			system("rz");
			vfile("******** SZ *******");
			goto listen;
		}
	}
}

/*
 * If called as sb use YMODEM protocol
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
	if (s[0]=='s' && s[1]=='b') {
		Nozmodem = TRUE; blklen=KSIZE;
	}
	if (s[0]=='s' && s[1]=='x') {
		Modem2 = TRUE;
	}
}
/* End of sz.c */
