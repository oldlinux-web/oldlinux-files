/* curses.h */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This is the header file for a small, fast, fake curses package */

/* termcap stuff */
extern char	*tgoto();
extern char	*tgetstr();

/* faddch() is a function.  a pointer to it is passed to tputs() */
extern int	faddch();

/* data types */
#define ushort	unsigned short
#define WINDOW	char

/* CONSTANTS & SYMBOLS */
#define TRUE		1
#define FALSE		0
#define A_NORMAL	0
#define A_STANDOUT	1
#define A_BOLD		2
#define A_UNDERLINE	3
#define A_ALTCHARSET	4
#define KBSIZ		6144

/* extern variables, defined in curses.c */
extern short	ospeed;		/* tty speed, eg B2400 */
extern WINDOW	*stdscr;	/* pointer into kbuf[] */
extern WINDOW	kbuf[KBSIZ];	/* a very large output buffer */
extern int	LINES;		/* :li#: number of rows */
extern int	COLS;		/* :co#: number of columns */
extern int	AM;		/* :am:  boolean: auto margins? */
extern int	PT;		/* :pt:  boolean: physical tabs? */
extern char	*VB;		/* :vb=: visible bell */
extern char	*UP;		/* :up=: move cursor up */
extern char	*SC;		/* :sc=: save cursor posn & char attributes */
extern char	*RC;		/* :rc=: restore cursor posn & char attr */
extern char	*SO;		/* :so=: standout start */
extern char	*SE;		/* :se=: standout end */
extern char	*US;		/* :us=: underline start */
extern char	*UE;		/* :ue=: underline end */
extern char	*VB_s;		/* :VB=: bold start */
extern char	*VB_e;		/* :Vb=: bold end */
extern char	*AS;		/* :as=: alternate (italic) start */
extern char	*AE;		/* :ae=: alternate (italic) end */
extern char	*CM;		/* :cm=: cursor movement */
extern char	*CE;		/* :ce=: clear to end of line */
extern char	*CL;		/* :cl=: home cursor & clear screen */
extern char	*CD;		/* :cd=: clear to end of screen */
extern char	*AL;		/* :al=: add a line */
extern char	*DL;		/* :dl=: delete a line */
extern char	*SR;		/* :sr=: scroll reverse */
extern char	*KU;		/* :ku=: sequence sent by up key */
extern char	*KD;		/* :kd=: sequence sent by down key */
extern char	*KL;		/* :kl=: sequence sent by left key */
extern char	*KR;		/* :kr=: sequence sent by right key */
extern char	*PU;		/* :PU=: key sequence sent by PgUp key */
extern char	*PD;		/* :PD=: key sequence sent by PgDn key */
extern char	*HM;		/* :HM=: key sequence sent by Home key */
extern char	*EN;		/* :EN=: key sequence sent by End key */
extern char	*IM;		/* :im=: insert mode start */
extern char	*IC;		/* :ic=: insert following char */
extern char	*EI;		/* :ei=: insert mode end */
extern char	*DC;		/* :dc=: delete a character */
extern char	*aend;		/* end an attribute -- either UE or VB_e */
extern char	ERASEKEY;	/* taken from the ioctl structure */

#define _addCR		(stdscr[-1] == '\n' ? qaddch('\r') : 0)
#define qaddch(ch)	(*stdscr++ = (ch))
#define addch(ch)	if (qaddch(ch) == '\n') qaddch('\r'); else
#ifdef CRUNCH
extern void attrset();
extern void insch();
extern void qaddstr();
#else
# define attrset(a)	{tputs(aend, 1, faddch); if ((a) == A_BOLD)\
				{tputs(VB_s, 1, faddch); aend = VB_e;}\
			 else if ((a) == A_UNDERLINE)\
				{tputs(US, 1, faddch); aend = UE;}\
			 else if ((a) == A_ALTCHARSET)\
				{tputs(AS, 1, faddch); aend = AE;}\
			 else aend = "";}
# define insch(ch)	{if (IM) tputs(IM, 1, faddch);\
			 tputs(IC, 1, faddch);\
			 qaddch(ch);\
			 if (EI) tputs(EI, 1, faddch);\
			}
# define qaddstr(str)	{register char *s_, *d_;\
			 for (s_=(str), d_=stdscr; *d_++ = *s_++; ){}\
			 stdscr = d_ - 1;\
			}
#endif
#define addstr(str)	{qaddstr(str); _addCR;}
#define move(y,x)	tputs(tgoto(CM, x, y), 1, faddch);
#define wprintw		for (*stdscr = 1; *stdscr; stdscr += strlen(stdscr), _addCR, *stdscr = 0)\
				sprintf
#define mvaddch(y,x,ch)	{move(y,x); addch(ch);}
#define refresh()	wrefresh(stdscr)
#define wrefresh(w)	if ((w) != kbuf) {write(1, kbuf, (int)((w) - kbuf)); (w) = kbuf;}else;
#define wqrefresh(w)	if ((w) - kbuf > 2000)  {write(1, kbuf, (int)((w) - kbuf)); (w) = kbuf;}else;
#define standout()	tputs(SO, 1, faddch)
#define standend()	tputs(SE, 1, faddch)
#define clear()		tputs(CL, 1, faddch)
#define clrtoeol()	tputs(CE, 1, faddch)
#define clrtobot()	tputs(CD, 1, faddch)
#define insertln()	tputs(AL, LINES, faddch)
#define deleteln()	tputs(DL, LINES, faddch)
#define delch()		tputs(DC, COLS, faddch)
#define scrollok(w,b)
#define raw()
#define echo()
#define cbreak()
#define noraw()
#define noecho()
#define nocbreak()

