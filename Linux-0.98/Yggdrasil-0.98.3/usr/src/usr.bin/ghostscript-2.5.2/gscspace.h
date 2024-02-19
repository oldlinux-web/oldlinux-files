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

/* gscspace.h */
/* Client interface to color spaces for Ghostscript library */

#ifndef gscspace_INCLUDED
#  define gscspace_INCLUDED

/* Color spaces.  Note that Level 1 configurations */
/* support only the Device color spaces. */
typedef enum {
	gs_color_space_DeviceGray = 0,
	gs_color_space_DeviceRGB,
	gs_color_space_DeviceCMYK,
	/* FOLLOWING ARE NOT USED YET */
	gs_color_space_CIEBasedABC,
	gs_color_space_CIEBasedA,
	gs_color_space_Separation,
	gs_color_space_Indexed,
	gs_color_space_Pattern
} gs_color_space_type;
/* Map a (non-pattern) color space to the number of components */
/* in a color drawn from that space. */
extern const int gs_color_space_num_components[];

/*
 * Color spaces are complicated because different spaces involve
 * different kinds of parameters, as follows:

Space		Space parameters		Color parameters
-----		----------------		----------------
DeviceGray	(none)				1 real [0-1]
DeviceRGB	(none)				3 reals [0-1]
DeviceCMYK	(none)				4 reals [0-1]
CIEBasedABC	dictionary			3 reals
CIEBasedA	dictionary			1 real
Separation	name, alt_space, tint_xform	1 real [0-1]
Indexed		hival, lookup, base_space	1 int [0-maxval]
Pattern		colored: (none)			dictionary
		uncolored: base_space		dictionary + base space params

 */

	/* Base color spaces (Device and CIE) */

typedef struct gs_cie_abc_s gs_cie_abc;
typedef struct gs_cie_a_s gs_cie_a;
#define gs_base_cspace_params\
	gs_cie_abc *abc;\
	gs_cie_a *a
typedef struct gs_base_color_space_s {
	gs_color_space_type type;
	union {
		gs_base_cspace_params;
	} params;
} gs_base_color_space;

	/* Non-pattern color spaces (base + Separation + Indexed) */

typedef struct gs_separation_params_s {
	uint sname;	/* BOGUS, should be some other type */
	gs_base_color_space alt_space;
	int (*tint_transform)(P2(floatp, float*));
} gs_separation_params;
typedef struct gs_indexed_params_s {
	const byte *lookup;
	int hival;
	gs_base_color_space base_space;
} gs_indexed_params;
#define gs_non_pattern_cspace_params\
	gs_base_cspace_params;\
	gs_separation_params separation;\
	gs_indexed_params indexed
typedef struct gs_non_pattern_color_space_s {
	gs_color_space_type type;
	union {
		gs_non_pattern_cspace_params;
	} params;
} gs_non_pattern_color_space;

	/* General color spaces (including patterns) */

typedef struct gs_pattern_params_s {
	int has_base_space;
	gs_non_pattern_color_space base_space;
} gs_pattern_params;
typedef struct gs_color_space_s {
	gs_color_space_type type;
	union {
		gs_non_pattern_cspace_params;
		gs_pattern_params pattern;
	} params;
} gs_color_space;

#endif					/* gscspace_INCLUDED */
