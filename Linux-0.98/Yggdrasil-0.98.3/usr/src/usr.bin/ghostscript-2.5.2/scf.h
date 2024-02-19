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

/* scf.h */
/* Common definitions for CCITTFax encoding and decoding filters */

/*
 * The CCITT Group 3 and Group 4 fax specifications map run lengths to
 * Huffman codes.  White and black have different mappings.
 * If the run length is 64 or greater, two codes are needed,
 * a 'make-up' code that encodes the multiple of 64, and a 'termination'
 * code for the remainder; for runs of 63 or less, only the 'termination'
 * code is needed.
 */

/* ------ Encoding tables ------ */

/* Define the structure for the encoding tables. */
typedef struct cfe_run_s {
	byte code;		/* (low 8 bits of) code: */
				/* remember to widen this to uint! */
	char code_length;
} cfe_run;
#define cfe_entry(c, len) { c, len }

/* Codes common to 1-D and 2-D encoding. */
/* The decoding algorithms know that EOL is 0....01. */
#define run_eol_code_length 12
#define run_eol_code_value 1
extern const cfe_run cf_run_eol;
extern const cfe_run cf_white_termination[64];
extern const cfe_run cf_white_make_up[41];
extern const cfe_run cf_black_termination[64];
extern const cfe_run cf_black_make_up[41];
extern const cfe_run cf_uncompressed[6];
extern const cfe_run cf_uncompressed_exit[10];	/* indexed by 2 x length of */
			/* white run + (1 if next run black, 0 if white) */
/* 1-D encoding. */
extern const cfe_run cf1_run_uncompressed;
/* 2-D encoding. */
extern const cfe_run cf2_run_pass;
extern const cfe_run cf2_run_vertical[7];	/* indexed by b1 - a1 */
extern const cfe_run cf2_run_horizontal;
extern const cfe_run cf2_run_uncompressed;
/* 2-D Group 3 encoding. */
extern const cfe_run cf2_run_eol_1d;
extern const cfe_run cf2_run_eol_2d;

/* ------ Decoding tables ------ */

/*
 * Define the structure for the decoding tables.
 * First-level nodes are either leaves, which have
 *	run_length = white or black run length
 *	code_length <= initial_bits
 * or non-leaves, which have
 *	run_length = the index of a sub-table
 *	code_length = initial_bits + the number of additional dispatch bits
 * Second-level nodes are always leaves, with
 *	code_length = the actual number of bits in the code - initial_bits.
 */
#define run_error (-1)
#define run_zeros (-2)	/* EOL follows, possibly with more padding first */
#define run_uncompressed (-3)
/* 2-D codes */
#define run2_pass (-4)
#define run2_horizontal (-5)
typedef struct cfd_node_s cfd_node;
struct cfd_node_s {
	short run_length;
	ushort code_length;
};

#define cfd_white_initial_bits 8
extern const cfd_node cf_white_decode[];
#define cfd_black_initial_bits 7
extern const cfd_node cf_black_decode[];
#define cfd_2d_initial_bits 7
extern const cfd_node cf_2d_decode[];
#define cfd_uncompressed_initial_bits 6		/* must be 6 */
extern const cfd_node cf_uncompressed_decode[];

/* ------ Run detection macros ------ */

/* For the run detection macros, white_byte is */
/* 0 or 0xff for BlackIs1 or !BlackIs1 respectively. */
/* data holds p[-1].  count is the number of valid bits remaining */
/* in the scan line. */

/* Tables in scftab.c. */
extern const byte cf_left_bits[8];
extern const byte cf_top_bit[256];
	  
/* Skip over white pixels to find the next black pixel in the input. */
/* There are a lot more white pixels than black pixels, */
/* so we go to some extra trouble to make this efficient. */

#define skip_white_pixels(data, p, count, white_byte, w0_unused)\
{ int top = cf_top_bit[data & cf_left_bits[count & 7]];\
  if ( top ) count = ((count - 1) & ~7) + top;\
  else\
   { for ( ; ; )\
      { if ( p[0] != white_byte )\
	 { data = p[0] ^ white_byte; p += 1; count -= 9; break; }\
	if ( p[1] != white_byte )\
	 { data = p[1] ^ white_byte; p += 2; count -= 17; break; }\
	if ( p[2] != white_byte )\
	 { data = p[2] ^ white_byte; p += 3; count -= 25; break; }\
	if ( p[3] != white_byte )\
	 { data = p[3] ^ white_byte; p += 4; count -= 33; break; }\
	p += 4; count -= 32;\
      }\
     count = (count & ~7) + cf_top_bit[data];\
   }\
}

/* Skip over black pixels to find the next white pixel in the input. */

#define skip_black_pixels(data, p, count, white_byte, b0_unused, b1_unused)\
{ int top = cf_top_bit[~data & cf_left_bits[count & 7]];\
  if ( top ) count = ((count - 1) & ~7) + top;\
  else\
   { while ( (data = *p++ ^ white_byte) == 0xff ) count -= 8;/* Skip long runs quickly. */\
     count = ((count - 9) & ~7) + cf_top_bit[data ^ 0xff];\
   }\
}
