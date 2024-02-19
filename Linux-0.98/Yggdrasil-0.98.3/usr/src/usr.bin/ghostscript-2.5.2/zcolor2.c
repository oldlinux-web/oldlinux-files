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

/* zcolor2.c */
/* Level 2 color operators for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "gscolor.h"
#include "gscspace.h"
#include "gscolor2.h"
#include "gsmatrix.h"
#include "dict.h"
#include "dparam.h"
#include "name.h"		/* for name_eq */
#include "state.h"
#include "store.h"

/*
 * NOTE: The only color spaces currently implemented by Ghostscript
 * are DeviceGray, DeviceRGB, and DeviceCMYK.  This is enforced by
 * gs_setcolorspace; the routines here assume all color spaces are
 * implemented.
 */


/* Forward references */
typedef enum { cs_allow_base, cs_allow_non_pattern, cs_allow_all } cs_allowed;
private int cspace_param(P3(const ref *pcsref, gs_color_space *pcs, cs_allowed allow));

/* Names of color spaces: */
static ref color_space_names[8];
#define name_DeviceGray color_space_names[0]
#define name_DeviceRGB color_space_names[1]
#define name_DeviceCMYK color_space_names[2]
#define name_CIEBasedABC color_space_names[3]
#define name_CIEBasedA color_space_names[4]
#define name_Separation color_space_names[5]
#define name_Indexed color_space_names[6]
#define name_Pattern color_space_names[7]

/* Names of keys in pattern dictionaries: */
static ref name_PatternType;
static ref name_PaintType;
static ref name_TilingType;
static ref name_BBox;
static ref name_XStep;
static ref name_YStep;
static ref name_PaintProc;
static ref name_Implementation;

/* Initialization */
private void
zcolor2_init()
{	static const names_def patn[] = {

	/* Create the names of the color spaces. */
	   { "DeviceGray", &name_DeviceGray },
	   { "DeviceRGB", &name_DeviceRGB },
	   { "DeviceCMYK", &name_DeviceCMYK },
	   { "CIEBasedABC", &name_CIEBasedABC },
	   { "CIEBasedA", &name_CIEBasedA },
	   { "Separation", &name_Separation },
	   { "Indexed", &name_Indexed },
	   { "Pattern", &name_Pattern },

	/* Create the names of the known entries in */
	/* a pattern dictionary. */
	   { "PatternType", &name_PatternType },
	   { "PaintType", &name_PaintType },
	   { "TilingType", &name_TilingType },
	   { "BBox", &name_BBox },
	   { "XStep", &name_XStep },
	   { "YStep", &name_YStep },
	   { "PaintProc", &name_PaintProc },
	   { "Implementation", &name_Implementation },

	/* Mark the end of the initalized name list. */
	   names_def_end
	};

	init_names(patn);
}


/* currentcolor */
int
zcurrentcolor(register os_ptr op)
{	gs_color_space cs;
	gs_currentcolorspace(igs, &cs);
	switch ( cs.type )
	{
	case gs_color_space_DeviceGray:
		return zcurrentgray(op);
	case gs_color_space_DeviceRGB:
		return zcurrentrgbcolor(op);
	case gs_color_space_DeviceCMYK:
		return zcurrentcmykcolor(op);
	default:
		return e_typecheck;
	}
}

/* currentcolorspace */
int
zcurrentcolorspace(register os_ptr op)
{	gs_color_space cs;
	gs_currentcolorspace(igs, &cs);
	push(1);
	*op = istate.colorspace;	/* default */
	switch ( cs.type )
	{
	case gs_color_space_Pattern:
		if ( cs.params.pattern.has_base_space ) break;
	case gs_color_space_DeviceGray:
	case gs_color_space_DeviceRGB:
	case gs_color_space_DeviceCMYK:
		/* Create the 1-element array on the fly. */
		make_tasv(op, t_array, a_readonly, 1, refs,
			  &color_space_names[(int)cs.type]);
	}
	return 0;
}

/* makepattern */
int
zmakepattern(os_ptr op)
{	os_ptr op1 = op - 1;
	int code;
	gs_matrix mat;
	int PatternType, PaintType, TilingType;
	float BBox[4];
	float XStep, YStep;
	ref *pPaintProc;
	check_type(*op1, t_dictionary);
	check_dict_read(*op1);
	if ( (code = read_matrix(op, &mat)) < 0 ||
	     (code = dict_int_param(op1, &name_PatternType, 1, 1, 0, &PatternType)) < 0 ||
	     (code = dict_int_param(op1, &name_PaintType, 1, 2, 0, &PaintType)) < 0 ||
	     (code = dict_int_param(op1, &name_TilingType, 1, 3, 0, &TilingType)) < 0 ||
	     (code = dict_float_array_param(op1, &name_BBox, 4, BBox, NULL)) != 4 ||
	     (code = dict_float_param(op1, &name_XStep, 0.0, &XStep)) != 0 ||
	     (code = dict_float_param(op1, &name_YStep, 0.0, &YStep)) != 0 ||
	     (code = dict_find(op1, &name_PaintProc, &pPaintProc)) <= 0
	   )
		return_error((code < 0 ? code : e_typecheck));
	check_proc(*pPaintProc);
	if ( XStep == 0 || YStep == 0 )
		return_error(e_rangecheck);
	/* NOT IMPLEMENTED YET */
	if ( 1 ) return_error(e_undefined);
	pop(1);
	return 0;
}

/* setcolor */
int
zsetcolor(register os_ptr op)
{	gs_color_space cs;
	gs_currentcolorspace(igs, &cs);
	switch ( cs.type )
	{
	case gs_color_space_DeviceGray:
		return zsetgray(op);
	case gs_color_space_DeviceRGB:
		return zsetrgbcolor(op);
	case gs_color_space_DeviceCMYK:
		return zsetcmykcolor(op);
	default:
		return e_typecheck;
	}
}

/* setcolorspace */
int
zsetcolorspace(register os_ptr op)
{	gs_color_space cs;
	int code = cspace_param((const ref *)op, &cs, cs_allow_all);
	if ( code < 0 ) return code;
	code = gs_setcolorspace(igs, &cs);
	if ( code < 0 ) return code;
	if ( r_has_type(op, t_name) )
		make_null(&istate.colorspace);		/* no params */
	else
		istate.colorspace = *op;
	pop(1);
	return 0;
}

/* ------ Internal procedures ------ */

/* Extract the parameters for a color space. */
private int
cspace_param(const ref *pcsref, gs_color_space *pcs, cs_allowed allow)
{	const ref *pcsa, *pcsn;
	uint asize;
	int csi;
	int code;
	if ( r_has_type(pcsref, t_array) )
	{	check_read(*pcsref);
		pcsa = pcsref->value.const_refs;
		asize = r_size(pcsa);
		if ( asize == 0 ) return e_rangecheck;
	}
	else
	{	pcsa = pcsref;
		asize = 1;
	}
	pcsn = pcsa++;
	asize--;
	check_type(*pcsn, t_name);
	for ( csi = 0; !name_eq(pcsn, &color_space_names[csi]); )
	{	if ( ++csi == countof(color_space_names) )
			return e_typecheck;
	}
	pcs->type = (gs_color_space_type)csi;
	switch ( pcs->type )
	{
	case gs_color_space_DeviceGray:
	case gs_color_space_DeviceRGB:
	case gs_color_space_DeviceCMYK:
		if ( asize != 0 ) return e_rangecheck;
		break;
	case gs_color_space_CIEBasedABC:
		if ( asize != 1 ) return e_rangecheck;
		/*return zcolorspace_CIEBasedABC(pcsa, &cs.params.abc);*/
		/* NOT IMPLEMENTED YET */
		return e_undefined;
	case gs_color_space_CIEBasedA:
		if ( asize != 1 ) return e_rangecheck;
		/*return zcolorspace_CIEBasedA(pcsa, &cs.params.abc);*/
		/* NOT IMPLEMENTED YET */
		return e_undefined;
	case gs_color_space_Separation:
		if ( allow == cs_allow_base ) return e_rangecheck;
		if ( asize != 3 ) return e_rangecheck;
		code = cspace_param(pcsa + 1, (gs_color_space *)&pcs->params.separation.alt_space, cs_allow_base);
		if ( code < 0 ) return code;
		/* NOT IMPLEMENTED YET */
		return e_undefined;
	case gs_color_space_Indexed:
		if ( allow == cs_allow_base ) return e_rangecheck;
		if ( asize != 3 ) return e_rangecheck;
		code = cspace_param(pcsa, (gs_color_space *)&pcs->params.indexed.base_space, cs_allow_base);
		if ( code < 0 ) return code;
		check_type(pcsa[1], t_integer);
		if ( pcsa[1].value.intval < 0 || pcsa[1].value.intval > 4095 )
			return e_rangecheck;
		pcs->params.indexed.hival = pcsa[1].value.intval;
		if ( r_has_type(&pcsa[2], t_string) )
		{	check_read(pcsa[2]);
			if ( r_size(&pcsa[2]) !=
			      (pcs->params.indexed.hival + 1) *
			      gs_color_space_num_components[(int)pcs->params.indexed.base_space.type]
			   )
				return e_rangecheck;
			pcs->params.indexed.lookup =
				pcsa[2].value.const_bytes;
		}
		else
		{	check_proc(pcsa[2]);
			/* NOW WHAT? */
			return e_undefined;
		}
		return 0;
	case gs_color_space_Pattern:
		if ( allow != cs_allow_all ) return e_rangecheck;
		switch ( asize )
		{
		case 0:		/* no base space */
			pcs->params.pattern.has_base_space = 0;
			return 0;
		default:
			return e_rangecheck;
		case 1:
			;
		}
		pcs->params.pattern.has_base_space = 1;
		return cspace_param(pcsa, (gs_color_space *)&pcs->params.pattern.base_space, cs_allow_non_pattern);
	default:
		return e_typecheck;
	}
	return 0;
}

/* ------ Initialization procedure ------ */

op_def zcolor2_op_defs[] = {
	{"0currentcolor", zcurrentcolor},
	{"0currentcolorspace", zcurrentcolorspace},
	{"2makepattern", zmakepattern},
	{"1setcolor", zsetcolor},
	{"1setcolorspace", zsetcolorspace},
	op_def_end(zcolor2_init)
};
