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

/* bfont.h */
/* Internal routines and data needed for building fonts */
#include "font.h"

/* In zfont.c */
extern ref name_FID;
extern ref name_FontMatrix;
extern int add_FID(P2(ref *pfdict, gs_font *pfont));

/* In zfont2.c */
extern ref name_UniqueID;
extern int build_gs_font(P4(os_ptr, gs_font **, font_type, const ref *));
extern int build_gs_simple_font(P4(os_ptr, gs_font **, font_type,
				   const ref *));
extern int font_int_param(P6(const ref *pdict, const ref *pname,
			     int minval, int maxval,
			     int defaultval, int *pvalue));
