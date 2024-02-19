/* Copyright (C) 1991, 1992 Aladdin Enterprises.  All rights reserved.
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

/* zfont1.c */
/* Type 1 font creation operator for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "gxfixed.h"
#include "gsmatrix.h"
#include "gxdevice.h"
#include "gschar.h"
#include "gxfont.h"
#include "bfont.h"
#include "dict.h"
#include "dparam.h"
#include "name.h"
#include "store.h"

/* Type 1 auxiliary procedures (defined in zchar.c) */
extern int z1_subr_proc(P3(gs_type1_data *, int, const byte **));
extern int z1_pop_proc(P2(gs_type1_data *, fixed *));

/* Names of system-known keys in font dictionaries: */
static ref name_PaintType;
extern ref name_UniqueID;		/* in zfont2 */
static ref name_Type1BuildChar;
static ref name_Private;
static ref name_CharStrings;		/* only needed for seac */
/* Names of system-known keys in type 1 font Private dictionary: */
static ref name_BlueFuzz;
static ref name_BlueScale;
static ref name_BlueShift;
static ref name_BlueValues;
static ref name_ExpansionFactor;
static ref name_FamilyBlues;
static ref name_FamilyOtherBlues;
static ref name_ForceBold;
static ref name_LanguageGroup;
static ref name_lenIV;
static ref name_OtherBlues;
static ref name_RndStemUp;
static ref name_StdHW;
static ref name_StdVW;
static ref name_StemSnapH;
static ref name_StemSnapV;
static ref name_Subrs;

/* Default value of lenIV */
#define default_lenIV 4

/* Initialize the font building operators */
private void
zfont1_init()
{	static const names_def fnd1[] = {

	/* Create the names of the standard elements of */
	/* a font dictionary. */
	   { "PaintType", &name_PaintType },
	   { "Type1BuildChar", &name_Type1BuildChar },
	   { "Private", &name_Private },
	   { "CharStrings", &name_CharStrings },

	/* Create the names of the known entries in */
	/* a type 1 font Private dictionary. */
	   { "BlueFuzz", &name_BlueFuzz },
	   { "BlueScale", &name_BlueScale },
	   { "BlueShift", &name_BlueShift },
	   { "BlueValues", &name_BlueValues },
	   { "ExpansionFactor", &name_ExpansionFactor },
	   { "FamilyBlues", &name_FamilyBlues },
	   { "FamilyOtherBlues", &name_FamilyOtherBlues },
	   { "ForceBold", &name_ForceBold },
	   { "LanguageGroup", &name_LanguageGroup },
	   { "lenIV", &name_lenIV },
	   { "OtherBlues", &name_OtherBlues },
	   { "RndStemUp", &name_RndStemUp },
	   { "StdHW", &name_StdHW },
	   { "StdVW", &name_StdVW },
	   { "StemSnapH", &name_StemSnapH },
	   { "StemSnapV", &name_StemSnapV },
	   { "Subrs", &name_Subrs },

	/* Mark the end of the initalized name list. */
	   names_def_end
	};

	init_names(fnd1);

	/* Make the standard BuildChar procedures executable. */
	r_set_attrs(&name_Type1BuildChar, a_executable);
}

/* .buildfont1 */
/* Build a type 1 (Adobe encrypted) font. */
int
zbuildfont1(os_ptr op)
{	gs_type1_data data1;
	ref *psubrs;
	ref *pcharstrings;
	ref *pprivate;
	ref *puniqueid;
	static ref no_subrs;
	gs_font *pfont;
	font_data *pdata;
	int code;
	check_type(*op, t_dictionary);
	code = dict_int_param(op, &name_PaintType, 0, 3, 0, &data1.PaintType);
	if ( code < 0 ) return code;
	if ( dict_find(op, &name_CharStrings, &pcharstrings) <= 0 ||
	    !r_has_type(pcharstrings, t_dictionary) ||
	    dict_find(op, &name_Private, &pprivate) <= 0 ||
	    !r_has_type(pprivate, t_dictionary)
	   )
		return e_invalidfont;
	if ( dict_find(pprivate, &name_Subrs, &psubrs) > 0 )
	   {	check_array_else(*psubrs, e_invalidfont);
	   }
	else
	   {	make_tasv(&no_subrs, t_string, 0, 0, bytes, (byte *)0),
		psubrs = &no_subrs;
	   }
	/* Get the rest of the information from the Private dictionary. */
	if ( (code = dict_int_param(pprivate, &name_lenIV, 0, 255,
				    default_lenIV, &data1.lenIV)) < 0 ||
	     (code = dict_int_param(pprivate, &name_BlueFuzz, 0, 1999, 1,
				    &data1.BlueFuzz)) < 0 ||
	     (code = dict_float_param(pprivate, &name_BlueScale, 0.039625,
				      &data1.BlueScale)) < 0 ||
	     (code = dict_float_param(pprivate, &name_BlueShift, 7.0,
				    &data1.BlueShift)) < 0 ||
	     (code = data1.BlueValues.count = dict_float_array_param(pprivate,
		&name_BlueValues, max_BlueValues * 2,
		&data1.BlueValues.data[0], NULL)) < 0 ||
	     (code = dict_float_param(pprivate, &name_ExpansionFactor, 0.06,
				      &data1.ExpansionFactor)) < 0 ||
	     (code = data1.FamilyBlues.count = dict_float_array_param(pprivate,
		&name_FamilyBlues, max_FamilyBlues * 2,
		&data1.FamilyBlues.data[0], NULL)) < 0 ||
	     (code = data1.FamilyOtherBlues.count = dict_float_array_param(pprivate,
		&name_FamilyOtherBlues, max_FamilyOtherBlues * 2,
		&data1.FamilyOtherBlues.data[0], NULL)) < 0 ||
	     (code = dict_bool_param(pprivate, &name_ForceBold, 0,
				     &data1.ForceBold)) < 0 ||
	     (code = dict_int_param(pprivate, &name_LanguageGroup, 0, 1, 0,
				    &data1.LanguageGroup)) < 0 ||
	     (code = data1.OtherBlues.count = dict_float_array_param(pprivate,
		&name_OtherBlues, max_OtherBlues * 2,
		&data1.OtherBlues.data[0], NULL)) < 0 ||
	     (code = dict_bool_param(pprivate, &name_RndStemUp, 0,
				     &data1.RndStemUp)) < 0 ||
	     (code = data1.StdHW.count = dict_float_array_param(pprivate,
		&name_StdHW, 1, &data1.StdHW.data[0], NULL)) < 0 ||
	     (code = data1.StdVW.count = dict_float_array_param(pprivate,
		&name_StdVW, 1, &data1.StdVW.data[0], NULL)) < 0 ||
	     (code = data1.StemSnapH.count = dict_float_array_param(pprivate,
		&name_StemSnapH, max_StemSnap,
		&data1.StemSnapH.data[0], NULL)) < 0 ||
	     (code = data1.StemSnapV.count = dict_float_array_param(pprivate,
		&name_StemSnapV, max_StemSnap,
		&data1.StemSnapV.data[0], NULL)) < 0
	   )
		return code;
	/* Do the work common to all non-composite font types. */
	code = build_gs_simple_font(op, &pfont, ft_encrypted, &name_Type1BuildChar);
	if ( code != 0 ) return code;
	/* This is a new font, fill it in. */
	pdata = (font_data *)pfont->client_data;
	pfont->data.base.type1_data = data1;
	ref_assign(&pdata->CharStrings, pcharstrings);
	ref_assign(&pdata->Subrs, psubrs);
	pfont->data.base.type1_data.subr_proc = z1_subr_proc;
	pfont->data.base.type1_data.pop_proc = z1_pop_proc;
	pfont->data.base.type1_data.proc_data = (char *)pdata;
	/* Check that the UniqueIDs match.  This is part of the */
	/* Adobe protection scheme, but we may as well emulate it. */
	if ( pfont->data.base.UniqueID >= 0 )
	   {	if ( dict_find(pprivate, &name_UniqueID, &puniqueid) <= 0 ||
		     !r_has_type(puniqueid, t_integer) ||
		     puniqueid->value.intval != pfont->data.base.UniqueID
		   )
			pfont->data.base.UniqueID = -1;
	   }
	return 0;
}

/* ------ Initialization procedure ------ */

op_def zfont1_op_defs[] = {
	{"1.buildfont1", zbuildfont1},
	op_def_end(zfont1_init)
};
