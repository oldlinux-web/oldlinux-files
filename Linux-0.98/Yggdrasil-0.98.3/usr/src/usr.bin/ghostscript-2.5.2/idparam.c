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

/* idparam.c */
/* Utilities for getting parameters out of dictionaries. */
#include "memory_.h"
#include "ghost.h"
#include "dict.h"
#include "gsmatrix.h"		/* for dict_matrix_param */
#include "dparam.h"		/* interface definition */
#include "errors.h"
#include "iutil.h"		/* for num_params */

/* Get a Boolean parameter from a dictionary. */
/* Return 0 if found, 1 if defaulted, <0 if wrong type. */
int
dict_bool_param(const ref *pdict, const ref *pname, int defaultval, int *pvalue)
{	ref *pdval;
	if ( dict_find(pdict, pname, &pdval) <= 0 )
	   {	*pvalue = defaultval;
		return 1;
	   }
	if ( !r_has_type(pdval, t_boolean) ) return e_typecheck;
	*pvalue = pdval->value.index;
	return 0;
}		

/* Get an integer parameter from a dictionary. */
/* Return 0 if found, 1 if defaulted, <0 if missing or out of range. */
/* Note that the default value may be out of range, in which case */
/* a missing value will return e_rangecheck rather than 1. */
int
dict_int_param(const ref *pdict, const ref *pname, int minval, int maxval,
  int defaultval, int *pvalue)
{	int code;
	ref *pdval;
	long ival;
	if ( dict_find(pdict, pname, &pdval) <= 0 )
	   {	ival = defaultval;
		code = 1;
	   }
	else
	   {	switch ( r_type(pdval) )
		{
		case t_integer:
			ival = pdval->value.intval;
			break;
		case t_real:
			/* Allow an integral real, because Fontographer */
			/* (which violates the Adobe specs in other ways */
			/* as well) sometimes generates output that */
			/* needs this. */
			if ( pdval->value.realval < minval || pdval->value.realval > maxval )
				return e_rangecheck;
			ival = (long)pdval->value.realval;
			if ( ival != pdval->value.realval )
				return e_rangecheck;
			break;
		default:
			return e_typecheck;
		}
		code = 0;
	   }
	if ( ival < minval || ival > maxval )
		return e_rangecheck;
	*pvalue = (int)ival;
	return code;
}		

/* Get a float parameter from a dictionary. */
/* Return 0 if found, 1 if defaulted, <0 if wrong type. */
int
dict_float_param(const ref *pdict, const ref *pname, floatp defaultval, float *pvalue)
{	ref *pdval;
	if ( dict_find(pdict, pname, &pdval) <= 0 )
	   {	*pvalue = defaultval;
		return 1;
	   }
	switch ( r_type(pdval) )
	   {
	case t_integer: *pvalue = pdval->value.intval; return 0;
	case t_real: *pvalue = pdval->value.realval; return 0;
	   }
	return e_typecheck;
}		

/* Get an integer array from a dictionary. */
/* Return the element count if OK, 0 if missing, <0 if invalid. */
int
dict_int_array_param(const ref *pdict, const ref *pname, uint maxlen, int *ivec)
{	ref *pdval;
	const ref *pa;
	int *pi = ivec;
	uint size;
	int i;
	if ( dict_find(pdict, pname, &pdval) <= 0 ) return 0;
	if ( !r_has_type(pdval, t_array) ) return e_typecheck;
	size = r_size(pdval);
	if ( size > maxlen ) return e_limitcheck;
	pa = pdval->value.const_refs;
	for ( i = 0; i < size; i++, pa++, pi++ )
	   {	/* See dict_int_param above for why we allow reals here. */
		switch ( r_type(pa) )
		{
		case t_integer:
			if ( pa->value.intval != (int)pa->value.intval )
				return e_rangecheck;
			*pi = (int)pa->value.intval;
			break;
		case t_real:
			/* We don't have max_int and min_int.... */
			if ( pa->value.realval < -(int)(max_uint>>1)-1 ||
			     pa->value.realval > max_uint>>1 ||
			     pa->value.realval != (int)pa->value.realval
			   )
				return e_rangecheck;
			*pi = (int)pa->value.realval;
			break;
		default:
			return e_typecheck;
		}
	   }
	return size;
}		

/* Get a float array from a dictionary. */
/* Return the element count if OK, <0 if invalid. */
/* If the parameter is missing, then if defaultvec is NULL, return 0; */
/* if defaultvec is not NULL, copy it into fvec (maxlen elements) */
/* and return maxlen. */
int
dict_float_array_param(const ref *pdict, const ref *pname, uint maxlen, float *fvec, float *defaultvec)
{	ref *pdval;
	uint size;
	int code;
	if ( dict_find(pdict, pname, &pdval) <= 0 )
	{	if ( defaultvec == NULL ) return 0;
		memcpy(fvec, defaultvec, maxlen * sizeof(float));
		return maxlen;
	}
	if ( !r_has_type(pdval, t_array) ) return e_typecheck;
	size = r_size(pdval);
	if ( size > maxlen ) return e_limitcheck;
	code = num_params(pdval->value.refs + size - 1, size, fvec);
	return (code >= 0 ? size : code);
}		

/* Get a matrix from a dictionary. */
int
dict_matrix_param(const ref *pdict, const ref *pname, gs_matrix *pmat)
{	ref *pdval;
	if ( dict_find(pdict, pname, &pdval) <= 0 ) return e_typecheck;
	return read_matrix(pdval, pmat);
}
