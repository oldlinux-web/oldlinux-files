/*
 *			D R A W P S . C 
 *
 * $Revision: 1.1 $
 *
 * $Log:	drawPS.c,v $
 * Revision 1.1  88/09/18  15:38:05  grunwald
 * Initial revision
 * 
 * Revision 1.4  87/05/07  15:06:24  dorab
 * relinked back hh to h and vv to v undoing a previous change.
 * this was causing subtle quantization problems. the main change
 * is in the definition of hconvPS and vconvPS.
 * 
 * changed the handling of the UC seal. the PS file should now be
 * sent via the -i option.
 * 
 * Revision 1.3  86/04/29  23:20:55  dorab
 * first general release
 * 
 * Revision 1.3  86/04/29  22:59:21  dorab
 * first general release
 * 
 * Revision 1.2  86/04/29  13:23:40  dorab
 * Added distinctive RCS header
 * 
 */
#ifndef lint
static char RCSid[] = "@(#)$Header: drawPS.c,v 1.1 88/09/18 15:38:05 grunwald Locked $ (UCLA)";
#endif

/*
 the driver for handling the \special commands put out by
 the tpic program as modified by Tim Morgan <morgan@uci.edu>
 the co-ordinate system is with the origin at the top left
 and the x-axis is to the right, and the y-axis is to the bottom.
 when these routines are called, the origin is set at the last dvi position,
 which has to be gotten from the dvi-driver (in this case, dvips) and will
 have to be converted to device co-ordinates (in this case, by [hv]convPS).
 all dimensions are given in milli-inches and must be converted to what
 dvips has set up (i.e. there are convRESOLUTION units per inch).

 it handles the following \special commands
    pn n			set pen size to n
    pa x y			add path segment to (x,y)
    fp				flush path
    da l			flush dashed - each dash is l (inches)
    dt l			flush dotted - one dot per l (inches)
    sp				flush spline
    ar x y xr yr sa ea		arc centered at (x,y) with x-radius xr
    				and y-radius yr, start angle sa (in rads),
    				end angle ea (in rads)
    sh				shade last path (box, circle, ellipse)
    wh				whiten last path (box, circle, ellipse)
    bk				blacken last path (box,circle, ellipse)
    tx				texture command - ignored

  this code is in the public domain

  written by dorab patel <dorab@cs.ucla.edu>
  december 1985
  released feb 1986
  changes for dvips july 1987

  */

#ifdef TPIC			/* rest of the file !! */

#include "structures.h"

#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */


/*
 * external functions used here
 */
extern void cmdout();
extern void znumout();
extern void error();

/*
 * external variables used here
 */
extern shalfword hh,vv;		/* the current x,y position in pixel units */

#define convRESOLUTION DPI
#define TRUE 1
#define FALSE 0
#define tpicRESOLUTION 1000	/* number of tpic units per inch */

/* convert from tpic units to PS units */
#ifdef UNDEF

#define PixRound(a,b) zPixRound((integer)(a),(integer)(b))
#define convPS(x) PixRound((x)*convRESOLUTION,tpicRESOLUTION)

#else

#define	convPS(x)	((int) ((x) * ( ((float) convRESOLUTION) / (float)(tpicRESOLUTION)) + 0.5))

#endif

/* convert from tpic locn to abs horiz PS locn */
#define hconvPS(x) (integer)(hh + convPS(x))
/* convert from tpic locn to abs vert PS locn */
#define vconvPS(x) (integer)(vv + convPS(x))
#define convDeg(x) (integer)(360*(x)/(2*3.14159265358)+0.5) /* convert to degs */

/* if PostScript had splines, i wouldnt need to store the path */
#define MAXPATHS 300		/* maximum number of path segments */
#define NONE 0			/* for shading */
#define GREY 1			/* for shading */
#define WHITE 2			/* for shading */
#define BLACK 3			/* for shading */

/* the following defines are used to make the PostScript shorter - make sure
 that the corresponding defines are in the PostScript prologue */
#define MOVETO "a"
#define LINETO "li"
#define RCURVETO "rc"
#define RLINETO "rl"
#define STROKE "st"
#define FILL "fi"
#define NEWPATH "np"
#define CLOSEPATH "closepath"
#define DOT "dot"
/*
 * STROKE and FILL must restore the current point to that
 * saved by NEWPATH
 */

static integer xx[MAXPATHS], yy[MAXPATHS]; /* the current path in milli-inches */
static integer pathLen = 0;		/* the current path length */
static integer shading = NONE;	/* what to shade the last figure */
static integer penSize = 2;		/* pen size in PS units */

/* forward declarations */
static void doShading();
static integer zPixRound();		/* (int)(x/y)PixRound((int)x,(int)y) */

void
setPenSize(cp)
     char *cp;
{
  long ps;

  if (sscanf(cp, " %ld ", &ps) != 1) 
    {
      error("Illegal .ps command format");
      return;
    }

  penSize = convPS(ps);
  numout(penSize);
  cmdout("setlinewidth\n");
}				/* end setPenSize */

void
addPath(cp)
     char *cp;
{
  long x,y;

  if (++pathLen >= MAXPATHS) error("! Too many points");
  if (sscanf(cp, " %ld %ld ", &x, &y) != 2)
    error("! Malformed path expression");
  xx[pathLen] = x;
  yy[pathLen] = y;
}				/* end of addPath */

void
arc(cp)
     char *cp;
{
  long xc, yc, xrad, yrad;
  float startAngle, endAngle;

  if (sscanf(cp, " %ld %ld %ld %ld %f %f ", &xc, &yc, &xrad, &yrad,
	     &startAngle, &endAngle) != 6)
    {
      error("Illegal arc specification");
      return;
    }

/* we need the newpath since STROKE doesnt do a newpath */
  
    cmdout(NEWPATH);
    numout(hconvPS(xc));
    numout(vconvPS(yc));
    numout(convPS(xrad));
    if (xrad != yrad) numout(convPS(yrad));
    numout(convDeg(startAngle));
    numout(convDeg(endAngle));
    if (xrad == yrad) 		/* for arcs and circles */
      cmdout("arc");
    else
      cmdout("ellipse");

  doShading();
}				/* end of arc */

void
  flushPath()
{
  register int i;
  
  if (pathLen < 2) 
    {
      error("Path less than 2 points - ignored");
      return;
    }
  
#ifdef DEBUG
  if (dd(D_SPECIAL))
    fprintf(stderr,
	    "flushpath(1): hh=%d, vv=%d, x=%d, y=%d, xPS=%d, yPS=%d\n",
	    hh, vv, xx[1], yy[1], hconvPS(xx[1]), vconvPS(yy[1]));
#endif /* DEBUG */
  
  if ((pathLen == 2) && (xx[1] == xx[2] && yy[1] == yy[2])) {
    numout(hconvPS(xx[1]));
    numout(vconvPS(yy[1]));
    cmdout(DOT);
  }
  else {
    cmdout(NEWPATH); /* to save the current point */
    numout(hconvPS(xx[1]));
    numout(vconvPS(yy[1]));
    cmdout(MOVETO);
    for (i=2; i < pathLen; i++) {
#ifdef DEBUG
      if (dd(D_SPECIAL))
	fprintf(stderr,
		"flushpath(%d): hh=%d, vv=%d, x=%d, y=%d, xPS=%d, yPS=%d\n",
		i, hh, vv, xx[i], yy[i], hconvPS(xx[i]), vconvPS(yy[i]));
#endif /* DEBUG */
      numout(hconvPS(xx[i]));
      numout(vconvPS(yy[i]));
      cmdout(LINETO);
    }
    
    if (xx[1] == xx[pathLen] && yy[1] == yy[pathLen])  {
      cmdout(CLOSEPATH);
    }
    else {
      numout(hconvPS(xx[pathLen]));
      numout(vconvPS(yy[pathLen]));
      cmdout(LINETO);
    }
    doShading();
  }
  pathLen = 0;
}				/* end of flushPath */

void
flushDashed(cp, dotted)
     char *cp;
     int dotted;
{
  float inchesPerDash;

  if (sscanf(cp, " %f ", &inchesPerDash) != 1) 
    {
      error ("Illegal format for dotted/dashed line");
      return;
    }
  
  if (inchesPerDash <= 0.0)
    {
      error ("Length of dash/dot cannot be negative");
      return;
    }

  inchesPerDash = 1000 * inchesPerDash;	/* to get milli-inches */
  
  cmdout("[");
  if (dotted) {
    numout(penSize);
    numout(convPS((int)inchesPerDash) - penSize);
  } else				/* if dashed */
    numout(convPS((int)inchesPerDash));

  cmdout("]");
  numout(0);
  cmdout("setdash");

  flushPath();

  cmdout("[] 0 setdash\n");
}				/* end of flushDashed */

void
flushSpline()
{				/* as exact as psdit!!! */
  register long i, dxi, dyi, dxi1, dyi1;

  if (pathLen < 2)
    {
      error("Spline less than two points - ignored");
      return;
    }
  
  cmdout(NEWPATH);	/* to save the current point */
  numout(hconvPS(xx[1]));
  numout(vconvPS(yy[1]));
  cmdout(MOVETO);
  numout(convPS((xx[2]-xx[1])/2));
  numout(convPS((yy[2]-yy[1])/2));
  cmdout(RLINETO);

  for (i=2; i < pathLen; i++)
    {
      dxi = convPS(xx[i] - xx[i-1]);
      dyi = convPS(yy[i] - yy[i-1]);
      dxi1 = convPS(xx[i+1] - xx[i]);
      dyi1 = convPS(yy[i+1] - yy[i]);

      numout(dxi/3);
      numout(dyi/3);
      numout((3*dxi+dxi1)/6);
      numout((3*dyi+dyi1)/6);
      numout((dxi+dxi1)/2);
      numout((dyi+dyi1)/2);
      cmdout(RCURVETO);
    }

  numout(hconvPS(xx[pathLen]));
  numout(vconvPS(yy[pathLen]));
  cmdout(LINETO);

  doShading();
  pathLen = 0;
	 
}				/* end of flushSpline */

void
shadeLast()
{
  shading = GREY;
  cmdout("0.75 setgray\n");
}				/* end of shadeLast */

void
whitenLast()
{
  shading = WHITE;
  cmdout("1 setgray\n");
}				/* end of whitenLast */

void
blackenLast()
{
  shading = BLACK;
  cmdout("0 setgray\n");	/* actually this aint needed */
}				/* end of whitenLast */

static void
doShading()
{
  if (shading) 
    {
      cmdout(FILL);
      shading = NONE;
      cmdout("0 setgray\n");	/* default of black */
    }
  else
    cmdout(STROKE);
}				/* end of doShading */

static integer
zPixRound(x, conv)	/* return rounded number of pixels */
	register integer x;		/* in DVI units */
	register integer conv;		/* conversion factor */
{
    return((integer)((x + (conv >> 1)) / conv));
}

#endif /* TPIC */
