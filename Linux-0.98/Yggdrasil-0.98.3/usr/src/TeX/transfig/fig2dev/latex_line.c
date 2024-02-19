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

#include <stdio.h>
#include <math.h>
/* 
 *	latex_line.c: 
 *	    Subroutines for drawing and translating lines for the LaTeX
 *	    picture environment.
 *	Written by Frank Schmuck (schmuck@svax.cs.cornell.edu)
 *	June 1988
 *
 *	The LaTeX picture environment allows generating pictures in standard
 *	LaTeX.  However, some restrictions apply:  lines and vectors (a vector
 *	is a line with an arrow at the end) can only be drawn with a finite
 *	number of slopes.  The available slopes are given by dy/dx where
 *	dx and dy must be integers <= 6 for lines and <= 4 for vectors.
 *
 *	The subroutines in this file are used in fig2latex to approximate
 *	an arbitrary line or vector by a LaTeX line/vector, and in fig to 
 *	restrict line drawing to slopes supported by LaTeX.
 */

/*
 * Constant for converting from radian to degrees
 */
double rad2deg = 57.295779513082320877;

int  pgcd(a,b)
    int a, b;
/*
 *  compute greatest common divisor, assuming 0 < a <= b
 */
{
    b = b % a;
    return (b)? gcd(b, a): a;
}

int  gcd(a, b)
    int a, b;
/*
 *  compute greatest common divisor
 */
{
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    return (a <= b)? pgcd(a, b): pgcd(b, a);
}

int  lcm(a, b)
    int a, b;
/*
 *  Compute least common multiple
 */
{
    return abs(a*b)/gcd(a,b);
}

/*
 *  Tables of line and vector slopes supported by LaTeX
 */
 
struct angle_table {
    int    x, y;
    double angle;
};

#define N_LINE 25

struct angle_table line_angles[N_LINE] =
  { {0, 1, 90.0}, 
    {1, 0,  0.0},
    {1, 1, 45.0}, 
    {1, 2, 63.434948822922010648},
    {1, 3, 71.565051177077989351},
    {1, 4, 75.963756532073521417},
    {1, 5, 78.690067525979786913},
    {1, 6, 80.537677791974382609},
    {2, 1, 26.565051177077989351},
    {2, 3, 56.309932474020213086},
    {2, 5, 68.198590513648188229}, 
    {3, 1, 18.434948822922010648},
    {3, 2, 33.690067525979786913},
    {3, 4, 53.130102354155978703},
    {3, 5, 59.036243467926478582},
    {4, 1, 14.036243467926478588},
    {4, 3, 36.869897645844021297},
    {4, 5, 51.340191745909909396},
    {5, 1, 11.309932474020213086},
    {5, 2, 21.801409486351811770},
    {5, 3, 30.963756532073521417},
    {5, 4, 38.659808254090090604},
    {5, 6, 50.194428907734805993},
    {6, 1, 9.4623222080256173906},
    {6, 5, 39.805571092265194006}
  };

#define N_ARROW 13

struct angle_table arrow_angles[N_ARROW] =
  { {0, 1, 90.0}, 
    {1, 0,  0.0},
    {1, 1, 45.0}, 
    {1, 2, 63.434948822922010648},
    {1, 3, 71.565051177077989351},
    {1, 4, 75.963756532073521417},
    {2, 1, 26.565051177077989351},
    {2, 3, 56.309932474020213086},
    {3, 1, 18.434948822922010648},
    {3, 2, 33.690067525979786913},
    {3, 4, 53.130102354155978703},
    {4, 1, 14.036243467926478588},
    {4, 3, 36.869897645844021297},
  };

get_slope(dx, dy, sxp, syp, arrow)
    int  dx, dy, *sxp, *syp, arrow;
/*
 *  Find the LaTeX line slope that is closest to the one given by dx, dy.
 *  Result is returned in *sxp, *syp.  If (arrow != 0) the closest LaTeX
 *  vector slope is selected.
 */
{
    double angle;
    int    i, s, max;
    double d, d1;
    struct angle_table *st;

    if (dx == 0) {
	*sxp = 0;
	*syp = (dy < 0)? -1: 1;
	return;
    }
    angle = atan((double)abs(dy) / (double)abs(dx)) * rad2deg;
    if (arrow) {
	st = arrow_angles;
	max = N_ARROW;
    } else {
	st = line_angles;
	max = N_LINE;
    }
    s = 0;
    d = 9.9e9;
    for (i = 0; i < max; i++) {
	d1 = fabs(angle - st[i].angle);
	if (d1 < d) {
	    s = i;
	    d = d1;
	} 
    }
    *sxp = st[s].x;
    if (dx < 0) *sxp = -*sxp;
    *syp = st[s].y;
    if (dy < 0) *syp = -*syp;
}

latex_endpoint(x1, y1, x2, y2, xout, yout, arrow, magnet)
    int  x1, y1, x2, y2;
    int  *xout, *yout;
    int  arrow, magnet;
/*
 *  Computes a point "close" to (x2,y2) that is reachable from (x1,y1)
 *  by a LaTeX line (LaTeX vector if arrow != 0).  The result is returned
 *  in *xout, *yout.  If (magnet > 1) the point returned is selected such that
 *  (*xout - x1) and (*yout - y1) are both multiples of magnet.
 */
{
    int    dx, dy, sx, sy, ds, dsx, dsy;

    dx = x2-x1;
    dy = y2-y1;
    get_slope(dx, dy, &sx, &sy, arrow);
    if (abs(sx) >= abs(sy)) {
	ds = lcm(sx, magnet*gcd(sx,magnet));
	dsx = (2*abs(dx)/ds + 1)/2;
	dsx = (dx >= 0)? dsx*ds: -dsx*ds;
	*xout = x1 + dsx;
	*yout = y1 + dsx*sy/sx;
    } else {
	ds = lcm(sy, magnet*gcd(sy,magnet));
	dsy = (2*abs(dy)/ds + 1)/2;
	dsy = (dy >= 0)? dsy*ds: -dsy*ds;
	*yout = y1 + dsy;
	*xout = x1 + dsy*sx/sy;
    }
}
