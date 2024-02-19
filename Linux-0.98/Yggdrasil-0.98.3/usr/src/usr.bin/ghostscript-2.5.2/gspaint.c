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

/* gspaint.c */
/* Painting procedures for GhostScript library */
#include "gx.h"
#include "gpcheck.h"
#include "gserrors.h"
#include "gxfixed.h"
#include "gxmatrix.h"			/* for gs_state */
#include "gspaint.h"
#include "gzpath.h"
#include "gzstate.h"
#include "gzdevice.h"
#include "gzcolor.h"
#include "gxcpath.h"
#include "gxdevmem.h"
#include "gximage.h"

/* Erase the page */
int
gs_erasepage(gs_state *pgs)
{	device *pdev = pgs->device;
	gx_device *dev = pdev->info;
	(*dev->procs->fill_rectangle)(dev, 0, 0, dev->width, dev->height, pdev->white);
	gp_check_interrupts();
	return 0;
}

/* Fill using the winding number rule */
int
gs_fill(gs_state *pgs)
{	return gs_fill_adjust(pgs, (fixed)0);
}
/* This is a hack, see gx_fill_path and gs_type1_interpret. */
int
gs_fill_adjust(gs_state *pgs, fixed adjust)
{	int code;
	/* If we're inside a charpath, just merge the current path */
	/* into the parent's path. */
	if ( pgs->in_charpath )
		code = gx_path_add_path(pgs->saved->path, pgs->path);
	else
	   {	gx_color_load(pgs->dev_color, pgs);
		code = gx_fill_path(pgs->path, pgs->dev_color, pgs,
				    gx_rule_winding_number, adjust);
		if ( !code ) gs_newpath(pgs);
	   }
	return code;
}

/* Fill using the even/odd rule */
int
gs_eofill(gs_state *pgs)
{	int code;
	/* If we're inside a charpath, just merge the current path */
	/* into the parent's path. */
	if ( pgs->in_charpath )
		code = gx_path_add_path(pgs->saved->path, pgs->path);
	else
	   {	gx_color_load(pgs->dev_color, pgs);
		code = gx_fill_path(pgs->path, pgs->dev_color, pgs,
				    gx_rule_even_odd, (fixed)0);
		if ( !code ) gs_newpath(pgs);
	   }
	return code;
}

/* Stroke the current path */
int
gs_stroke(gs_state *pgs)
{	int code;
	/* If we're inside a charpath, just merge the current path */
	/* into the parent's path. */
	if ( pgs->in_charpath )
		code = gx_path_add_path(pgs->saved->path, pgs->path);
	else
	   {	gx_color_load(pgs->dev_color, pgs);
		code = gx_stroke_fill(pgs->path, pgs);
		if ( !code ) gs_newpath(pgs);
	   }
	return code;
}

/* Compute the stroked outline of the current path */
int
gs_strokepath(gs_state *pgs)
{	gx_path spath;
	int code;
	gx_path_init(&spath, &pgs->memory_procs);
	code = gx_stroke_add(pgs->path, &spath, pgs);
	if ( code < 0 ) return code;
	gx_path_release(pgs->path);
	*pgs->path = spath;
	return 0;
}
