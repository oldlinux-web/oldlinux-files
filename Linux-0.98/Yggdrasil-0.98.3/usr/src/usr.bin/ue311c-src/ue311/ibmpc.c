/*
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 * Supported monitor cards include CGA, MONO and EGA.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if     IBMPC
#define NROW	50			/* Max Screen size.		*/
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define	SPACE	32			/* space character		*/

#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xb8000000L		/* EGA/VGA address of screen RAM*/

#define MONOCRSR 0x0B0D			/* monochrome cursor		*/
#define CGACRSR 0x0607			/* CGA cursor			*/
#define EGACRSR 0x0709			/* EGA/VGA cursor		*/

#define	CDCGA	0			/* color graphics card		*/
#define	CDMONO	1			/* monochrome text card		*/
#define	CDEGA	2			/* EGA color adapter		*/
#define	CDVGA	3			/* VGA color adapter		*/
#define	CDVGA12	4			/* VGA 12 line mode		*/
#define	CDCGA40	5			/* CGA 40 wide mode		*/
#define	CDSENSE	9			/* detect the card type		*/

#define NDRIVE	6			/* number of screen drivers	*/

typedef struct sdrive {
	char drv_name[8];		/* screen driver name	*/
	long drv_scradd;		/* address of segment of screen ram */
	int drv_rows;			/* # of rows for screen driver */
	int drv_cols;			/* # of columns for screen driver */
} SDRIVE;

SDRIVE scr_drive[] = {
	"CGA",   SCADC, 25, 80,		/* standard color card		*/
	"MONO",  SCADM, 25, 80,		/* momochrome graphics adapter	*/
	"EGA",   SCADE, 43, 80,		/* Enhanced graphics adapter	*/
	"VGA",   SCADE, 50, 80,		/* Very Enhanced graphics adapter*/
	"VGA12", SCADE, 12, 80,		/* 12 lines x 80 cols		*/
	"CGA40", SCADC, 25, 40,		/* low resolution CGA		*/
};

int dtype = -1;				/* current display type		*/

long scadd;				/* address of screen ram	*/
int *scptr[NROW];			/* pointer to screen lines	*/
unsigned int sline[NCOL];		/* screen line image		*/
int num_cols;				/* current number of columns	*/
int orig_mode;				/* screen mode on entry		*/
int egaexist = FALSE;			/* is an EGA card available?	*/
int vgaexist = FALSE;			/* is video graphics array available? */
extern union REGS rg;			/* cpu register for use of DOS calls */
int revflag = FALSE;			/* are we currently in rev video? */
int desk_rows;				/* number of rows on current desktop */
int desk_cols;				/* number of cols on current desktop */
int break_flag;			/* state of MSDOS control break processing */

PASCAL NEAR ibmmove();
PASCAL NEAR ibmeeol();
PASCAL NEAR ibmputc();
PASCAL NEAR ibmeeop();
PASCAL NEAR ibmclrdesk();
PASCAL NEAR ibmrev();
PASCAL NEAR ibmcres();
PASCAL NEAR spal();
PASCAL NEAR ibmbeep();
PASCAL NEAR ibmopen();
PASCAL NEAR ibmclose();
PASCAL NEAR ibmkopen();
PASCAL NEAR ibmkclose();
PASCAL NEAR scinit();
PASCAL NEAR screen_init();
int PASCAL NEAR getboard();
PASCAL NEAR egaopen();
PASCAL NEAR egaclose();
PASCAL NEAR cga40_open();
PASCAL NEAR cga40_close();
PASCAL NEAR change_width();
PASCAL NEAR fnclabel();

#if	COLOR
PASCAL NEAR ibmfcol();
PASCAL NEAR ibmbcol();
int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] =		/* ansi to ibm color translation table */
	{0, 4, 2, 6, 1, 5, 3, 7,
	 8, 12, 10, 14, 9, 13, 11, 15};
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	0, 0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        ibmopen,
        ibmclose,
	ibmkopen,
	ibmkclose,
        ttgetc,
	ibmputc,
        ttflush,
        ibmmove,
        ibmeeol,
        ibmeeop,
        ibmclrdesk,
        ibmbeep,
	ibmrev,
	ibmcres
#if	COLOR
	, ibmfcol,
	ibmbcol
#endif
};

#if	COLOR
PASCAL NEAR ibmfcol(color)	/* set the current output color */

int color;	/* color to set */

{
	cfcolor = ctrans[color];
}

PASCAL NEAR ibmbcol(color)	/* set the current background color */

int color;	/* color to set */

{
        cbcolor = ctrans[color];
}
#endif

PASCAL NEAR ibmmove(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col + term.t_colorg;
	rg.h.dh = row + term.t_roworg;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

PASCAL NEAR ibmeeol()	/* erase to the end of the line */

{
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;	/* current column cursor lives */
	int crow;	/*	   row	*/

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl - term.t_colorg;	/* record current column */
	crow = rg.h.dh - term.t_roworg;	/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		if (revflag)
			attr = (((cfcolor & 15) << 4) | (cbcolor & 15)) << 8;
		else
			attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
	else
		if (revflag)
			attr = 0x7000;
		else
			attr = 0x0700;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i=0; i < term.t_ncol; i++)
		*lnptr++ = SPACE | attr;

	if (flickcode && (dtype == CDCGA || dtype == CDCGA40)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}			

	/* and send the string out */
	movmem(&sline[0], scptr[crow+term.t_roworg]+ccol+term.t_colorg, (term.t_ncol-ccol)*2);

}

PASCAL NEAR ibmputc(ch) /* put a character at the current position in the
		   current colors */

int ch;

{
	/* if its a newline, we have to move the cursor */
	if (ch == '\n' || ch == '\r') {
		rg.h.ah = 3;
		int86(0x10, &rg, &rg);
		if (rg.h.dh == 24) {
			ibmmove(20, 0);
			/* we must scroll the screen */
			rg.h.ah = 6;	/* scroll up */
			rg.h.al = 1;	/* # of lines to scroll by */
#if	COLOR
			rg.h.bh = cfcolor; /* attribute for blank line */
#else
			rg.h.bh = 0; /* attribute for blank line */
#endif
			rg.x.cx = 0;	/* upper left corner of scroll */
			rg.x.dx = 0x184f;/* lower right */
			int86(0x10, &rg, &rg);
			rg.h.dh = 23;
		}
		ibmmove(rg.h.dh + 1, 0);
		return;
	}

#if	1
	if (ch == '\b') {

		/* backup the cursor by 1 position */
		rg.h.ah = 3;		/* read current position */
		int86(0x10, &rg, &rg);
		rg.h.dl--;		/* move it forward one */
		rg.h.ah = 2;		/* set its new position */
		int86(0x10, &rg, &rg);
	
		rg.h.ah = 9;		/* write char with attributes to cursor position */
		rg.h.bh = 0;		/* display page zero */
		rg.x.cx = 1;		/* only one please! */
		rg.h.al = ' ';		/* character to write */
#if	COLOR
		rg.h.bl = ((ctrans[gbcolor] << 4) | ctrans[gfcolor]);/* attribute */
#else
		rg.h.bl = 07;
#endif
		int86(0x10, &rg, &rg);
		return;
	}

	if (ch == 7) {
		TTbeep();
		return;
	}

	rg.h.ah = 9;		/* write char with attributes to cursor position */
	rg.h.bh = 0;		/* display page zero */
	rg.x.cx = 1;		/* only one please! */
	rg.h.al = ch;		/* character to write */
#if	COLOR
	rg.h.bl = ((ctrans[gbcolor] << 4) | ctrans[gfcolor]);	/* attribute */
#else
	rg.h.bl = 07;
#endif
	int86(0x10, &rg, &rg);

	/* advance the cursor by 1 position */
	rg.h.ah = 3;		/* read current position */
	int86(0x10, &rg, &rg);
	rg.h.dl++;		/* move it forward one */
	rg.h.ah = 2;		/* set its new position */
	int86(0x10, &rg, &rg);
#else
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (dtype != CDMONO)
		rg.h.bl = cfcolor;
	else
		rg.h.bl = 0x07;
#else
	rg.h.bl = 0x07;
#endif
	int86(0x10, &rg, &rg);
#endif
}

PASCAL NEAR ibmeeop()

{
	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = (term.t_roworg << 8) | (term.t_colorg);
				/* upper left corner of scroll */
	rg.x.dx = ((term.t_nrow + term.t_roworg) << 8) |
			(term.t_ncol + term.t_colorg - 1);
				/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		if (revflag)
			rg.h.bh = ((ctrans[gfcolor] & 15) << 4) | (ctrans[gbcolor] & 15);
		else
			rg.h.bh = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		if (revflag)
			rg.h.bh = 70;
		else
			rg.h.bh = 07;
#else
	rg.h.bh = 07;
#endif
	int86(0x10, &rg, &rg);
}

PASCAL NEAR ibmclrdesk()

{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.x.dx = (desk_rows << 8) | desk_cols;
				/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		if (revflag)
			attr = ((ctrans[gfcolor] & 15) << 4) | (ctrans[deskcolor] & 15);
		else
			attr = ((ctrans[deskcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		if (revflag)
			attr = 70;
		else
			attr = 07;
#else
	attr = 07;
#endif
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
}

PASCAL NEAR ibmrev(state)	/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
	revflag = state;
}

extern dumpscreens();

PASCAL NEAR ibmcres(res) /* change screen resolution */

char *res;	/* resolution to change to */

{
	int i;		/* index */

	for (i = 0; i < NDRIVE; i++)
		if (strcmp(res, scr_drive[i].drv_name) == 0) {
			scinit(i);
			return(TRUE);
		}
	return(FALSE);
}

PASCAL NEAR spal(mode)	/* reset the pallette registers */

char *mode;

{
	/* nothin here now..... */
}

PASCAL NEAR ibmbeep()
{
#if	MWC
	ttputc(BEL);
#else
	bdos(6, BEL, 0);
#endif
}

PASCAL NEAR ibmopen()
{
	scinit(CDSENSE);
	revexist = TRUE;
	revflag = FALSE;
        ttopen();
}

PASCAL NEAR ibmclose()

{
#if	COLOR
	ibmfcol(7);
	ibmbcol(0);
#endif

	/* exit in the same mode that we entered */
	if (dtype != orig_mode) {
		scinit(orig_mode);
		movecursor(term.t_nrow, 0);
		TTflush();
	}
	dtype = -1;
	ttclose();
}

PASCAL NEAR ibmkopen()	/* open the keyboard */

{
}
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

PASCAL NEAR ibmkclose() /* close the keyboard */

{
}
	if (break_flag == 1) {
		rg.h.ah = 0x33;	/* ctrl-break check */
		rg.h.al = 1;	/* set the state of the ctrl-break check */
		rg.h.dl = 1;	/* turn it on */
		intdos(&rg, &rg);
	}

PASCAL NEAR scinit(type) /* initialize the screen head pointers */

int type;	/* type of adapter to init for */

{
	/* if asked...find out what display is connected */
	if (type == CDSENSE)
		type = getboard();

	/* if we have nothing to do....don't do it */
	if (dtype == type)
		return(TRUE);

	/* if we try to switch to EGA and there is none, don't */
	if (type == CDEGA && !egaexist)
		return(FALSE);

	/* if we try to switch to VGA and there is none, don't */
	if (type == CDVGA && !vgaexist )
		return(FALSE);

	/* if we had the EGA or VGA open... close it */
	if (dtype == CDEGA || dtype == CDVGA || dtype == CDVGA12)
		egaclose();

	/* if we had the CGA40 open... close it */
	if (dtype == CDCGA40)
		cga40_close();

	/* and set up the various parameters as needed */
	scadd = scr_drive[type].drv_scradd;
	switch (type) {
		case CDMONO:	/* Monochrome adapter */
		case CDCGA:	/* Color graphics adapter */
				break;

		case CDCGA40:	/* Color graphics adapter */
				cga40_open();
				break;

		case CDEGA:	/* Enhanced graphics adapter */
				egaopen(CDEGA);
				break;

		case CDVGA:	/* video graphics array - acts as EGA but more lines */
				egaopen(CDVGA);
				break;

		case CDVGA12:	/* Video Graphics Array 12 line mode */
				egaopen(CDVGA12);
				break;

	}
	maxlines(scr_drive[type].drv_rows);

	/* reset the $sres environment variable */
	strcpy(sres, scr_drive[type].drv_name);

	/* resize any screens that need it! */
	screen_init(dtype, type);
	dtype = type;

	return(TRUE);
}

PASCAL NEAR screen_init(dtype, type) /* initialize the screen head pointers */

int dtype;	/* original screen type (-1 if first time!) */
int type;	/* new type of adapter to adjust screens for */

{
	int full_screen;	/* is the active screen full size */

	/* is the front screen full size? */
	if ((dtype != -1) &&
	   (scr_drive[dtype].drv_rows == (term.t_nrow + 1)) &&
	    (scr_drive[dtype].drv_cols == term.t_ncol))
		full_screen = TRUE;
	else
		full_screen = FALSE;

	/* set up the new desktop size */
	desk_rows = scr_drive[type].drv_rows;
	desk_cols = scr_drive[type].drv_cols;

	/* recalculate the screen line pointer array */
	change_width(desk_cols);

	/* first time, or if we are full screen */
	if ((dtype == -1) || full_screen) {
		newsize(TRUE, desk_rows);
		newwidth(TRUE, desk_cols);
	}

#if	WINDOW_TEXT
	refresh_screen(first_screen);
#endif
	return(TRUE);
}

int PASCAL NEAR change_width(ncols)

int ncols;	/* number of columns across */

{
	union {
		long laddr;	/* long form of address */
		int *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* re-initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
		addr.laddr = scadd + (long)(ncols * i * 2);
		scptr[i] = addr.paddr;
	}
}

/* getboard:	Determine which type of display board is attached.
		Current known types include:

		CDMONO	Monochrome graphics adapter
		CDCGA	Color Graphics Adapter
		CDEGA	Extended graphics Adapter
		CDVGA	Vidio Graphics Array

		if MONO	set to MONO
		   CGA40 set to CGA40	test as appropriate
		   CGA	set to CGA	EGAexist = FALSE VGAexist = FALSE
		   EGA	set to CGA	EGAexist = TRUE  VGAexist = FALSE
		   VGA	set to CGA	EGAexist = TRUE  VGAexist = TRUE
*/

int PASCAL NEAR getboard()

{
	int type;	/* board type to return */

	type = CDCGA;
	int86(0x11, &rg, &rg);
	if ((((rg.x.ax >> 4) & 3) == 3))
		type = CDMONO;

	/* test for 40 col mode */
	rg.h.ah = 15;
	int86(0x10, &rg, &rg);
	if (rg.h.al == 1)
		type = CDCGA40;

	/* save the original video mode */
	orig_mode = type;

	/* test if EGA present */
	rg.x.ax = 0x1200;
	rg.x.bx = 0xff10;
	int86(0x10,&rg, &rg);		/* If EGA, bh=0-1 and bl=0-3 */
	egaexist = !(rg.x.bx & 0xfefc);	/* Yes, it's EGA */
	if (egaexist) {
		/* Adapter says it's an EGA. We'll get the same response
		   from a VGA, so try to tell the two apart */
		rg.x.ax = 0x1a00;	/* read display combination */
		int86(0x10,&rg,&rg);
		if (rg.h.al == 0x1a && (rg.h.bl == 7 || rg.h.bl == 8)) {
			/* Function is supported and it's a PS/2 50,60,80 with
			   analog display, so it's VGA (I hope!) */
			vgaexist = TRUE;
		} else {
			/* Either BIOS function not supported or something
			   other then VGA so set it to be EGA */
			vgaexist = FALSE;
		}
	}
	return(type);
}

PASCAL NEAR egaopen(mode) /* init the computer to work with the EGA or VGA */

int mode;	/* mode to select [CDEGA/CDVGA] */

{
	/* set the proper number of scan lines */
	rg.h.ah = 18;
	switch (mode) {

		case CDEGA:	rg.h.al = 1;
				break;

		case CDVGA:	rg.h.al = 2;
				break;

		case CDVGA12:	rg.h.al = 0;
				break;

	}
	rg.h.bl = 48;
	int86(16, &rg, &rg);

	/* put the beast into EGA 43/VGA 50/VGA 12 line mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);

	/* set the proper character set */
	if (mode == CDVGA12) {
		rg.h.al = 20;	/*  to 8 by 16 double dot ROM         */
	} else {
		rg.h.al = 18;	/*  to 8 by 8 double dot ROM         */
	}
	rg.h.ah = 17;		/* set char. generator function code */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);

	/* select the alternative Print Screen function */
	rg.h.ah = 18;		/* alternate select function code    */
	rg.h.al = 0;		/* clear AL for no good reason       */
	rg.h.bl = 32;		/* alt. print screen routine         */
	int86(16, &rg, &rg);

	/* resize the cursor */
	rg.h.ah = 1;		/* set cursor size function code */
	rg.x.cx = 0x0607;	/* turn cursor on code */
	int86(0x10, &rg, &rg);

	/* video bios bug patch */
	outp(0x3d4, 10);
	outp(0x3d5, 6);
}

PASCAL NEAR egaclose()

{
	/* set the proper number of scan lines for CGA */
	rg.h.ah = 18;
	rg.h.al = 2;
	rg.h.bl = 48;
	int86(16, &rg, &rg);

	/* put the beast into 80 column mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
}

PASCAL NEAR cga40_open()

{
	/* put the beast into 40 column mode */
	rg.x.ax = 1;
	int86(16, &rg, &rg);
}

PASCAL NEAR cga40_close()

{
	/* put the beast into 80 column mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
}

PASCAL NEAR scwrite(row, outstr, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		if (revflag)
			attr = (ctrans[forg] << 12) | (ctrans[bacg] << 8);
		else
			attr = (ctrans[bacg] << 12) | (ctrans[forg] << 8);
	else
		if ((bacg || revflag) && !(bacg && revflag))
			attr = 0x7000;
		else
			attr = 0x0700;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i=0; i<term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}

	/* and send the string out */
	movmem(&sline[0], scptr[row+term.t_roworg]+term.t_colorg,term.t_ncol*2);
}

#if	FLABEL
PASCAL NEAR fnclabel(f, n)	/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
ibmhello()
{
}
#endif

