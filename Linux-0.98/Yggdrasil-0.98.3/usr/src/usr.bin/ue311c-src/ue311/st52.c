/*	ST52.C:	Atari ST1040/520 screen support functions
		written by lots of people (see below)

		Daniel Lawrence
		James Turner
		Jeff Lomicka
		J. C. Benoist

	Modification History:
	31-dec-87	Jeff Lomicka
	- massive changes/additions for accessories and mouse
	20-jan-87	Daniel Lawrence
	- changed code in domousekey() to pass five bytes, two zeros
	  (to tell input.c that this is a mouse event), x/ypos
	  and the mouse event type.
	may 88		Jeff Lomicka and Dan Lawrence
	- a lot of changes.  Through out aline.h, use the MWC and
	  ATARI names for things now.
	- moving the mouse to the extreme upper left brings up
	  the desk accessory menu.  EMACS won't replot on the
	  way out yet.
	- cleaned up behavior of the mouse and the cursor on exit
	26-feb-89	Daniel Lawrence
	- rewote input layer to conform to new specs in input.c
	01-may-91	Daniel Lawrence/Suggested by Allan Pratt
	- renamed atari source files to TOS.C and ST52.c
	- changed symbols ATARI => ST52 and ST520 => TOS
*/

#define termdef 1		/* don't define "term" external */

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#if	ST52

/*
	These routines provide support for the ATARI 1040ST and 520ST
using the virtual VT52 Emulator

*/

#include	<aesbind.h>
#include	<gemdefs.h>
#include	<obdefs.h>
#include	<osbind.h>
#include	<xbios.h>
#include	<linea.h>

#define NROW	50	/* Screen size. 		*/
#define NCOL	80	/* Edit if you want to. 	*/
#define MARGIN	8	/* size of minimim margin and	*/
#define SCRSIZ	64	/* scroll size for extended lines */
#define NPAUSE	300	/* # times thru update to pause */
#define BIAS	0x20	/* Origin 0 coordinate bias.	*/
#define ESC	0x1B	/* ESC character.		*/
#define SCRFONT 2	/* index of 8x16 monochrome system default font */
#define DENSIZE 50	/* # of lines in a dense screen */

/****	ST Internals definitions		*****/

/*	BIOS calls */

#define BCONSTAT	1	/* return input device status */
#define CONIN		2	/* read character from device */
#define BCONOUT 	3	/* write character to device */

/*	XBIOS calls */

#define INITMOUS	0	/* initialize the mouse 	*/
#define GETREZ		4	/* get current resolution	*/
#define SETSCREEN	5	/* set screen resolution	*/
#define SETPALETTE	6	/* set the color pallette	*/
#define SETCOLOR	7	/* set or read a color		*/
#define CURSCONF	21	/* set cursor configuration	*/
#define IKBDWS		25	/* intelligent keyboard send command */
#define KBDVBASE	34	/* get keyboard table base	*/

/*	GEMDOS calls */

#define EXEC		0x4b	/* Exec off a process */
#define CON		2	/* CON: Keyboard and screen device */

/*	Palette color definitions	*/

#define LOWPAL	"000700070770007707077777333733373773337737377777"
#define MEDPAL	"000700007777"
#define HIGHPAL "111000"

/*	ST Global definitions		*/

int initrez;			/* initial screen resolution */
int currez;			/* current screen resolution */
int gemflag;			/* were we called from the desktop? */
int mouseon;			/* True if mouse is on */
char resname[][8] = {		/* screen resolution names */
	"LOW", "MEDIUM", "HIGH", "DENSE"
};
short spalette[16];		/* original color palette settings */
short palette[16];		/* current palette settings */
static short scursor;		/* original text cursor mode */

struct la_data *aline;		/* Line A data structure */

struct la_ext *naline;		/* extended Line A data structure */

struct la_font **fonts;	/* Array of pointers to the three system
 			 font headers returned by init (in register A1) */

struct la_font *system_font;	/* pointer to default system font */
struct la_font *small_font;	/* pointer to small font */

/*
	These are needed to make GEM happy
*/
int contrl[ 11], intin[ 128], intout[ 128], ptsin[ 256], ptsout[ 12];
static int worki[ 11] = {1,1,1,1,1,1,1,1,1,1,2}, worko[ 57];

/*
	Some useful information about our environment
*/
static int
	g_wchar, g_hchar,	/* VDI's idea of current font size */
	junk, gid, wkid;	/* Graphics ID, workstation ID */

static int oldbut = 0;		/* Previous state of mouse buttons */
static int mctrl;		/* current BEG_MOUSE state */
static struct { char *norm, *shift, *caps;} *kt;	/* Keyboard mapping */

/*
	This object tree is for displaying the desk accessory menu.
	Actual positions are written into this structure when the
	screen size is known.
*/
OBJECT menu[] =
    {
    -1, 1, 4,	G_IBOX,   NONE,  NORMAL, 0x0L,	    0 , 0,   0,   0,/* Root */
     4, 2, 2,	 G_BOX,   NONE,  NORMAL, 0x10F0L,   0,	0,   1,   1,/* BAR */
     1, 3, 3,	G_IBOX,   NONE,  NORMAL, 0x0L,	    0,	0,   1,   1,/* Active */
     2,-1,-1,  G_TITLE,   NONE,  NORMAL,  "",	    0,	0,   1,   1,/* title */
     0, 5, 5,	G_IBOX,   NONE,  NORMAL, 0x0L,	    0,	0,   0,   0,/* screen */
     4, 6,13,	 G_BOX,   NONE,  NORMAL, 0xFF1100L, 0,	0,   0,   0,/* box */
     7,-1,-1, G_STRING,   NONE,  NORMAL, TEXT179,0,0,168,	16,
/*                                       "  About MicroEmacs" */
     8,-1,-1, G_STRING,   NONE,DISABLED, "---------------------",
						    0, 16, 168,  16,
     9,-1,-1, G_STRING,   NONE,  NORMAL, "",	    0, 32, 168,  16,
    10,-1,-1, G_STRING,   NONE,  NORMAL, "",	    0, 48, 168,  16,
    11,-1,-1, G_STRING,   NONE,  NORMAL, "",	    0, 64, 168,  16,
    12,-1,-1, G_STRING,   NONE,  NORMAL, "",	    0, 80, 168,  16,
    13,-1,-1, G_STRING,   NONE,  NORMAL, "",	    0, 96, 168,  16,
     5,-1,-1, G_STRING, LASTOB,  NORMAL, "",	    0,112, 168,  16
    };

extern mymouse();		/* .s file for calling two mouse handlers */
int (*first_mhandler)();	/* Address of first handler */
int (*second_mhandler)();	/* Address of second handler */
struct kbdvbase *kv;		/* Keyboard vector table */
static int msevntsiz = 0;	/* Events in private mouse event queue */
static unsigned short msevntq[16];/* State of mouse keys at event */
static int msevntin = 0;	/* Queue input ptr */
static int msevntout = 0;	/* Queue output ptr */
static int mousekeys1 = 0;	/* Last mouse key event */
static int mousecol = HUGE;	/* current mouse column */
static int mouserow = HUGE;	/* current mouse row */

extern	int	ttopen();		/* Forward references.		*/
extern	int	ttgetc();
extern	int	ttputc();
extern	int	ttflush();
extern	int	ttclose();
extern	int	stmove();
extern	int	steeol();
extern	int	steeop();
extern	int	stbeep();
extern	int	stopen();
extern	int	stclose();
extern	int	stgetc();
extern	int	stputc();
extern	int	strev();
extern	int	strez();
extern	int	stkopen();
extern	int	stkclose();

#if	COLOR
extern	int	stfcol();
extern	int	stbcol();
#endif

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM	term	= {
	NROW-1,
	NROW-1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	&stopen,
	&stclose,
	&stkopen,
	&stkclose,
	&stgetc,
	&stputc,
	&ttflush,
	&stmove,
	&steeol,
	&steeop,
	&steeop,
	&stbeep,
	&strev,
	&strez
#if	COLOR
	, &stfcol,
	&stbcol
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

void init_aline()

{
	int *ld_contrl;
	int *ld_intin;

	linea0();

	/* save if the current mouse is hidden (ie we are not in GEM) */
	gemflag = (mousehidden == 0);
	while (mousehidden) {
		showmouse();
		}
	aline = (struct la_data *)(la_init.li_a0);
	fonts = (struct la_font **)(la_init.li_a1);
	naline = ((struct la_ext *)aline) - 1;
	scursor = naline->ld_status;		/* State of text cursor */
	ld_contrl = aline->ld_contrl;		/* -> control junk	*/
	ld_intin  = aline->ld_intin;		/* -> intin junk	*/
}

init()

{
	init_aline();
	system_font = fonts[SCRFONT];	    /* save it */
	small_font = fonts[1];
}


switch_font(fp)

struct la_font *fp;

{
       /* See linea.h for description of fields */

       V_CEL_HT = fp->font_height;
       V_CEL_WR = aline->ld_vwrap * fp->font_height;
       V_CEL_MY = (naline->ld_y_max / fp->font_height) - 1;
       V_CEL_MX = (naline->ld_x_max / fp->font_fat_cell) - 1;
       V_FNT_WR = fp->font_width;
       V_FNT_ST = fp->font_low_ade;
       V_FNT_ND = fp->font_hi_ade;
       V_OFF_AD = (long) fp->font_char_off;
       V_FNT_AD = (long) fp->font_data;
}

stmove(row, col)

{
	stputc(ESC);
	stputc('Y');
	stputc(row+BIAS);
	stputc(col+BIAS);
}

steeol()

{
	stputc(ESC);
	stputc('K');
}

steeop()

{
#if	COLOR
	stfcol(gfcolor);
	stbcol(gbcolor);
#endif
	stputc(ESC);
	stputc('J');
}

strev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{
	if (currez > 1) {
		stputc(ESC);
		stputc(status ? 'p' : 'q');
	}
}

#if	COLOR
mapcol(clr)	/* medium rez color translation */

int clr;	/* emacs color number to translate */

{
	static int mctable[] = {0, 1, 2, 3, 2, 1, 2, 3};

	if (currez != 1)
		return(clr);
	else
		return(mctable[clr]);
}

stfcol(color)	/* set the forground color */

int color;	/* color to set forground to */

{
	if (currez < 2) {
		stputc(ESC);
		stputc('b');
		stputc(mapcol(color));
	}
}

stbcol(color)	/* set the background color */

int color;	/* color to set background to */

{
	if (currez < 2) {
		stputc(ESC);
		stputc('c');
		stputc(mapcol(color));
	}
}
#endif

static char beep[] = {
	0x00, 0x00,
	0x01, 0x01,
	0x02, 0x01,
	0x03, 0x01,
	0x04, 0x02,
	0x05, 0x01,
	0x07, 0x38,
	0x08, 0x10,
	0x09, 0x10,
	0x0A, 0x10,
	0x0B, 0x00,
	0x0C, 0x30,
	0x0D, 0x03,
	0xFF, 100,
	0x09, 0x00,
	0x0A, 0x00,
	0xFF, 0x00
};

stbeep()

{
	Dosound(beep);
}

mouse_on()	/* turn the gem mouse ON */

{
	if (!mouseon) {
	    graf_mouse( M_ON, 0L);
	    graf_mouse(ARROW, 0L);
	    mouseon = 1;
	}
}

mouse_off()	/* turn the gem mouse OFF */

{
	if (mouseon) {
	    graf_mouse( M_OFF, 0L);
	    mouseon = 0;
	}
	if( !mousehidden) stbeep();
}

/*
	me_mh - MicroEmacs Mouse interrupt Handler.  This handler is called,
	in addition to the regular mouse handler, in order for microemacs
	to have access to some more information about the mouse events.

	What it does is:

	- Queue a copy of the mouse button state at the actual moment of
	the event, rather than after GEM is finished playing with timers.

	- Convert all right-mouse-button events into left-mouse-button
	events BEFORE GEM's mouse interrupt handler, so that GEM will generate
	a left-mouse-button event.
*/
me_mh(a)

char *a;
{
	register unsigned mousekeys;
	
	mousekeys = a[0] & 7;
	if (mousekeys != 0)
		a[0] = 0xFA;	 /* Any key down means button 1 down */
	else
		a[0] = 0xf8;	/* Otherwise button 1 up */
	if (msevntsiz < 16) {
		/* Previous event was processed, look for next one */
		if (mousekeys != mousekeys1) {
			/* A button change is detected, insert it in queue */
			msevntq[msevntin++] = mousekeys;
			msevntin &= 15;
			msevntsiz++;
			mousekeys1 = mousekeys;
		}
	}
}


stkopen()	/* open the keyboard (and mouse) */
{
}

stopen()	/* open the screen */

{
	register int i;

	init();

/*
	Set up an interrupt handler for the mouse that performs both
	me_mh() and the default mouse handling.  The .s file "stmouse.s"
	contains some special code for this purpose.
*/
	first_mhandler = me_mh;
	kv = Kbdvbase();
	second_mhandler = kv->kb_mousevec;
	kv->kb_mousevec = mymouse;
/*
	In order to have both the mouse cursor and the text cursor on the
	screen at the same time, we have to flash it ourselves, turning
	the mouse cursor on and off for each flash.

	The cursors are both off whenever we are not in an input wait.
*/
	Cursconf(3, 0);	/* Stop text cursor from flashing */   
	Cursconf(0, 0);	/* Turn text cursor off */
/*
	Wake up GEM and the VDI
*/
	appl_init();
	gid = graf_handle(&g_wchar, &g_hchar, &junk, &junk);
#if	0
	/* We never use the VDI, but if you do, turn this back on */
	v_opnvwk(worki, &wkid, worko);
#endif
	graf_mouse( M_OFF, 0L);
	mouseon = 0;
/*
	Set up the menu bar's coordinates to match the font and screen size
	for this screen resolution
*/
	wind_get(0, WF_CURRXYWH,	/* Fetch actual screen size for menu */
		&menu[0].ob_x, &menu[0].ob_y,
		&menu[0].ob_width, &menu[0].ob_height);
	menu[1].ob_x = menu[0].ob_width - 1;	/* Locate "desk" box */
	menu[4].ob_width = menu[0].ob_width;
	menu[4].ob_height = 8*g_hchar;
	menu[5].ob_width = 21*g_wchar;
	menu[5].ob_height = 8*g_hchar;
	menu[5].ob_x = menu[0].ob_width - menu[5].ob_width;
	for(i=6; i<14; i++) {
		/* Desk accessory items */
		menu[i].ob_y = (i-6)*g_hchar;
		menu[i].ob_width = menu[5].ob_width;
		menu[i].ob_height = g_hchar;
	}
	menu_bar(menu, 1);
/*
	Shut off GEM's user interface until we enter an input wait.
	Note that unless we claim the mouse with BEG_MCTRL, we will not
	get scheduled to run any time the left mouse button goes down while
	the mouse is on the top 16 pixels of the screen.  We keep Emacs
	"hung" whenever MCTRL is given to desk accessories or GEM, and keep
	GEM hung whenever we have control.
*/
	wind_update(BEG_UPDATE);	/* Shuts off GEM drawing */
	wind_update(BEG_MCTRL);		/* Shuts off GEM use of mouse */

	mctrl = 0;			/* Flag that we have mouse control */
	kt = Keytbl( -1L, -1L, -1L);

	ttopen();
	eolexist = TRUE;

	/* save the current color palette */
	for (i=0; i<16; i++)
		spalette[i] = xbios(SETCOLOR, i, -1);

	/* and find the current resolution */
	initrez = currez = xbios(GETREZ);
	strcpy(sres, resname[currez]);

	/* set up the screen size and palette */
	switch (currez) {
		case 0: term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			term.t_ncol = 40;
			strcpy(palstr, LOWPAL);
			break;

		case 1: term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, MEDPAL);
			break;

		case 2: term.t_mrow = DENSIZE - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, HIGHPAL);
	}

	/* and set up the default palette */
	spal(palstr);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');

	/* initialize the input buffer */
	in_init();
}

stkclose()	/* close the keyboard (and mouse) */
{
}

stclose()

{
	stputc(ESC);	/* auto overflow on */
	stputc('v');

	/* restore the original screen resolution */
	if (currez == 3)
		switch_font(system_font);
	strez(resname[initrez]);

	/* restore the original palette settings */
	xbios(SETPALETTE, spalette);

	ttclose();

	kv->kb_mousevec = second_mhandler;	/* Restore original handler */

	mouse_on();		/* turn the mouse on */
	if (gemflag != 0) {
		/* coming from gem..... we */
		Cursconf(0, 0);		/* turn the cursor off */
	} else {
		/* coming from a shell, we */
		Cursconf(1, 0);		/* turn the cursor on */
		Cursconf(2, 0);		/* Turn text cursor blinking */
		hidemouse();		/* Turn mouse off for shell */
	}

	wind_update(END_MCTRL);		/* Return GEM's control of screen */
	wind_update(END_UPDATE);
#if	0
	v_clsvwk(wkid);			/* Clean up GEM */
#endif
	appl_exit();
}

/*	spal(pstr):	reset the current palette according to a
			"palette string" of the form

	000111222333444555666777

	which contains the octal values for the palette registers
*/

spal(pstr)

char *pstr;	/* palette string */

{
	int pal;	/* current palette position */
	int clr;	/* current color value */
	int i;

	for (pal = 0; pal < 16; pal++) {
		if (*pstr== 0)
			break;

		/* parse off a color */
		clr = 0;
		for (i = 0; i < 3; i++)
			if (*pstr)
				clr = clr * 16 + (*pstr++ - '0');
		palette[pal] = clr;
	};

	/* and now set it */
	xbios(SETPALETTE, palette);
}

static domousekey(newbut, sk)

int sk;

{
	register int k;

	for(k=1; k!=4; k = k<<1) {	/* J is butnum, k is bit num */ 
		/* For each button on the mouse */
		if ((oldbut&k) != (newbut&k)) {
			/* This button changed, generate an event */
			in_put(0);		/* escape indicator */
			in_put(MOUS >> 8);	/* mouse event */
			in_put(mousecol);	/* x-position */
			in_put(mouserow);	/* y-position */
			in_put(((newbut&k) ? 0 : 1)
				+ (k==1 ? 4 : 0)
				+ ((sk&3) ? 'A' : 'a'));
		}
	}
	oldbut = newbut;
}

stgetc()	/* get a char from the keyboard */

{
	register int flashcounter;	/* Counter for flashing cursor */
	register int ev_which;		/* Event returned */
	register int sc;		/* Keyboard scan code */
	static int bexpected = 1;	/* Expected next mouse state */
	int mx, my, bstate, sk, key, mc;/* Event parameters */
	int mes[8];

	/* Working out a multi-byte input sequence */
	if (in_check())
		return(in_get());

	Cursconf(1, 0);		/* Turn text cursor on */
	mouse_on();		/* Let mouse work */
	wind_update(END_UPDATE);
	if (mctrl)
		wind_update(END_MCTRL);
	flashcounter = 0;
	for(;;) {
		if (!mctrl) {
			/* We have control, so it is okay to flash cursor */
			if (flashcounter == 6) {
				mouse_off();
				Cursconf(0, 0); /* After 60ms, turn it off */
				mouse_on();
			} else if (flashcounter == 10) {
				/* Wrapped around one cycle of cursor flashing,
				   turn it on */
				mouse_off();
				Cursconf(1, 0);
				mouse_on();
				flashcounter = 0;
			}
		}

		/* do the event-multi thing */
		ev_which = evnt_multi(
			MU_TIMER | MU_MESAG | MU_KEYBD | MU_BUTTON | MU_M1,
			1,		/* Maximum clicks to wait for */
			1,		/* Buttons that generate events */
			bexpected,	/* Button states that generate events */
			0, menu[1].ob_x, menu[1].ob_y,
			menu[1].ob_width, menu[1].ob_height,
			0, 0, 0, 0, 0,	/* enter/exit, x, y, w, h for rect 2 */
			mes,		/* Buffer to receive mesasge */
			/* Low and high order miliseconds of counter */
			100, 0,
			&mx, &my,	/* Mouse location */
			&bstate,	/* State of the mouse buttons */
			&sk,		/* State of the shift keys */
			&key,		/* Key pressed */
			&mc);		/* Actual number of clicks */

		if (ev_which & MU_KEYBD) {
			/* Keyboard events cause keystrokes, add SPC prefix to fn keys */
			if (mctrl)
				wind_update(BEG_MCTRL);
			Giaccess(0, 9+128);
			Giaccess(0, 10+128);

			/* Cancel any double-clicking */
			nclicks = 0;
			mctrl = 0;

	 	        sc = key>>8;
		        key &= 0xff;
			extcode( sk, sc, key);
		} else if (ev_which & MU_BUTTON) {
			/* Mouse event, look at our actual mouse event */
			int top, junk;
			wind_get(0, WF_TOP, &top, &junk, &junk, &junk);
			if (top == 0) {
				/* Desktop is top window,
				   allow Emacs to continue */
				bexpected = (~bstate)&1;
				mousecol = mx/g_wchar;
				mouserow = my/g_hchar;
				while(msevntsiz > 0) {
					/* Process each queued event */
					domousekey(msevntq[msevntout++], sk);
					msevntout &= 15;
					msevntsiz--;
					}
				if (mctrl) wind_update(BEG_MCTRL);
				mctrl = 0;
			}
		} else if (ev_which & MU_M1) {
			/* Mouse entered desk menu, allow it to appear */
			if (!mctrl) wind_update(END_MCTRL);
			mctrl = 1;
			sgarbf = TRUE;
		} else if (ev_which & MU_TIMER) {
			flashcounter++;
		} else {
			/* Most likely is the about message */
		}

		/* is there now a pending event? */
		if (in_check()) {
			key = in_get();
			break;
		}
	}

	if (mctrl)
		wind_update(BEG_MCTRL);
	wind_update(BEG_UPDATE);
	mouse_off();
	Cursconf(0, 0);			/* Turn text cursor off */
	return(key & 0xFF);
}

stputc(c)	/* output char c to the screen */

char c; 	/* character to print out */

{
	bios(BCONOUT, CON, c);
}

strez(newrez)	/* change screen resolution */

char *newrez;	/* requested resolution */

{
	int nrez;	/* requested new resolution */

	/* first, decode the resolution name */
	for (nrez = 0; nrez < 4; nrez++)
		if (strcmp(newrez, resname[nrez]) == 0)
			break;
	if (nrez == 4) {
		mlwrite(TEXT180);
/*                      "%%No such resolution" */
		return(FALSE);
	}

	/* next, make sure this resolution is legal for this monitor */
	if ((currez < 2 && nrez > 1) || (currez > 1 && nrez < 2)) {
		mlwrite(TEXT181);
/*                      "%%Resolution illegal for this monitor" */
		return(FALSE);
	}

	/* eliminate non-changes */
	if (currez == nrez)
		return(TRUE);

	/* finally, make the change */
	switch (nrez) {
		case 0: /* low resolution - 16 colors */
			newwidth(TRUE, 40);
			strcpy(palstr, LOWPAL);
			xbios(SETSCREEN, -1L, -1L, 0);
			g_wchar = g_hchar = 8;
			break;

		case 1: /* medium resolution - 4 colors */
			newwidth(TRUE, 80);
			strcpy(palstr, MEDPAL);
			xbios(SETSCREEN, -1L, -1L, 1);
			g_wchar = g_hchar = 8;
			break;

		case 2: /* High resolution - 2 colors - 25 lines */
			newsize(TRUE, 25);
			strcpy(palstr, HIGHPAL);
			switch_font(system_font);
			g_wchar = g_hchar = 16;
			break;

		case 3: /* Dense resolution - 2 colors - 40 lines */
			newsize(TRUE, DENSIZE);
			strcpy(palstr, HIGHPAL);
			switch_font(small_font);
			g_wchar = g_hchar = 8;
			break;
	}

	/* and set up the default palette */
	spal(palstr);
	currez = nrez;
	strcpy(sres, resname[currez]);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');
	return(TRUE);
}

/*	extcode:	resolve Atari-ST extended character codes
			encoding the proper sequences into emacs
			printable character specifications
*/

int extcode( sk, sc, key)
unsigned sk;	/* Shift keys from event */
unsigned sc;	/* ST scancode */
unsigned key;	/* GEMDOS translation of the key */
    {
    int shift;	/* CAPS LOCK flag doesn't come from EVENT_MULTI */
    unsigned code;	/* Build up special function code */
/*
	Identify any shit-key codes associated with this keystorke
*/
    code = 0;
/*
	I don't know why, but for some reason the codes for ALT of top row and
	for CTRL of left, right and HOME come up wrong, and this fixes them.
*/
    if( sc == 0x77) sc = 0x47;
    else if( sc == 0x73) sc = 0x4b;
    else if( sc == 0x74) sc = 0x4d;
    else if( sc > 0x76) sc -= 0x76;
/*
	Bring the shifted function key scan codes back into regular range
*/
    if( sc >= 0x54 && sc <= 0x5d) sc -= (0x54-0x3B);
    if( sk & K_ALT)
	{ /* ALT shift requires special handling */
	code |= ALTD;
/*
	ALT of ordinary keys always returns '0' for the key.  Look up
	the actual key as if ALT weren't there.
*/
	shift = Getshift(-1);	/* Get state of shift keys (CAPS lock) */
/*
	Map the key to a normal keystroke.  Keypad keys are treated special,
	everyone else is mapped using the installed keytables.  This
	means it will work right on German and other keyboards.
*/
	if( sk & K_CTRL) { code |= CTRL; key = kt->caps[ sc];}
	else if( sk & 3) key = kt->shift[ sc];	/* shift */
	else if( shift & 16) key = kt->caps[ sc];	/* Caps lock */
	else key = kt->norm[ sc];
	}
/*
	Peel of the numeric keypad keys
*/
    if( sc == 0x72) key = 'E';
    if( sc >= 0x63 || sc == 0x4A || sc == 0x4E)
	{ /* Keypad keys are SPEC or CTRL of what's on the key */
	code |= SPEC | CTRL;
	}
/*
	translate function keys into digits
*/
    if( sc >= 0x3b && sc <= 0x5d)
	{ /* This is a F1 thry F9 */
	code |= SPEC;
	key = sc - 0x3b + '1';
	if( key == '9'+1) key = '0';
	}
    if( sc == 0x62) { code |= SPEC; key = 'H';}
    if( sc == 0x61) { code |= SPEC; key = 'X';}
    if( sc == 0x52) { code |= SPEC; key = 'C';}
    if( sc == 0x47) { code |= SPEC; key = 'D';}
    if( sc == 0x48) { code |= SPEC; key = 'P';}
    if( sc == 0x4b) { code |= SPEC; key = 'B';}
    if( sc == 0x50) { code |= SPEC; key = 'N';}
    if( sc == 0x4d) { code |= SPEC; key = 'F';}
/*
	translate CTRL-shifted of keys that don't usually CTRL
*/
    if((sk & K_CTRL) && (sc <= 0x0D || key == 0))
	{ /* Control of a non-usually-control key */
	shift = Getshift(-1);	/* Get state of CAPS lock */
	code |= CTRL;
	if( sk & 3) key = kt->shift[ sc];		/* shift */
	else if( shift & 16) key = kt->caps[ sc];	/* Caps lock */
	else key = kt->norm[ sc];
	}
    if( key == 0) key = '@';				/* Catch junk */
    if( code != 0)
	{ /* This is a special key */
	if( code & SPEC)
	    { /* Get shift and ctrl of function keys */
	    if( sk & 3) code |= SHFT;
	    if( sk & K_CTRL) code |= CTRL;
	    }
	in_put( 0);
	in_put( code>>8);
	in_put( key);
	}
    else in_put( key);
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
sthello()
{
}
#endif
