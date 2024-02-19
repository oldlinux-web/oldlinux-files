/*	FMR.C:	Fujitsu FMR series Driver
 *		for MicroEMACS 3.10
 *		(C)Copyright 1990 by Daniel M. Lawrence
 *
 *	Note that this driver relies on GDS.SYS being loaded!
 */

#define	termdef	1			/* don't define term external */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if     FMR

/*	FMR special key definitions	*/

typedef struct KEYDEF {
	unsigned int kf_JIScode;	/* JIS code of key */
	int kf_len;			/* length of key sequence */
	char *kf_def;			/* keystroke sequence */
} KEYDEF;

KEYDEF functab[] = {

	/* F1 - F10 */
	0x8001, 0, NULL,
	0x8002, 0, NULL,
	0x8003, 0, NULL,
	0x8004, 0, NULL,
	0x8005, 0, NULL,
	0x8006, 0, NULL,
	0x8007, 0, NULL,
	0x8008, 0, NULL,
	0x8009, 0, NULL,
	0x800a, 0, NULL,

	/* S-F1 - S-F10 */
	0x800b, 0, NULL,
	0x801d, 0, NULL,
	0x8021, 0, NULL,
	0x8022, 0, NULL,
	0x8023, 0, NULL,
	0x8024, 0, NULL,
	0x8025, 0, NULL,
	0x8026, 0, NULL,
	0x8027, 0, NULL,
	0x8028, 0, NULL,

	/* other special keys */

	/* cursor arrows */
	0x1e, 0, NULL,
	0x1f, 0, NULL,
	0x1d, 0, NULL,
	0x1c, 0, NULL

};

#define	NUMFKEYS	sizeof(functab)/sizeof(KEYDEF)

/*	EMACS internal key sequences mapped from FMR keys	*/

KEYDEF newtab[NUMFKEYS] = {

	/* F1 - F10 */
	0x8001, 3, "\000\0101",
	0x8002, 3, "\000\0102",
	0x8003, 3, "\000\0103",
	0x8004, 3, "\000\0104",
	0x8005, 3, "\000\0105",
	0x8006, 3, "\000\0106",
	0x8007, 3, "\000\0107",
	0x8008, 3, "\000\0108",
	0x8009, 3, "\000\0109",
	0x800a, 3, "\000\0100",

	/* S-F1 - S-F10 */
	0x800b, 3, "\000\0501",
	0x801d, 3, "\000\0502",
	0x8021, 3, "\000\0503",
	0x8022, 3, "\000\0504",
	0x8023, 3, "\000\0505",
	0x8024, 3, "\000\0506",
	0x8025, 3, "\000\0507",
	0x8026, 3, "\000\0508",
	0x8027, 3, "\000\0509",
	0x8028, 3, "\000\0500",

	/* other special keys */

	/* cursor arrows */
	0x1e, 3, "\000\010P",
	0x1f, 3, "\000\010N",
	0x1d, 3, "\000\010B",
	0x1c, 3, "\000\010F"
};

union REGS rg;		/* cpu register for use of DOS calls */
struct SREGS sg;	/* cpu segment registers	     */

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

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

/* Forward references.          */
extern int PASCAL NEAR fmrmove();
extern int PASCAL NEAR fmreeol();
extern int PASCAL NEAR fmreeop();
extern int PASCAL NEAR fmrbeep();
extern int PASCAL NEAR fmropen();
extern int PASCAL NEAR fmrrev();
extern int PASCAL NEAR fmrclose();
extern int PASCAL NEAR fmrkopen();
extern int PASCAL NEAR fmrkclose();
extern int PASCAL NEAR fmrcres();
extern int PASCAL NEAR fmrparm();

unsigned int octype;		/* original cursor type */
unsigned int ocraster;		/* original cursor raster line limits */

#if	COLOR
extern int PASCAL NEAR fmrfcol();
extern int PASCAL NEAR fmrbcol();

static int cfcolor = -1;	/* current forground color */
static int cbcolor = -1;	/* current background color */

int bcmap[16] =			/* background color map */
	{0, 4, 8, 12, 2, 6, 10, 14,
	 1, 5, 9, 13, 3, 7, 11, 15};
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
        fmropen,
        fmrclose,
	fmrkopen,
	fmrkclose,
        ttgetc,
        ttputc,
        ttflush,
        fmrmove,
        fmreeol,
        fmreeop,
        fmreeop,
        fmrbeep,
	fmrrev,
	fmrcres
#if	COLOR
	, fmrfcol,
	fmrbcol
#endif
};

#if	COLOR
PASCAL NEAR fmrfcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
	fmrparm((color & 7)+30);
	ttputc('m');
	cfcolor = color;
}

PASCAL NEAR fmrbcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
#if	0
	ttputc(ESC);
	ttputc('[');
	fmrparm(color+40);
	ttputc('m');
#endif
        cbcolor = color;
}
#endif

PASCAL NEAR fmrmove(row, col)
{
        ttputc(ESC);
        ttputc('[');
        fmrparm(row+1);
        ttputc(';');
        fmrparm(col+1);
        ttputc('H');
}

PASCAL NEAR fmreeol()
{
        ttputc(ESC);
        ttputc('[');
        ttputc('K');
}

PASCAL NEAR fmreeop()
{
#if	COLOR
	fmrfcol(gfcolor);
	fmrbcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('[');
	ttputc('2');
        ttputc('J');

	gds_erase();	/* dump the background colors */
}

PASCAL NEAR fmrrev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
	ttputc(ESC);
	ttputc('[');
	ttputc(state ? '7': '0');
	ttputc('m');
	if (state)
		fmrfcol(7);
}

PASCAL NEAR fmrcres()	/* change screen resolution */

{
	return(TRUE);
}

PASCAL NEAR spal(char *dummy)		/* change pallette settings */

{
	/* none for now */
}

PASCAL NEAR fmrbeep()
{
        ttputc(BEL);
        ttflush();
}

PASCAL NEAR fmrparm(n)
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

PASCAL NEAR fmropen()

{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
        ttopen();

	ttputc(ESC);		/* no attributes set */
	ttputc('[');
	fmrparm(40);
	ttputc('m');

	gds_init();		/* initialize the GDS driver */
}

PASCAL NEAR fmrclose()

{
#if	COLOR
	fmrfcol(7);
	fmrbcol(0);
#endif
	gds_erase();		/* close the GDS driver, clear background */
	ttclose();
}

PASCAL NEAR fmrkopen()	/* open the keyboard */

{
	/* save the original function key definitions */
	savekeys(&functab);
	setkeys(&newtab);

	/* save the cursor type */
	rg.h.ah = 0x0a;		/* read cursor form */
	int86(0x91, &rg, &rg);
	octype = rg.h.al;	/* save old cursor type */
	ocraster = rg.x.dx;	/* save old cursor rasters */

	/* and set it as a block */
	rg.h.ah = 0x09;		/* set cursor form */
	rg.h.al = 0xd1;		/* full-size, fast blink,
				   high intensity, box */
	int86(0x91, &rg, &rg);

	/* make sure the windows are re-framed */
	upwind();
}

PASCAL NEAR fmrkclose()	/* close the keyboard (a noop here) */

{
	setkeys(&functab);
	gds_erase();		/* close the GDS driver, clear bhackground */

	/* retore the original cursor */
	rg.h.ah = 0x09;		/* set cursor form */
	rg.h.al = octype;	/* restore original cursor type */
	rg.x.dx = ocraster;	/* and raster limits */
	int86(0x91, &rg, &rg);
}

savekeys(ftable)	/* save function key definitions */

KEYDEF *ftable;		/* table to save definitions to */

{
	char *sp;	/* ptr to new keystroke definition */
	int index;	/* index into table to save */
	int jindex;	/* an index into the keystroke buffer */
	int len;	/* length of the keystroke buffer */
	char *kptr;	/* ptr into kbuf */
	char kbuf[16];	/* buffer to hold definition */

	/* set up the pointers to the temp keystroke buffer */
	kptr = kbuf;
	sg.ds = FP_SEG(kptr);
	rg.x.di = FP_OFF(kptr);
	kptr[0] = 15; /* set length of buffer */

	/* for each key to save */
	for (index = 0; index < NUMFKEYS; index++) {

		/* set parameters to reading key assignment call */
		rg.h.ah = 0x0f;				/* subfunction code */
		rg.x.dx = ftable[index].kf_JIScode;	/* set key */

		/* call the BIOS for the info */
		int86x(0x90, &rg, &rg, &sg);

		/* and record the length */
		len = rg.x.cx;
		ftable[index].kf_len = len;

		/* and the keystrokes */
		sp = malloc(len);
		if (sp == NULL)
			return;
		for (jindex = 0; jindex < len; jindex++)
			sp[jindex] = kptr[jindex + 1];
		ftable[index].kf_def = sp;
	}
}

setkeys(ftable)	/* set the function key definitions */

KEYDEF *ftable;		/* table to set definitions from */

{
	char *sp;	/* ptr to new keystroke definition */
	int index;	/* index into table to save */
	int jindex;	/* an index into the keystroke buffer */
	int len;	/* length of the keystroke buffer */
	char *kptr;	/* ptr into kbuf */
	char kbuf[16];	/* buffer to hold definition */

	kptr = kbuf;

	/* for each key to set */
	for (index = 0; index < NUMFKEYS; index++) {

		/* set up the pointers to the temp keystroke buffer */
		sg.ds = FP_SEG(kptr);
		rg.x.di = FP_OFF(kptr);

		/* set parameters to reading key assignment call */
		rg.h.ah = 0x0e;				/* subfunction code */
		rg.h.al = 1;				/* add 0 to the key address */
		rg.x.dx = ftable[index].kf_JIScode;	/* set key */
		rg.x.cx = len = ftable[index].kf_len;

		/* copy the keystrokes in */
		sp = ftable[index].kf_def;
		for (jindex = 0; jindex < len; jindex++) {
			kptr[jindex] = sp[jindex];
		}

		/* call the BIOS for the info */
		int86x(0x90, &rg, &rg, &sg);
	}
}

PASCAL NEAR scwrite(row, outstr, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
	/* move to the begining of the destination line */
	fmrmove(row, 0);

	/* set the proper forground color */
	fmrfcol(forg);

	/* write the text to the screen */
	rg.h.ah = 0x1e;			/* Character string output */
	rg.x.cx = term.t_ncol;		/* # of chars to display */
	sg.ds = FP_SEG(outstr);		/* point to string to display */
	rg.x.di = FP_OFF(outstr);
	int86x(0x91, &rg, &rg, &sg);	/* call bios to write */

	/* set the background color for this line */
	gds_bline(row, bacg);

	/* scwrite moves the cursor.... */
	ttrow++;
	ttcol = 0;
}

#if	FLABEL
int PASCAL NEAR fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

/****	FMR  GDS functions		****/

/*	Some globals	*/

unsigned char param[32];	/* parameter block for GDS_outdata */

/* draw operations code definitions */

#define	OP(class, elem, parlen)	class * 0x1000 + elem * 0x20 + parlen
#define setpar(n, val)	param[n] = (val) & 255; param[n+1] = ((val) >> 8) & 255

gds_init()

{
	/* set input parameters */
	rg.h.ah = 0x80;		/* Graphics initialization function code */

	/* call GDS */
	int86x(0x92, &rg, &rg, &sg);

	/* return success if AH == 0 */
	if (rg.h.ah != 0)
		return(FALSE);

	gds_paintmode(1);
	return(TRUE);
}

gds_erase()

{
	/* set input parameters */
	rg.h.ah = 0x84;		/* Graphics initialization function code */

	/* call GDS */
	int86x(0x92, &rg, &rg, &sg);

	/* return success if AH == 0 */
	return(rg.h.ah == 0);
}

gds_outdata(data)	/* execute graphic data commands */

unsigned char *data;	/* graphic command buffer */

{
	/* set input parameters */
	rg.h.ah = 0x8f;		/* Output of graphic data function code */

	sg.ds = FP_SEG(data);	/* set address of command buffer */
	rg.x.di = FP_OFF(data);

	/* call GDS */
	int86x(0x92, &rg, &rg, &sg);

	/* return success if AH == 0 */
	return(rg.h.ah == 0);
}

gds_paintmode(mode)	/* set paint mode */

int mode;	/* forground mode for painting */

{
	/* set command buffer length */
	setpar(0, 4);

	/* set the graphic function code */
	setpar(2, OP(5, 22, 2));

	/* set the mode */
	setpar(4, mode);

	/* send it out */
	return(gds_outdata(param));
}

gds_boxcolor(color)	/* set box color */

int color;

{
	/* set command buffer length */
	setpar(0, 4);

	/* set the graphic function code */
	setpar(2, OP(5, 23, 2));

	/* set the colorion */
	setpar(4, color);

	/* send it out */
	gds_outdata(param);

	/* set command buffer length */
	setpar(0, 4);

	/* set the graphic function code */
	setpar(2, OP(5, 28, 2));

	/* set the colorion */
	setpar(4, color);

	/* send it out */
	return(gds_outdata(param));
}

/* draw a line's background color box */

gds_bline(line, color)

int line;	/* screen line to reset color on */
int color;	/* color to draw background line in */

{
	int typos;	/* top (upper left) coord of rectangle */

	typos = line * 30;

	/* set the proper color for the line */
	gds_boxcolor(bcmap[color]);

	/* set command buffer length */
	setpar(0, 10);

	/* set the graphic function code */
	setpar(2, OP(4, 11, 8));

	/* set the upper left point */
	setpar(4, 0);
	setpar(6, typos);

	/* set the lower right point */
	setpar(8, 1120);
	setpar(10, typos + 30);

	/* send it out */
	return(gds_outdata(param));
}
#else
fmrhello()
{
}
#endif
