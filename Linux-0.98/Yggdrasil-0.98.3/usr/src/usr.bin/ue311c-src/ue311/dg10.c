/*
 * The routines in this file provide support for the Data General Model 10
 * Microcomputer.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if     DG10

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     30                      /* DG10 ESC character.          */

extern PASCAL NEAR ttopen();               /* Forward references.          */
extern PASCAL NEAR ttgetc();
extern PASCAL NEAR ttputc();
extern PASCAL NEAR ttflush();
extern PASCAL NEAR ttclose();
extern PASCAL NEAR dg10kopen();
extern PASCAL NEAR dg10kclose();
extern PASCAL NEAR dg10move();
extern PASCAL NEAR dg10eeol();
extern PASCAL NEAR dg10eeop();
extern PASCAL NEAR dg10beep();
extern PASCAL NEAR dg10open();
extern PASCAL NEAR dg10rev();
extern PASCAL NEAR dg10close();
extern PASCAL NEAR dg10cres();

#if	COLOR
extern PASCAL NEAR dg10fcol();
extern PASCAL NEAR dg10bcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] = {		/* emacs -> DG10 color translation table */
	0, 4, 2, 6, 1, 5, 3, 7};
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
        dg10open,
        dg10close,
	dg10kopen,
	dg10kclose,
        ttgetc,
        ttputc,
        ttflush,
        dg10move,
        dg10eeol,
        dg10eeop,
        dg10eeop,
        dg10beep,
	dg10rev,
	dg10cres
#if	COLOR
	, dg10fcol,
	dg10bcol
#endif
};

#if	COLOR
PASCAL NEAR dg10fcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc(0101);
	ttputc(ctrans[color]);
	cfcolor = color;
}

PASCAL NEAR dg10bcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc(0102);
	ttputc(ctrans[color]);
        cbcolor = color;
}
#endif

PASCAL NEAR dg10move(row, col)
{
	ttputc(16);
        ttputc(col);
	ttputc(row);
}

PASCAL NEAR dg10eeol()
{
        ttputc(11);
}

PASCAL NEAR dg10eeop()
{
#if	COLOR
	dg10fcol(gfcolor);
	dg10bcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc(0106);
        ttputc(0106);
}

PASCAL NEAR dg10rev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	if (state == TRUE) {
		dg10fcol(0);
		dg10bcol(7);
	}
#else
	ttputc(ESC);
	ttputc(state ? 0104: 0105);
#endif
}

PASCAL NEAR dg10cres()	/* change screen resolution */

{
	return(TRUE);
}

PASCAL NEAR spal()		/* change palette string */

{
	/*	Does nothing here	*/
}

PASCAL NEAR dg10beep()
{
        ttputc(BEL);
        ttflush();
}

PASCAL NEAR dg10open()
{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
        ttopen();
}

PASCAL NEAR dg10close()

{
#if	COLOR
	dg10fcol(7);
	dg10bcol(0);
#endif
	ttclose();
}

PASCAL NEAR dg10kopen()

{
}

PASCAL NEAR dg10kclose()

{
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
dg10hello()
{
}
#endif
