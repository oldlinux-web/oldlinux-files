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

/* dparam.h */
/* Interface to idparam.c */

#ifndef gs_matrix_DEFINED
#  define gs_matrix_DEFINED
typedef struct gs_matrix_s gs_matrix;
#endif

int dict_bool_param(P4(const ref *pdict, const ref *pname,
		       int defaultval, int *pvalue));
int dict_int_param(P6(const ref *pdict, const ref *pname,
		      int minval, int maxval, int defaultval, int *pvalue));
int dict_float_param(P4(const ref *pdict, const ref *pname,
			floatp defaultval, float *pvalue));
int dict_int_array_param(P4(const ref *pdict, const ref *pname,
			    uint maxlen, int *ivec));
int dict_float_array_param(P5(const ref *pdict, const ref *pname,
			      uint maxlen, float *fvec, float *defaultvec));
int dict_matrix_param(P3(const ref *pdict, const ref *pname,
			 gs_matrix *pmat));
