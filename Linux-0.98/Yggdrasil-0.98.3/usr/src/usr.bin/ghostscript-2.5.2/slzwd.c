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

/* slzwd.c */
/* LZW decoding filter */
#include "stdio_.h"	/* includes std.h */
#include "gdebug.h"
#include "stream.h"

/********************************************************/
/* LZW routines are based on:				*/
/* Dr. Dobbs Journal --- Oct. 1989. 			*/
/* Article on LZW Data Compression by Mark R. Nelson 	*/
/********************************************************/

/*
 * This code implements enhancements to the LZW algorithm.
 * For a full explanation of the algorithm, see the file slzwe.c.
 */

/* Define the special codes */
#define code_reset 256
#define code_eod 257
#define code_0 258			/* first assignable code */

/* ------ LZWDecode filter ------ */

typedef struct lzw_decode_s {
	byte datum;
	byte len;			/* length of code */
	ushort prefix;			/* code to be prefixed */
} lzw_decode;
#define lzw_decode_max 4096		/* must be 4096 */

typedef struct lzw_decode_table_s {
	lzw_decode decode[lzw_decode_max+1];
} lzw_decode_table;

/* Export the size of the LZW decoding table. */
const uint s_LZWD_table_sizeof = sizeof(lzw_decode_table);

/* Initialize LZWDecode filter */
void
s_LZWD_init(register stream *s, lzw_decode_table *table, int enhanced)
{	register lzw_decode *dc;
	register int i;
	s->lzws.bits_left = 0;
	s->lzws.next_code = code_0;
	s->lzws.code_size = 9;
	s->lzws.prev_code = -1;
	s->odd = -1;
	s->lzws.enhanced = enhanced;
	s->lzws.decode_table = table;
	dc = table->decode;
	dc[code_reset].len = 255;
	dc[code_eod].len = 255;
	for ( i = 0; i < 256; i++, dc++ )
	  dc->datum = i, dc->len = 1, dc->prefix = code_eod;
}

/* Buffer refill for LZWDecode filter */
/****** DOESN'T HANDLE CODES LONGER THAN THE BUFFER SIZE ******/
private int
s_LZWD_read_buf(register stream *s)
{	int code = s->odd;
	int prev_code = s->lzws.prev_code;
	byte bits = s->lzws.bits;
	int bits_left = s->lzws.bits_left;
	int code_size = s->lzws.code_size;	/* cache only */
	int code_mask = (1 << code_size) - 1;
	int next_code = s->lzws.next_code;
	register stream *strm = s->strm;
	register byte *p = s->cbuf;
	byte *limit = p + s->bsize;
	lzw_decode *table = s->lzws.decode_table->decode;
	lzw_decode *dc_next = table + next_code;
	int enhanced = s->lzws.enhanced;
	lzw_decode *dc;
	uint prev_len, len;
	int c;
	byte *p1;
	if ( prev_code >= 0 )
		prev_len = table[prev_code].len;
	if ( code >= 0 ) goto add;
top:	code = bits << (code_size - bits_left);
	bits = sgetc(strm);
	if ( (bits_left += 8 - code_size) < 0 )
	   {	code += bits << -bits_left;
		bits = sgetc(strm);
		bits_left += 8;
	   }
	code = (code + (bits >> bits_left)) & code_mask;
#ifdef DEBUG
if ( gs_debug['w'] )
	dprintf2("[w]reading 0x%x,%d", code, code_size);
#endif
	/* Invert the code-shortening algorithm described in slzwd.c. */
	if ( enhanced )
	   {	uint N = code_mask + 1;
		uint M = (prev_code < 0 ? next_code : next_code + 1);
		uint D = N - M;
		if ( code < D << 1 )	/* S-1 bits */
			code >>= 1, ++bits_left;
		else if ( !(code & 1) )	/* S bits, < N/2 */
			code >>= 1;
		else			/* S bits, >= N/2 */
			code = (code >> 1) - N / 2 + M;
#ifdef DEBUG
if ( gs_debug['w'] )
   {	if ( enhanced )
		dprintf2(" -> 0x%x,%d", code,
			 (code < D ? code_size - 1 : code_size));
   }
#endif
	   }
#ifdef DEBUG
if ( gs_debug['w'] )
	dputc('\n');
#endif
add:	/*
	 * There is an anomalous case where a code S is followed
	 * immediately by another occurrence of the S string.
	 * In this case, the next available code will be defined as
	 * S followed by the first character of S, and will be
	 * emitted immediately after the code S.  We have to
	 * recognize this case specially, by noting that the code is
	 * equal to next_code.
	 */
	if ( code == next_code )
	   {	/* Fabricate the entry for the code.  It will be */
		/* overwritten immediately, of course. */
		for ( c = prev_code; c != code_eod; c = table[c].prefix )
			dc_next->datum = c;
		len = prev_len + 1;
		dc_next->len = min(len, 255);
		dc_next->prefix = prev_code;
#ifdef DEBUG
if ( gs_debug['w'] )
		dprintf3("[w]decoding anomalous 0x%x=0x%x+%c\n",
		         next_code, prev_code, dc_next->datum);
#endif
	   }
	/* See if there is enough room for the code. */
	len = table[code].len;
	if ( len == 255 )
	   {	/* Check for special code (reset or end). */
		/* We set their lengths to 255 to avoid doing */
		/* an extra check in the normal case. */
		switch ( code )
		   {
		case code_reset:
			next_code = code_0;
			dc_next = table + code_0;
			s->lzws.code_size = code_size = 9;
			code_mask = (1 << 9) - 1;
			prev_code = -1;
			goto top;
		case code_eod:
			s->end_status = EOFC;
			goto out;
		   }
		/* The code length won't fit in a byte, */
		/* compute it the hard way. */
		for ( c = code, len = 0; c != code_eod; len++ )
			c = table[c].prefix;
	   }
	if ( limit - p < len )
	   {	s->odd = code;
		goto out;
	   }
	/* Copy the string to the buffer (back to front). */
	c = code;
	p1 = p += len;
	do
	   {	*--p1 = (dc = &table[c])->datum;
	   }
	while ( (c = dc->prefix) != code_eod );
	/* Add a new entry to the table */
	if ( prev_code >= 0 )
	   {	dc_next->datum = *p1;	/* first char of string */
		++prev_len;
		dc_next->len = min(prev_len, 255);
		dc_next->prefix = prev_code;
		dc_next++;
#ifdef DEBUG
if ( gs_debug['w'] )
		dprintf4("[w]decoding 0x%x=0x%x+%c(%d)\n",
		         next_code, prev_code, *p1, min(len, 255));
#endif
		if ( ++next_code == code_mask )
			/* Crossed a power of 2 */
			code_size = ++(s->lzws.code_size),
			code_mask = (1 << code_size) - 1;
	   }
	prev_code = code;
	prev_len = len;
	goto top;
out:	s->cptr = s->cbuf - 1;
	s->endptr = p - 1;
	s->lzws.prev_code = prev_code;
	s->lzws.bits = bits;
	s->lzws.bits_left = bits_left;
	s->lzws.next_code = next_code;
	return 0;
}

/* Stream procedures */
const stream_procs s_LZWD_procs =
   {	s_std_noavailable, NULL, s_std_read_flush, s_std_close,
	s_LZWD_read_buf, NULL
   };
