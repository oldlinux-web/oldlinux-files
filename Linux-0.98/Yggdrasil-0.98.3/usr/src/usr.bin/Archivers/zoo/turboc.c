/* @(#) turboc.c 1.2 87/06/21 16:08:54 */

int _stklen = 30000;		/* stack size in bytes */
void _setenvp() {}      /* don't initialize environment pointer etc. */
#include <stdio.h>		/* to get fileno() */

/* following not needed any more since zoocreate() is fixed in portable.c */
/* unsigned _fmode = O_BINARY; */

void dosname PARMS((char *, char *));
#ifdef ANSI_HDRS
# include <stdlib.h>
#else
char *strcpy PARMS((char *, char *));
#endif

#include <signal.h>

/* register definitions specific for Turbo C */
union	REGS	{
	struct { unsigned ax, bx, cx, dx, si, di, carry, flags; } x;
	struct { unsigned char al, ah, bl, bh, cl, ch, dl, dh; }  h;
};

/****************
function zootrunc() truncates a file at the current seek position.
*/

int zootrunc (f)
FILE *f;
{
	int handle = fileno(f);
	extern long tell();
	extern int chsize();
	return chsize(handle, tell(handle));
}

/****************
Function fixfname() converts the supplied filename to a syntax
legal for the host system.  It is used during extraction.
*/

char *fixfname(fname)
char *fname;
{
	char tmpname[PATHSIZE];
	dosname (nameptr(fname), tmpname);
	strcpy(fname,tmpname);
	return(fname);
}

static int set_break (int flag)
{
	extern int intdos();
	int retval;
	union REGS regs;
	regs.x.ax = 0x3300;				/* get ctrl-break flag */
	intdos (&regs, &regs);
	retval = regs.h.dl;				/* retval is old value of setting */
	regs.x.ax = 0x3301;				/* set ctrl-break flag */
	regs.h.dl = flag;				/* status to set to */
	intdos (&regs, &regs);
	return (retval);
}

static int break_flag;

void zooexit (int status)
{
	set_break (break_flag);			/* restore control_break setting */
	exit (status);
}

void gentab (void);

void spec_init(void)
{
	break_flag = set_break (0);
	signal (SIGINT, zooexit);		/* install our own control-C handler */
}

#ifndef fileno
/* To allow compilation with -A (for testing), which makes fileno()
unavailable, we define a function here by that name.  This may be 
compiler-specific for Turbo C++ 1.0. */

int fileno(f)
FILE *f;
{
   return f->fd;
}
#endif /* ! fileno */
