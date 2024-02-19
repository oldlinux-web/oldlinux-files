/* Copyright (C) 1989, 1990, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gxmatrix.h */
/* Internal matrix routines for Ghostscript library */
#include "gsmatrix.h"

/* A matrix with a cached fixed-point copy of the translation. */
/* This is only used by a few routines; they are responsible */
/* for ensuring the validity of the cache */
/* (by calling gs_update_matrix_fixed). */
typedef struct gs_matrix_fixed_s {
	_matrix_body;
	fixed tx_fixed, ty_fixed;
} gs_matrix_fixed;
extern	void	gs_update_matrix_fixed(P1(gs_matrix_fixed *));

/* Coordinate transformations to fixed point */
int	gs_point_transform2fixed(P4(const gs_matrix_fixed *, floatp, floatp, gs_fixed_point *)),
	gs_distance_transform2fixed(P4(const gs_matrix_fixed *, floatp, floatp, gs_fixed_point *));

/* Macro for testing whether matrix coefficients are zero, */
/* for shortcuts when the matrix has no skew. */
#define is_skewed(pmat) !is_fzero2((pmat)->xy, (pmat)->yx)

/* Define the fixed-point coefficient structure for avoiding */
/* floating point in coordinate transformations. */
/* Currently this is used only by the Type 1 font interpreter. */
/* The setup is in gscoord.c. */
typedef struct {
	long l;
	fixed f;
} coeff1;
typedef struct {
	coeff1 xx, xy, yx, yy;
	int skewed;
	int shift;			/* see m_fixed */
	int max_bits;			/* max bits of coefficient */
	fixed round;			/* ditto */
} fixed_coeff;
/* Multiply an integer not exceeding max_bits in magnitude */
/* by a coefficient from a fixed_coeff. */
#define m_fixed(iv, c, fc, maxb)\
  (((iv) + (1 << (maxb - 1))) & (-1 << maxb) ?	/* out of range */\
   (long)((iv) * (fc).c.f) :\
   arith_rshift((iv) * (fc).c.l + (fc).round, (fc).shift))
