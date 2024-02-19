/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * genepic.c: (E)EPIC driver for fig2dev
 *
 * Converted from fig2epic 5/89 by Micah Beck
 */
/*==================================================================*/
/*	fig2epic (Fig to EPIC converter) 			    */
/*	     Version 1.1d <March 30, 1988>			    */
/*								    */
/*	Written by Conrad Kwok, Division of Computer Science, UCD   */
/*								    */
/*	Permission is granted for freely distribution of this file  */
/*		provided that this message is included.		    */
/*==================================================================*/

/*====================================================================
  Changes:

  Version 1.0d:<September 18, 1988>
  1. Add the option -P for Page mode. Two more configurable parameter---
     Preamble and Postamble.

  Version 1.1a: <January 18, 1989>
  1. Fix the bug in closed control splines. The routine convertCS(p) is being
     called once too often.

  2. Add supports to Variable line width
  3. Add supports to black, white or shaded area fill.

  Version 1.1b: <Febrary 2, 1989>
  1. Draw outline for area-filled figures when using dvips.

  Version 1.1c: <Febrary 7, 1989>
  1. Supports all 5 gray level in area fill.

  Version 1.1d: <March 30, 1989>
  1. Add supports for Gould NP1 (Bsd4.3) (Recieve the changes from
		mcvax!presto.irisa.fr!hleroy@uunet.uu.net. Sorry
		I don't have his/her name)
  2. Add exit(0) before exit in the main.
====================================================================*/

  
#include <stdio.h>
#include <math.h>
#include <varargs.h>
#include <ctype.h>
#include "object.h"
#include "fig2dev.h"
#include "texfonts.h"
#include "pi.h"

#ifdef MSDOS
#define getopt egetopt
#define M_PI 3.14159265358979324
#endif

#define DrawOutLine
#ifdef DrawOutLine
int OutLine=0;
#endif

#define TopCoord 840		/* 10.5 in * 80 (DPI)            */
				/* Actually, it can be any value */
#define PtPerLine 3
#define ThinLines 0
#define ThickLines 1
#define FALSE 0
#define TRUE 1
#define Epic 0
#define EEpic_emu 1
#define EEpic 2
#define None 0
#define SolidLineBox 1
#define DashLineBox 2
#define BothBoxType 3
#define Normal 0
#define Economic 1
#define DottedDash 2

void genepic_ctl_spline(), genepic_int_spline(); 
void genepic_open_spline(), genepic_closed_spline(); 

/* Structure for Point with "double" values */
struct fp_struct {
    double x,y;
};

typedef struct fp_struct FPoint;

/* Local to the file only */
static int CoordSys = 2;
static double Threshold;
static int DPI;
static int CurWidth = 0;
static int LineStyle = SOLID_LINE;
static int LLX = 0, LLY = 0;
static char *LnCmd;
static int MaxCircleRadius;
static double DashLen;
static int PageMode = FALSE;
static int PatternType=UNFILLED;
static struct {
    double mag;
    int size;
} ScaleTbl[5] = {
    { 0.6667, 8 },
    { 0.75  , 9 },
    { 0.8333, 10 },
    { 0.9167, 11 },
    { 1.0   , 12 }
};

/* Definition of Keywords for some of the configurable parameter */
char *Tlangkw[] = { /* The order must match the definition of corr. constants */
    "Epic", "EEpicemu", "EEpic", NULL
};

char *EllCmdkw[] = {
    "ellipse", "oval", NULL
};

char *EllCmdstr[] = {
    "\\%s%s{%d}{%d}}\n", "\\%s%s(%d,%d)}\n"
};

char *FillCommands[] = {
    "", "\\whiten",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\shade", "\\shade",
    "\\shade", "\\blacken"
};

#define TEXT_LINE_SEP '\n'
/* The following two arrays are used to translate characters which
   are special to LaTeX into characters that print as one would expect.
   Note that the <> characters aren't really special LaTeX characters
   but they will not print as themselves unless one is using a font
   like tt. */
char latex_text_specials[] = "\\{}><^~$&#_%";
char *latex_text_mappings[] = {
  "$\\backslash$",
  "$\\{$",
  "$\\}$",
  "$>$",
  "$<$",
  "\\^{}",
  "\\~{}",
  "\\$",
  "\\&",
  "\\#",
  "\\_",
  "\\%"};


/* Configurable parameters */
int LowerLeftX=0, LowerLeftY=0;
double SegLen = 0.0625; /* inch */
int Verbose = FALSE;
int TopMargin = 5;
int BottomMargin = 10;
int DotDist = 5;
int LineThick = 2;
int TeXLang = EEpic;
double DashScale=1;
int EllipseCmd=0;
int UseBox=None;
int DashType=Normal;
char *Preamble="\\documentstyle[epic,eepic]{article}\n\\begin{document}\n\\begin{center}\n";
char *Postamble="\\end{center}\n\\end{document}\n";
int VarWidth=FALSE;

void genepic_option(opt, optarg)
char opt, *optarg;
{
  	int loop, i;

        switch (opt) {
	case 'a':
	    capfonts = 1;
	    break;

	case 'f':
	    for ( i = 1; i <= MAX_FONT + 1; i++ )
		if ( !strcmp(optarg, texfontnames[i]) ) break;

	    if ( i > MAX_FONT + 1 )
		fprintf(stderr,
			"warning: non-standard font name %s\n", optarg);
	
    	    texfontnames[0] = texfontnames[1] = optarg;
	    break;

        case 'l':
            LineThick = atoi(optarg);
            break;

	case 'L':
	    for (loop=0; loop < 3; loop++) {
	    	if (stricmp(optarg, Tlangkw[loop]) == 0) break;
	    }
	    TeXLang = loop;
	    break;


	case 'm':
	    break;

	case 's':
	    font_size = atoi(optarg);
	    if (font_size <= 0 || font_size > MAXFONTSIZE) {
		fprintf(stderr,
			"warning: font size %d out of bounds\n", font_size);
	    }
	    break;

        case 'S':
            loop = atoi(optarg);
            if (loop < 8 || loop > 12) {
            	put_msg("Scale must be between 8 and 12 inclusively\n");
            	exit(1);
            }
            loop -= 8;
            mag = ScaleTbl[loop].mag;
            font_size = ScaleTbl[loop].size;
            break;

        case 'v':
            Verbose = TRUE;
            break;

	case 'w':
	case 'W':
	    VarWidth = opt=='W';
	    break;

	default:
	    put_msg(Err_badarg, opt, "epic");
	    exit(1);
	    break;
        }
}

static fconvertCS(fpt)
FPoint *fpt;
{
    if (CoordSys) {
        fpt->y = TopCoord - fpt->y;
    }
    fpt->x -= LLX;
    fpt->y -= LLY;
}

convertCS(pt)
F_point *pt;
{
    if (CoordSys) {
        pt->y = TopCoord - pt->y;
    }
    pt->x -= LLX;
    pt->y -= LLY;
}

void genepic_start(objects)
F_compound *objects;
{
    int temp;
    F_point pt1, pt2;
    F_arc *arc;
    F_compound *comp;
    F_ellipse *ell;
    F_line *line;
    F_spline *spl;
    F_text *text;

    texfontsizes[0] = texfontsizes[1] = TEXFONTSIZE(font_size);

    switch (TeXLang) {
    case Epic:
        EllipseCmd = 1; /* Oval */
        LnCmd = "drawline";
        break;
    case EEpic_emu:
    case EEpic:
        LnCmd = "path";
        break;
    default:
        put_msg("Program error in main\n");
        break;
    }
    if (PageMode) {
        fputs(Preamble, stdout);
    }

    DPI = objects->nwcorner.x;
    if (DPI <= 0) {
        put_msg("Resolution has to be positive. Default to 80!\n");
        DPI = 80;
    }
    coord_system = objects->nwcorner.y;
    switch (coord_system) {
    case 1:
        CoordSys = 0;
        break;
    case 2:
        CoordSys = 1;
        break;
    default:
        put_msg("Unknown Coordinate system -- %d\n", coord_system);
        exit(1);
    }
    pt1.x = llx;
    pt1.y = lly;
    pt2.x = urx;
    pt2.y = ury;
    convertCS(&pt1);
    convertCS(&pt2);
    if (pt1.x > pt2.x) {
        temp = pt1.x;
        pt1.x = pt2.x;
        pt2.x = temp;
    }
    if (pt1.y > pt2.y) {
        temp = pt1.y;
        pt1.y = pt2.y;
        pt2.y = temp;
    }
    LLX = pt1.x - LowerLeftX;
    LLY = pt1.y - LowerLeftY;
    if (Verbose) {
        fprintf(tfp, "%%\n%% Language in use is %s\n%%\n", Tlangkw[TeXLang]);
    }
    Threshold = 1.0 / DPI * mag;
    fprintf(tfp, "\\setlength{\\unitlength}{%.4fin}\n", Threshold);
    MaxCircleRadius = (int) (40 / 72.27 / Threshold);
    Threshold = SegLen / Threshold;
    fprintf(tfp, "\\begin{picture}(%d,%d)(%d,%d)\n",
           pt2.x-pt1.x, pt2.y-pt1.y + TopMargin + BottomMargin,
           LowerLeftX, LowerLeftY-BottomMargin);
}

void genepic_end()
{
    fprintf(tfp, "\\end{picture}\n");
    if (PageMode)
        fputs(Postamble, stdout);
}

static set_linewidth(w)
int w;
{
    int old_width;

    if (w < 0) return;
    old_width=CurWidth;
    CurWidth = (w >= LineThick) ? (VarWidth ? w : ThickLines) : ThinLines;
    if (old_width != CurWidth) {
	if (CurWidth==ThinLines) {
	    fprintf(tfp, "\\thinlines\n");
	} else if (VarWidth) {
	    fprintf(tfp, "\\allinethickness{%d}%%\n",w);
	} else {
	    fprintf(tfp, "\\thicklines\n");
	}
    }
}

set_pattern(type)
int type;
{
    static unsigned long patterns[3][32] = {
	{ 0xc0c0c0c0, 0, 0, 0, 0, 0, 0, 0,
	  0xc0c0c0c0, 0, 0, 0, 0, 0, 0, 0,
	  0xc0c0c0c0, 0, 0, 0, 0, 0, 0, 0,
	  0xc0c0c0c0, 0, 0, 0, 0, 0, 0, 0},
	{ 0xcccccccc, 0, 0, 0, 0xcccccccc, 0, 0, 0,
	  0xcccccccc, 0, 0, 0, 0xcccccccc, 0, 0, 0,
	  0xcccccccc, 0, 0, 0, 0xcccccccc, 0, 0, 0,
	  0xcccccccc, 0, 0, 0, 0xcccccccc, 0, 0, 0},
	{ 0x55555555, 0, 0x55555555, 0, 0x55555555, 0, 0x55555555, 0,
	  0x55555555, 0, 0x55555555, 0, 0x55555555, 0, 0x55555555, 0,
	  0x55555555, 0, 0x55555555, 0, 0x55555555, 0, 0x55555555, 0,
	  0x55555555, 0, 0x55555555, 0, 0x55555555, 0, 0x55555555, 0}};
    int count, loop1, loop2, i;

    if (type <= WHITE_FILL || type >= BLACK_FILL) return;
    if (type != PatternType) {
	PatternType=type;
	i = ((int) PatternType - WHITE_FILL - 1) / 6;
	fprintf(tfp, "\\texture{");
	count=0;
	for (loop1=4; loop1>0;) {
	    for (loop2=8; loop2>0; loop2--) 
		fprintf(tfp, "%lx ", patterns[i][count++]);
	    if (--loop1 > 0)
		fprintf(tfp, "\n\t");
	    else
		fprintf(tfp, "}%\n");
	}
    }
}

void genepic_line(line)
F_line *line;
{
    F_point *p, *q;
    int pt_count = 0, temp;
    int boxflag = FALSE, llx, lly, urx, ury;
    double dtemp;

    set_linewidth(line->thickness);
    set_style(line->style, line->style_val);
    p = line->points;
    q = p->next;
    convertCS(p);
    if (q == NULL) {
	fprintf(tfp, "\\drawline(%d,%d)(%d,%d)\n", p->x, p->y, p->x, p->y);
	return;
    }
    if (line->type == T_ARC_BOX) { /* A box with rounded corners */
	  fprintf(stderr, "Arc box not implemented; substituting box.\n");
	  line->type = T_BOX;
    }
    if (line->type == T_BOX) {
	if (Verbose) {
	    fprintf(tfp, "%%\n%% A box\n%%\n");
	}
	switch (LineStyle) {
	case SOLID_LINE:
	    if (UseBox == BothBoxType || UseBox == SolidLineBox) {
	        boxflag = TRUE;
	    }
	    break;
	case DASH_LINE:
	    if (UseBox == BothBoxType || UseBox == DashLineBox) {
	        boxflag = TRUE;
	    }
	    break;
	}
	if (boxflag) {
	    llx = urx = p->x;
	    lly = ury = p->y;
	    while (q != NULL) {
	        convertCS(q);
	        if (q->x < llx) {
	            llx = q->x;
	        } else if (q->x > urx) {
	            urx = q->x;
	        }
	        if (q->y < lly) {
	            lly = q->y;
	        } else if (q->y > ury) {
	            ury = q->y;
	        }
	        q = q->next;
	    }
	    switch(LineStyle) {
	    case SOLID_LINE:
	        fprintf(tfp, "\\put(%d,%d){\\framebox(%d,%d){}}\n",
	            llx, lly, urx-llx, ury-lly);
	        break;
	    case DASH_LINE:
		temp = (int) ((urx-llx) / DashLen);
		dtemp = (double) (urx-llx) / temp;
	        fprintf(tfp, "\\put(%d,%d){\\dashbox{%4.3f}(%d,%d){}}\n",
	            llx, lly, dtemp , urx-llx, ury-lly);
	        break;
	    default:
	        put_msg("Program Error! No other line styles allowed.\n");
	        break;
	    }
	    return;
	  }
    }
    set_pattern(line->area_fill);
    convertCS(q);
    if (line->back_arrow) {
	draw_arrow_head(q, p, line->back_arrow->ht, line->back_arrow->wid);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
    switch (LineStyle) {
    case SOLID_LINE:
	if (q->next != NULL && strcmp(LnCmd,"path")==0) {
	    if (line->area_fill < UNFILLED) line->area_fill = UNFILLED;
	    fprintf(tfp, "%s", FillCommands[line->area_fill]);
	}
	fprintf(tfp, "\\%s", LnCmd);
#ifdef DrawOutLine
	if (line->area_fill != UNFILLED && OutLine == 0) OutLine=1;
#endif
	break;
    case DASH_LINE:
        if ((TeXLang==Epic || TeXLang ==EEpic_emu) && DashType == Economic) {
            fprintf(tfp, "\\drawline[-50]");
        } else {
	    fprintf(tfp, "\\dashline{%4.3f}", DashLen);
	}
	break;
    case DOTTED_LINE:
	fprintf(tfp, "\\dottedline{%d}", DotDist);
	break;
    default:
	fprintf(stderr,"Unknown Style\n");
	exit(1);
    }
    fprintf(tfp, "(%d,%d)", p->x, p->y);
    pt_count++;
    while(q->next != NULL) {
	if (++pt_count > PtPerLine) {
	    pt_count=1;
	    fprintf(tfp, "\n\t");
	}
	fprintf(tfp, "(%d,%d)", q->x, q->y);
	p=q;
	q = q->next;
	convertCS(q);
    }
    fprintf(tfp, "(%d,%d)\n", q->x, q->y);
#ifdef DrawOutLine
    if (OutLine == 1) {
	OutLine=0;
	fprintf(tfp, "\\%s", LnCmd);
	p=line->points;
	pt_count=0;
	q=p->next;
	fprintf(tfp, "(%d,%d)", p->x, p->y);
	pt_count++;
	while(q->next != NULL) {
	    if (++pt_count > PtPerLine) {
		pt_count=1;
		fprintf(tfp, "\n\t");
	    }
	    fprintf(tfp, "(%d,%d)", q->x, q->y);
	    p=q;
	    q = q->next;
	}
	fprintf(tfp, "(%d,%d)\n", q->x, q->y);
    }
#endif
    if (line->for_arrow) {
	draw_arrow_head(p, q, line->for_arrow->ht, line->for_arrow->wid);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
}

set_style(style, dash_len)
int style;
float dash_len;
{
    LineStyle = style;
    if (LineStyle == DASH_LINE) {
        switch (DashType) {
        case DottedDash:
            LineStyle = DOTTED_LINE;
            break;
        default:
            DashLen = dash_len * DashScale;
            break;
        }
    }
}


void genepic_spline(spl)
F_spline *spl;
{
    set_linewidth(spl->thickness);
    set_style(SOLID_LINE, 0.0);
    if (int_spline(spl)) {
	genepic_itp_spline(spl);
    } else {
	genepic_ctl_spline(spl);
    }
}

void genepic_ctl_spline(spl)
F_spline *spl;
{
    if (closed_spline(spl)) {
	genepic_closed_spline(spl);
    } else {
	genepic_open_spline(spl);
    }
}

static void genepic_open_spline(spl)
F_spline *spl;
{
    F_point *p, *q, *r;
    FPoint first, mid;
    int pt_count = 0;

    p = spl->points;
    q = p->next;
    convertCS(p);
    convertCS(q);
    if (spl->back_arrow) {
	draw_arrow_head(q, p, spl->back_arrow->ht, spl->back_arrow->wid);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
    if (q->next == NULL) {
	fprintf(tfp, "\\%s(%d,%d)(%d,%d)\n", LnCmd,
	       p->x, p->y, q->x, q->y);
	return;
    }
    if (TeXLang == EEpic || TeXLang == EEpic_emu) {
        fprintf(tfp, "\\spline(%d,%d)\n", p->x, p->y);
        pt_count++;
        while(q->next != NULL) {
             if (++pt_count > PtPerLine) {
                 pt_count=1;
                 fprintf(tfp, "\n\t");
             }
             fprintf(tfp, "(%d,%d)", q->x, q->y);
             p=q;
             q = q->next;
             convertCS(q);
        }
        fprintf(tfp, "(%d,%d)\n", q->x, q->y);
    } else {
        fprintf(tfp, "\\%s(%d,%d)\n", LnCmd, p->x, p->y);
        r = q->next;
        convertCS(r);
        first.x = p->x;
        first.y = p->y;
        while (r->next != NULL) {
            mid.x = (q->x + r->x) / 2.0;
            mid.y = (q->y + r->y) / 2.0;
            chaikin_curve(first.x, first.y, (double) q->x, (double) q->y,
                            mid.x, mid.y);
            first = mid;
            q=r;
            r = r->next;
            convertCS(r);
        }
        chaikin_curve(first.x, first.y, (double) q->x, (double) q->y,
                        (double) r->x, (double) r->y);
        p=q;
        q=r;
	fprintf(tfp, "\n");
    }
    if (spl->for_arrow) {
	draw_arrow_head(p, q, spl->for_arrow->ht, spl->for_arrow->wid);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
}

static void genepic_closed_spline(spl)
F_spline *spl;
{
    F_point *p;
    double cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
    double x1, y1, x2, y2;

    p = spl->points;
    convertCS(p);
    x1 = p->x;  y1 = p->y;
    p = p->next;
    convertCS(p);
    x2 = p->x;  y2 = p->y;
    cx1 = (x1 + x2) / 2;      cy1 = (y1 + y2) / 2;
    cx2 = (x1 + 3 * x2) / 4;  cy2 = (y1 + 3 * y2) / 4;
    for (p = p->next; p != NULL; p = p->next) {
	fprintf(tfp, "\\%s(%.3f,%.3f)", LnCmd, cx1, cy1);
	x1 = x2;  y1 = y2;
	convertCS(p);
	x2 = p->x;  y2 = p->y;
	cx3 = (3 * x1 + x2) / 4;  cy3 = (3 * y1 + y2) / 4;
	cx4 = (x1 + x2) / 2;      cy4 = (y1 + y2) / 2;
	quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);
	fprintf(tfp, "\n");
	cx1 = cx4;  cy1 = cy4;
	cx2 = (x1 + 3 * x2) / 4;  cy2 = (y1 + 3 * y2) / 4;
    }
    x1 = x2;  y1 = y2;
    p = spl->points->next;
    x2 = p->x;  y2 = p->y;
    cx3 = (3 * x1 + x2) / 4;  cy3 = (3 * y1 + y2) / 4;
    cx4 = (x1 + x2) / 2;      cy4 = (y1 + y2) / 2;
    fprintf(tfp, "\\%s(%.3f,%.3f)", LnCmd, cx1, cy1);
    quadratic_spline(cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4);
    fprintf(tfp, "\n");
}

chaikin_curve(a1, b1, a2, b2, a3, b3)
double a1, b1, a2, b2, a3, b3;
{
    double xm1, xmid, xm2, ym1, ymid, ym2;

    if (fabs(a1-a3) < Threshold && fabs(b1-b3) < Threshold) {
        fprintf(tfp, "\t(%.3f,%.3f)\n", a3, b3);
    } else {
        xm1 = (a1 + a2) / 2;
        ym1 = (b1 + b2) / 2;
        xm2 = (a2 + a3) / 2;
        ym2 = (b2 + b3) / 2;
        xmid = (xm1 + xm2) / 2;
        ymid = (ym1 + ym2) / 2;
        chaikin_curve(a1, b1, xm1, ym1, xmid, ymid);
        chaikin_curve(xmid, ymid, xm2, ym2, a3, b3);
    }
}

static quadratic_spline(a1, b1, a2, b2, a3, b3, a4, b4)
double	a1, b1, a2, b2, a3, b3, a4, b4;
{
    double	x1, y1, x4, y4;
    double	xmid, ymid;

    x1 = a1; y1 = b1;
    x4 = a4; y4 = b4;

    xmid = (a2 + a3) / 2;
    ymid = (b2 + b3) / 2;
    if (fabs(x1 - xmid) < Threshold && fabs(y1 - ymid) < Threshold) {
	fprintf(tfp, "\t(%.3f,%.3f)\n", xmid, ymid);
    } else {
	quadratic_spline(x1, y1, ((x1+a2)/2), ((y1+b2)/2),
			 ((3*a2+a3)/4), ((3*b2+b3)/4), xmid, ymid);
    }

    if (fabs(xmid - x4) < Threshold && fabs(ymid - y4) < Threshold) {
	fprintf(tfp, "\t(%.3f,%.3f)\n", x4, y4);
    } else {
	quadratic_spline(xmid, ymid, ((a2+3*a3)/4), ((b2+3*b3)/4),
			 ((a3+x4)/2), ((b3+y4)/2), x4, y4);
    }
}

genepic_itp_spline(spl)
F_spline *spl;
{
    F_point *p1, *p2;
    FPoint pt1l, pt1r, pt2l, pt2r, tmpfpt;
    F_control *cp1, *cp2;

    p1 = spl->points;
    convertCS(p1);
    cp1 = spl->controls;
    pt1l.x = cp1->lx;
    pt1l.y = cp1->ly;
    pt1r.x = cp1->rx;
    pt1r.y = cp1->ry;
    fconvertCS(&pt1l);
    fconvertCS(&pt1r);

    if (spl->back_arrow) {
	tmpfpt.x = p1->x;
	tmpfpt.y = p1->y;
	fdraw_arrow_head(&pt1r, &tmpfpt, 
		spl->back_arrow->ht, spl->back_arrow->wid);
    	if (Verbose) fprintf(tfp, "%%\n");
    }

    for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
	 p1 = p2, pt1r = pt2r, p2 = p2->next, cp2 = cp2->next) {
	fprintf(tfp, "\\%s(%d,%d)", LnCmd, p1->x, p1->y);
	convertCS(p2);
	pt2l.x = cp2->lx;
	pt2l.y = cp2->ly;
	pt2r.x = cp2->rx;
	pt2r.y = cp2->ry;
	fconvertCS(&pt2l);
	fconvertCS(&pt2r);
	bezier_spline((double) p1->x, (double) p1->y,
		      pt1r.x, pt1r.y,
		      pt2l.x, pt2l.y,
		      (double) p2->x, (double) p2->y);
	fprintf(tfp, "\n");
    }

    if (spl->for_arrow) {
	tmpfpt.x = p1->x;
	tmpfpt.y = p1->y;
	fdraw_arrow_head(&pt2l, &tmpfpt, 
			 spl->for_arrow->ht, spl->for_arrow->wid);
	if (Verbose) fprintf(tfp, "%%\n");
    }
}

static bezier_spline(a0, b0, a1, b1, a2, b2, a3, b3)
double	a0, b0, a1, b1, a2, b2, a3, b3;
{
    double	x0, y0, x3, y3;
    double	sx1, sy1, sx2, sy2, tx, ty, tx1, ty1, tx2, ty2, xmid, ymid;

    x0 = a0; y0 = b0;
    x3 = a3; y3 = b3;
    if (fabs(x0 - x3) < Threshold && fabs(y0 - y3) < Threshold) {
	fprintf(tfp, "\t(%.3f,%.3f)\n", x3, y3);
    } else {
	tx = (a1 + a2) / 2;		ty = (b1 + b2) / 2;
	sx1 = (x0 + a1) / 2;	sy1 = (y0 + b1) / 2;
	sx2 = (sx1 + tx) / 2;	sy2 = (sy1 + ty) / 2;
	tx2 = (a2 + x3) / 2;	ty2 = (b2 + y3) / 2;
	tx1 = (tx2 + tx) / 2;	ty1 = (ty2 + ty) / 2;
	xmid = (sx2 + tx1) / 2;	ymid = (sy2 + ty1) / 2;

	bezier_spline(x0, y0, sx1, sy1, sx2, sy2, xmid, ymid);
	bezier_spline(xmid, ymid, tx1, ty1, tx2, ty2, x3, y3);
    }
}

void genepic_ellipse(ell)
F_ellipse *ell;
{
    F_point pt;

    set_linewidth(ell->thickness);
    pt.x = ell->center.x;
    pt.y = ell->center.y;
    convertCS(&pt);
    if (TeXLang == EEpic || TeXLang == EEpic_emu ||
	  ell->radiuses.x != ell->radiuses.y ||
          ell->radiuses.x > MaxCircleRadius) {
	set_pattern(ell->area_fill);
        fprintf(tfp, "\\put(%d,%d){", pt.x, pt.y );
#ifndef OLDCODE
        if (EllipseCmd == 0) {
	    if (ell->area_fill < UNFILLED) ell->area_fill = UNFILLED;
	    fprintf(tfp, "%s", FillCommands[ell->area_fill]);
#  ifdef DrawOutLine
	    if (ell->area_fill != UNFILLED && OutLine == 0) OutLine = 1;
#  endif
        }
 	fprintf(tfp, EllCmdstr[EllipseCmd],EllCmdkw[EllipseCmd], "",
	       2 * ell->radiuses.x, 2 * ell->radiuses.y);
#  ifdef DrawOutLine
	if (OutLine == 1) {
	    OutLine=0;
            fprintf(tfp, "\\put(%d,%d){", pt.x, pt.y );
	    fprintf(tfp, EllCmdstr[EllipseCmd],EllCmdkw[EllipseCmd], "",
		   2 * ell->radiuses.x, 2 * ell->radiuses.y);
	}
#  endif
#else
	fprintf(tfp, EllCmdstr[EllipseCmd], EllCmdkw[EllipseCmd],
	       (EllipseCmd==0 && ell->area_fill==BLACK_FILL ? "*" : ""),
	       2 * ell->radiuses.x, 2 * ell->radiuses.y);
#endif
    } else {
        fprintf(tfp, "\\put(%d,%d){\\circle", pt.x, pt.y);
        if (ell->area_fill == BLACK_FILL) {
            fputc('*', tfp);
        }
        fprintf(tfp, "{%d}}\n", 2*ell->radiuses.x);
    }
}

extern char *ISOtoTeX[];
void genepic_text(text)
F_text *text;
{
    F_point pt;
    char *tpos, *esc_cp, *special_index;
    unsigned char   *cp;

    pt.x=text->base_x;
    pt.y=text->base_y;
    convertCS(&pt);
    switch (text->type) {
    case T_LEFT_JUSTIFIED:
    case DEFAULT:
	tpos = "[lb]";
	break;
    case T_CENTER_JUSTIFIED:
	tpos = "[b]";
	break;
    case T_RIGHT_JUSTIFIED:
	tpos = "[rb]";
	break;
    default:
	fprintf(stderr, "unknown text position type\n");
	exit(1);
    }
    fprintf(tfp, "\\put(%d,%d){\\makebox(0,0)%s{\\raisebox{0pt}[0pt][0pt]{",
           pt.x, pt.y, tpos);
    /* Output a shortstack in case there are multiple lines. */
    fprintf(tfp, "\\shortstack" );
    /* Output the justification for the shortstack. */
    switch (text->type) {
    case T_LEFT_JUSTIFIED:
    case DEFAULT:
	fprintf(tfp, "[l]");
	break;
    case T_CENTER_JUSTIFIED:
	break;
    case T_RIGHT_JUSTIFIED:
	fprintf(tfp, "[r]");
	break;
    default:
	fprintf(stderr, "unknown text position type\n");
	exit(1);
    }

    unpsfont(text);
    fprintf(tfp, "{{\\%s%s ", TEXFONTMAG(text), TEXFONT(text->font));
    if (!special_text(text))
	/* This loop escapes special LaTeX characters. */
	for(cp = (unsigned char*)text->cstring; *cp; cp++) {
      	    if (special_index=strchr(latex_text_specials, *cp)) {
	      /* Write out the replacement.  Implementation note: we can't
		 use puts since that will output an additional newline. */
	      esc_cp=latex_text_mappings[special_index-latex_text_specials];
	      while (*esc_cp)
		fputc(*esc_cp++, tfp);
	    }
	    else if (*cp == TEXT_LINE_SEP) {
	      /* Handle multi-line text strings. The problem being addressed here
		 is a LaTeX bug where LaTeX is unable to handle a font which
		 spans multiple lines.  What we are doing here is closing off
		 the current font, starting a new line, and then resuming with
		 the current font. */
	      fprintf(tfp, "} \\\\\n");
	      fprintf(tfp, "{\\%s%s ", TEXFONTMAG(text), TEXFONT(text->font));
	    }
	    else
		fputc(*cp, tfp);
      	}
    else 
	for(cp = (unsigned char*)text->cstring; *cp; cp++) {
	  if (*cp == TEXT_LINE_SEP) {
	      /* Handle multi-line text strings. */
	      fprintf(tfp, "} \\\\\n");
	      fprintf(tfp, "{\\%s%s ", TEXFONTMAG(text), TEXFONT(text->font));
	    }
	    else
	        if (*cp >= 0xa0)	/* we escape 8-bit char */
	    		fprintf(tfp, "%s", ISOtoTeX[(int)*cp-0xa0]);
		else
			fputc(*cp, tfp);
	  }
    fprintf(tfp, "}}}}}\n");
}

void genepic_arc(arc)
F_arc *arc;
{
    FPoint pt1, pt2, ctr, tmp;
    double r1, r2, th1, th2, theta;
    double dx1, dy1, dx2, dy2;
    double arrowfactor;

    ctr.x = arc->center.x;
    ctr.y = arc->center.y;
    pt1.x = arc->point[0].x;
    pt1.y = arc->point[0].y;
    pt2.x = arc->point[2].x;
    pt2.y = arc->point[2].y;
    fconvertCS(&ctr);
    fconvertCS(&pt1);
    fconvertCS(&pt2);

    dx1 = pt1.x - ctr.x;
    dy1 = pt1.y - ctr.y;
    dx2 = pt2.x - ctr.x;
    dy2 = pt2.y - ctr.y;

    rtop(dx1, dy1, &r1, &th1);
    rtop(dx2, dy2, &r2, &th2);
    arrowfactor = (r1+r2) / 30.0;
    if (arrowfactor > 1) arrowfactor = 1;
    set_linewidth(arc->thickness);
    if (arc->for_arrow) {
	arc_tangent(&ctr, &pt2, arc->direction, &tmp);
	fdraw_arrow_head(&tmp, &pt2,
			 arc->for_arrow->ht*arrowfactor,
			 arc->for_arrow->wid*arrowfactor);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
    if (arc->back_arrow) {
	arc_tangent(&ctr, &pt1, !arc->direction, &tmp);
	fdraw_arrow_head(&tmp, &pt1,
			 arc->back_arrow->ht*arrowfactor,
			 arc->back_arrow->wid*arrowfactor);
    	if (Verbose) fprintf(tfp, "%%\n");
    }
    if (TeXLang == EEpic) {
	set_pattern(arc->area_fill);
        fprintf(tfp, "\\put(%4.3lf,%4.3lf){", ctr.x, ctr.y);
    } else {
	fprintf(tfp, "\\drawline");
    }
    if (TeXLang == EEpic) {
	if (arc->area_fill < UNFILLED) arc->area_fill = UNFILLED;
	fprintf(tfp, "%s", FillCommands[arc->area_fill]);
#ifdef DrawOutLine
	if (arc->area_fill != UNFILLED && OutLine==0) OutLine=1;
#endif
    }
    if (arc->direction) {
	theta = th2 - th1;
	if (theta < 0) theta += 2 * M_PI;
	th2 = 2*M_PI-th2;
	if (TeXLang == EEpic) {
	    fprintf(tfp, "\\arc{%4.3f}{%2.4f}{%2.4f}}\n", 2*r1, th2, th2+theta);
#ifdef DrawOutLine
	    if (OutLine==1) {
		OutLine=0;
	        fprintf(tfp, "\\put(%4.3lf,%4.3lf){", ctr.x, ctr.y);
		fprintf(tfp, "\\arc{%4.3f}{%2.4f}{%2.4f}}\n", 2*r1, th2, th2+theta);
	    }
#endif
        } else {
            drawarc(&ctr, r1, 2*M_PI - th2 - theta, theta);
        }
    } else {
	theta = th1 - th2;
	if (theta < 0) theta += 2 * M_PI;
	th1 = 2*M_PI-th1;
	if (TeXLang == EEpic) {
	    fprintf(tfp, "\\arc{%4.3f}{%2.4f}{%2.4f}}\n", 2*r2, th1, th1+theta);
#ifdef DrawOutLine
	    if (OutLine==1) {
		OutLine=0;
		fprintf(tfp, "\\arc{%4.3f}{%2.4f}{%2.4f}}\n", 2*r2, th1, th1+theta);
	    }
#endif
        } else {
            drawarc(&ctr, r2, 2*M_PI - th1 - theta, theta);
        }
    }
}

drawarc(ctr, r, th1, angle)
FPoint *ctr;
double r, th1, angle;
{
    double arclength, delta;
    int division, pt_count = 0;


    division = angle * r / Threshold;
    delta = angle / division;
    division++;
    while (division-- > 0) {
        if (++pt_count > PtPerLine) {
            fprintf(tfp, "\n\t");
            pt_count = 1;
        }
        fprintf(tfp, "(%.3lf,%.3lf)", ctr->x + cos(th1) * r,
                                ctr->y + sin(th1) * r);
        th1 += delta;
    }
    fprintf(tfp, "\n");
}

static arc_tangent(pt1, pt2, direction, pt3)
FPoint *pt1, *pt2, *pt3;
int direction;
{
    if (direction) {
	pt3->x = pt2->x + (pt2->y - pt1->y);
	pt3->y = pt2->y - (pt2->x - pt1->x);
    } else {
	pt3->x = pt2->x - (pt2->y - pt1->y);
	pt3->y = pt2->y + (pt2->x - pt1->x);
    }
}

rtop(x, y, r, th)
double x, y, *r, *th;
{
    *r = hypot(x,y);
    *th = acos(x/(*r));
    if (*th < 0) *th = M_PI + *th;
    if (y < 0) *th = 2*M_PI - *th;
}

static draw_arrow_head(pt1, pt2, arrowht, arrowwid)
F_point *pt1, *pt2;
double arrowht, arrowwid;
{
    FPoint fpt1, fpt2;

    fpt1.x = pt1->x;
    fpt1.y = pt1->y;
    fpt2.x = pt2->x;
    fpt2.y = pt2->y;
    fdraw_arrow_head(&fpt1, &fpt2, arrowht, arrowwid);
}

fdraw_arrow_head(pt1, pt2, arrowht, arrowwid)
FPoint *pt1, *pt2;
double arrowht, arrowwid;
{
    double x1, y1, x2, y2;
    double x,y, xb,yb,dx,dy,l,sina,cosa;
    double xc, yc, xd, yd;

    x1 = pt1->x;
    y1 = pt1->y;
    x2 = pt2->x;
    y2 = pt2->y;

    dx = x2 - x1;  dy = y1 - y2;
    l = hypot(dx,dy);
    if (l == 0) {
	 return;
    }
    else {
	 sina = dy / l;  cosa = dx / l;
    }
    xb = x2*cosa - y2*sina;
    yb = x2*sina + y2*cosa;
    x = xb - arrowht;
    y = yb - arrowwid / 2;
    xc = x*cosa + y*sina;
    yc = -x*sina + y*cosa;
    y = yb + arrowwid / 2;
    xd = x*cosa + y*sina;
    yd = -x*sina + y*cosa;

    if (Verbose) fprintf(tfp, "%%\n%% arrow head\n%%\n");

    fprintf(tfp, "\\%s(%4.3f,%4.3f)(%4.3f,%4.3f)(%4.3f,%4.3f)\n", LnCmd,
		xc, yc, x2, y2, xd, yd);
}

#ifndef MSDOS
stricmp(s, t)
char *s, *t;
{
    char a, b;

    for (;;) {
        a= *s++; b= *t++;
        a = islower(a) ? toupper(a) : a;
        b = islower(b) ? toupper(b) : b;
        if (a != b) break;
        if (a == '\0') return(0);
    }
    return(a - b);
}
#endif

struct driver dev_epic = {
     	genepic_option,
	genepic_start,
	genepic_arc,
	genepic_ellipse,
	genepic_line,
	genepic_spline,
	genepic_text,
	genepic_end,
	INCLUDE_TEXT
};
