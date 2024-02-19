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

/* gscolor2.c */
/* Level 2 color and halftone operators for Ghostscript library */
#include "gx.h"
#include "gserrors.h"
#include "gxfixed.h"			/* ditto */
#include "gxmatrix.h"			/* for gzstate.h */
#include "gxdevice.h"			/* for gx_color_index */
#include "gzstate.h"
#include "gzcolor.h"
#include "gscspace.h"
#include "gscolor2.h"

/*
 * NOTE: The only color spaces currently implemented by Ghostscript
 * are DeviceGray, DeviceRGB, and DeviceCMYK.  This is the module where that
 * limitation is enforced, since setcolorspace is the only way that
 * any other color space can be made current (other than specialized
 * operators such as setpattern.)
 */

int
gs_setcolorspace(gs_state *pgs, gs_color_space *pcs)
{	/****** ONLY IMPLEMENTED FOR Device SPACES ******/
	int code;
	switch ( pcs->type )
	{
	case gs_color_space_DeviceGray:
		code = gs_setgray(pgs, 0.0);
		break;
	case gs_color_space_DeviceRGB:
		code = gs_setrgbcolor(pgs, 0.0, 0.0, 0.0);
		break;
	case gs_color_space_DeviceCMYK:
		code = gs_setcmykcolor(pgs, 0.0, 0.0, 0.0, 1.0);
		break;
	default:
		code = gs_error_undefined;
	}
	return code;
}

/* currentcolorspace */
int
gs_currentcolorspace(const gs_state *pgs, gs_color_space *pcs)
{	/****** ONLY IMPLEMENTED FOR Device SPACES ******/
	int num_comp;
	pcs->type = (gs_color_space_type)pgs->color->space;
	if ( (num_comp = gs_color_space_num_components[(int)pcs->type]) < 0 )
	{	/* Uncolored pattern space, consult underlying space. */
		/* NOT IMPLEMENTED YET */
	}
	return num_comp;
}
