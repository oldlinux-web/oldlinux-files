/* Copyright (C) 1990, 1992 Aladdin Enterprises.  All rights reserved.
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

/* gxtype1.h */
/* Private Adobe Type 1 font definitions for GhostScript library */
#include "gscrypt1.h"
#include "gstype1.h"

/* Define the structures for the state of a Type 1 interpreter. */

/* Define the control state of the interpreter. */
/* This is what must be saved and restored */
/* when calling a CharString subroutine. */
typedef struct {
	const byte *ip;
	crypt_state dstate;
} ip_state;

/* Define the stem hint tables. */
/* Each stem hint table is kept sorted. */
/* Stem hints are in device coordinates. */
#define max_stems 3			/* arbitrary */
typedef struct {
	fixed v0, v1;			/* coordinates (widened a little) */
	fixed dv0, dv1;			/* adjustment values */
} stem_hint;
typedef struct {
	int count;
	stem_hint *current;		/* cache cursor for search */
	stem_hint data[max_stems];
} stem_hint_table;

/* Define the standard stem width tables. */
/* Each table is sorted, since the StemSnap arrays are sorted. */
#define max_snaps (1 + max_StemSnap)
typedef struct {
	int count;
	fixed data[max_snaps];
} stem_snap_table;

/* Define the alignment zone structure. */
/* These are in device coordinates also. */
#define max_a_zones (max_BlueValues + max_OtherBlues)
typedef struct {
	int is_top_zone;
	fixed v0, v1;			/* range for testing */
	fixed flat;			/* flat position */
} alignment_zone;

/* Define the structure for hints that depend only on the font and CTM, */
/* not on the individual character.  Eventually these should be cached */
/* with the font/matrix pair. */
typedef struct font_hints_s {
	int axes_swapped;		/* true if x & y axes interchanged */
					/* (only set if using hints) */
	int x_inverted, y_inverted;	/* true if axis is inverted */
	int use_x_hints;		/* true if we should use hints */
					/* for char space x coords (vstem) */
	int use_y_hints;		/* true if we should use hints */
					/* for char space y coords (hstem) */
	stem_snap_table snap_h;		/* StdHW, StemSnapH */
	stem_snap_table snap_v;		/* StdVW, StemSnapV */
	fixed blue_fuzz, blue_shift;	/* alignment zone parameters */
					/* in device pixels */
	int suppress_overshoot;		/* (computed from BlueScale) */
	int a_zone_count;		/* # of alignment zones */
	alignment_zone a_zones[max_a_zones];	/* the alignment zones */
} font_hints;

/* This is the full state of the Type 1 interpreter. */
#define ostack_size 24			/* per documentation */
#define ipstack_size 10			/* per documentation */
struct gs_type1_state_s {
		/* The following are set at initialization */
	gs_show_enum *penum;		/* show enumerator */
	gs_state *pgs;			/* graphics state */
	gs_type1_data *pdata;		/* font-specific data */
	int charpath_flag;		/* 0 for show, 1 for false */
					/* charpath, 2 for true charpath */
	int paint_type;			/* 0/3 for fill, 1/2 for stroke */
	fixed_coeff fc;			/* cached fixed coefficients */
	float flatness;			/* flatness for character curves */
	font_hints fh;			/* font-level hints */
		/* The following are updated dynamically */
	fixed ostack[ostack_size];	/* the Type 1 operand stack */
	int os_count;			/* # of occupied stack entries */
	ip_state ipstack[ipstack_size+1];	/* control stack */
	int ips_count;			/* # of occupied entries */
	gs_fixed_point lsb;		/* left side bearing */
	gs_fixed_point width;		/* character width (char coords) */
	int seac_base;			/* base character code for seac, */
					/* or -1 */
		/* The following are set dynamically. */
	int in_dotsection;		/* true if inside dotsection */
	int vstem3_set;			/* true if vstem3 seen */
	gs_fixed_point vs_offset;	/* device space offset for centering */
					/* middle stem of vstem3 */
	stem_hint_table hstem_hints;	/* horizontal stem hints */
	stem_hint_table vstem_hints;	/* vertical stem hints */
};

/* Interface between main Type 1 interpreter and hint routines. */
extern void compute_font_hints(P3(font_hints *, const gs_matrix_fixed *,
				  const gs_type1_data *));
extern void reset_stem_hints(P1(gs_type1_state *));
extern void find_stem_hints(P6(gs_type1_state *, fixed, fixed,
			       fixed, fixed, gs_fixed_point *));
extern void type1_hstem(P3(gs_type1_state *, fixed, fixed));
extern void type1_vstem(P3(gs_type1_state *, fixed, fixed));
