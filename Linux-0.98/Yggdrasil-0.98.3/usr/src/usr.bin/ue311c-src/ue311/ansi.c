/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * termio.c. It compiles into nothing if not an ANSI device.
 *
 * 12-Dec-1989	KAM
 * If we're on VMS, leave the keypad turned on if the terminal is set to
 * application keypad mode.
 */

#define	termdef	1			/* don't define term external */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if     ANSI

#if	PROTO
int PASCAL NEAR fnclabel(int f, int n);
int PASCAL NEAR readparam( int *v);
void PASCAL NEAR dobbnmouse(void);
void PASCAL NEAR docsi( int oh);
void PASCAL NEAR ttputs(char *string);
#else
int PASCAL NEAR fnclabel();
int PASCAL NEAR readparam();
void PASCAL NEAR dobbnmouse();
void PASCAL NEAR docsi();
void PASCAL NEAR ttputs();
#endif

#if VMS
#include ttdef
#include tt2def

/*
	This structure, along with ttdef.h, is good for manipulating
	terminal characteristics.
*/
typedef struct
{/* Terminal characteristics buffer */
    unsigned char class, type;
    unsigned short width;
    unsigned tt1 : 24;
    unsigned char page;
    unsigned long tt2;
} TTCHAR;
extern NOSHARE TTCHAR orgchar;			/* Original characteristics */
#endif

#define NROW    25                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

/* Forward references.          */
extern int PASCAL NEAR ansimove();
extern int PASCAL NEAR ansieeol();
extern int PASCAL NEAR ansieeop();
extern int PASCAL NEAR ansibeep();
extern int PASCAL NEAR ansiopen();
extern int PASCAL NEAR ansirev();
extern int PASCAL NEAR ansiclose();
extern int PASCAL NEAR ansikopen();
extern int PASCAL NEAR ansikclose();
extern int PASCAL NEAR ansicres();
extern int PASCAL NEAR ansiparm();
extern int PASCAL NEAR ansigetc();

#if	COLOR
extern int PASCAL NEAR ansifcol();
extern int PASCAL NEAR ansibcol();

static int cfcolor = -1;	/* current forground color */
static int cbcolor = -1;	/* current background color */

#if	AMIGA
/* Apparently the AMIGA does not follow the ANSI standards as
 * regards to colors....maybe because of the default pallette
 * settings?  Color translation table needed.
 */

int coltran[16] = {2,  3,  5,  7, 0,  4,  6,  1,
		   8, 12, 10, 14, 9, 13, 11, 15};
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
NOSHARE TERM term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	0, 0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        ansiopen,
        ansiclose,
	ansikopen,
	ansikclose,
        ansigetc,
        ttputc,
        ttflush,
        ansimove,
        ansieeol,
        ansieeop,
        ansieeop,
        ansibeep,
	ansirev,
	ansicres
#if	COLOR
	, ansifcol,
	ansibcol
#endif
};

#if	COLOR
PASCAL NEAR ansifcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+30);
#else
	ansiparm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

PASCAL NEAR ansibcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+40);
#else
	ansiparm(color+40);
#endif
	ttputc('m');
        cbcolor = color;
}
#endif

PASCAL NEAR ansimove(row, col)
{
        ttputc(ESC);
        ttputc('[');
        ansiparm(row+1);
        ttputc(';');
        ansiparm(col+1);
        ttputc('H');
}

PASCAL NEAR ansieeol()
{
        ttputc(ESC);
        ttputc('[');
        ttputc('K');
}

PASCAL NEAR ansieeop()
{
#if	COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('[');
        ttputc('J');
}

PASCAL NEAR ansirev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	int ftmp, btmp;		/* temporaries for colors */
#endif

	ttputc(ESC);
	ttputc('[');
	ttputc(state ? '7': '0');
	ttputc('m');
#if	COLOR
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif
}

PASCAL NEAR ansicres()	/* change screen resolution */

{
	return(TRUE);
}

#if	PROTO
PASCAL NEAR spal(char *dummy)		/* change pallette settings */
#else
PASCAL NEAR spal(dummy)		/* change pallette settings */

char *dummy;
#endif

{
	/* none for now */
}

PASCAL NEAR ansibeep()
{
        ttputc(BEL);
        ttflush();
}

PASCAL NEAR ansiparm(n)
register int    n;
{
        register int q,r;

        q = n/10;
        if (q != 0) {
		r = q/10;
		if (r != 0) {
			ttputc((r%10)+'0');
		}
		ttputc((q%10) + '0');
        }
        ttputc((n%10) + '0');
}

PASCAL NEAR ansiopen()
{
#if     V7 | USG | HPUX | BSD | SUN | XENIX
        register char *cp;
        char *getenv();

        if ((cp = getenv("TERM")) == NULL) {
                puts(TEXT4);
/*                   "Shell variable TERM not defined!" */
                meexit(1);
        }
        if (strcmp(cp, "vt100") != 0 && strcmp(cp,"vt200") != 0 &&
					strcmp(cp,"vt300") != 0) {
                puts(TEXT5);
/*                   "Terminal type not 'vt100'!" */
                meexit(1);
        }
#endif
#if	MOUSE & ( V7 | USG | HPUX | BSD | SUN | XENIX | VMS)
/*
	If this is an ansi terminal of at least DEC level
	2 capability, some terminals of this level, such as the "Whack"
	emulator, the VWS terminal emulator, and some versions of XTERM,
	support access to the workstation mouse via escape sequences.  In
	addition, any terminal that conforms to level 2 will, at very least,
	IGNORE the escape sequences for the mouse.
*/
	{
	char *s;

	s = getenv( "MICROEMACS$MOUSE_ENABLE");
	if( !s) s = "\033[1)u\033[1;3'{\033[1;2'z";
	ttputs( s);
	}
#endif
	strcpy(sres, "NORMAL");
	revexist = TRUE;
        ttopen();

#if	KEYPAD
	ttputc(ESC);
	ttputc('=');
#endif
}

PASCAL NEAR ansiclose()

{
#if	COLOR
	ansifcol(7);
	ansibcol(0);
#endif
#if	MOUSE & ( V7 | USG | HPUX | BSD | SUN | XENIX | VMS)
	{
		char *s;

		s = getenv( "MICROEMACS$MOUSE_DISABLE");

		if( !s)		/* Regular DEC workstation */
			s = "\033[0'{\033[0;0'z";
		ttputs( s);
	}
#endif
#if	KEYPAD
#if 	VMS
	if ((orgchar.tt2 & TT2$M_APP_KEYPAD)==0)
#endif
	{
	    ttputc(ESC);
	    ttputc('>');
	}
#endif
	ttclose();
}

PASCAL NEAR ansikopen()	/* open the keyboard (a noop here) */

{
}

PASCAL NEAR ansikclose()	/* close the keyboard (a noop here) */

{
}

#if     V7 | USG | HPUX | BSD | SUN | XENIX | VMS
/***
 *  ttputs  -  Send a string to ttputc
 *
 *  Nothing returned
 ***/
void PASCAL NEAR ttputs(string)
char * string;				/* String to write		*/
{
	if (string)
		while (*string != '\0')
			ttputc(*string++);
}

/*
	On the "real" ansi terminals, used on "mainframe" type
	terminal/CPU connections of the above operating systems, we do
	conversion from VT100/VT200 style function keys into the Emacs
	standard key sequence form.
*/
static unsigned char inbuffer[ 10];
static int inpos=0;
static int inlen=0;

NOSHARE int mouserow, mousecol;

int PASCAL NEAR readparam( v)	/* Read an ansi parameter */
int *v;	/* Place to put parameter value */
{
    int ch;

    *v = 0;
    for(;;)
    { /* Read in a number */
   	ch = ttgetc();
	if( ch>='0' && ch<='9') *v = 10 * *v + (ch - '0');
	else return( ch);
    }
}

/*
 * Handle the CSI (<esc>[) and SS3 (<esc>O) sequences.
 * Static arrays are set up to translate the ANSI escape sequence
 * into the MicroEMACS keycode.
 *
 * The 'x' in the arrays keypad[] and dec_fnkey[] are merely placeholders.
 */
void PASCAL NEAR docsi( oh)
int oh;
{
    static char crsr[4] = {'P', 'N', 'F', 'B'};
    static char keypad[14] = {',', '-', '.', 'x', '0', '1', '2', '3',
			      '4', '5', '6', '7', '8', '9'};

    static char dec_fnkey[32] = {'x', 'S', 'C', 'D', '@', 'Z', 'V', 'x',
    				 'x', '1', '2', '3', '4', '5', '6', '7',
				 '8', '9', '0', 'x', '1', '2', '3', '4',
				 'x', '5', '6', 'x', '7', '8', '9', '0',
				};

    unsigned int params[ 5];
    int i;
    unsigned int ch;

    params[ 0] = params[ 1] = params[ 2] = params[ 3] = params[ 4] = 0;
    for( i=0;;)
    {
	ch = readparam( &params[ i]);
	if( ch >= '@')
	{ /* This ends the sequence, check for the ones we care about */
	    mousecol = params[ 0];
	    mouserow = params[ 1];
	    if( ch == 'R' && oh != 'O')
	    {	/* Cursor pos report */
		inbuffer[ inlen++] = 0x0;
		inbuffer[ inlen++] = MOUS>>8;
		inbuffer[ inlen++] = mouserow;
		inbuffer[ inlen++] = mousecol;
		inbuffer[ inlen++] = '1';
	    }
	    else if( ch == '~')
	    {/* LK201 function key */
		inbuffer[ inlen++] = 0x0;
		if (params[0] > 8) params[0] -= 3;
		if (params[0] > 18)
		    inbuffer[ inlen++] = (SHFT|SPEC)>>8;
		else
		    inbuffer[ inlen++] = SPEC>>8;
		inbuffer[ inlen++] = dec_fnkey[ params[ 0]];
	    }
	    else if( ch == 'w' && oh != 'O')
	    { /* mouse report */
		mousecol = params[ 3]-1;
		mouserow = params[ 2]-1;
		inbuffer[ inlen++] = 0x0;
		inbuffer[ inlen++] = MOUS>>8;
		inbuffer[ inlen++] = mousecol;
		inbuffer[ inlen++] = mouserow;
		inbuffer[ inlen++] = ('a'-2)+params[ 0];
	    }
	    else if( ch == 'd' && oh != 'O')
	    { /* Pixette mouse report */
		mousecol = params[ 0]-1;
		mouserow = params[ 1]-1;
		inbuffer[ inlen++] = 0x0;
		inbuffer[ inlen++] = MOUS>>8;
		inbuffer[ inlen++] = mousecol;
		inbuffer[ inlen++] = mouserow;
		inbuffer[ inlen++] = ('a'-2)+params[ 2];
	    }
	    else /* Ordinary keypad or arrow key */
	    {
		inbuffer[ inlen++] = 0x0;
		if( ch <= 'D' && ch >= 'A') /* Arrow keys.*/
		{
		    inbuffer[ inlen++] = (SPEC)>>8;
		    inbuffer[ inlen++] = crsr[ ch - 'A'];
		}
		else if (ch <= 'S' && ch >= 'P')    /* PF keys.*/
		{
		    inbuffer[ inlen++] = (SPEC|CTRL)>>8;
		    inbuffer[ inlen++] = ch - ('P' - '1');
		}
		else
		{
		    inbuffer[ inlen++] = (ALTD)>>8;
		    if (ch == 'M')
			inbuffer[ inlen++] = 'E';
		    else
			inbuffer[ inlen++] = keypad[ ch - 'l'];
		}
	    }
	    return;
	}
	if( i<5) i++;
    }
}

void PASCAL NEAR dobbnmouse()
{
	int params[ 5];
	int i, ch;
	static char prev = 0;
	int event, flags;

	params[ 0] = 0;
	params[ 1] = 0;
	params[ 2] = 0;
	params[ 3] = 0;
	params[ 4] = 0;
	for( i=0;;)
	{
		/* Is the sequence finished?
		 * check for the ones we care about.
		 */
		if( (ch = readparam( &params[ i])) >= '@')
		{
			mousecol = (params[ 1]+4)/9;
			mouserow = (1015-params[ 2])/16;
			flags = params[ 3] & 7;
			event = flags ^ prev;
			prev = flags;
			flags = ((flags & event) == 0);
			event = flags + (6 - (event & 6));
			if( ch == 'c')
			{	/* Cursor pos report */
				inbuffer[ inlen++] = 0x0;
				inbuffer[ inlen++] = MOUS>>8;
				inbuffer[ inlen++] = mousecol;
				inbuffer[ inlen++] = mouserow;
				inbuffer[ inlen++] = ('a'-2)+event;
			}
			return;
		}
	if( i<5) i++;
	}
}

/*
 *	Read a keystroke from the terminal.  Interpret escape sequences
 *	that come from function keys, mouse reports, and cursor location
 *	reports, and return them using Emacs's coding of these events.
 */
PASCAL NEAR ansigetc()
{
    int ch;

    for(;;)
    {/* Until we get a character to return */
	if( inpos < inlen)
	{ /* Working out a multi-byte input sequence */
	    return( inbuffer[ inpos++]);
	}
	inpos = 0;
	inlen = 0;
	ch = ttgetc();
	if( ch == 27)
	{ /* ESC, see if sequence follows */
/*
	If the "terminator" is ESC, and if we are currently reading a
	string where the terminator is ESC, then return the ESC and do
	not allow function keys or mouse to operate properly.  This
	makes VT100 users much happier.
*/
	    ch = ttgetc_shortwait();
	    if( ch < 0) return( 27);	/* Wasn't a function key */
	    if( ch == '[') docsi( ch);
	    else if( ch == ':') dobbnmouse();
	    else if( ch == 'O') docsi( ch);
	    else
	    { /* This isn't an escape sequence, return it unmodified */
		inbuffer[ inlen++] = ch;
		return( 27);
	    }
	}
	else if( ch == 27+128) docsi( ch);
	else return( ch);
    }
}
#else
PASCAL NEAR  ansigetc() {return( ttgetc());}
#endif

#if	FLABEL
int PASCAL NEAR fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
ansihello()
{
}
#endif
