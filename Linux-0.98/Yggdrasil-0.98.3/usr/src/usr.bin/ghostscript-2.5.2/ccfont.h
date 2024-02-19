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

/* ccfont.h */
/* Header for fonts compiled into C. */

/* Define type-specific refs for initializing arrays. */
#define ref_(t) struct { struct tas_s tas; t value; }
#define boolean_v(b) { {t_boolean<<r_type_shift}, (ushort)(b) }
#define integer_v(i) { {t_integer<<r_type_shift}, (long)(i) }
#define null_v() { {t_null<<r_type_shift} }
#define real_v(v) { {t_real<<r_type_shift}, (float)(v) }

/* Define ref creation macros. */
#include "store.h"
#define make_array(pref,attrs,size,elts)\
  make_tasv(pref,t_array,attrs,size,refs,elts)
#define make_string(pref,attrs,size,chars)\
  make_tasv(pref,t_string,attrs,size,const_bytes,chars)
 
/* Define other initialization structures. */
typedef struct { byte encx, charx; } charindex;
/* Arrays of strings/names/nulls are represented by byte strings, */
/* containing two bytes of big-endian string length (255,255=null) */
/* followed by the string characters. */
typedef const char *cfont_string_array;

/* Support routines in iccfont.c */
typedef struct {
	const charindex *enc_keys;	/* keys from encoding vectors */
	uint num_enc_keys;
	uint num_str_keys;
	uint extra_slots;		/* (need 1 extra for fonts) */
	uint dict_attrs;		/* protection for dictionary */
	uint value_attrs;		/* protection for values */
					/* (only used for string dicts) */
} cfont_dict_keys;
/* We pass a procedure vector to the font initialization routine */
/* to avoid having externs, which compromise sharability. */
/* On MS-DOS, each compiled font has its own data segment, */
/* so all of these procedures must be declared 'huge' for Borland C. */
typedef struct cfont_procs_s {
	int huge (*ref_dict_create)(P4(ref *, const cfont_dict_keys *,
				       cfont_string_array, const ref *));
	int huge (*string_dict_create)(P4(ref *, const cfont_dict_keys *,
					  cfont_string_array,
					  cfont_string_array));
	int huge (*num_dict_create)(P4(ref *, const cfont_dict_keys *,
				       cfont_string_array, const float *));
	int huge (*name_array_create)(P3(ref *, cfont_string_array, int));
	int huge (*string_array_create)(P4(ref *, cfont_string_array,
					   int /*size*/,
					   uint /*protection*/));
	int huge (*name_create)(P2(ref *, const char *));
} cfont_procs;
