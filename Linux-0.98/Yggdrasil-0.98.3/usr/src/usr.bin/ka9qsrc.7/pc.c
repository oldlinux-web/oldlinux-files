/* OS- and machine-dependent stuff for IBM-PC running MS-DOS */
#include <stdio.h>
#ifdef __TURBOC__
#include <dir.h>
#include <sys/stat.h>
#include <string.h>
#include <process.h>
#include <fcntl.h>
#else
#include <sgtty.h>
#endif
#include "config.h"
#include "global.h"
#include "mbuf.h"
#include "internet.h"
#include "iface.h"
#include "cmdparse.h"

void	_Cdecl	__int__		(int interruptnum);

#ifdef TRACE
extern FILE *trfp;			/* trace file pointer */
extern char trname[];			/* trace file name */
#endif

/* This flag is set by setirq() if IRQ 8-15 is used, indicating
 * that the machine is a PC/AT with a second 8259 interrupt controller.
 * If this flag is set, the interrupt return code in pcgen.asm will
 * send an End of Interrupt command to the second 8259 as well as the
 * first.
 */
char isat;

/* Interface list header */
struct interface *ifaces;

#ifdef PC9801
extern char escape;
#endif

/* Aztec memory allocation control */
int _STKLOW = 0;	/* Stack above heap */
int _STKSIZ = 16384/16;	/* 16K stack -- overridden in grabcore */
int _HEAPSIZ = 4096/16;	/* Isn't really used */
int _STKRED = 4096;	/* Stack red zone in bytes -- this really matters */

char ttbuf[24*80];
int saved_break;

/* Called at startup time to set up console I/O, memory heap */
ioinit()
{
#ifndef __TURBOC__
	struct sgttyb ttybuf;
#endif
	unsigned grabcore();

	/* Save these two file table entries for something more useful */
	fclose(stdaux);
#ifdef __TURBOC__
	fclose(stdprn);
#else
	fclose(stdprt);
#endif

	/* Interrupts use a special stack deep in data space.
	 * Calls to sbrk() (invoked by malloc when it needs more memory
	 * from the system) at interrupt time will fail because sbrk()
	 * will think that the stack has overwritten the heap. So
	 * grab all the memory we can now for the heap so that malloc
	 * won't have to call sbrk and alloc_mbuf() won't fail unnecessarily
	 * at interrupt time.
	 */
	grabcore();

#ifdef __TURBOC__
	/*_fmode = O_BINARY;*/
#endif
	setbuf(stdout,ttbuf);

#ifdef __TURBOC__
	saved_break = getcbrk();
	setcbrk(0);
	ioctl(fileno(stdout), 1, ioctl(fileno(stdout), 0) & 0xff | 0x20);  /* put stdout in raw mode */
#else
	/* Put display in raw mode. Note that this breaks tab expansion,
	 * so you need to run NANSI.SYS or equivalent.
	 */
	ioctl(1,TIOCGETP,&ttybuf);
	ttybuf.sg_flags = RAW;
	ioctl(1,TIOCSETP,&ttybuf);
#endif
}
/* Called just before exiting to restore console state */
iostop()
{
#ifndef __TURBOC__
	struct sgttyb ttybuf;
#endif

	setbuf(stdout,NULLCHAR);
#ifdef __TURBOC__
	setcbrk(saved_break);
	ioctl(fileno(stdout), 1, ioctl(fileno(stdout), 0) & 0xff & ~0x20);  /* put stdout in cooked mode */
#else
	ioctl(1,TIOCGETP,&ttybuf);
	ttybuf.sg_flags &= ~RAW;
	ioctl(1,TIOCSETP,&ttybuf);
#endif
	while(ifaces != NULLIF){
		if(ifaces->stop != NULLFP)
			(*ifaces->stop)(ifaces);
		ifaces = ifaces->next;
	}
}
/* Spawn subshell */
doshell(argc,argv)
int argc;
char *argv[];
{
	char *command,*getenv();
	int ret;

#if defined(PLUS)
/*
 * set: cursor to block, attributes off, keyboard to ALT mode,
 *      transmit functions off, use HP fonts
 */
	printf(/* "\033*dK"	   cursor to block		*/
	          "\033&d@"	/* display attributes off  	*/
	       /* "\033&k1\\"	   KBD to ALT mode		*/
	       /* "\033&s1A"	   transmit functions on	*/
	          "\033[11m");	/* use ALT fonts		*/
#endif
#ifdef __TURBOC__

#ifdef TRACE
	if (trfp != stdout)		/* trace to file? */
	    fclose(trfp);		/* close it during shell exec */
#endif

	ioctl(fileno(stdout), 1, ioctl(fileno(stdout), 0) & 0xff & ~0x20);  /* put stdout in cooked mode */
	if((command = getenv("COMSPEC")) == NULLCHAR)
		command = "/COMMAND.COM";
	ret = spawnv(P_WAIT,command,argv);

	ioctl(fileno(stdout), 1, ioctl(fileno(stdout), 0) & 0xff | 0x20);  /* put stdout in raw mode */
	return ret;
#else
	struct sgttyb ttybuf,ttysav;

	ioctl(1,TIOCGETP,&ttysav);
	ioctl(1,TIOCGETP,&ttybuf);
	ttybuf.sg_flags &= ~RAW;
	ioctl(1,TIOCSETP,&ttybuf);

	if((command = getenv("COMSPEC")) == NULLCHAR)
		command = "/COMMAND.COM";
	ret = fexecl(command,command,NULLCHAR);
	ioctl(1,TIOCSETP,&ttysav);
#ifdef TRACE
	if (trfp != stdout)		/* re-open tracefile if not stdout */
	    if ((trfp = fopen(trname,"a+")) == NULLFILE)
		trfp = stdout;
#endif
	if(ret == -1)
		return -1;
	else
		return wait();
#endif
}

/* checks the time then ticks and updates ISS */
static short clockstart =0;
static unsigned clkval = 0;
#if defined(NOMAD)   /* compiling for a HP-110 */
/*
 * returns the second value of the clock
 */
int clksec()
{
	_AH=0x2c;
	__int__(0x21);
	_AL=_DH;
	_AH=0;
}

void
check_time()
{
	int32 iss();
	if(clkval != clksec()) {
		clkval = clksec();
		icmpclk();
		tick();
		(void)iss();
	}
}
#else
void
check_time()
{
	int32 iss();
	short ntime;

	if(!clockstart){
		/* Executed only once */
#if defined(PLUS)   /* compiling for a HP-110+  */
		clkval = peek(0,0x472);
#else
		clkval = peekw(0x6c,0x40);
#endif
		clockstart = 1;
		return;
	}
	/* Read the low order word of the BIOS tick counter directly. The
	 * INT 1Ah call isn't used because it stupidly clears the
	 * "midnight passed" flag and we'd have to update the date ourselves.
	 * (See Norton, p222).
	 * 
 	 * The PC's time-of-day handling is a real crock of shit. Why not a
	 * nice simple long binary count from a fixed UTC epoch, as in UNIX??
	 */
#if defined(PLUS)
	ntime = peek(0,0x472);
#else
	ntime = peekw(0x6c,0x40);
#endif
	while(ntime != clkval){	/* Handle possibility of several missed ticks */
		clkval++;
		icmpclk();	/* Call this one before tick */
		tick();
		(void)iss();
	}
}
#endif

/* Read characters from the keyboard, translating them to "real" ASCII
 * If none are ready, return the -1 from kbraw()
 */
int
kbread()
{
	int kbraw(),c;

#ifdef PC9801
	if((c = kbraw()) == escape){
	  c = -2;
#else
	if((c = kbraw()) == 0){
		/* Lead-in to a special char */
		c = kbread();
		switch(c){
		case 3:		/* NULL (bizzare!) */
			c = 0;
			break;
#if defined(PLUS)
		case 66:	/* f-8 key too for HP 110 Plus */
#endif
		case 68:	/* F-10 key (used as command-mode escape) */
			c = -2;
			break;
		case 83:	/* DEL key */
			c = 0x7f;
			break;
		default:	/* Dunno what it is */
			c = -1;
		}
#endif /* PC9801 */
	}
#if defined(PLUS)
	if(c==27 && kbhit()) {
		if((c = kbraw()) == 119) return -2;
		return -1;
	}
#endif
	return c;
}
#define	CTLZ	26
/* Special version of aputc() (used by putchar and printf) that filters
 * out nasty characters that screw up the DDOS and ANSI terminal drivers
 */
aputc(c,file)
char c;
FILE *file;
{
	/* Nulls get displayed as spaces by ansi.sys (wrong)
	 * ^Z's seem to hang the DoubleDos and DesqView screen drivers
	 */
	if((c == '\0' || c == CTLZ) && file == stdout)
		return c;
	/* Do end-of-line translations */
	if(c == '\n')
		putc('\r',file);
	return putc(c,file);
}
/* Reset the CPU, reboot DOS */
sysreset()
{
}

/* Install hardware interrupt handler.
 * Takes IRQ numbers from 0-7 (0-15 on AT) and maps to actual 8086/286 vectors
 * Note that bus line IRQ2 maps to IRQ9 on the AT
 */
setirq(irq,handler)
unsigned irq;
void (*handler)();
{
	/* Set interrupt vector */
	if(irq < 8){
		setvect(8+irq,handler);
	} else if(irq < 16){
		isat = 1;
		setvect(0x70 + irq - 8,handler);
	} else {
		return -1;
	}
	return 0;
}
/* Return pointer to hardware interrupt handler.
 * Takes IRQ numbers from 0-7 (0-15 on AT) and maps to actual 8086/286 vectors
 */
void
(*getirq(irq))()
unsigned int irq;
{
	void (*getvect())();

	/* Set interrupt vector */
	if(irq < 8){
		return getvect(8+irq);
	} else if(irq < 16){
		return getvect(0x70 + irq - 8);
	} else {
		return NULLVFP;
	}
}
/* Disable hardware interrupt */
maskoff(irq)
unsigned irq;
{
	if(irq < 8){
		setbit(0x21,(char)(1<<irq));
	} else if(irq < 16){
		irq -= 8;
		setbit(0xa1,(char)(1<<irq));
	} else {
		return -1;
	}
	return 0;
}
/* Enable hardware interrupt */
maskon(irq)
unsigned irq;
{
	if(irq < 8){
		clrbit(0x21,(char)(1<<irq));
	} else if(irq < 16){
		irq -= 8;
		clrbit(0xa1,(char)(1<<irq));
	} else {
		return -1;
	}
	return 0;
}
/* Return 1 if specified interrupt is enabled, 0 if not, -1 if invalid */
getmask(irq)
unsigned irq;
{
	if(irq < 8)
		return (inportb(0x21) & (1 << irq)) ? 0 : 1;
	else if(irq < 16){
		irq -= 8;
		return (inportb(0xa1) & (1 << irq)) ? 0 : 1;
	} else
		return -1;
}

