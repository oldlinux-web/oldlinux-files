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

/* zcie.c */
/* CIE color operators for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "gscspace.h"
#include "gscie.h"
#include "dict.h"
#include "dparam.h"
#include "state.h"

/* There are actually only two CIE-specific operators, */
/* but CIE color dictionaries are so complex that */
/* we handle the CIE case of setcolorspace here as well. */
/* CIE color takes up so much space that we intend to */
/* make it a separate option. */

/* Elements in a CIEBasedABC color space dictionary: */
static ref name_RangeABC;
static ref name_DecodeABC;
static ref name_MatrixABC;
static ref name_RangeLMN;
static ref name_DecodeLMN;
static ref name_MatrixLMN;
static ref name_WhitePoint;
static ref name_BlackPoint;
/* Additional or alternate elements in a CIEBasedA c.s. dictionary: */
static ref name_RangeA;
static ref name_DecodeA;
static ref name_MatrixA;
/* (Additional) elements in a CIE color rendering dictionary: */
static ref name_ColorRenderingType;
static ref name_EncodeLMN;
static ref name_EncodeABC;
static ref name_MatrixPQR;
static ref name_RangePQR;
static ref name_TransformPQR;
static ref name_RenderTable;

/* Initialization */
private void
zcie_init()
{	static const names_def cien[] = {

	/* Elements in a CIEBasedABC color space dictionary: */
	   { "RangeABC", &name_RangeABC },
	   { "DecodeABC", &name_DecodeABC },
	   { "MatrixABC", &name_MatrixABC },
	   { "RangeLMN", &name_RangeLMN },
	   { "DecodeLMN", &name_DecodeLMN },
	   { "MatrixLMN", &name_MatrixLMN },
	   { "WhitePoint", &name_WhitePoint },
	   { "BlackPoint", &name_BlackPoint },

	/* Additional or alternate elements in a CIEBasedA c.s. dictionary: */
	   { "RangeA", &name_RangeA },
	   { "DecodeA", &name_DecodeA },
	   { "MatrixA", &name_MatrixA },

	/* (Additional) elements in a CIE color rendering dictionary: */
	   { "ColorRenderingType", &name_ColorRenderingType },
	   { "EncodeLMN", &name_EncodeLMN },
	   { "EncodeABC", &name_EncodeABC },
	   { "MatrixPQR", &name_MatrixPQR },
	   { "RangePQR", &name_RangePQR },
	   { "TransformPQR", &name_TransformPQR },
	   { "RenderTable", &name_RenderTable },

	/* Mark the end of the initalized name list. */
	   names_def_end
	};

	init_names(cien);
}

/* ------ Operators ------ */

/* Get a 3-element range parameter from a dictionary. */
#define dict_range3_param(op, pname, prange)\
	dict_float_array_param(op, pname, 6, (float *)prange, (float *)&Range3_default)

/* Get a 3x3 matrix parameter from a dictionary. */
#define dict_matrix3_param(op, pname, pmat)\
	dict_float_array_param(op, pname, 9, (float *)pmat, (float *)&Matrix3_default)
		
/* Shared code for getting WhitePoint and BlackPoint values. */
private int
cie_points_param(ref *pdref, gs_cie_wb *pwb)
{	int code;
	if ( (code = dict_float_array_param(pdref, &name_WhitePoint, 3, (float *)&pwb->WhitePoint, NULL)) < 0 ||
	     (code = dict_float_array_param(pdref, &name_BlackPoint, 3, (float *)&pwb->BlackPoint, (float *)&BlackPoint_default)) < 0
	   )
		return code;
	if ( pwb->WhitePoint.u <= 0 ||
	     pwb->WhitePoint.v != 1 ||
	     pwb->WhitePoint.w <= 0 ||
	     pwb->BlackPoint.u < 0 ||
	     pwb->BlackPoint.v < 0 ||
	     pwb->BlackPoint.w < 0
	   )
		return e_rangecheck;
	return 0;
}

/* Common code for the CIEBasedA[BC] cases of setcolorspace. */
private int
cie_lmnp_param(ref *pdref, gs_cie_common *pcie)
{	int code;
	if ( (code = dict_range3_param(pdref, &name_RangeLMN, &pcie->RangeLMN)) < 0 ||
	     /****** DecodeLMN ******/
	     (code = dict_matrix3_param(pdref, &name_MatrixLMN, &pcie->MatrixLMN)) < 0 ||
	     (code = cie_points_param(pdref, &pcie->points)) < 0
	   )
		return code;
	return 0;
}

/* Get the parameters of a CIEBasedABC color space. */
/* This doesn't actually implement setcolorspace, */
/* since it has to be usable for the base color space */
/* of Separation, Indexed, and Pattern spaces as well. */
int
zcolorspace_CIEBasedABC(ref *pdref, gs_cie_abc *pcie)
{	int code;
	check_type(*pdref, t_dictionary);
	check_dict_read(*pdref);
	if ( (code = dict_range3_param(pdref, &name_RangeABC, &pcie->RangeABC)) < 0 ||
	     /****** DecodeABC ******/
	     (code = dict_matrix3_param(pdref, &name_MatrixABC, &pcie->MatrixABC)) < 0 ||
	     (code = cie_lmnp_param(pdref, &pcie->common)) < 0
	   )
		return code;
	/* NOT IMPLEMENTED YET */
	return e_undefined;
}

/* Get the parameters of a CIEBasedA color space. */
/* See above. */
int
zcolorspace_CIEBasedA(ref *pdref, gs_cie_a *pcie)
{	int code;
	check_type(*pdref, t_dictionary);
	check_dict_read(*pdref);
	if ( (code = dict_float_array_param(pdref, &name_RangeA, 2, (float *)&pcie->RangeA, (float *)&RangeA_default)) < 0 ||
	     /****** DecodeA ******/
	     (code = dict_float_array_param(pdref, &name_MatrixA, 3, (float *)&pcie->MatrixA, (float *)&MatrixA_default)) < 0 ||
	     (code = cie_lmnp_param(pdref, &pcie->common)) < 0
	   )
		return code;
	/* NOT IMPLEMENTED YET */
	return e_undefined;
}

/* currentcolorrendering */
private int
zcurrentcolorrendering(register os_ptr op)
{	push(1);
	*op = istate.colorrendering;
	return 0;
}

/* setcolorrendering */
private int
zsetcolorrendering(register os_ptr op)
{	int code;
	int discard;
	gs_cie_render render;
	ref *pRT;
	check_read_type(*op, t_dictionary);
	check_dict_read(*op);
	if ( (code = dict_int_param(op, &name_ColorRenderingType, 1, 1, 0, &discard)) < 0 ||
	     (code = dict_matrix3_param(op, &name_MatrixLMN, &render.MatrixLMN)) < 0 ||
	     /****** EncodeLMN ******/
	     (code = dict_range3_param(op, &name_RangeLMN, &render.RangeLMN)) < 0 ||
	     (code = dict_matrix3_param(op, &name_MatrixABC, &render.MatrixABC)) < 0 ||
	     /****** EncodeABC ******/
	     (code = dict_range3_param(op, &name_RangeABC, &render.RangeABC)) < 0 ||
	     (code = cie_points_param(op, &render.points)) < 0 ||
	     (code = dict_matrix3_param(op, &name_MatrixPQR, &render.MatrixPQR)) < 0 ||
	     (code = dict_range3_param(op, &name_RangePQR, &render.RangePQR)) < 0 ||
	     0 /****** TransformPQR ******/
	   )
		return code;
#define rRT render.RenderTable
	if ( dict_find(op, &name_RenderTable, &pRT) > 0 )
	{	const ref *prte;
		int i;
		uint n2;
		check_read_type(*pRT, t_array);
		prte = pRT->value.const_refs;
		check_type(prte[0], t_integer);
		check_type(prte[1], t_integer);
		check_type(prte[2], t_integer);
		check_read_type(prte[3], t_array);
		check_type(prte[4], t_integer);
		if ( prte[0].value.intval < 1 ||
		     prte[1].value.intval < 1 ||
		     prte[2].value.intval < 1 ||
		     !(prte[4].value.intval == 3 || prte[4].value.intval == 4)
		   )
			return e_rangecheck;
		rRT.NA = prte[0].value.intval;
		rRT.NB = prte[1].value.intval;
		rRT.NC = prte[2].value.intval;
		rRT.m = prte[4].value.intval;
		n2 = rRT.NB * rRT.NC;
		if ( r_size(pRT) != rRT.m + 5 || r_size(&prte[3]) != rRT.NA )
			return e_rangecheck;
		for ( i = 0; i < rRT.m; i++ )
		{	const ref *prt2 = prte[3].value.const_refs + i;
			check_read_type(*prt2, t_string);
			if ( r_size(prt2) != n2 )
				return e_rangecheck;
		}
		/****** Make table ******/
	}
	else
		rRT.table = 0;
#undef rRT
	/*istate.colorrendering = *op;*/
	/*pop(1);*/
	/* NOT IMPLEMENTED YET */
	return e_undefined;
}

/* ------ Initialization procedure ------ */

op_def zcie_op_defs[] = {
	{"0currentcolorrendering", zcurrentcolorrendering},
	{"1setcolorrendering", zsetcolorrendering},
	op_def_end(zcie_init)
};
