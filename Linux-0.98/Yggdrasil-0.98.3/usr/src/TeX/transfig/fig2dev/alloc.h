/*
 * TransFig: Facility for Translating Fig code
 * Copyright (c) 1991 Micah Beck, Cornell University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cornell University not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cornell University makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORNELL UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORNELL UNIVERSITY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

extern char	*malloc();
extern char	*calloc();

#define		Line_malloc(z)		z = (F_line*)malloc(LINOBJ_SIZE)
#define		Eps_malloc(z)		z = (F_eps*)malloc(EPS_SIZE)
#define		Spline_malloc(z)	z = (F_spline*)malloc(SPLOBJ_SIZE)
#define		Ellipse_malloc(z)	z = (F_ellipse*)malloc(ELLOBJ_SIZE)
#define		Arc_malloc(z)		z = (F_arc*)malloc(ARCOBJ_SIZE)
#define		Compound_malloc(z)	z = (F_compound*)malloc(COMOBJ_SIZE)
#define		Text_malloc(z)		z = (F_text*)malloc(TEXOBJ_SIZE)
#define		Point_malloc(z)		z = (F_point*)malloc(POINT_SIZE)
#define		Control_malloc(z)	z = (F_control*)malloc(CONTROL_SIZE)
#define		Arrow_malloc(z)		z = (F_arrow*)malloc(ARROW_SIZE)

extern char	Err_mem[];
extern char	Err_incomp[];
