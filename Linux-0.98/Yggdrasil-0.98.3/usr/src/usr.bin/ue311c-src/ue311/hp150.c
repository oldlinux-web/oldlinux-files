/*
 * The routines in this file provide support for HP150 screens
 * and routines to access the Keyboard through KEYCODE mode.
 * It compiles into nothing if not an HP150 screen device.
 * added by Daniel Lawrence
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#if     HP150

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	15			/* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

extern int PASCAL NEAR  openhp();	/* Forward references.          */
extern int PASCAL NEAR	hpflush();
extern int PASCAL NEAR  closehp();
extern int PASCAL NEAR	hp15kopen();
extern int PASCAL NEAR	hp15kclose();
extern int PASCAL NEAR  hp15move();
extern int PASCAL NEAR  hp15eeol();
extern int PASCAL NEAR  hp15eeop();
extern int PASCAL NEAR  hp15beep();
extern int PASCAL NEAR	gethpkey();
extern int PASCAL NEAR	hp15rev();
extern int PASCAL NEAR	hp15cres();
#if	COLOR
extern int PASCAL NEAR	hp15fcol();
extern int PASCAL NEAR	hp15bcol();
#endif

PASCAL NEAR hp15parm();
PASCAL NEAR rawon();
PASCAL NEAR rawoff();
PASCAL NEAR ckeyoff();
PASCAL NEAR ckeyon();
PASCAL NEAR agios();
PASCAL NEAR keycon();
PASCAL NEAR keycoff();
PASCAL NEAR defkey();
PASCAL NEAR undefkey();
PASCAL NEAR dsplbls();

/*	Some needed locals	*/

union REGS r;		/* register set for bios and dos (AGIOS) calls */
int capslock = 0;	/* caps lock flag */

int break_flag;		/* state of MSDOS control break processing */
/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	0, 0,
	openhp,
        closehp,
	hp15kopen,
	hp15kclose,
	gethpkey,
        ttputc,
        hpflush,
        hp15move,
        hp15eeol,
        hp15eeop,
        hp15eeop,
        hp15beep,
        hp15rev,
        hp15cres
#if	COLOR
	, hp15fcol,
	hp15bcol
#endif
};

PASCAL NEAR hp15move(row, col)
{
        ttputc(ESC);
        ttputc('&');
        ttputc('a');
        hp15parm(col);
        ttputc('c');
        hp15parm(row);
        ttputc('R');
}

PASCAL NEAR hpflush()

{

}

PASCAL NEAR hp15eeol()
{
        ttputc(ESC);
        ttputc('K');
}

PASCAL NEAR hp15eeop()
{
        ttputc(ESC);
        ttputc('J');
}

PASCAL NEAR hp15rev(status)	/* change the reverse video status */

int status;	/* TRUE = on, FALSE = off */

{
	ttputc(ESC);
	ttputc('&');
	ttputc('d');
	ttputc((status != FALSE) ? 'B': '@');
}

PASCAL NEAR hp15cres()	/* change screen resolution */

{
	return(TRUE);
}

PASCAL NEAR spal()		/* change pallette register */

{
	/*   not here */
}

PASCAL NEAR hp15beep()
{
        ttputc(BEL);
        ttflush();
}

PASCAL NEAR hp15parm(n)

register int    n;

{
        register int    q;

        q = n/10;
        if (q != 0)
                hp15parm(q);
        ttputc((n%10) + '0');
}

#if	COLOR
PASCAL NEAR hp15fcol()	/* we really can't do colors here, so just ignore it */
{
}

PASCAL NEAR hp15bcol()	/* we really can't do colors here, so just ignore it */
{
}
#endif

PASCAL NEAR gethpkey()	/* get a key from the HP keyboard while in keycode mode */

{
	unsigned c;		/* character to translate */
	int devid;		/* device ID */
	int ctype;		/* type of character gotten */
	unsigned shiftb;	/* state of shift keys */
	int i;			/* index in first translation loop */

	/* return any keystrokes waiting in the
	   type ahead buffer */
	if (in_check())
		return(in_get());

	/* grab the next 4 char sequence */
next:	shiftb = ttgetc();
	devid = ttgetc();
	c = ttgetc();
	ttgetc();		/* skip null byte */

	/* make sure we are from the keyboard */
	if (devid != 192)
		goto next;

	/* if normal ascii, return it */
	if ((shiftb & 0x80) == 0) {
		if (capslock && c >= 'a' && c <= 'z')
			c -= 32;
		return(c);
	}

	/* check specifically for the caps lock key */
	if (c == 0x56) {
		capslock = ~capslock;
		goto next;
	}

	/* interpet it as an extended HP sequence */
	c = extcode(shiftb, c);

	/* if it becomes standard ascii... just return it */
	if ((c >> 8) == 0)
		return(c & 255);

	/* or return it as en extended emacs internal sequence */
	in_put(c >> 8);		/* prefix byte */
	in_put(c & 255);	/* event code byte */
	return(0);		/* extended escape sequence */
}

/*	extcode:	resolve MSDOS extended character codes
			encoding the proper sequences into emacs
			printable character specifications
*/

int extcode(shiftb, c)

unsigned shiftb;	/* shift flag */
unsigned c;		/* byte following a zero extended char byte */

{
	int sstate;	/* state of the various shift keys */

	/* remember if we are shifted */
	if ((shiftb & 0x04) != 0)
		sstate = SHFT;
	else
		sstate = 0;

	/* remember if we are alted (extended char keys) */
	if ((shiftb & 0x30) != 0)
		sstate |= ALTD;

	/* remember if we are controled */
	if ((shiftb & 0x08) != 0)
		sstate |= CTRL;

	/* function keys 1 through 9 */
	if (c >= 0 && c < 9)
		return(sstate | SPEC | c + 1 + '0');

	/* function key 10 */
	if (c == 9)
		return(sstate | SPEC | '0');

	/* function key 11 */
	if (c == 10)
		return(sstate | SPEC | 'E');

	/* function key 12 */
	if (c == 11)
		return(sstate | SPEC | 'T');

	/* some others as well */
	switch (c) {

		case 36:	return(sstate | 9);	/* tab */
		case 37:	return(sstate | 13);	/* ret */
		case 39:	return(sstate | 8);	/* backspace */
		case 48:	return(sstate | 48);	/* zero */
		case 49:	return(sstate | 49);	/* one */
		case 50:	return(sstate | 50);	/* two */
		case 51:	return(sstate | 51);	/* three */
		case 52:	return(sstate | 52);	/* four */
		case 53:	return(sstate | 53);	/* five */
		case 54:	return(sstate | 54);	/* six */
		case 55:	return(sstate | 55);	/* seven */
		case 56:	return(sstate | 56);	/* eight */
		case 57:	return(sstate | 57);	/* nine */
		case 80:	return(sstate | 13);	/* enter */
		case 84:	return(sstate | 27);	/* break -> ESC */
		case 85:	return(sstate | 27);	/* esc */
		case 88:	return(sstate | 24);	/* stop -> ^X */
		case 112:	return(sstate | 45);	/* N-minus */
		case 113:	return(sstate | 42);	/* N-asterisk */
		case 114:	return(sstate | 43);	/* N-plus */
		case 115:	return(sstate | 47);	/* N-slash */
		case 116:	return(sstate | 44);	/* N-comma */
		case 117:	return(sstate | 13);	/* N-enter */
		case 118:	return(sstate | 9);	/* N-tab */
		case 119:	return(sstate | 46);	/* N-period */

		case 44:
		case 45:
		case 110:	return(sstate | SPEC | '<');	/* HOME */

		case 32:
		case 41:
		case 101:	return(sstate | SPEC | 'P');	/* cursor up */

		case 47:	return(sstate | SPEC | 'Z');	/* page up */

		case 35:
		case 42:
		case 97:	return(sstate | SPEC | 'B');	/* cursor left */

		case 34:
		case 43:
		case 99:	return(sstate | SPEC | 'F');	/* cursor right */

		case 82:	return(sstate | SPEC | '>');	/* end */

		case 33:
		case 40:
		case 98:	return(sstate | SPEC | 'N');	/* cursor down */

		case 46:
		case 108:	return(sstate | SPEC | 'V');	/* page down */

		case 64:
		case 70:
		case 107:	return(sstate | SPEC | 'C');	/* insert */

		case 65:
		case 71:
		case 109:	return(sstate | SPEC | 'D');	/* delete */

		/* the HP has some extra keys we need to map */

		case 83:
		case 89:	return(sstate | SPEC | 'Q');	/* reformat paragraph */
		case 81:	return(sstate | CTLX | 'C');	/* shell up to system */
		case 67:	return(sstate | SPEC | CTRL | 'L'); /* center display */
		case 68:	return(sstate | CTRL | 'O');	/* open line */
		case 69:	return(sstate | CTRL | 'K');	/* Kill to end of line */
	}

	return(sstate | c);
}

PASCAL NEAR openhp()		/* open the HP150 screen for input */

{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
}

PASCAL NEAR closehp()		/* close the HP150 screen for input */

{
}

PASCAL NEAR hp15kopen()		/* open the HP150 keyboard for input */

{
	/* define key charectoristics with AGIOS call (0, 40) */
	defkey();

	/* Turn on RAW mode with MSDOS call 44h */
	rawon();

	/* Turn off Control-C checking  MS-DOS 33h */
	ckeyoff();

	/* Turn on keycode mode with AGIOS call (0,43) */
	keycon();

	/* display the application softkey labels */
	dsplbls();
}

PASCAL NEAR hp15kclose()	/* close the HP150 keyboard for input */

{
	/* define key charectoristics with AGIOS call (0, 40) */
	undefkey();
	
	/* Turn off RAW mode with MSDOS call 44h */
	rawoff();

	/* Turn on Control-C checking  MS-DOS 33h */
	ckeyon();

	/* Turn off keycode mode with AGIOS call (0,43) */
	keycoff();
}

PASCAL NEAR rawon()	/* put the HP150 keyboard into RAW mode */

{
	/* get the IO control info */

	r.x.ax = 0x4400;	/* IO ctrl get device information */
	r.x.bx = 0x0001;	/* File handle; 1 for console */
	intdos(&r, &r);		/* go fer it */

	r.h.dh = 0;		/* clear high byte for put */
	r.h.dl |= 0x20;		/* set raw bit */

	/* and put it back */

	r.x.ax = 0x4401;	/* IO ctrl put device information */
	r.x.bx = 0x0001;	/* File handle; 1 for console */
	intdos(&r, &r);		/* go fer it */
}

PASCAL NEAR rawoff()	/* put the HP150 keyboard into COOKED mode */

{
	/* get the IO control info */

	r.x.ax = 0x4400;	/* IO ctrl get device information */
	r.x.bx = 0x0001;	/* File handle; 1 for console */
	intdos(&r, &r);		/* go fer it */

	r.h.dh = 0;		/* clear high byte for put */
	r.h.dl &= 0xdf;		/* set raw bit */

	/* and put it back */

	r.x.ax = 0x4401;	/* IO ctrl put device information */
	r.x.bx = 0x0001;	/* File handle; 1 for console */
	intdos(&r, &r);		/* go fer it */
}

PASCAL NEAR ckeyoff()	/* turn control-C trapping off */

{
	/* find the current state of the control break inturrupt */
	rg.h.ah = 0x33;	/* ctrl-break check */
	rg.h.al = 0;	/* request the state of the ctrl-break check */
	intdos(&rg, &rg);
	break_flag = rg.h.dl;

	/* set the break processing off if it is on */
	if (break_flag == 1) {
		rg.h.ah = 0x33;	/* ctrl-break check */
		rg.h.al = 1;	/* set the state of the ctrl-break check */
		rg.h.dl = 0;	/* turn it off */
		intdos(&rg, &rg);
	}
}

PASCAL NEAR ckeyon()	/* turn control-C trapping on */

{
	if (break_flag == 1) {
		rg.h.ah = 0x33;	/* ctrl-break check */
		rg.h.al = 1;	/* set the state of the ctrl-break check */
		rg.h.dl = 1;	/* turn it on */
		intdos(&rg, &rg);
	}
}

#ifdef	unsigned
#undef	unsigned
#endif

PASCAL NEAR agios(buf, len)	/* perform an AGIOS call */

char *buf;	/* sequence of bytes in command */
int len;	/* length of command in bytes */

{
	r.x.ax = 0x4403;	/* I/O ctrl write */
	r.x.bx = 1;		/* console handle */
	r.x.cx = len;		/* buffer length */
	r.x.dx = (unsigned)buf;	/* buffer address */
	return(intdos(&r, &r));	/* do it */
}

PASCAL NEAR keycon()	/* turn keycode mode on */

{
	static char cmd[] = {43, 0, 1};

	return(agios(&cmd[0], 3));
}

PASCAL NEAR keycoff()	/* turn keycode mode off */

{
	static char cmd[] = {43, 0, 0};

	return(agios(&cmd[0], 3));
}

PASCAL NEAR defkey()	/* change all special keys to intercept mode */

{
	static char cmd[] = {40, 0, 2, 0, 0xfe, 0};

	return(agios(&cmd[0], 6));
}

PASCAL NEAR undefkey()	/* change all special keys to intercept mode */

{
	static char cmd[] = {40, 0, 0, 0, 0xfe, 0};

	return(agios(&cmd[0], 6));
}

PASCAL NEAR dsplbls()	/* display the application softkey labels on the screen */

{
	static char cmd[] = {11, 0};

	return(agios(&cmd[0], 2));
}

#if	FLABEL
PASCAL NEAR fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument */

{
	register int status;	/* return status */
	register int i;		/* loop index */
	char lbl[17];	/* returned label contents */
	/* AGIOS command buffer */
	static char cmd[] = {8, 0, 1, 0, 7, 7, 7, 7, 10, 0, 10, 0};
	/*                   code  key#  ptr to      top    bottom
	                                 label string  attribute */
	union {		/* union to cast ptr into AGIOS arg string */
		char *ptr;	/* pointer to arg string */
		char cstr[4];
	} ptru;

	/* must have a numeric argument */
	if (f == FALSE) {
		mlwrite(TEXT159);
/*                      "%Need function key number" */
		return(FALSE);
	}

	/* and it must be a legal key number */
	if (n < 1 || n > 8) {
		mlwrite(TEXT160);
/*                      "%Function key number out of range" */
		return(FALSE);
	}

	/* get the string to send */
	status = mlreply(TEXT161, &lbl[0], 17);
/*                       "Label contents: " */
	if (status != TRUE)
		return(status);

	/* pad the label out */
	for (i=0; i < 17; i++) {
		if (lbl[i] == 0)
			break;
	}
	for (; i < 16; i++)
		lbl[i] = ' ';
	lbl[16] = 0;

	/* set up the parameters */
	cmd[2] = n;			/* function key number */
	ptru.ptr = &lbl[0];		/* set up pointer to label string */
force:	cmd[4] = ptru.cstr[0];
	cmd[5] = ptru.cstr[1];
	cmd[6] = ptru.cstr[2];
	cmd[7] = ptru.cstr[3];

	/* and send it out */
	agios(&cmd[0], 12);
	return(TRUE);
}
#endif
#else
PASCAL NEAR h15hello()

{
}
#endif
