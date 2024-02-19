/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gxpath2.c */
/* Path tracing procedures for Ghostscript library */
#include "math_.h"
#include "gx.h"
#include "gserrors.h"
#include "gxfixed.h"
#include "gxarith.h"
#include "gzpath.h"

/* Read the current point of a path. */
int
gx_path_current_point(const gx_path *ppath, gs_fixed_point *ppt)
{	if ( !ppath->position_valid )
	  return_error(gs_error_nocurrentpoint);
	/* Copying the coordinates individually */
	/* is much faster on a PC, and almost as fast on other machines.... */
	ppt->x = ppath->position.x, ppt->y = ppath->position.y;
	return 0;
}

/* Read the bounding box of a path. */
int
gx_path_bbox(gx_path *ppath, gs_fixed_rect *pbox)
{	if ( ppath->first_subpath == 0 )
	   {	/* The path is empty, use the current point if any. */
		gx_path_current_point(ppath, &pbox->p);
		return gx_path_current_point(ppath, &pbox->q);
	   }
	/* The stored bounding box may not be up to date. */
	/* Correct it now if necessary. */
	if ( ppath->box_last == ppath->current_subpath->last )
	   {	/* Box is up to date */
		*pbox = ppath->bbox;
	   }
	else
	   {	gs_fixed_rect box;
		register segment *pseg = ppath->box_last;
		if ( pseg == 0 )	/* box is uninitialized */
		   {	pseg = (segment *)ppath->first_subpath;
			box.p.x = box.q.x = pseg->pt.x;
			box.p.y = box.q.y = pseg->pt.y;
		   }
		else
		   {	box = ppath->bbox;
			pseg = pseg->next;
		   }
/* Macro for adjusting the bounding box when adding a point */
#define adjust_bbox(pt)\
  if ( (pt).x < box.p.x ) box.p.x = (pt).x;\
  else if ( (pt).x > box.q.x ) box.q.x = (pt).x;\
  if ( (pt).y < box.p.y ) box.p.y = (pt).y;\
  else if ( (pt).y > box.q.y ) box.q.y = (pt).y
		while ( pseg )
		   {	switch ( pseg->type )
			   {
			case s_curve:
#define pcurve ((curve_segment *)pseg)
				adjust_bbox(pcurve->p1);
				adjust_bbox(pcurve->p2);
#undef pcurve
				/* falls through */
			default:
				adjust_bbox(pseg->pt);
			   }
			pseg = pseg->next;
		   }
#undef adjust_bbox
		ppath->bbox = box;
		ppath->box_last = ppath->current_subpath->last;
		*pbox = box;
	   }
	return 0;
}

/* Test if a path has any curves. */
int
gx_path_has_curves(const gx_path *ppath)
{	return ppath->curve_count != 0;
}

/* Test if a path has any segments. */
int
gx_path_is_void(const gx_path *ppath)
{	return ppath->first_subpath == 0;
}

/* Test if a path is a rectangle. */
/* If so, return its bounding box. */
/* Note that this must recognize open as well as closed rectangles. */
int
gx_path_is_rectangle(const gx_path *ppath, gs_fixed_rect *pbox)
{	const subpath *pseg0;
	const segment *pseg1, *pseg2, *pseg3, *pseg4;
	if (	ppath->subpath_count == 1 &&
		(pseg1 = (pseg0 = ppath->first_subpath)->next) != 0 &&
		(pseg2 = pseg1->next) != 0 &&
		(pseg3 = pseg2->next) != 0 &&
		((pseg4 = pseg3->next) == 0 || pseg4->type == s_line_close) &&
		ppath->curve_count == 0
	   )
	   {	fixed x0 = pseg0->pt.x, y0 = pseg0->pt.y;
		fixed x2 = pseg2->pt.x, y2 = pseg2->pt.y;
		if (	(x0 == pseg1->pt.x && pseg1->pt.y == y2 &&
			 x2 == pseg3->pt.x && pseg3->pt.y == y0) ||
			(x0 == pseg3->pt.x && pseg3->pt.y == y2 &&
			 x2 == pseg1->pt.x && pseg1->pt.y == y0)
		   )
		   {	/* Path is a rectangle.  Return bounding box. */
			if ( x0 < x2 )
				pbox->p.x = x0, pbox->q.x = x2;
			else
				pbox->p.x = x2, pbox->q.x = x0;
			if ( y0 < y2 )
				pbox->p.y = y0, pbox->q.y = y2;
			else
				pbox->p.y = y2, pbox->q.y = y0;
			return 1;
		   }
	   }
	return 0;
}

/* Translate an already-constructed path (in device space). */
/* Don't bother to translate the cbox. */
int
gx_path_translate(gx_path *ppath, fixed dx, fixed dy)
{	segment *pseg;
#define translate_xy(pt)\
  pt.x += dx, pt.y += dy
	translate_xy(ppath->bbox.p);
	translate_xy(ppath->bbox.q);
	translate_xy(ppath->position);
	pseg = (segment *)(ppath->first_subpath);
	while ( pseg )
	   {	switch ( pseg->type )
		   {
		case s_curve:
		   {	curve_segment *pc = (curve_segment *)pseg;
			translate_xy(pc->p1);
			translate_xy(pc->p2);
		   }
		default:
			translate_xy(pseg->pt);
		   }
		pseg = pseg->next;
	   }
	return 0;
}

/* Reverse a path. */
/* We know ppath != ppath_old. */
int
gx_path_copy_reversed(const gx_path *ppath_old, gx_path *ppath, int init)
{	const subpath *psub = ppath_old->first_subpath;
#ifdef DEBUG
if ( gs_debug['p'] )
	gx_dump_path(ppath_old, "before reversepath");
#endif
	if ( init )
		gx_path_init(ppath, &ppath_old->memory_procs);
nsp:	while ( psub )
	   {	const segment *pseg = psub->last;
		const segment *prev;
		int code = gx_path_add_point(ppath, pseg->pt.x, pseg->pt.y);
		if ( code < 0 )
		   {	gx_path_release(ppath);
			return code;
		   }
		for ( ; ; pseg = prev )
		   {	prev = pseg->prev;
			switch ( pseg->type )
			   {
			case s_start:
				/* Finished subpath */
				if ( psub->closed )
					code = gx_path_close_subpath(ppath);
				psub = (const subpath *)psub->last->next;
				goto nsp;
			case s_curve:
			   {	const curve_segment *pc = (const curve_segment *)pseg;
				code = gx_path_add_curve(ppath,
					pc->p2.x, pc->p2.y,
					pc->p1.x, pc->p1.y,
					prev->pt.x, prev->pt.y);
				break;
			   }
			case s_line:
			case s_line_close:
				code = gx_path_add_line(ppath, prev->pt.x, prev->pt.y);
				break;
			   }
			if ( code )
			   {	gx_path_release(ppath);
				return code;
			   }
		   }
		/* not reached */
	}
	ppath->position = ppath_old->position;		/* restore current point */
#ifdef DEBUG
if ( gs_debug['p'] )
	gx_dump_path(ppath, "after reversepath");
#endif
	return 0;
}
