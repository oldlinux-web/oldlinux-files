/* Copyright (C) 1990, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gxhint1.c */
/* Adobe Type 1 font hint routines */
#include "gx.h"
#include "gserrors.h"
#include "gxarith.h"
#include "gxfixed.h"
#include "gxmatrix.h"
#include "gzstate.h"
#include "gzdevice.h"			/* for gxchar */
#include "gxdevmem.h"			/* ditto */
#include "gxchar.h"
#include "gxfont.h"
#include "gxtype1.h"
#include "gxop1.h"

/* Define the tolerance for testing whether a point is in a zone, */
/* in device pixels.  (Maybe this should be variable??) */
#define stem_tolerance float2fixed(0.05)

/* ------ Initialization ------ */

typedef zone_table(1) a_zone_table;
private alignment_zone *near
  compute_zones(P6(const gs_matrix_fixed *, const font_hints *,
    const a_zone_table *, const a_zone_table *, alignment_zone *, int));
private void near
  transform_zone(P4(const gs_matrix_fixed *, const font_hints *,
    const float *, alignment_zone *));

/* Compute the font-level hints from the font and the matrix. */
/* We should cache this with the font/matrix pair.... */
void
compute_font_hints(font_hints *pfh, const gs_matrix_fixed *pmat,
  const gs_type1_data *pdata)
{	int h_count = 0, v_count = 0;
	alignment_zone *zp = &pfh->a_zones[0];
	/* Figure out which hints, if any, to use, */
	/* and the orientation of the axes. */
	pfh->axes_swapped = pfh->x_inverted = pfh->y_inverted = 0;
	pfh->use_x_hints = pfh->use_y_hints = 0;
	if ( is_fzero(pmat->xy) )
		pfh->y_inverted = is_fneg(pmat->yy),
		pfh->use_y_hints = 1;
	else if ( is_fzero(pmat->xx) )
		pfh->y_inverted = is_fneg(pmat->xy),
		pfh->axes_swapped = 1,
		pfh->use_y_hints = 1;
	if ( is_fzero(pmat->yx) )
		pfh->x_inverted = is_fneg(pmat->xx),
		pfh->use_x_hints = 1;
	else if ( is_fzero(pmat->yy) )
		pfh->x_inverted = is_fneg(pmat->yx),
		pfh->axes_swapped = 1,
		pfh->use_x_hints = 1;
	if_debug6('1', "[1]ctm=[%g %g %g %g %g %g]\n",
		  pmat->xx, pmat->xy, pmat->yx, pmat->yy,
		  pmat->tx, pmat->ty);
	if_debug5('1', "[1]swapped=%d, x/y_hints=%d,%d, x/y_inverted=%d,%d\n",
		  pfh->axes_swapped, pfh->use_x_hints, pfh->use_y_hints,
		  pfh->x_inverted, pfh->y_inverted);
	/* Transform the actual hints. */
	if ( pfh->use_x_hints )
	{	gs_fixed_point hw;
		fixed *hp = (pfh->axes_swapped ? &hw.y : &hw.x);
		int i;
		if ( pdata->StdHW.count )
		{	gs_distance_transform2fixed(pmat,
				(float)pdata->StdHW.data[0], 0.0, &hw);
			pfh->snap_h.data[0] = any_abs(*hp);
			if_debug1('1', "[1]snap_h[0]=%g\n",
				  fixed2float(pfh->snap_h.data[0]));
			h_count = 1;
		}
		for ( i = 0; i < pdata->StemSnapH.count; i++ )
		{	gs_distance_transform2fixed(pmat,
				(float)pdata->StemSnapH.data[i], 0.0, &hw);
			pfh->snap_h.data[h_count++] = any_abs(*hp);
			if_debug2('1', "[1]snap_h[%d]=%g\n", h_count - 1,
				  fixed2float(pfh->snap_h.data[h_count - 1]));
		}
	}
	if ( pfh->use_y_hints )
	{	gs_fixed_point vw;
		fixed *vp = (pfh->axes_swapped ? &vw.x : &vw.y);
		int i;
		/* Convert blue parameters to device pixels. */
		gs_distance_transform2fixed(pmat, 0.0,
					    (float)pdata->BlueFuzz, &vw);
		pfh->blue_fuzz = any_abs(*vp);
		gs_distance_transform2fixed(pmat, 0.0, 1.0, &vw);
		pfh->suppress_overshoot = any_abs(*vp) < pdata->BlueScale;
		gs_distance_transform2fixed(pmat, 0.0, pdata->BlueShift, &vw);
		pfh->blue_shift = any_abs(*vp);
		/* Tweak up blue_shift if it is less than half a pixel. */
		/* See the discussion of BlueShift in section 5.7 of */
		/* "Adobe Type 1 Font Format." */
		if ( pfh->blue_shift < fixed_half )
			pfh->blue_shift = fixed_half;
		if_debug5('1', "[1]blue_fuzz=%d->%g, blue_shift=%g->%g, sup_ov=%d\n",
			  pdata->BlueFuzz, fixed2float(pfh->blue_fuzz),
			  pdata->BlueShift, fixed2float(pfh->blue_shift),
			  pfh->suppress_overshoot);
		zp = compute_zones(pmat, pfh,
				   (const a_zone_table *)&pdata->BlueValues,
				   (const a_zone_table *)&pdata->FamilyBlues,
				   zp, 1);
		zp = compute_zones(pmat, pfh,
				   (const a_zone_table *)&pdata->OtherBlues,
				   (const a_zone_table *)&pdata->FamilyOtherBlues,
				   zp, max_OtherBlues);
		if ( pdata->StdVW.count )
		{	gs_distance_transform2fixed(pmat, 0.0,
				(float)pdata->StdVW.data[0], &vw);
			pfh->snap_v.data[0] = any_abs(*vp);
			if_debug1('1', "[1]snap_v[0]=%g\n",
				  fixed2float(pfh->snap_v.data[0]));
			v_count = 1;
		}
		for ( i = 0; i < pdata->StemSnapV.count; i++ )
		{	gs_distance_transform2fixed(pmat, 0.0,
				(float)pdata->StemSnapV.data[i], &vw);
			pfh->snap_v.data[v_count++] = any_abs(*vp);
			if_debug2('1', "[1]snap_v[%d]=%g\n", v_count - 1,
				  fixed2float(pfh->snap_v.data[v_count - 1]));
		}
	}
	pfh->snap_h.count = h_count;
	pfh->snap_v.count = v_count;
	pfh->a_zone_count = zp - &pfh->a_zones[0];
}

/* Compute the alignment zones for one set of 'blue' values. */
private alignment_zone *near
compute_zones(const gs_matrix_fixed *pmat, const font_hints *pfh,
  const a_zone_table *blues, const a_zone_table *family_blues,
  alignment_zone *zp, int bottom_count)
{	int i;
	fixed fuzz = pfh->blue_fuzz;
	int inverted =
		(pfh->axes_swapped ? pfh->x_inverted : pfh->y_inverted);
	for ( i = 0; i < blues->count; i += 2, zp++ )
	{	const float *vp = &blues->data[i];
		zp->is_top_zone = i >> 1 >= bottom_count;
		transform_zone(pmat, pfh, vp, zp);
		if_debug5('1', "[1]blues[%d]=%g,%g -> %g,%g\n",
			  i >> 1, vp[0], vp[1],
			  fixed2float(zp->v0), fixed2float(zp->v1));
		if ( i < family_blues->count )
		{	/* Check whether family blues should supersede. */
			alignment_zone fz;
			const float *fvp = &family_blues->data[i];
			fixed diff;
			transform_zone(pmat, pfh, fvp, &fz);
			if_debug5('1', "[1]f_blues[%d]=%g,%g -> %g,%g\n",
				  i >> 1, fvp[0], fvp[1],
				  fixed2float(fz.v0), fixed2float(fz.v1));
			diff = (zp->v1 - zp->v0) - (fz.v1 - fz.v0);
			if ( diff > -fixed_1 && diff < fixed_1 )
				zp->v0 = fz.v0, zp->v1 = fz.v1;
		}
		/* Compute the flat position, and add the fuzz. */
		if ( (inverted ? zp->is_top_zone : !zp->is_top_zone) )
			zp->flat = zp->v1, zp->v0 -= fuzz;
		else
			zp->flat = zp->v0, zp->v1 += fuzz;
	}
	return zp;
}

/* Transform a single alignment zone to device coordinates, */
/* taking axis swapping into account. */
private void near
transform_zone(const gs_matrix_fixed *pmat, const font_hints *pfh,
  const float *vp, alignment_zone *zp)
{	gs_fixed_point p0, p1;
	fixed v0, v1;
	gs_point_transform2fixed(pmat, 0.0, vp[0], &p0);
	gs_point_transform2fixed(pmat, 0.0, vp[1], &p1);
	if ( pfh->axes_swapped ) v0 = p0.x, v1 = p1.x;
	else v0 = p0.y, v1 = p1.y;
	if ( v0 <= v1 ) zp->v0 = v0, zp->v1 = v1;
	else zp->v0 = v1, zp->v1 = v0;
}

/* Reset the stem hints. */
void
reset_stem_hints(register gs_type1_state *pis)
{	pis->hstem_hints.count = 0;
	pis->hstem_hints.current = &pis->hstem_hints.data[0];
	pis->vstem_hints.count = 0;
	pis->vstem_hints.current = &pis->vstem_hints.data[0];
}

/* ------ Add hints ------ */

private stem_hint *near type1_stem(P3(stem_hint_table *, fixed, fixed));
private fixed near find_snap(P2(fixed, const stem_snap_table *));
private alignment_zone *near
find_zone(P3(gs_type1_state *, fixed, fixed));

#define c_fixed(d, c) m_fixed(fixed2int_var(d), c, pis->fc, max_coeff_bits)

/* Add a horizontal stem hint. */
void
type1_hstem(register gs_type1_state *pis, fixed y, fixed dy)
{	stem_hint *psh;
	alignment_zone *pz;
	fixed v, dv, adj_dv;
	fixed vtop, vbot;
	fixed center, diff_v, diff2_dv;
	if ( !pis->fh.use_y_hints ) return;
	y += pis->lsb.y;
	if ( pis->fh.axes_swapped )
		v = pis->vs_offset.x + c_fixed(y, yx) +
			pis->pgs->ctm.tx_fixed,
		dv = c_fixed(dy, yx);
	else
		v = pis->vs_offset.y + c_fixed(y, yy) +
			pis->pgs->ctm.ty_fixed,
		dv = c_fixed(dy, yy);
	if ( dy < 0 )
		vbot = v + dv, vtop = v;
	else
		vbot = v, vtop = v + dv;
	if ( dv < 0 ) v += dv, dv = -dv;
	psh = type1_stem(&pis->hstem_hints, v, dv);
	if ( psh == 0 ) return;
	adj_dv = find_snap(dv, &pis->fh.snap_h);
	pz = find_zone(pis, vbot, vtop);
	if ( pz != 0 )
	{	/* Use the alignment zone to align the outer stem edge. */
		int inverted =
		  (pis->fh.axes_swapped ? pis->fh.x_inverted : pis->fh.y_inverted);
		int adjust_v1 =
		  (inverted ? !pz->is_top_zone : pz->is_top_zone);
		fixed flat_v = pz->flat;
		fixed overshoot =
			(pz->is_top_zone ? vtop - flat_v : flat_v - vbot);
		fixed pos_over =
			(inverted ? -overshoot : overshoot);
		fixed ddv = adj_dv - dv;
		fixed shift = fixed_rounded(flat_v) - flat_v;
		if ( pos_over > 0 )
		{ if ( pos_over < pis->fh.blue_shift || pis->fh.suppress_overshoot )
		  {	/* Character is small, suppress overshoot. */
			if_debug0('1', "[1]suppress overshoot\n");
			if ( pz->is_top_zone )
				shift -= overshoot;
			else
				shift += overshoot;
		  }
		  else if ( pos_over < fixed_1 )
		  {	/* Enforce overshoot. */
			if_debug0('1', "[1]enforce overshoot\n");
			if ( overshoot < 0 )
				overshoot = -fixed_1 - overshoot;
			else
				overshoot = fixed_1 - overshoot;
			if ( pz->is_top_zone )
				shift += overshoot;
			else
				shift -= overshoot;
		  }
		}
		if ( adjust_v1 )
			psh->dv1 = shift, psh->dv0 = shift - ddv;
		else
			psh->dv0 = shift, psh->dv1 = shift + ddv;
		if_debug4('1', "[1]flat_v = %g, overshoot = %g, dv=%g,%g\n",
			  fixed2float(flat_v), fixed2float(overshoot),
			  fixed2float(psh->dv0), fixed2float(psh->dv1));
		return;
	}
	/* Align the stem so its edges fall on pixel boundaries, */
	/* moving the center as little as possible. */
	center = v + arith_rshift_1(dv);
	if ( adj_dv & fixed_1 )
	{	/* Odd width, align center on half-pixel. */
		center += fixed_half;
	}
	diff_v = fixed_rounded(center) - center;
	diff2_dv = arith_rshift_1(adj_dv - dv);
	psh->dv0 = diff_v - diff2_dv;
	psh->dv1 = diff_v + diff2_dv;
	if_debug6('1', "[1]hstem %g,%g -> %g,%g ; d = %g,%g\n",
		  fixed2float(y), fixed2float(dy),
		  fixed2float(v), fixed2float(dv),
		  fixed2float(psh->dv0), fixed2float(psh->dv1));
}

/* Add a vertical stem hint. */
void
type1_vstem(register gs_type1_state *pis, fixed x, fixed dx)
{	stem_hint *psh;
	fixed v, dv, adj_dv;
	fixed center, diff_v, diff2_dv;
	if ( !pis->fh.use_x_hints ) return;
	x += pis->lsb.x;
	if ( pis->fh.axes_swapped )
		v = pis->vs_offset.y + c_fixed(x, xy) +
			pis->pgs->ctm.ty_fixed,
		dv = c_fixed(dx, xy);
	else
		v = pis->vs_offset.x + c_fixed(x, xx) +
			pis->pgs->ctm.tx_fixed,
		dv = c_fixed(dx, xx);
	if ( dv < 0 ) v += dv, dv = -dv;
	psh = type1_stem(&pis->vstem_hints, v, dv);
	if ( psh == 0 ) return;
	adj_dv = find_snap(dv, &pis->fh.snap_v);
	if ( pis->pdata->ForceBold && adj_dv < fixed_1 )
		adj_dv = fixed_1;
	/* Align the stem so its edges fall on pixel boundaries, */
	/* moving the center as little as possible. */
	center = v + arith_rshift_1(dv);
	if ( adj_dv & fixed_1 )
	{	/* Odd width, align center on half-pixel. */
		center += fixed_half;
	}
	diff_v = fixed_rounded(center) - center;
	diff2_dv = arith_rshift_1(adj_dv - dv);
	psh->dv0 = diff_v - diff2_dv;
	psh->dv1 = diff_v + diff2_dv;
	if_debug6('1', "[1]vstem %g,%g -> %g,%g ; d = %g,%g\n",
		  fixed2float(x), fixed2float(dx),
		  fixed2float(v), fixed2float(dv),
		  fixed2float(psh->dv0), fixed2float(psh->dv1));
}

/* Add a stem hint, keeping the table sorted. */
/* Return the stem hint pointer, or 0 if the table is full. */
private stem_hint *near
type1_stem(stem_hint_table *psht, fixed v0, fixed d)
{	stem_hint *bot = &psht->data[0];
	stem_hint *top = bot + psht->count;
	if ( psht->count >= max_stems ) return 0;
	while ( top > bot && v0 < top[-1].v0 )
	   {	*top = top[-1];
		top--;
	   }
	/* Add a little fuzz for insideness testing. */
	top->v0 = v0 - stem_tolerance;
	top->v1 = v0 + d + stem_tolerance;
	psht->count++;
	return top;
}

/* Compute the adjusted width of a stem. */
private fixed near
find_snap(fixed dv, const stem_snap_table *psst)
{	fixed best = fixed_1 * 2;
	fixed adj_dv;
	int i;
	for ( i = 0; i < psst->count; i++ )
	{	fixed diff = psst->data[i] - dv;
		if ( any_abs(diff) < any_abs(best) )
		{	if_debug3('1', "[1]snap %g to [%d]%g\n",
				  fixed2float(dv), i,
				  fixed2float(psst->data[i]));
			best = diff;
		}
	}
	adj_dv = fixed_rounded(any_abs(best) <= fixed_1 ? dv + best : dv);
	if ( adj_dv == 0 ) adj_dv = fixed_1;
	if_debug2('1', "[1]snap %g to %g\n",
		  fixed2float(dv), fixed2float(adj_dv));
	return adj_dv;
}

/* Find the applicable alignment zone for a stem, if any. */
/* vbot and vtop are the bottom and top of the stem, */
/* but without interchanging if the y axis is inverted. */
private alignment_zone *near
find_zone(gs_type1_state *pis, fixed vbot, fixed vtop)
{	alignment_zone *pz;
	for ( pz = &pis->fh.a_zones[pis->fh.a_zone_count]; --pz >= &pis->fh.a_zones[0]; )
	{	fixed v = (pz->is_top_zone ? vtop : vbot);
		if ( v >= pz->v0 && v <= pz->v1 )
		{	if_debug2('1', "[1]stem crosses %s-zone %d\n",
				  (pz->is_top_zone ? "top" : "bottom"),
				  (int)(pz - &pis->fh.a_zones[0]));
			return pz;
		}
	}
	return 0;
}

/* ------ Apply hints ------ */

private stem_hint *near
search_hints(P2(stem_hint_table *, fixed));

/* Adjust a point according to the relevant hints. */
/* x and y are the current point in device space after moving; */
/* dx and dy are the delta components in character space. */
void
find_stem_hints(gs_type1_state *pis, fixed x, fixed y,
  fixed dx, fixed dy, gs_fixed_point *ppt)
{	ppt->x = x, ppt->y = y;
	if ( pis->in_dotsection ) return;
	/* Note that if use_x/y_hints is false, */
	/* no entries ever get made in the stem hint tables, */
	/* so we don't have to check those flags here. */
	/* Check the vertical stem hints. */
	if ( pis->vstem_hints.count )
	{	fixed *pv = (pis->fh.axes_swapped ? &ppt->y : &ppt->x);
		stem_hint *ph = search_hints(&pis->vstem_hints, *pv);
		if ( ph != 0 )
		{	/* Decide which side of the stem we are on. */
			/* If we're moving horizontally, just use the */
			/* x coordinate; otherwise, assume outside */
			/* edges move clockwise and inside edges move */
			/* counter-clockwise.  (This algorithm was */
			/* taken from the IBM X11R5 rasterizer; I'm not */
			/* sure I believe it.) */
#define adjust_stem(pv, dxy, ph, inverted)\
  *pv +=\
    ((/*dy == 0 ?*/ *pv < arith_rshift_1(ph->v0 +\
      ph->v1) /*: pis->fh.y_inverted ? dy < 0 : dy > 0*/) ?\
     ph->dv0 : ph->dv1)
			if_debug2('1', "[1]use vstem %d: %g",
				  (int)(ph - &pis->vstem_hints.data[0]),
				  fixed2float(*pv));
			adjust_stem(pv, dy, ph, pis->fh.y_inverted);
			if_debug1('1', " -> %g\n", fixed2float(*pv));
		}
	}
	/* Check the horizontal stem hints. */
	if ( pis->hstem_hints.count )
	{	fixed *pv = (pis->fh.axes_swapped ? &ppt->x : &ppt->y);
		stem_hint *ph = search_hints(&pis->hstem_hints, *pv);
		if ( ph != 0 )
		{	if_debug2('1', "[1]use hstem %d: %g",
				  (int)(ph - &pis->hstem_hints.data[0]),
				  fixed2float(*pv));
			adjust_stem(pv, dx, ph, pis->fh.x_inverted);
			if_debug1('1', " -> %g\n", fixed2float(*pv));
		}
	}
	return;
#undef adjust_stem
}

/* Search one hint table for an adjustment. */
private stem_hint *near
search_hints(stem_hint_table *psht, fixed v)
{	stem_hint *ph = psht->current;
	if ( v >= ph->v0 && v <= ph->v1 ) return ph;
	/* We don't bother with binary or even up/down search, */
	/* because there won't be very many hints. */
	for ( ph = &psht->data[psht->count]; --ph >= &psht->data[0]; )
	  if ( v >= ph->v0 && v <= ph->v1 )
	    return (psht->current = ph);
	return 0;
}
