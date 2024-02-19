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

/* zimage2.c */
/* image operator extensions for Level 2 PostScript */
#include "memory_.h"
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "gscolor.h"
#include "gscspace.h"
#include "gscolor2.h"
#include "gsmatrix.h"
#include "dict.h"
#include "dparam.h"
#include "state.h"		/* for igs */

/* Imported from zpaint.c */
extern int zimage_setup(P9(int width, int height, gs_matrix *pmat,
  ref *sources, int bits_per_component, int num_components, int masked,
  const float *decode, int npop));

/* Names of keys in image dictionaries: */
static ref name_ImageType;
static ref name_Width;
static ref name_Height;
static ref name_ImageMatrix;
static ref name_MultipleDataSources;
static ref name_DataSource;
static ref name_BitsPerComponent;
static ref name_Decode;
static ref name_Interpolate;

/* Initialization */
private void
zimage2_init()
{	static const names_def imn[] = {

	/* Create the names of the known entries in */
	/* an image dictionary. */
	   { "ImageType", &name_ImageType },
	   { "Width", &name_Width },
	   { "Height", &name_Height },
	   { "ImageMatrix", &name_ImageMatrix },
	   { "MultipleDataSources", &name_MultipleDataSources },
	   { "DataSource", &name_DataSource },
	   { "BitsPerComponent", &name_BitsPerComponent },
	   { "Decode", &name_Decode },
	   { "Interpolate", &name_Interpolate },

	/* Mark the end of the initalized name list. */
	    names_def_end
	};

	init_names(imn);
}


/* Define a structure for acquiring image parameters. */
typedef struct image_params_s {
	int Width;
	int Height;
	gs_matrix ImageMatrix;
	int /*boolean*/ MultipleDataSources;
	ref DataSource[4];
	int BitsPerComponent;
	int num_components;
	float Decode[2*4];
	int /*boolean*/ Interpolate;
} image_params;

/* Common code for unpacking an image dictionary. */
private int
image_dict_unpack(os_ptr op, image_params *pip, int max_bits_per_component)
{	int code;
	int num_components;
	gs_color_space cs;
	ref *pds;
	check_type(*op, t_dictionary);
	check_dict_read(*op);
	num_components = gs_currentcolorspace(igs, &cs);
	if ( num_components < 1 ) return e_rangecheck;
	if ( (code = dict_int_param(op, &name_ImageType, 1, 1, 1,
				    &code)) < 0 ||
	     (code = dict_int_param(op, &name_Width, 0, 0x7fff, -1,
				    &pip->Width)) < 0 ||
	     (code = dict_int_param(op, &name_Height, 0, 0x7fff, -1,
				    &pip->Height)) < 0 ||
	     (code = dict_matrix_param(op, &name_ImageMatrix,
				    &pip->ImageMatrix)) < 0 ||
	     (code = dict_bool_param(op, &name_MultipleDataSources, 0,
				    &pip->MultipleDataSources)) < 0 ||
	     (code = dict_int_param(op, &name_BitsPerComponent, 0,
				    max_bits_per_component, -1,
				    &pip->BitsPerComponent)) < 0 ||
	     (code = dict_float_array_param(op, &name_Decode,
				    num_components * 2,
				    &pip->Decode[0], NULL)) < 0 ||
	     (code = dict_bool_param(op, &name_Interpolate, 0,
				    &pip->Interpolate)) < 0
	   )
		return code;
	/* Extract and check the data sources. */
	if ( (code = dict_find(op, &name_DataSource, &pds)) < 0 )
		return code;
	if ( pip->MultipleDataSources )
	{	check_type(*pds, t_array);
		if ( r_size(pds) != num_components ) return e_rangecheck;
		memcpy(&pip->DataSource[0], pds->value.refs, sizeof(ref) * num_components);
	}
	else
		pip->DataSource[0] = *pds;
	pip->num_components = num_components;
	/* We don't support arbitrary decoding yet: */
	/* all the decoding pairs must be the same. */
	{	int i;
		float *dp;
		for ( i = 1, dp = &pip->Decode[2]; i < num_components;
		      i++, dp += 2
		    )
			if ( dp[-2] != dp[0] || dp[-1] != dp[1] )
				return e_rangecheck;
	}
	return 0;
}

/* .dictimage */
int
zdictimage(register os_ptr op)
{	image_params ip;
	int code = image_dict_unpack(op, &ip, 12);
	if ( code < 0 ) return code;
	return zimage_setup(ip.Width, ip.Height, &ip.ImageMatrix,
		&ip.DataSource[0], ip.BitsPerComponent,
		ip.num_components, 0, &ip.Decode[0], 1);
}

/* .dictimagemask */
int
zdictimagemask(register os_ptr op)
{	image_params ip;
	int code = image_dict_unpack(op, &ip, 1);
	if ( code < 0 ) return code;
	if ( ip.MultipleDataSources ) return e_rangecheck;
	return zimage_setup(ip.Width, ip.Height, &ip.ImageMatrix,
		&ip.DataSource[0], 1, 1, 1, &ip.Decode[0], 1);
}

/* ------ Initialization procedure ------ */

op_def zimage2_op_defs[] = {
	{"1.dictimage", zdictimage},
	{"1.dictimagemask", zdictimagemask},
	op_def_end(zimage2_init)
};
