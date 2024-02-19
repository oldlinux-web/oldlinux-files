/* Copyright (C) 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* gxpcopy.c */
/* Path copying and flattening for Ghostscript library */
#include "math_.h"
#include "gx.h"
#include "gserrors.h"
#include "gxfixed.h"
#include "gxarith.h"
#include "gzpath.h"

/* Forward declarations */
private int copy_path(P4(const gx_path *, gx_path *, fixed, int));
private int flatten_recur(P8(gx_path *,
  fixed, fixed, fixed, fixed, fixed, fixed, fixed));
private int flatten_sample(P8(gx_path *, int,
  fixed, fixed, fixed, fixed, fixed, fixed));

/* Copy a path */
int
gx_path_copy(const gx_path *ppath_old, gx_path *ppath, int init)
{	return copy_path(ppath_old, ppath, max_fixed, init);
}

/* Flatten a path. */
/* If flatness is zero, use sampling rather than subdivision: */
/* this is important for Type 1 fonts. */
private fixed
scale_flatness(floatp flatness)
{	fixed scaled_flat = float2fixed(flatness);
	return (scaled_flat > int2fixed(100) ? int2fixed(100) :
		scaled_flat < fixed_half ? fixed_0 :
		scaled_flat);
}
int
gx_path_flatten(const gx_path *ppath_old, gx_path *ppath, floatp flatness, int in_BuildChar)
{	return copy_path(ppath_old, ppath,
			 (in_BuildChar ? fixed_0 : scale_flatness(flatness)),
			 1);
}

/* Add a flattened curve to a path. */
int
gx_path_add_flattened_curve(gx_path *ppath,
  fixed x1, fixed y1, fixed x2, fixed y2, fixed x3, fixed y3,
  floatp flatness)
{	return flatten_recur(ppath, x1, y1, x2, y2, x3, y3,
			     scale_flatness(flatness));
}

/* Copy a path, optionally flattening it. */
/* If the copy fails, free the new path. */
private int
copy_path(const gx_path *ppath_old, gx_path *ppath, fixed scaled_flat,
  int init)
{	gx_path old;
	const segment *pseg;
	int code;
#ifdef DEBUG
if ( gs_debug['p'] )
	gx_dump_path(ppath_old, "before copy_path");
#endif
	old = *ppath_old;
	if ( init )
		gx_path_init(ppath, &ppath_old->memory_procs);
	pseg = (const segment *)(old.first_subpath);
	while ( pseg )
	   {	switch ( pseg->type )
		   {
		case s_start:
			code = gx_path_add_point(ppath, pseg->pt.x, pseg->pt.y);
			break;
		case s_curve:
		   {	curve_segment *pc = (curve_segment *)pseg;
			if ( scaled_flat == max_fixed )	/* don't flatten */
				code = gx_path_add_curve(ppath,
					pc->p1.x, pc->p1.y,
					pc->p2.x, pc->p2.y,
					pc->pt.x, pc->pt.y);
			else
				code = flatten_recur(ppath,
					pc->p1.x, pc->p1.y,
					pc->p2.x, pc->p2.y,
					pc->pt.x, pc->pt.y,
					scaled_flat);
			break;
		   }
		case s_line:
			code = gx_path_add_line(ppath, pseg->pt.x, pseg->pt.y);
			break;
		case s_line_close:
			code = gx_path_close_subpath(ppath);
			break;
		   }
		if ( code )
		   {	gx_path_release(ppath);
			if ( ppath == ppath_old )
				*ppath = old;
			return code;
		   }
		pseg = pseg->next;
	}
	if ( !old.subpath_open && old.position_valid )
		gx_path_add_point(ppath, old.position.x, old.position.y);
#ifdef DEBUG
if ( gs_debug['p'] )
	gx_dump_path(ppath, "after copy_path");
#endif
	return 0;
}
/* Internal routine to flatten a curve. */
/* This calls itself recursively, using binary subdivision, */
/* until the approximation is good enough to satisfy the */
/* flatness requirement.  The starting point is ppath->position, */
/* which gets updated as line segments are added. */

/* Maximum number of points for sampling if we want accurate rasterizing. */
/* (num_sample_max - 1)^3 must fit into an int. */
#define num_sample_max (1 << ((sizeof(int) * 8 - 1) / 3))

/* Table of f(i) = 256 * sqrt(1 + (i/64)^2). */
/* This is good to within 1% or better. */
#define sqrt_index_shift 6		/* scaling of index */
#define sqrt_value_shift 8		/* scaling of value */
private int scaled_sqrt_tab[65] =
   {	256, 256, 256, 256, 256, 256, 257, 257,
	257, 258, 259, 259, 260, 261, 262, 262,
	263, 264, 265, 267, 268, 269, 270, 272,
	273, 274, 276, 277, 279, 281, 282, 284,
	286, 288, 289, 291, 293, 295, 297, 299,
	301, 304, 306, 308, 310, 312, 315, 317,
	320, 322, 324, 327, 329, 332, 334, 337,
	340, 342, 345, 348, 350, 353, 356, 359,
	362
   };

private int
flatten_recur(gx_path *ppath,
  fixed x1, fixed y1, fixed x2, fixed y2, fixed x3, fixed y3,
  fixed scaled_flat)
{	fixed
	  x0 = ppath->position.x,
	  y0 = ppath->position.y;
top:
#ifdef DEBUG
if ( gs_debug['2'] )
	dprintf4("[2]x0=%f y0=%f x1=%f y1=%f\n",
		 fixed2float(x0), fixed2float(y0),
		 fixed2float(x1), fixed2float(y1)),
	dprintf4("   x2=%f y2=%f x3=%f y3=%f\n",
		 fixed2float(x2), fixed2float(y2),
		 fixed2float(x3), fixed2float(y3));
#endif
	/*
	 * Compute the maximum distance of the curve from
	 * the line (x0,y0)->(x3,y3).  We do this conservatively
	 * by observing that the curve is enclosed by the
	 * quadrilateral of its control points, so we simply
	 * compute the distances of (x1,y1) and (x2,y2)
	 * from the line.  Letting dx = x3-x0 and dy = y3-y0,
	 * the distance of (xp,yp) from the line is
	 * abs(N)/sqrt(D), where N = dy*(xp-x0)-dx*(yp-y0) and
	 * D = dx*dx+dy*dy; hence we want to test abs(N) <= sqrt(D)*F,
	 * where F is the flatness parameter from the graphics state.
	 * We can do this more efficiently by letting t=dy/dx, and
	 * testing abs(N1) <= sqrt(D1)*f, where N1=t*(xp-x0)-(yp-y0) and
	 * D1 = 1+t*t.  If dx < dy, we swap x and y for this
	 * computation.  This guarantees abs(t) <= 1, which allows us to
	 * compute sqrt(1+t*t) by table lookup on the high bits of abs(t).
	 *
	 * To avoid replacing shallow curves by short straight lines,
	 * we halve the flatness if the curve is very short.
	 * We don't do anything about long, nearly flat curves.
	 *
	 * Note that if scaled_flat is very small, we don't do any of this;
	 * instead, we just check whether abs(dx) and abs(dy) are
	 * small enough to switch over to sampling rather than dividing.
 	 */
	 { fixed dx3 = x3 - x0;
	   fixed adx3 = any_abs(dx3);
	   fixed dy3 = y3 - y0;
	   fixed ady3 = any_abs(dy3);
	   fixed flat = scaled_flat;
	   /* We have to be quite careful to ensure that */
	   /* none of the multiplications will overflow. */
#define short_max 0x7ff0L
#define reduce_3(ad3, maxv)\
  while ( ad3 > maxv )\
    adx3 >>= 1, ady3 >>= 1,\
    dx3 = arith_rshift_1(dx3), dy3 = arith_rshift_1(dy3)
#define reduce_d(d)\
  for ( shift = 0; (d < 0 ? d < -short_max : d > short_max); shift++ )\
    d = arith_rshift_1(d)
	   if ( adx3 > ady3 )
	    {	fixed d, dx, dy, dist;
		int shift;
		if ( scaled_flat == 0 )
		{ if ( adx3 < int2fixed(num_sample_max / 2) )
		  { int n = fixed2int_var_rounded(adx3) * 2 + 1;
		    return flatten_sample(ppath, max(n, 3), x1, y1,
					  x2, y2, x3, y3);
		  }
		  else goto sub;
		}
		if ( adx3 < 16 ) flat >>= 1;
		reduce_3(ady3, short_max);
		d = (scaled_sqrt_tab[(ady3 << sqrt_index_shift) / adx3] * flat) >> sqrt_value_shift;
		dx = x1 - x0, dy = y1 - y0;
		reduce_d(dx);
		if ( ((dist = ((dx * dy3 / dx3) << shift) - dy) < 0 ?
		      -dist : dist) > d )
		  goto sub;	/* not flat enough */
		dx = x2 - x0, dy = y2 - y0;
		reduce_d(dx);
		if ( ((dist = ((dx * dy3 / dx3) << shift) - dy) < 0 ?
		      -dist : dist) > d )
		  goto sub;	/* not flat enough */
	    }
	   else if ( ady3 != 0 )
	    {	fixed d, dy, dx, dist;
		int shift;
		if ( scaled_flat == 0 )
		{ if ( ady3 < int2fixed(num_sample_max / 2) )
		  { int n = fixed2int_var_rounded(ady3) * 2 + 1;
		    return flatten_sample(ppath, max(n, 3), x1, y1,
					  x2, y2, x3, y3);
		  }
		  else goto sub;
		}
		if ( ady3 < 16 ) flat >>= 1;
		reduce_3(adx3, short_max);
		d = (scaled_sqrt_tab[(adx3 << sqrt_index_shift) / ady3] * flat) >> sqrt_value_shift;
		dy = y1 - y0, dx = x1 - x0;
		reduce_d(dy);
		if ( ((dist = ((dy * dx3 / dy3) << shift) - dx) < 0 ?
		      -dist : dist) > d )
		  goto sub;	/* not flat enough */
		dy = y2 - y0, dx = x2 - x0;
		reduce_d(dy);
		if ( ((dist = ((dy * dx3 / dy3) << shift) - dx) < 0 ?
		      -dist : dist) > d )
		  goto sub;	/* not flat enough */
	    }
	   else				/* adx3 = ady3 = 0 */
	    {	/* (x0,y0) is the same point as (x3,y3). */
		/* This is an anomalous case.  If the entire curve */
		/* is a single point, stop now, otherwise subdivide. */
		if ( x1 != x0 || y1 != y0 || x2 != x0 || y2 != y0 )
		  goto sub;
	    }
	 }
	/* Curve is flat enough.  Add a line and exit. */
	if_debug2('2', "[2]\t*** x=%f, y=%f ***\n",
		  fixed2float(x3), fixed2float(y3));
	return gx_path_add_line(ppath, x3, y3);

	/* Curve isn't flat enough.  Break into two pieces and recur. */
	/* Algorithm is from "The Beta2-split: A special case of the */
	/* Beta-spline Curve and Surface Representation," B. A. Barsky */
	/* and A. D. DeRose, IEEE, 1985, courtesy of Crispin Goswell. */
sub:
	/* We have to define midpoint carefully to avoid overflow. */
	/* (If it overflows, something really pathological is going on, */
	/* but we could get infinite recursion that way....) */
#define midpoint(a,b)\
  (arith_rshift_1(a) + arith_rshift_1(b) + ((a) & (b) & 1))
   {	fixed x01 = midpoint(x0, x1), y01 = midpoint(y0, y1);
	fixed x12 = midpoint(x1, x2), y12 = midpoint(y1, y2);
	fixed x02 = midpoint(x01, x12), y02 = midpoint(y01, y12);
	int code;
	/* Update x/y1, x/y2, and x/y0 now for the second half. */
	x2 = midpoint(x2, x3), y2 = midpoint(y2, y3);
	x1 = midpoint(x12, x2), y1 = midpoint(y12, y2);
	code = flatten_recur(ppath, x01, y01, x02, y02,
		(x0 = midpoint(x02, x1)), (y0 = midpoint(y02, y1)),
		scaled_flat);
	if ( code < 0 ) return code;
   }	goto top;
}

/* Flatten a segment of the path by repeated sampling. */
/* n is the number of points to sample, including the endpoints; */
/* we require n >= 3. */
private int
flatten_sample(gx_path *ppath, int n,
  fixed x1, fixed y1, fixed x2, fixed y2, fixed x3, fixed y3)
{	const fixed
		x0 = ppath->position.x,
		y0 = ppath->position.y;
	/* We spell out some multiplies by 3, */
	/* for the benefit of compilers that don't optimize this. */
	const fixed
		x01 = x1 - x0,
		cx = (x01 << 1) + x01,		/* 3*(x1-x0) */
		x12 = x2 - x1,
		bx = (x12 << 1) + x12 - cx,	/* 3*(x2-2*x1+x0) */
		ax = x3 - bx - cx - x0;		/* x3-3*x2+3*x1-x0 */
	const fixed
		y01 = y1 - y0,
		cy = (y01 << 1) + y01,
		y12 = y2 - y1,
		by = (y12 << 1) + y12 - cy,
		ay = y3 - by - cy - y0;
	const int
		n1 = n - 1,
		n12 = n1 * n1,
		n13 = n12 * n1;
	fixed ptx = x0, pty = y0;
	fixed x, y;
	/*
	 * If all the coefficients lie between min_fast and max_fast,
	 * we can do everything in fixed point.  In this case we compute
	 * successive values by finite differences, using the formulas:
		x(t) =
		  a*t^3 + b*t^2 + c*t + d =>
		dx(t) = x(t+e)-x(t) =
		  a*(3*t^2*e + 3*t*e^2 + e^3) + b*(2*t*e + e^2) + c*e =
		  (3*a*e)*t^2 + (3*a*e^2 + 2*b*e)*t + (a*e^3 + b*e^2 + c*e) =>
		d2x(t) = dx(t+e)-dx(t) =
		  (3*a*e)*(2*t*e + e^2) + (3*a*e^2 + 2*b*e)*e =
		  (6*a*e^2)*t + (6*a*e^3 + 2*b*e^2) =>
		d3x(t) = d2x(t+e)-d2x(t) =
		  6*a*e^3;
		x(0) = d, dx(0) = (a*e^3 + b*e^2 + c*e),
		  d2x(0) = 6*a*e^3 + 2*b*e^2;
	 * In these formulas, e = 1/n1; of course, there are separate
	 * computations for the x and y values.
	 */
#define max_fast (max_fixed / 6)
#define min_fast (-max_fast)
	int fast;
	float dt;		/* only if !fast */
	int i;
	/*
	 * We do exact rational arithmetic to avoid accumulating error.
	 * Each quantity is represented as I+R/n13, where I is an "integer"
	 * and the "remainder" R lies in the range 0 <= R < n13.  Note that
	 * R may temporarily exceed n13, and hence possibly overflow.
	 */
	fixed idx, idy, id2x, id2y, id3x, id3y;		/* I */
	int rx, ry, rdx, rdy, rd2x, rd2y, rd3x, rd3y;

	if_debug6('2', "[2]ax=%f bx=%f cx=%f\n   ay=%f by=%f cy=%f\n",
		  fixed2float(ax), fixed2float(bx), fixed2float(cx),
		  fixed2float(ay), fixed2float(by), fixed2float(cy));
#define in_range(v) (v < max_fast && v > min_fast)
	if ( n1 < num_sample_max &&	/* so n13 fits */
	     in_range(ax) && in_range(bx) && in_range(cx) &&
	     in_range(ay) && in_range(by) && in_range(cy)
	   )
	{	fast = 1;
		x = x0, y = y0;
		rx = ry = 0;
		/* Fast check for n == 3, a common special case */
		/* for small characters. */
		if ( n == 3 )
#define poly2(a,b,c)\
  arith_rshift_1(arith_rshift_1(arith_rshift_1(a) + b) + c)
			idx = poly2(ax, bx, cx),
			idy = poly2(ay, by, cy),
			rdx = rdy = 0;
#undef poly2
		else
		{	fixed bx2 = bx << 1, by2 = by << 1;
			fixed ax6 = ((ax << 1) + ax) << 1,
			      ay6 = ((ay << 1) + ay) << 1;
			fixed qx, qy;
#define adjust_rem(r, q)\
  if ( r < 0 ) r += n13, q--
#define adjust_rem_loop(r, q)\
  while ( r < 0 ) r += n13, q--;\
  while ( r >= n13 ) r -= n13, q++
			/* We can compute all the remainders as ints, */
			/* because we know they are less than n13. */
			/* bx/y terms */
			id2x = bx2 / n12, id2y = by2 / n12;
			rd2x = ((int)bx2 - (int)id2x * n12) * n1,
			  rd2y = ((int)by2 - (int)id2y * n12) * n1;
			idx = bx / n12, idy = by / n12;
			rdx = ((int)bx - (int)idx * n12) * n1,
			  rdy = ((int)by - (int)idy * n12) * n1;
			/* cx/y terms */
			idx += qx = cx / n1, idy += qy = cy / n1;
			rdx += ((int)cx - (int)qx * n1) * n12,
			  rdy += ((int)cy - (int)qy * n1) * n12;
			/* ax/y terms */
			idx += qx = ax / n13, idy += qy = ay / n13;
			rdx += (int)ax - (int)qx * n13,
			  rdy += (int)ay - (int)qy * n13;
			id2x += id3x = ax6 / n13, id2y += id3y = ay6 / n13;
			rd2x += rd3x = (int)ax6 - (int)id3x * n13,
			  rd2y += rd3y = (int)ay6 - (int)id3y * n13;
			adjust_rem_loop(rdx, idx);
			adjust_rem_loop(rdy, idy);
			adjust_rem_loop(rd2x, id2x);
			adjust_rem_loop(rd2y, id2y);
			adjust_rem(rd3x, id3x);
			adjust_rem(rd3y, id3y);
#undef adjust_rem
#undef adjust_rem_loop
		}
	}
	else
		fast = 0, dt = 1.0 / (float)n1;
	if_debug4('2', "[2]sampling %s n=%d\n[2]x=%g, y=%g\n",
		  (fast ? "fast" : "slow"), n,
		  fixed2float(x), fixed2float(y));
	for ( i = 1; i < n1; i++ )
	{	int code;
		if ( fast )
		{
#ifdef DEBUG
if ( gs_debug['2'] )
			dprintf4("[2]dx=%f+%d, dy=%f+%d\n",
				 fixed2float(idx), rdx,
				 fixed2float(idy), rdy),
			dprintf4("   d2x=%f+%d, d2y=%f+%d\n",
				 fixed2float(id2x), rd2x,
				 fixed2float(id2y), rd2y),
			dprintf4("   d3x=%f+%d, d3y=%f+%d\n",
				 fixed2float(id3x), rd3x,
				 fixed2float(id3y), rd3y);
#endif
#define accum(i, r, di, dr)\
  if ( (unsigned)(r += dr) >= (unsigned)n13 ) r -= n13, i++;\
  i += di
			accum(x, rx, idx, rdx);
			accum(idx, rdx, id2x, rd2x);
			accum(id2x, rd2x, id3x, rd3x);
			accum(y, ry, idy, rdy);
			accum(idy, rdy, id2y, rd2y);
			accum(id2y, rd2y, id3y, rd3y);
#undef accum
		}
		else
		{	const float t = dt * (float)(i);
			x = ((ax*t + bx)*t + cx)*t + x0;
			y = ((ay*t + by)*t + cy)*t + y0;
		}
		if_debug3('2', "[2]%s x=%g, y=%g\n",
			  (((x ^ ptx) | (y ^ pty)) & float2fixed(-0.5) ?
			   "add" : "skip"),
			  fixed2float(x), fixed2float(y));
		/* Skip very short segments */
		if ( ((x ^ ptx) | (y ^ pty)) & float2fixed(-0.5) )
		{	if ( (code = gx_path_add_line(ppath, x, y)) < 0 )
				return code;
			ptx = x, pty = y;
		}
	}
	if_debug2('2', "[2]last x=%g, y=%g\n",
		  fixed2float(x3), fixed2float(y3));
	return gx_path_add_line(ppath, x3, y3);
}

/*
 *	The rest of this file is an analysis that will eventually
 *	allow us to rasterize curves on the fly, by finding points
 *	where Y reaches a local maximum or minimum, which allows us to
 *	divide the curve into locally Y-monotonic sections.
 */

/*
	Let y(t) = a*t^3 + b*t^2 + c*t + d, 0 <= t <= 1.
	Then dy(t) = 3*a*t^2 + 2*b*t + c.
	y(t) has a local minimum or maximum (or inflection point)
	precisely where dy(t) = 0.  Now the roots of dy(t) are
		( -2*b +/- sqrt(4*b^2 - 12*a*c) ) / 6*a
	   =	( -b +/- sqrt(b*2 - 3*a*c) ) / 3*a
	(Note that real roots exist iff b^2 >= 3*a*c.)
	We want to know if these lie in the range (0..1).
	(We don't care about the endpoints.)  Call such a root
	a "valid zero."  We proceed as follows:
		If sign(3*a + 2*b + c) ~= sign(c), a valid zero exists.
		If sign(a) = sign(b), no valid zero exists.
	Otherwise, we look for a local extremum of dy(t) by observing
		d2y(t) = 6*a*t + 2*b
	which has a zero only at
		t1 = -b / 3*a
	Now if t1 <= 0 or t1 >= 1, no valid zero exists.  Otherwise,
	we compute
		dy(t1) = c - (b^2 / 3*a)
	Then a valid zero exists (at t1) iff sign(dy(t1)) ~= sign(c).
 */
