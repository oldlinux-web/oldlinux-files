/*
 * The routines in this file provide support for the Zenith Z-100 PC
 * family.  It goes directly to the graphics RAM to do screen output. 
 * It compiles into nothing if not a Zenith driver.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if     Z309

/* set NROW to 25 for 25-line interlaced mode */
#define NROW    50                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define	SPACE	32			/* space character		*/

#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/

#define	CDMONO	0			/* monochrome text card		*/
#define	CDCGA50	1			/* 50-line color graphics card	*/
#define CDCGI25 2			/* 25-line interlaced CGA text	*/
#define CDCGA25 3			/* 25-line color graphics card	*/
#define	CDSENSE	9			/* detect the card type		*/

int dtype = CDCGA50;			/* current display type		*/
long scadd;				/* address of screen ram	*/
int *scptr[NROW];			/* pointer to screen lines	*/
int sline[NCOL];			/* screen line image		*/
extern union REGS rg;			/* cpu register for use of DOS calls */

extern PASCAL NEAR ttopen();               /* Forward references.          */
extern PASCAL NEAR ttgetc();
extern PASCAL NEAR ttputc();
extern PASCAL NEAR ttflush();
extern PASCAL NEAR ttclose();
extern PASCAL NEAR z309move();
extern PASCAL NEAR z309eeol();
extern PASCAL NEAR z309eeop();
extern PASCAL NEAR z309beep();
extern PASCAL NEAR z309open();
extern PASCAL NEAR z309rev();
extern PASCAL NEAR z309cres();
extern PASCAL NEAR z309close();
extern PASCAL NEAR z309putc();
extern PASCAL NEAR z309kopen();
extern PASCAL NEAR z309kclose();
extern PASCAL NEAR scinit();
extern PASCAL NEAR spal();

#if	COLOR
extern PASCAL NEAR z309fcol();
extern PASCAL NEAR z309bcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] =		/* ansi to z309 color translation table */
	{0, 4, 2, 6, 1, 5, 3, 7};
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
	MARGIN,
	SCRSIZ,
	NPAUSE,
        z309open,
        z309close,
	z309kopen,
	z309kclose,
        ttgetc,
	z309putc,
        ttflush,
        z309move,
        z309eeol,
        z309eeop,
        z309eeop,
        z309beep,
	z309rev,
	z309cres
#if	COLOR
	, z309fcol,
	z309bcol
#endif
};

extern union REGS rg;

#if	COLOR
PASCAL NEAR z309fcol(color)		/* set the current output color */

int color;	/* color to set */

{
	cfcolor = ctrans[color];
}

PASCAL NEAR z309bcol(color)		/* set the current background color */

int color;	/* color to set */

{
        cbcolor = ctrans[color];
}
#endif
PASCAL NEAR z309move(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

PASCAL NEAR z309eeol()	/* erase to the end of the line */

{
	int attr;	/* attribute byte mask to place in RAM */
	int *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;	/* current column cursor lives */
	int crow;	/*	   row	*/

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
	else
		attr = 0x0700;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i=0; i < term.t_ncol; i++)
		*lnptr++ = SPACE | attr;

#if 0	/* Heath/Zenith builds flicker-less CGAs */
	if (flickcode) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}
#endif

	/* and send the string out */
	movmem(&sline[0], scptr[crow]+ccol, (term.t_ncol-ccol)*2);

}

PASCAL NEAR z309putc(ch)	/* put a character at the current position in the
		   current colors */

int ch;

{
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
}

PASCAL NEAR z309eeop()
{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
/*HERE*/	rg.x.dx = 0x184f;	/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		attr = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		attr = 0;
#else
	attr = 0;
#endif
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
}

PASCAL NEAR z309rev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
	/* This never gets used under the z309-PC driver */
}

PASCAL NEAR z309cres(res)	/* change screen resolution */ 

char *res;	/* resolution to change to */

{
	if (strcmp(res, "CGA50") == 0) {
		scinit(CDCGA50);
		return(TRUE);
	} else if (strcmp(res, "MONO") == 0) {
		scinit(CDMONO);
		return(TRUE);
	} else
		return(FALSE);
}

PASCAL NEAR z309beep()
{
#if	MWC
	putcnb(BEL);
#else
	bdos(6, BEL, 0);
#endif
}

PASCAL NEAR z309open()
{
	scinit(CDSENSE);
	revexist = TRUE;
        ttopen();
}

PASCAL NEAR z309close()

{
	rg.h.ah = 101;
	rg.h.al = 1;	/* 25-line interlace mode */
	int86(0x10, &rg, &rg); 
#if	COLOR
	z309fcol(7);
	z309bcol(0);
#endif
	ttclose();
}

PASCAL NEAR spal()	/* reset the pallette registers */

{
	/* nothin here now..... */
}

PASCAL NEAR z309kopen()	/* open the keyboard */

{
}

PASCAL NEAR z309kclose()	/* close the keyboard */

{
}

PASCAL NEAR scinit(type)	/* initialize the screen head pointers */

int type;	/* type of adapter to init for */

{
	union {
		long laddr;	/* long form of address */
		int *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* if asked...find out what display is connected */
	int86(0x11, &rg, &rg);
	dtype = CDCGA50;
	scadd = SCADC;
	strcpy(sres, "CGA50");
	if ((((rg.x.ax >> 4) & 11) == 3) || type == CDMONO) {
		strcpy(sres, "MONO");
		dtype = CDMONO;
		scadd = SCADM;
	}
	else {
		rg.h.ah = 101;
/* set al = 1 for 25-line interlace mode */		
		rg.h.al = 2;	/* 50-line interlace mode */
		int86(0x10, &rg, &rg); 
	}

	/* initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
		addr.laddr = scadd + (long)(NCOL * i * 2);
		scptr[i] = addr.paddr;
	}
}

PASCAL NEAR scwrite(row, outstr, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
	int attr;	/* attribute byte mask to place in RAM */
	int *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
	else
		attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i=0; i<term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

#if 0	/* Heath/Zenith builds flicker-less CGAs */
	if (flickcode) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}
#endif	

	/* and send the string out */
	movmem(&sline[0], scptr[row],term.t_ncol*2);
}

#if	FLABEL
PASCAL NEAR fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
PASCAL NEAR z309hello()
{
}
#endif
