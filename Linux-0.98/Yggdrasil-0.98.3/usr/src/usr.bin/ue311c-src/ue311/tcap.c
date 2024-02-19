/*	tcap:	Unix V5, SUN OS, SCO XENIX, V7 and BS4.2 Termcap video driver
		for MicroEMACS 3.10

         12-10-88 - Modifications made by Guy Turcotte to accomodate
                    SunOS V4.0 and Xenix V2.2.1 :
 
                  SunOS mods:
                  
                  o p_seq field of TBIND struct augmented to 10 chars
                    to take into account longer definitions for keys
                    (some Sun's keys definitions need at least 7 chars...)
                    as such, the code in get1key has been modified to take
                    care of the longer p_seq string.
 
                  o tcapopen modified to take care of the tgetstr problem
                    (returns NULL on undefined keys instead of a valid
                    string pointer...)
 
                  o The timout algorithm of get1key has been modified to
                    take care of the following select() function problem:
                    if some chars are already in the terminal buffer before
                    select is called and no others char appears on the terminal,
                    it will timeout anyway... (maybe a feature of SunOs V4.0)
 
                  Xenix mods:
 
                  o The first two points indicated above are applicable for
                    the Xenix OS
 
                  o With my current knowledge, I can't find a clean solution
                    to the timeout problem of the get1key function
                    under Xenix. I modified the code to get rid of the BSD code 
                    (via the #if directive) and use the Xenix nap() and rdchk()
                    functions to 
                    make a 1/30 second wait. Seems to work as long as there is
                    not to much of activity from other processes on the system.
                    (The link command of the makefile must be modified to
                    link with the x library... you must add the option -lx)
 
                  o The input.c file has been modified to not include the
                    get1key function defined there in the case of USG. The
                    #if directive preceeding the get1key definition has been
                    modified from:
 
                     #if (V7 == 0) && (BSD == 0)
 
                    to:
 
                     #if (V7 == 0) && (BSD == 0) && (USG == 0)
                     
                  o The following lines define the new termcap entry for
                    the ansi kind of terminal: it permits the use of functions
                    keys F1 .. F10 and keys HOME,END,PgUp,PgDn on the IBM PC
                    keyboard (the last 3 lines of the definition have been
                    added):
 
 li|ansi|Ansi standard crt:\
 	:al=\E[L:am:bs:cd=\E[J:ce=\E[K:cl=\E[2J\E[H:cm=\E[%i%d;%dH:co#80:\
 	:dc=\E[P:dl=\E[M:do=\E[B:bt=\E[Z:ei=:ho=\E[H:ic=\E[@:im=:li#25:\
 	:nd=\E[C:pt:so=\E[7m:se=\E[m:us=\E[4m:ue=\E[m:up=\E[A:\
 	:kb=^h:ku=\E[A:kd=\E[B:kl=\E[D:kr=\E[C:eo:sf=\E[S:sr=\E[T:\
 	:GS=\E[12m:GE=\E[10m:GV=\63:GH=D:\
 	:GC=E:GL=\64:GR=C:RT=^J:G1=?:G2=Z:G3=@:G4=Y:GU=A:GD=B:\
 	:CW=\E[M:NU=\E[N:RF=\E[O:RC=\E[P:\
 	:WL=\E[S:WR=\E[T:CL=\E[U:CR=\E[V:\
 	:HM=\E[H:EN=\E[F:PU=\E[I:PD=\E[G:\
 	:k1=\E[M:k2=\E[N:k3=\E[O:k4=\E[P:k5=\E[Q:\
 	:k6=\E[R:k7=\E[S:k8=\E[T:k9=\E[U:k0=\E[V:\
 	:kh=\E[H:kH=\E[F:kA=\E[L:kN=\E[G:kP=\E[I:
                    
*/

#define termdef 1			/* don't define "term" external */

#include <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#if TERMCAP

#if	USG | HPUX | SMOS
#include	<time.h>
#endif
#if	BSD | V7
#include	<sys/types.h>
#include	<sys/time.h>
#endif

#define MARGIN	8
#define SCRSIZ	64
#define NPAUSE	10			/* # times thru update to pause */
#define BEL	0x07
#define ESC	0x1B

/*	Termcap Sequence definitions	*/

typedef struct TBIND {
	char p_name[4]; /* sequence name */
	short p_code;	/* resulting keycode of sequence */
	char p_seq[10];	/* terminal escape sequence */
} TBIND;

TBIND ttable[] = {
	"bt",	SHFT | CTRL | 'i',	"",	/* backtab */
	"k1",	SPEC | '1',		"",	/* function key 1 */
	"k2",	SPEC | '2',		"",	/* function key 2 */
	"k3",	SPEC | '3',		"",	/* function key 3 */
	"k4",	SPEC | '4',		"",	/* function key 4 */
	"k5",	SPEC | '5',		"",	/* function key 5 */
	"k6",	SPEC | '6',		"",	/* function key 6 */
	"k7",	SPEC | '7',		"",	/* function key 7 */
	"k8",	SPEC | '8',		"",	/* function key 8 */
	"k9",	SPEC | '9',		"",	/* function key 9 */
	"k0",	SPEC | '0',		"",	/* function key 10 */
	"kA",	CTRL | 'O',		"",	/* insert line */
	"kb",	CTRL | 'H',		"",	/* backspace */
	"kC",	CTRL | 'L',		"",	/* clear screen */
	"kD",	SPEC | 'D',		"",	/* delete character */
	"kd",	SPEC | 'N',		"",	/* down cursor */
	"kE",	CTRL | 'K',		"",	/* clear to end of line */
	"kF",	CTRL | 'V',		"",	/* scroll down */
	"kH",	SPEC | '>',		"",	/* home down [END?] key */
	"kh",	SPEC | '<',		"",	/* home */
	"kI",	SPEC | 'C',		"",	/* insert character */
	"kL",	CTRL | 'K',		"",	/* delete line */
	"kl",	SPEC | 'B',		"",	/* left cursor */
	"kN",	SPEC | 'V',		"",	/* next page */
	"kP",	SPEC | 'Z',		"",	/* previous page */
	"kR",	CTRL | 'Z',		"",	/* scroll down */
	"kr",	SPEC | 'F',		"",	/* right cursor */
	"ku",	SPEC | 'P',		"",	/* up cursor */
#if	SMOS
	"ka",	SPEC | 'J',		"",	/* function key 11*/
	"F1",	SPEC | 'K',		"",	/* function key 12*/
	"F2",	SPEC | 'L',		"",	/* function key 13*/
	"F3",	SPEC | 'M',		"",	/* function key 14*/
	"F4",	SPEC | 'N',		"",	/* function key 15*/
	"F5",	SPEC | 'O',		"",	/* function key 16*/
	"F6",	SHFT | SPEC | '1',	"",	/* S-function key 1 */
	"F7",	SHFT | SPEC | '2',	"",	/* S-function key 2 */
	"F8",	SHFT | SPEC | '3',	"",	/* S-function key 3 */
	"F9",	SHFT | SPEC | '4',	"",	/* S-function key 4 */
	"FA",	SHFT | SPEC | '5',	"",	/* S-function key 5 */
	"FB",	SHFT | SPEC | '6',	"",	/* S-function key 6 */
	"FC",	SHFT | SPEC | '7',	"",	/* S-function key 7 */
	"FD",	SHFT | SPEC | '8',	"",	/* S-function key 8 */
	"FE",	SHFT | SPEC | '9',	"",	/* S-function key 9 */
	"FF",	SHFT | SPEC | '0',	"",	/* S-function key 10*/
	"FG",	SHFT | SPEC | 'J',	"",	/* S-function key 11*/
	"FH",	SHFT | SPEC | 'K',	"",	/* S-function key 12*/
	"FI",	SHFT | SPEC | 'L',	"",	/* S-function key 13*/
	"FJ",	SHFT | SPEC | 'M',	"",	/* S-function key 14*/
	"FK",	SHFT | SPEC | 'N',	"",	/* S-function key 15*/
	"FL",	SHFT | SPEC | 'O',	"",	/* S-function key 16*/
#endif
};

#define	NTBINDS	sizeof(ttable)/sizeof(TBIND)

extern int	ttopen();
extern int	ttgetc();
extern int	ttputc();
extern int	tgetnum();
extern int	ttflush();
extern int	ttclose();
extern int	tcapkopen();
extern int	tcapkclose();
extern int	tcapgetc();
extern int	tcapmove();
extern int	tcapeeol();
extern int	tcapeeop();
extern int	tcapbeep();
extern int	tcaprev();
extern int	tcapcres();
extern int	tcapopen();
extern int	tcapclose();
extern int	tput();
extern char	*tgoto();
#if	COLOR
extern	int	tcapfcol();
extern	int	tcapbcol();
#endif

#define TCAPSLEN 1024
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE, *IS, *KS, *KE;

TERM term = {
	0, 0, 0, 0,	/* these four values are set dynamically at open time */
	0, 0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	tcapopen,
	tcapclose,
	tcapkopen,
	tcapkclose,
	tcapgetc,
	ttputc,
	ttflush,
	tcapmove,
	tcapeeol,
	tcapeeop,
	tcapbeep,
	tcaprev,
	tcapcres
#if	COLOR
	, tcapfcol,
	tcapbcol
#endif
};

/*	input buffers and pointers	*/

#define	IBUFSIZE	64	/* this must be a power of 2 */

unsigned char in_buf[IBUFSIZE];	/* input character buffer */
int in_next = 0;		/* pos to retrieve next input character */
int in_last = 0;		/* pos to place most recent input character */

in_init()	/* initialize the input buffer */

{
	in_next = in_last = 0;
}

in_check()	/* is the input buffer non-empty? */

{
	if (in_next == in_last)
		return(FALSE);
	else
		return(TRUE);
}

in_put(event)

int event;	/* event to enter into the input buffer */

{
	in_buf[in_last++] = event;
	in_last &= (IBUFSIZE - 1);
}

int in_get()	/* get an event from the input buffer */

{
	register int event;	/* event to return */

	event = in_buf[in_next++];
	in_next &= (IBUFSIZE - 1);
	return(event);
}

/*	Open the terminal
	put it in RA mode
	learn about the screen size
	read TERMCAP strings for function keys
*/

tcapopen()

{
	register int index;		/* general index */
	char *t, *p;
	char tcbuf[1024];
	char *tv_stype;
	char err_str[72];
	char *getenv();
	char *tgetstr();

	if ((tv_stype = getenv("TERM")) == NULL) {
		puts(TEXT182);
/*		     "Environment variable TERM not defined!" */
		meexit(1);
	}

	if ((tgetent(tcbuf, tv_stype)) != 1) {
		sprintf(err_str, TEXT183, tv_stype);
/*				 "Unknown terminal type %s!" */
		puts(err_str);
		meexit(1);
	}

 
	if ((term.t_nrow=(short)tgetnum("li")-1) == -1) {
	       puts(TEXT184);
/*		    "termcap entry incomplete (lines)" */
	       meexit(1);
	}
	term.t_mrow =  term.t_nrow;

	if ((term.t_ncol=(short)tgetnum("co")) == -1){
		puts(TEXT185);
/*		    "Termcap entry incomplete (columns)" */
		meexit(1);
	}
	term.t_mcol = term.t_ncol;

	p = tcapbuf;
	t = tgetstr("pc", &p);
	if (t)
		PC = *t;

	CL = tgetstr("cl", &p);
	CM = tgetstr("cm", &p);
	CE = tgetstr("ce", &p);
	UP = tgetstr("up", &p);
	SE = tgetstr("se", &p);
	SO = tgetstr("so", &p);
	if (SO != NULL)
		revexist = TRUE;

	if (CL == NULL || CM == NULL || UP == NULL)
	{
		puts(TEXT186);
/*		     "Incomplete termcap entry\n" */
		meexit(1);
	}

	if (CE == NULL) 	/* will we be able to use clear to EOL? */
		eolexist = FALSE;
		 
	IS = tgetstr("is", &p); /* extract init string */
	KS = tgetstr("ks", &p); /* extract keypad transmit string */
	KE = tgetstr("ke", &p); /* extract keypad transmit end string */
	        
	/* read definitions of various function keys into ttable */
	for (index = 0; index < NTBINDS; index++) {
		strcpy(ttable[index].p_seq,
			fixnull(tgetstr(ttable[index].p_name, &p)));
	}

	/* tell unix we are goint to use the terminal */
	ttopen();

	/* make sure we don't over run the buffer (TOO LATE I THINK) */
	if (p >= &tcapbuf[TCAPSLEN]) {
		puts(TEXT187);
/*		     "Terminal description too big!\n" */
		meexit(1);
	}

	/* send init strings if defined */
	if (IS != NULL)
		putpad(IS);
 
	if (KS != NULL)
		putpad(KS);

	/* initialize the input buffer */
	in_init();
}
 
tcapclose()
{
	/* send end-of-keypad-transmit string if defined */
	if (KE != NULL)
		putpad(KE);
	ttclose();
}

tcapkopen()

{
	strcpy(sres, "NORMAL");
}

tcapkclose()

{
}

unsigned int extcode(c)

unsigned int c;

{
	return(c);
}

/*	TCAPGETC:	Get on character.  Resolve and setup all the
			appropriate keystroke escapes as defined in
			the comments at the beginning of input.c
*/

int tcapgetc()

{
	int c;		/* current extended keystroke */

	/* if there are already keys waiting.... send them */
	if (in_check())
		return(in_get());

	/* otherwise... get the char for now */
	c = get1key();

	/* unfold the control bit back into the character */
	if (CTRL & c)
		c = (c & ~ CTRL) - '@';

	/* fold the event type into the input stream as an escape seq */
	if ((c & ~255) != 0) {
		in_put(0);		/* keyboard escape prefix */
		in_put(c >> 8);		/* event type */
		in_put(c & 255);	/* event code */
		return(tcapgetc());
	}

	return(c);
}

/*	GET1KEY:	Get one keystroke. The only prefixs legal here
			are the SPEC and CTRL prefixes.

	Note:

		Escape sequences that are generated by terminal function
		and cursor keys could be confused with the user typing
		the default META prefix followed by other chars... ie

		UPARROW  =  <ESC>A   on some terminals...
		apropos  =  M-A

		The difference is determined by measuring the time between
		the input of the first and second character... if an <ESC>
		is types, and is not followed by another char in 1/30 of
		a second (think 300 baud) then it is a user input, otherwise
		it was generated by an escape sequence and should be SPECed.
*/

int PASCAL NEAR get1key()

{
	register int c;
	register int index;	/* index into termcap binding table */
	char *sp;
#if	BSD | V7 | HPUX
	int fdset;
	struct timeval timeout;
#endif
	char cseq[10];		/* current sequence being parsed */

	c = ttgetc();

	/* if it is not an escape character */
	if (c != 27)
	        return(c);

	/* process a possible escape sequence */
	/* set up to check the keyboard for input */
#if	BSD | V7 | HPUX
	fdset = 1;
	timeout.tv_sec = 0;
	timeout.tv_usec = 35000L;

	/* check to see if things are pending soon */
	if (kbdmode != PLAY &&
		select(1, &fdset, (int *)NULL, (int *)NULL, &timeout) == 0)
		return(CTRL | '[');
#endif

#if XENIX | SUNOS
	if ((kbdmode != PLAY) && (rdchk(0) <= 0)) {
		nap(35L);
		if (rdchk(0) <= 0)
			return(CTRL | '[');
	}
#endif

#if	USG | SMOS
	/* we don't know how to do this check for a pending char within
	   1/30th of a second machine independantly in the general System V
	   case.... so we don't */
	if (kbdmode != PLAY)
		return(CTRL | '[');
#endif

	/* a key is pending within 1/30 of a sec... its an escape sequence */
	cseq[0] = 27;
	sp = &cseq[1];
	while (sp < &cseq[6]) {
		c = ttgetc();
		*sp++ = c;
		*sp = 0;
		for (index = 0; index < NTBINDS; index++) {
			if (strcmp(cseq, ttable[index].p_seq) == 0)
				return(ttable[index].p_code);
		}
	}
	return(SPEC | 0);
}

tcapmove(row, col)
register int row, col;
{
	putpad(tgoto(CM, col, row));
}

tcapeeol()
{
	putpad(CE);
}

tcapeeop()
{
	putpad(CL);
}

tcaprev(state)		/* change reverse video status */

int state;		/* FALSE = normal video, TRUE = reverse video */

{
/*	static int revstate = FALSE;*/

	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else
		if (SE != NULL)
			putpad(SE);
}

tcapcres()	/* change screen resolution */

{
	return(TRUE);
}

spal(dummy)	/* change palette string */

{
	/*	Does nothing here	*/
}

#if	COLOR
tcapfcol()	/* no colors here, ignore this */
{
}

tcapbcol()	/* no colors here, ignore this */
{
}
#endif

tcapbeep()
{
	ttputc(BEL);
}

putpad(str)
char	*str;
{
	tputs(str, 1, ttputc);
}

putnpad(str, n)
char	*str;
{
	tputs(str, n, ttputc);
}


#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else

hello()
{
}

#endif
