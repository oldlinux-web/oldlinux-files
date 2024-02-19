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

/* zht2.c */
/* Level 2 halftone operators for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "dict.h"
#include "dparam.h"
#include "name.h"		/* for name_eq */
#include "state.h"
#include "store.h"

/* Keys in halftone dictionaries: */
static ref name_HalftoneType;
static ref name_TransferFunction;
/* Types 1 and 2 only: */
static ref name_Frequency;
static ref name_Angle;
static ref name_SpotFunction;
static ref name_AccurateScreens;
static ref name_ActualFrequency;
/* Types 3 and 4 only: */
static ref name_Width;
static ref name_Height;
static ref name_Thresholds;
/* Type 5 only: */
static ref color_names[8];
#define name_Gray color_names[0]
#define name_Red color_names[1]
#define name_Green color_names[2]
#define name_Blue color_names[3]
#define name_Cyan color_names[4]
#define name_Magenta color_names[5]
#define name_Yellow color_names[6]
#define name_Black color_names[7]
static ref name_Default;

/* Initialization */
private void
zht2_init()
{	static const names_def htn[] = {

	/* Create the names of the halftone dictionary keys. */
	   { "HalftoneType", &name_HalftoneType },
	   { "TransferFunction", &name_TransferFunction },
		/* Types 1 and 2 only: */
	   { "Frequency", &name_Frequency },
	   { "Angle", &name_Angle },
	   { "SpotFunction", &name_SpotFunction },
	   { "AccurateScreens", &name_AccurateScreens },
	   { "ActualFrequency", &name_ActualFrequency },
		/* Types 3 and 4 only: */
	   { "Width", &name_Width },
	   { "Height", &name_Height },
	   { "Thresholds", &name_Thresholds },
		/* Type 5 only: */
	   { "Gray", &name_Gray },
	   { "Red", &name_Red },
	   { "Green", &name_Green },
	   { "Blue", &name_Blue },
	   { "Cyan", &name_Cyan },
	   { "Magenta", &name_Magenta },
	   { "Yellow", &name_Yellow },
	   { "Black", &name_Black },
	   { "Default", &name_Default },

	/* Mark the end of the initalized name list. */
	   names_def_end
	};

	init_names(htn);
}


/* currenthalftone */
private int
zcurrenthalftone(register os_ptr op)
{	return e_undefined;
}

/* sethalftone */
private int
zsethalftone(register os_ptr op)
{	return e_undefined;
}

/* ------ Initialization procedure ------ */

op_def zht2_op_defs[] = {
	{"0currenthalftone", zcurrenthalftone},
	{"1sethalftone", zsethalftone},
	op_def_end(zht2_init)
};
