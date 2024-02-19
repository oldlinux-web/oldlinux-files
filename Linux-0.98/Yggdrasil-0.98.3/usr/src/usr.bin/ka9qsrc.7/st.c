/* OS- and machine-dependent stuff for Atari-ST
 * Adapted from the PC version to compile under Lattice C
 * by Walter Doerr, DG2KK (dg2kk@cup.portal.com)
 *
 * 20-2-88:	added code from the Atari MWC version by Rob Janssen PE1CHL 
 * 13-1-88:	added code needed for the 871225.1 version
 * 24-12-87:	first version, adapted from PC.C from the 870412 release
 */

#include "stdio.h"
#include "global.h"
#include "config.h"
#include "mbuf.h"
#include "internet.h"
#include "iface.h"
#include "st.h"
#include "cmdparse.h"
#include "asy.h"

/* #include "stdlib.h"	chkml() , malloc , free */

#ifdef LATTICE
#include "dos.h"	/* dfind */
#endif
#include "osbind.h"

#ifdef	MWC
#include <stat.h>
#include <path.h>
#include <time.h>
#include <signal.h>
FILE *stdprt;
extern	char **environ;
#endif

#define TRUE -1
#define FALSE 0

struct asy asy[ASY_MAX];

/* Interface list header */
struct interface *ifaces;

static struct TempList {
	struct TempList *next;
	char *name;
} *Head;

unsigned nasy;			/* needed in v871225.1 */

char *ttbuf;
char *rsbuf;			/* location of memory alloc'd for rs232 buf */

#ifdef SCREEN
char *newscreen, *newscradr;	/* 32k memory allocated to 2nd video screen */
long logscreen, physcreen;
int toggle = 0;			/* toggle between normal and trace screen */
int printtrace = 0;		/* send trace to printer (controlled by F2) */
#endif

char *shell,*getenv();		/* Name of command shell for exec */
int	rows=25;		/* Number of text rows on screen (may be 50) */

/* called during init b4 anything is printed on the screen.
 * PC.C does a lot of memory allocation stuff here.
 * We just set the cursor to "blink" and allocate memory for a second video
 * screen.
 */
ioinit()
{
	char *lmalloc();	/* Takes a long arg */
	unsigned long ptr;

#ifdef SCREEN
	newscradr = lmalloc(32*1024L);	/* allocate 32k for 2nd video screen */
	ptr = (unsigned long)newscradr;
	newscreen = (char *)((ptr+255) & 0xffffff00L);	/* 256 byte boundary */

	physcreen = (long)Physbase();	/* remember displayed screen address */
	logscreen = (long)Logbase();	/* remember output screen address */
	Cconws("\033j");		/* Save cursor position */
	(void) Setscreen(newscreen,-1L,-1);	/* switch to 2nd screen */
	(void) Vsync();			/* wait for vsync */
	Cconws("\033H\033J");		/* cursor home, clear screen */
	Cconws("\033k");		/* restore cursor position */
	(void) Setscreen(logscreen,-1L,-1);	/* restore old screen */
	(void) Vsync();			/* wait for vsync */
#endif
	printf("\033v\033e\n");		/* Autowrap on, Cursor on */
	(void) Cursconf(2,0);		/* make the cursor blink (3=steady) */
#ifdef MWC
	stdprt = fopen("prn:","w");
	signal(SIGINT, SIG_IGN);	/* Ignore ^C in GEMDOS I/O  -- hyc */
#endif

	shell = getenv("NROWS");
	if (shell != NULL)
		rows=atoi(shell);

	shell = getenv("SHELL");
	if (shell == NULL)
		shell="\\bin\\gulam.prg";
}

/* Called just before exiting. 
 * delete temp files.
 * free memory allocated to the rs-232 and midi buffers.
 */
iostop()
{
	register struct TempList *tptr;

	/* free memory allocated to 2nd video screen */

#ifdef SCREEN
	dispscreen(0);		/* switch back to original screen */
	free(newscradr);
#endif

#ifndef	MWC
	/* delete all temp files that have accumulated */
	(void) tmpdel();
#endif

	/* free memory allocated to RS-232/MIDI I/O buffers */
	while (ifaces != NULLIF) {
		if (ifaces->stop != NULLFP)
			(*ifaces->stop)(ifaces);
		ifaces = ifaces->next;
	}

	tptr = Head;			/* Delete all temp files */
	while( tptr != NULL) {
		if(tptr->name != NULL)
			unlink(tptr->name);
		tptr = tptr->next;
	}
	printf("\n");
}

#ifndef	MWC
/* checks the time then ticks and updates ISS */
static int32 clkval = 0;
void
check_time()
{
	int32 iss();			/* initial sequence number */
	int32 clksec();
	if(clkval != clksec()){
 		clkval = clksec();
		icmpclk();
		tick();
		(void)iss();
	}
}

/* returns the number of seconds from system clock */
static int32
clksec()
{
	long tloc;
	time(&tloc);
	return (tloc);
}
#else
static	clock_t	clkval=0;
void
check_time()
{
	int32 iss();

	/* System clock is 200Hz or 5ms/tick. Skip 11 ticks to match the
	 * definition in timer.h (MSPTICK = 55 there.) -- hyc
	 */
	if((clock() - clkval) > 10) {
		clkval = clock();
		icmpclk();
		tick();
		(void)iss();
	}
}
#endif

/* Initialize async port "dev" (adapted from PE1CHL) */
int
asy_init(dev,addr,vec,bufsize)
int16 dev;
char *addr, *vec;
unsigned bufsize;
{
	register struct iorec *ip;
	register struct asy *ap;
	char *bufp;

#ifdef DEBUG
	printf("asy_init: dev=%d bufsize=%d\n",dev,bufsize);
	fflush(stdout);
#endif
	ap = &asy[dev];

/* ---- Moved here from slip.c - originally by DG2KK... -- hyc ----- */
	/* addr	(COM Port address) is the Atari device name
	 *         (either "AUX:" or "MIDI")
	 * vec	(Interrupt vector) is used as a flag to indicate if 
	 *         bytes received on that interface should be sent out on
	 *         another interface (1 = AUX: 3 = MIDI).
	 */
	ap->vec = atoi(vec);		/* dev to resend bytes to */
	ap->addr = 0;			/* use as error flag */
	if (strcmp(addr,"AUX:") == 0) {
		ap->addr = 1;
	} else if (strcmp(addr,"CON:") == 0) {	/* This would be stupid. */
		ap->addr = 2;
	} else if (strcmp(addr,"MIDI") == 0) {
		ap->addr = 3;
	}
/* ----- end of paraphrase of slip.c... -- hyc ------- */
	if (ap->addr == 0) {
		printf("asy_init(%d): unknown interface\n",dev);
		return -1;
	}

	/* force user to allocate more memory than he already has.
	 * If no memory is allocated, asy_stop() may behave funny...
	 */
	if (bufsize <= 256)	/* only allocate a bigger buffer */
		return -1;

	if ((bufp = malloc(bufsize)) == NULLCHAR){
		printf("asy_init(%d): no memory for rx buffer\n",dev);
		return -1;
	}

	/* Save original IOREC values */

	ip = Iorec((ap->addr)-1);	/* Iorec wants AUX: = 0, MIDI = 2 */

	Jdisint(12);			/* disable RS-232 interrupt */

	ap->in = ip;
	memcpy(&ap->oldin,ip);
	if (ap->addr == RS232) {	/* increase RS-232 transmit buffer? */
		ip++;
		ap->out = ip;
		memcpy(&ap->oldout,ip);
	}

	/* Set up receiver FIFO */
	ap->in->ibuf = bufp;
	ap->in->ibufsiz = bufsize;
	ap->in->ibufhd = ap->in->ibuftl = 0;
	ap->in->ibuflow = 0;
	ap->in->ibufhi = bufsize;

	if (ap->addr == RS232) {
		/* clear transmitter FIFO */
		ap->out->ibufhd = ap->out->ibuftl = 0;
		ap->out->ibuflow = 0;
		ap->out->ibufhi = ap->out->ibufsiz;
	}

	Jenabint(12);			/* enable RS-232 interrupts */

	if (ap->addr == RS232)
		Rsconf(-1,0,-1,0,0,0);	/* 8 bits, no parity */

#ifdef DEBUG
	printf("asy_init: Iorecs in: 0x%lx out: 0x%lx\n",ap->in,ap->out);
	printf("	inbuf: 0x%lx outbuf: 0x%lx\n",ap->in->ibuf,
		ap->out->ibuf);
#endif
}


/* asy_stop restores old iorec and frees memory allocated to the RS-232/MIDI
 * buffers. (from PE1CHL)
 */
int
asy_stop(iface)
struct interface *iface;
{
	register struct asy *ap;
/*	char i_state; */

#ifdef DEBUG
	printf("asy_stop: iface=0x%lx dev=%d\n",iface,iface->dev);
	fflush(stdout);
#endif
	ap = &asy[iface->dev];

	(void) Jdisint(12);		/* disable RS-232 interrupts */

	free(ap->in->ibuf);		/* free the buffer */

	/* Restore old iorecs */
	memcpy(ap->in,&ap->oldin,sizeof(struct iorec));
	if (ap->addr == RS232)
		memcpy(ap->out,&ap->oldout,sizeof(struct iorec));

	(void) Jenabint(12);		/* enable RS-232 interrupts */ 
}


/* Set async line speed */
int
asy_speed(dev,speed)
int dev;
int speed;
{
	int baud; /* int result; */
	register int sp;
	long sav_ssp;

	if (speed <= 0 || dev >= nasy)
		return -1;

	asy[dev].speed = speed;		/* shouldn't this be done in slip.c? */

	switch (asy[dev].addr) {

	case RS232:
		switch (speed) {
		case 300:
			baud = 9;	/* how slow can you get? :-) */
			break;
		case 1200:
			baud = 7;
			break;
		case 2400:
			baud = 4;
			break;
		case 4800:
			baud = 2;
			break;
		case 9600:
			baud = 1;
			break;
		case 19200:
			baud = 0;
			break;
		default:
			printf("asy_speed: unknown RS-232 speed (%d).\n",speed);
			return -1;
		}
		(void) Rsconf(baud,0,0x88,-1,-1,-1);	/* no flow control */
		break;

	case MIDI:
		/* midi can run on 500000 or 614400Hz clock (hardware mod)
		 */
		switch (speed)
		{
		case 0x9600:		/* = 38400 unsigned... (hardware mod) */
		case 31250:		/* normal MIDI baudrate */
			sp = 0x95;	/* /16 */
			break;

		case 9600:		/* 9600 Baud requires hardware mod */
		case 7812:		/* 7812 Baud on an unmodified Atari */
			sp = 0x96;	/* /64 */
			break;

		default:
			printf("asy_speed: unknown MIDI speed (%d).\n",speed);
			return -1;
		}

		sav_ssp = Super(NULL);		/* switch to Supervisor mode */
		*((char *) 0xfffffc04L) = 0x03; /* Reset 6850 */
		hiword(0);			/* spend some time */
		*((char *) 0xfffffc04L) = sp;	/* set new divider ratio */
		Super(sav_ssp);			/* back to User mode */

		break;
	}
	asy_flush(dev);
}


/* flush the input buffer of device dev (either aux: or midi).
 * May be useful, because setting the baudrate causes an 0x7f to be sent.
 */
asy_flush(dev)
int dev;
{
	int st_dev;
	long c;					/* Bconin returns a long */
	st_dev = asy[dev].addr;

	while (Bconstat(st_dev) == -1) {	/* at least 1 char avlb */
		c = Bconin(st_dev);
	}
}


/* Send a buffer to serial transmitter */
asy_output(dev,buf,cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
	int st_dev;
	unsigned short i = 0;

	if (dev >= nasy)
		return -1;

	st_dev = asy[dev].addr;
	for (i = 0 ; i < cnt ; ++i) {
		(void)Bconout(st_dev,buf[i]);	/* 1= AUX: 3=MIDI: */
	}
}


/* Receive characters from async line
 * Returns count of characters received
 * dev is the device 
 * buf is the buffer where received characters are stored
 * cnt is the number of characters the calling routine expects to find in buf.
 * On return, the no of chars still in the rs-232 buffer should be returned!??
 * Quick and dirty hack: 
 * since this routine is called from one location (slip.c) only, with cnt=1
 * we just return the AUX: state (char avlb or not (bios(1,1) bconstat)).
 * 
 */
unsigned
asy_recv(dev,buf,cnt)
int dev;
char *buf;
unsigned cnt;
{
	char c;
	int rs_state;
	int st_dev = asy[dev].addr;	/* AUX: device on the Atari ST */
	int snd_dev = asy[dev].vec;

	rs_state = Bconstat(st_dev);	/* 0 = no char, -1 = char avlb */

	if (rs_state == -1) {
		rs_state = 1;		/* at least one char available */
		c = (char)Bconin(st_dev);
#ifdef	RETRANSMIT
		/* this does the actual retransmitting of received bytes
		 * depending on the vector field in the 'attach asy' command.
		 * remove it, if you don't have use for it.
		 */
		if (snd_dev == 1 || snd_dev == 3) {	/* if AUX: or MIDI */
			(void)Bconout(snd_dev,c & 0xff);
		}
#endif
		*buf = c & 0xff;
	}
	return (rs_state);	/* 0 = no char, 1 = at least 1 more char */
}

#ifdef	MWC
int				/* Just for argument's sake... -- hyc */
kbhit()
{
	return(Cconis());
}
#endif

int kbread()
{
	int c;
	long	raw;

	if (Cconis() == 0){	/* no key hit: just return */
		return (-1);
	}

	raw = Crawcin();	/* Read ASCII and scan code from keyboard */
	c = raw & 0x7f;		/* get rid of scan codes !!!! */

#ifdef	ESCAPEOUT		/* Why would you *ever* do this???  -- hyc */
	if (c == 0x1b)
		c = -2;		/* ESCAPE behaves like F10 */
#endif
	/* process function keys */
	if (c == 0) {		/* we have an F key */
		switch((int) ((raw >> 16) & 0xff)) {
		case 0x3b:			/* F1 */
			toggle = ~toggle;
			(void) dispscreen(toggle);
			c = -1;
			break;

		case 0x3c:			/* F2 */
			printtrace = 0;		/* printer off */
			c = -1;
			break;

		case 0x55:			/* Shift F2 */
			printtrace = 1;		/* printer on */
			c = -1;
			break;

		case 0x44:			/* F10 */
			c = -2;
			break;
		default:
			c = -1;			/* ignore other keys */
			break;
		}
	}
	return (c);
}

#ifndef	MWC		/* see vfile.c for MWC -- hyc */
/* Create a temporary file and return the filepointer
 * (adapted from the routine in mac_io.c)
 */
FILE *
tmpfile()
{
	FILE *tmp;
	char	name[32];
	struct	TempList *tptr;
	long tt;

	tt = clksec();
	tt &= 0x007fffffL;
	sprintf(name,"\\X%07.7ld.tmp",tt);
	if ((tmp = fopen(name,"wrb")) == NULL) {
		printf("tmpfile: could not create temp file. (%s)\n",name);
		return(tmp);
	}
	tptr = (struct TempList *)malloc(sizeof(struct TempList));
	if (tptr == NULL) {
		printf("tmpfile: not enough memory for TempList\n");
		return(tptr);
	}
	if(Head != NULL)
		tptr->next = Head->next;
	else
		tptr->next = NULL;
	if((tptr->name = malloc(strlen(name)+1)) == NULL) {
		printf("tmpfile: not enough memory for name\n");
		return(tptr);
	}
	strcpy(tptr->name,name);
	if(Head != NULL)
		Head->next = tptr;
	else
		Head = tptr;
	return (tmp);
}
#endif

#ifndef	disable
int disable()
{
	return 1;
}

int restore(istate)
int istate;
{
}
#endif

void eihalt()
{
}


/* the following is new stuff from version 871225.1 */

asy_ioctl(interface,argc,argv)
struct interface *interface;
int argc;
char * argv[];
{
	if (argc < 1) {
		printf("speed: %d baud\n",asy[interface->dev].speed);
		return 0;
	}
	return asy_speed(interface->dev,atoi(argv[0]));
}

int asy_rxint()
{
	printf("asy_rxint\n");
}

int asy_txint()
{
	printf("asy_txint\n");
}

int setbit()
{
	printf("setbit\n");
}

int clrbit()
{
	printf("clrbit\n");
}

void
rename(s,d)
	char *s, *d;		/* Source and dest name */
{
	Frename(0, s, d);	/* Ignore return code... */
}

doshell(argc,argv)
int argc; char *argv[];
{
	long execstat;
	char *args[2];

#ifdef SCREEN
	/* make sure that we are on the right (main) screen */
	dispscreen(0);
	toggle = 0;
#endif
	if (argc < 2) {
		args[0]="shell";
		args[1]=NULL;
		execstat = exec(shell,args);
	} else
		execstat = exec(argv[1],&argv[1]);

	if (execstat != 0)
		printf("Pexec: errorcode: %ld\n",execstat);

	printf("\n");
	(void)Cursconf(1,0);		/* cursor on */
	(void)Cursconf(2,0);		/* cursor blink */
}


dotype(argc,argv)
int argc;
char *argv[];
{
	char tstring[200];
	char fname[50];
	FILE *tfile;
	int linecnt = 0;

	if (argc != 2) {
		printf("Usage: type <filename>\n");
		return;
	}

	strcpy(fname,argv[1]);
	if ((tfile = fopen(fname, "r")) == NULLFILE) {
		printf("type: cannot open '%s'\n",fname);
		return;
	}

	while (!feof(tfile)) {
		fgets(tstring,200,tfile);
		linecnt++;
		if (strlen(tstring) > 79)
			linecnt++;
		printf("%s",tstring);
		if ((linecnt % 23) == 0) {
			printf("\033p more \033q");
			if ((gemdos(7) & 0x7f) == 'q') {
				printf("\033l\n");
				break;
			}
			printf("\033l");
		}
	}
	fclose(tfile);
}

giveup()
{
}

int stxrdy(dev)
int dev;
{
	int stat;
	int st_dev;

	st_dev = asy[dev].addr;
	stat=(int)Bcostat(st_dev);	/* Bcostat:  -1 ready , 0 not ready */
	if (stat == -1) {
		stat = 1;
	}
	return (stat);
}

#ifdef	LATTICE
/*
 * replacement for buggy Lattice memchr function used in telnet.c
 */
char *
memchr_st(str,c,n)
char *str;
char c;
unsigned n;
{
	while (n-- != 0) {
		if (*str++ == c)
			return (str);
	}
	return NULLCHAR;
}


/* move (copy) a block of memory */
/* this function assumes the blocks do not overlap */
memcpy (d,s,c)
register char *d;			/* destination pointer */
register char *s;			/* source pointer */
register int  c;			/* bytecount */
{
	if (c)				/* nonzero count? */
	do
	{
		*d++ = *s++;		/* then move on */
	} while (--c);			/* while more to go */
}

/* fill a block of memory */
void memset (p,c,n)
register char *p;			/* block pointer */
register char c;			/* initialization value */
register int  n;			/* bytecount */
{
	if (n)				/* nonzero count? */
	do
	{
		*p++ = c;		/* then do it */
	} while (--n);			/* more to fill? */
}

/* compare memory blocks, return 0 if equal */
int memcmp (d,s,c)
register char *d;			/* destination pointer */
register char *s;			/* source pointer */
register int  c;			/* bytecount */
{
	if (c)				/* nonzero count? */
	do
	{
		if (*s++ != *d++)	/* compare */
		return (*--d - *--s);	/* when unequal, return diff */
	} while (--c);			/* while more to go */

	return (0);				/* equal! */
}

/* lookup some character in a memory block */
char *memchr (p,c,n)
register char *p;			/* block pointer */
register char c;			/* value to look for */
register int  n;			/* bytecount */
{
	while (n--)
	if (*p == c)
		return (p);
	else
		p++;

	return (NULLCHAR);
}
#endif	/* LATTICE */

memstat()
{
#ifdef	AX25
	extern int digisent;
#endif
	long size;

	size = Malloc(-1L);
	printf("\nFree memory: %ld bytes.\n\n",size);
#ifdef	AX25
	printf("Digisent: %d frames.\n\n",digisent);
#endif
	printf("AUX:");
	iosize(0);
	printf("MIDI:");
	iosize(2);
}

iosize(i)
int i;
{
	struct iorec *rsbuffer;

	rsbuffer = (struct iorec*) Iorec(i);		/* get iorec */
	printf("\tIorec:  %08lx\n",rsbuffer);
	printf("\tBuffer: %08x\n",rsbuffer->ibuf);
	printf("\tSize:   %d bytes\n",rsbuffer->ibufsiz);
}

#ifdef	LATTICE
#define	DMABUFFER	struct FILEINFO
#define d_fname		name

/* delete all temp files which have accumulated */
tmpdel()
{
	DMABUFFER info;
	char delnam[20];
	int error;

	Fsetdta(&info);

	if ((error = Fsfirst("\\x*.tmp",0)) != 0){
		info.d_fname[0] = '\0';
	}
	while (info.d_fname[0] != '\0') {
		sprintf(delnam,"\\%s",info.d_fname);
		unlink(delnam);
		if ((error = Fsnext()) != 0) {
			info.d_fname[0] = '\0';
		}
	}
}

	/* The MWC version seems OK. -- hyc */
/* the access() call doesn't seem to work in Lattice-C.
 * this is a quick and dirty hack...
 * it returns -1 if the file exists and 0 if it doesn't.
 */
int
access(name,mode)
char *name;
int mode;
{
	struct FILEINFO *info;
	int ret;

	Fsetdta(&info);
	if ((ret = Fsfirst(name,0)) != 0)
		return -1;
	return 0;
}
#endif	/* LATTICE */

#ifdef SCREEN

/* dispscreen selects the screen to be displayed by the video hardware.
 * this routine is called from within kbread()
 * flag:  =0: display normal screen
 *       <>0: display trace screen
 */
dispscreen(flag)
int flag;
{
	if (flag == 0) {
		(void) Setscreen(-1L,physcreen,-1);
		(void) Vsync();
		printf("\033e");	/* turn on cursor */
	} else {
		(void) Setscreen(-1L,newscreen,-1);
		(void) Vsync();
		printf("\033f");	/* turn off cursor */
	}
}


/* outscreen selects one of two screens where display output (printf's) should
 * go to.
 * outscreen is called from trace.c before/after trace output is done
 */
outscreen(flag)
int flag;
{
	if (flag == 0) {
		(void) Setscreen(logscreen,-1L,-1);
		(void) Vsync();
		printf("\033k");	/* restore cursor pos on main screen */
	} else {
		(void) Setscreen(newscreen,-1L,-1);
		(void) Vsync();
		printf("\033j");	/* save cursor pos on main screen */
		printf("\033Y%c%c",rows+31,32);
	}
}
#endif

#ifdef	MWC
static exec(command,args)
char *command;
char *args[];
{
	char **envx,*pt,*com;
	register int i,j,k,l;

	pt=getenv("PATH");		/* Find an executable file named */
	if(pt == NULL)			/* command in the path... */
		pt=DEFPATH;

	com=path(pt,command,1);
	if(com != NULL)
		command = com;

	j=(-1);
	for(i=0;environ[i]!=0;i++)	/* Count vars in environment */
		if(!strncmp(environ[i],"ARGV",4))	/* Skip ARGV */
			j=i;

	if(j<0)
		i++;

	envx=(char **)malloc(i*sizeof(char *));	/* Copy environment */
	i--;
	envx[i] = NULL;

	for(k=0,l=0;k<i;l++)
		if(l!=j) {
			envx[k]=malloc(strlen(environ[l])+1);
			strcpy(envx[k],environ[l]);
			k++;
		}
	
	return(execve(command,args,envx));
}
#endif
	
/* Do a warm boot, I think. -- hyc */
void
sysreset()
{
	long *resvec,(*resjump)();

	Super(NULL);	/* Get into supervisor mode to read sys vars */
	resvec = (long *)0x42a;
	resjump = *resvec;
	(*resjump)();
}

#ifdef	MWC
/*
 * set console output to binary or ascii mode (raw or cooked...)
 * mode == 1 means raw, anything else means cooked.
 * This is a compiler/library dependent hack. Too bad I can't
 * come up with a better way.  -- hyc
 */

void
set_stdout(mode)
int mode;
{
	FILE *fp, *_fopen();
	char *fmode;

	if (mode == 1)
		fmode = "wb";
	else
		fmode = "w";

	fflush(stdout);
	fp=_fopen(NULL, fmode, stdout, 1);
}
#endif
