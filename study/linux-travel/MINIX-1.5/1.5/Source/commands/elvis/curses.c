/* curses.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the functions & variables needed for a tiny subset of
 * curses.  The principle advantage of this version of curses is its
 * extreme speed.  Disadvantages are potentially larger code, few supported
 * functions, limited compatibility with full curses, and only stdscr.
 */

#include <sys/types.h>
#include "curses.h"
#ifdef M_SYSV
# include	<termio.h>
#else
# include	<sgtty.h>
#endif
#include <signal.h>

extern char	*malloc();
extern char	*getenv();

extern trapint();

/* variables, publicly available & used in the macros */
short	ospeed;		/* speed of the tty, eg B2400 */
WINDOW	*stdscr;	/* pointer into kbuf[] */
WINDOW	kbuf[KBSIZ];	/* a very large output buffer */
int	LINES;		/* :li#: number of rows */
int	COLS;		/* :co#: number of columns */
int	AM;		/* :am:  boolean: auto margins? */
int	PT;		/* :pt:  boolean: physical tabs? */
char	*VB;		/* :vb=: visible bell */
char	*UP;		/* :up=: move cursor up */
char	*SC;		/* :sc=: save cursor position & char attributes */
char	*RC;		/* :rc=: resore cursor position & char attributes */
char	*SO;		/* :so=: standout start */
char	*SE;		/* :se=: standout end */
char	*US = "";	/* :us=: underline start */
char	*UE = "";	/* :ue=: underline end */
char	*VB_s = "";	/* :VB=: bold start */
char	*VB_e = "";	/* :Vb=: bold end */
char	*AS;		/* :as=: alternate (italic) start */
char	*AE;		/* :ae=: alternate (italic) end */
char	*CM;		/* :cm=: cursor movement */
char	*CE;		/* :ce=: clear to end of line */
char	*CL;		/* :cl=: home cursor & clear screen */
char	*CD;		/* :cd=: clear to end of screen */
char	*AL;		/* :al=: add a line */
char	*DL;		/* :dl=: delete a line */
char	*SR;		/* :sr=: scroll reverse */
char	*KU;		/* :ku=: key sequence sent by up arrow */
char	*KD;		/* :kd=: key sequence sent by down arrow */
char	*KL;		/* :kl=: key sequence sent by left arrow */
char	*KR;		/* :kr=: key sequence sent by right arrow */
char	*PU;		/* :PU=: key sequence sent by PgUp key */
char	*PD;		/* :PD=: key sequence sent by PgDn key */
char	*HM;		/* :HM=: key sequence sent by Home key */
char	*EN;		/* :EN=: key sequence sent by End key */
char	*IM;		/* :im=: insert mode start */
char	*IC = "";	/* :ic=: insert the following character */
char	*EI;		/* :ei=: insert mode end */
char	*DC;		/* :dc=: delete a character */
char	*aend = "";	/* end an attribute -- either UE or VB_e */
char	ERASEKEY;	/* backspace key taken from ioctl structure */

#ifdef M_SYSV
static struct termio	oldtermio;	/* original tty mode */
static struct termio	newtermio;	/* raw/noecho tty mode */
#else
static struct sgttyb	oldsgttyb;	/* original tty mode */
static struct sgttyb	newsgttyb;	/* raw/nl/noecho tty mode */
#endif

static char	*capbuf;	/* capability string buffer */


initscr()
{
	/* make sure TERM variable is set */
	if (!getenv("TERM"))
	{
		printf("initscr: environment variable TERM must be set\n");
		exit(1);
	}

	/* start termcap stuff */
	starttcap();

	/* create stdscr and curscr */
	stdscr = kbuf;

	/* change the terminal mode to raw/noecho */
#ifdef M_SYSV
	ioctl(2, TCGETA, &oldtermio);
#else
	ioctl(2, TIOCGETP, &oldsgttyb);
#endif
	resume_curses(TRUE);
}


endwin()
{
	/* flush any last changes */
	refresh();

	/* change the terminal mode back the way it was */
	suspend_curses();
}

suspend_curses()
{
	/* change the terminal mode back the way it was */
#ifdef M_SYSV
	ioctl(2, TCSETAF, &oldtermio);
#else
	ioctl(2, TIOCSETP, &oldsgttyb);
#endif
}

resume_curses(quietly)
	int	quietly;
{	
	char	*src, *dest;

	/* change the terminal mode to raw/noecho */
#ifdef M_SYSV
	newtermio = oldtermio;
	newtermio.c_iflag &= (IXON|IXOFF|IXANY|ISTRIP|IGNBRK);
	newtermio.c_oflag &= ~OPOST;
	newtermio.c_lflag &= XCLUDE;
	newtermio.c_cc[VEOF] = 1; /* minimum # characters to read */
	newtermio.c_cc[VEOL] = 2; /* timeout after 0.2 seconds */
	ioctl(2, TCSETAF, &newtermio);

	ospeed = (oldtermio.c_cflag & CBAUD);
	ERASEKEY = oldtermio.c_cc[VERASE];
#else
	newsgttyb = oldsgttyb;
	newsgttyb.sg_flags |= CBREAK;
	newsgttyb.sg_flags &= ~(CRMOD|ECHO|XTABS);
	ioctl(2, TIOCSETP, &newsgttyb);

	ospeed = oldsgttyb.sg_ospeed;
	ERASEKEY = oldsgttyb.sg_erase;
#endif

	if (quietly) return;

	signal(SIGINT, SIG_IGN);

	/* Wait for a key from the user */
	for (dest = kbuf, src = tgoto(CM, 0, LINES - 1); *src; )
		*dest++ = *src++;
	for (src = SO; src && *src; )
		*dest++ = *src++;
	for (src = "[Press <RETURN> to continue]"; *src; )
		*dest++ = *src++;
	for (src = SE; src && *src; )
		*dest++ = *src++;
	write(1, kbuf, (int)(dest - kbuf));
	read(0, kbuf, 20); /* in RAW mode, so <20 is very likely */


	/* !!! special processing of the : key for Elvis' VI mode */
	if (kbuf[0] == ':')
	{
		ungetkey(':');
	}

	signal(SIGINT, trapint);
}

static lacking(s)
	char	*s;
{
	write(2, "This termcap entry lacks the :", 30);
	write(2, s, 2);
	write(2, "=: capability\n", 14);
	exit(1);
}

starttcap()
{
	char	*str;
	static char	cbmem[800];
#define MUSTHAVE(T,s)	if (!(T = tgetstr(s, &capbuf))) lacking(s)
#define MAYHAVE(T,s)	if (str = tgetstr(s, &capbuf)) T = str
#define PAIR(T,U,sT,sU)	T=tgetstr(sT,&capbuf);U=tgetstr(sU,&capbuf);if (!T||!U)T=U=""

	/* allocate memory for capbuf */
	capbuf = cbmem;
	
	/* get the termcap entry */
	if (tgetent(kbuf, getenv("TERM")) != 1)
	{
		perror("tgetent");
		exit(1);
	}

	/* get strings */
	MUSTHAVE(UP, "up");
	MAYHAVE(SC, "sc");
	MAYHAVE(RC, "rc");
	MAYHAVE(VB, "vb");
	MUSTHAVE(CM, "cm");
	PAIR(SO, SE, "so", "se");
	if (tgetnum("ug") <= 0)
	{
		PAIR(US, UE, "us", "ue");
		PAIR(VB_s, VB_e, "VB", "Vb");

		/* get italics, or have it default to underline */
		PAIR(AS, AE, "as", "ae");
		if (!*AS)
		{
			AS = US;
			AE = UE;
		}
	}
	MAYHAVE(AL, "al");
	MAYHAVE(DL, "dl");
	MUSTHAVE(CE, "ce");
	MUSTHAVE(CL, "cl");
	MAYHAVE(CD, "cd");
	MAYHAVE(SR, "sr");
	PAIR(IM, EI, "im", "ei");
	MAYHAVE(IC, "ic");
	MAYHAVE(DC, "dc");

	/* other termcap stuff */
	AM = tgetflag("am");
	PT = tgetflag("pt");
	getsize(0);

	/* Key sequences */
	MAYHAVE(KU, "ku");
	MAYHAVE(KD, "kd");
	MAYHAVE(KL, "kl");
	MAYHAVE(KR, "kr");
	MAYHAVE(PU, "PU");
	MAYHAVE(PD, "PD");
	MAYHAVE(HM, "HM");
	MAYHAVE(EN, "EN");

#undef MUSTHAVE
#undef MAYHAVE
#undef PAIR
}


/* This function gets the window size.  It uses the TIOCGWINSZ ioctl call if
 * your system has it, or tgetnum("li") and tgetnum("co") if it doesn't.
 * This function is called once during initialization, and thereafter it is
 * called whenever the SIGWINCH signal is sent to this process.
 */
getsize(signo)
	int	signo;
{
#ifdef TIOCGWINSZ
	struct winsize size;
	int	lines;
	int	cols;
#endif

#ifdef SIGWINCH
	/* reset the signal vector */
	signal(SIGWINCH, getsize);
#endif

	/* get the window size, one way or another. */
#ifdef TIOCGWINSZ
	lines = cols = 0;
	if (ioctl(2, TIOCGWINSZ, &size) >= 0)
	{
		lines = size.ws_row;
		cols = size.ws_col;
	}
	if ((lines == 0 || cols == 0) && signo == 0)
	{
		LINES = tgetnum("li");
		COLS = tgetnum("co");
	}
	if (lines >= 2 && cols >= 30)
	{
		LINES = lines;
		COLS = cols;
	}
#else
	LINES = tgetnum("li");
	COLS = tgetnum("co");
#endif

	/* Make sure we got values that we can live with */
	if (LINES < 2 || COLS < 30)
	{
		write(2, "Screen too small\n", 17);
		endwin();
		exit(2);
	}

	/* !!! copy the new values into the corresponding elvis options */
	{
		extern char	o_columns[], o_lines[];

		*o_columns = COLS;
		*o_lines = LINES;
	}
}


/* This is a function version of addch() -- it is used by tputs() */
int faddch(ch)
	int	ch;
{
	addch(ch);
}

#ifdef CRUNCH
/* These functions are equivelent to the macros of the same names... */

void qaddstr(str)
	char	*str;
{
	register char *s_, *d_;

	for (s_=(str), d_=stdscr; *d_++ = *s_++; )
	{
	}
	stdscr = d_ - 1;
}

void attrset(a)
	int	a;
{
	tputs(aend, 1, faddch);
	if (a == A_BOLD)
	{
		tputs(VB_s, 1, faddch);
		aend = VB_e;
	}
	else if (a == A_UNDERLINE)
	{
		tputs(US, 1, faddch);
		aend = UE;
	}
	else if (a == A_ALTCHARSET)
	{
		tputs(AS, 1, faddch);
		aend = AE;
	}
	else
	{
		aend = "";
	}
}


void insch(ch)
	int	ch;
{
	if (IM)
		tputs(IM, 1, faddch);
	tputs(IC, 1, faddch);
	qaddch(ch);
	if (EI)
		tputs(EI, 1, faddch);
}
#endif
