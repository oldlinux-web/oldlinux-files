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

/* scfe.c */
/* CCITTFax encoding filter */
#include "stdio_.h"	/* includes std.h */
#include "memory_.h"
#include "gdebug.h"
#include "scf.h"
#include "stream.h"

/* Imported procedures */
extern int s_filter_write_flush(P1(stream *));

/* ------ Macros and support routines ------ */

/* Put a code onto the target stream. */
/* Relevant invariants: 0 <= bits_left <= bits_size; only the leftmost */
/* (bits_size - bits_left) bits of bits contain valid data. */

#define bits_size (arch_sizeof_int == 2 ? 16 : 32)

#ifdef DEBUG
#  define p_c(rp)\
    (gs_debug['w'] ?\
     (dprintf2("[w]0x%x,%d\n", (rp)->code, (rp)->code_length), 0) : 0)
#else
#  define p_c(rp) 0
#endif

#define cf_put_code(s, rp)\
  (p_c(rp),\
   ((s->cfs.bits_left -= (rp)->code_length) >= 0 ?\
    s->cfs.bits += (rp)->code << s->cfs.bits_left :\
    cf_put_code_out(s, (rp)->code)))

private uint
cf_put_code_out(register stream *s, uint code)
{	int left = s->cfs.bits_left;
	const uint cw = s->cfs.bits + (code >> -left);
	stream *strm = s->strm;
#if bits_size > 16
	sputc(strm, cw >> 24);
	sputc(strm, (byte)(cw >> 16));
#endif
	sputc(strm, (byte)(cw >> 8));
	sputc(strm, (byte)cw);
	return (s->cfs.bits = code << (s->cfs.bits_left = left + bits_size));
}

/* Put a run onto the output stream. */

#define cf_put_run(s, lenv, tt, mut)\
{	const cfe_run _ds *rp;\
	if ( lenv >= 64 )\
	{	rp = &mut[lenv >> 6];\
		cf_put_code(s, rp);\
		lenv &= 63;\
	}\
	rp = &tt[lenv];\
	cf_put_code(s, rp);\
}

#define cf_put_white_run(s, lenv)\
  cf_put_run(s, lenv, cf_white_termination, cf_white_make_up)

#define cf_put_black_run(s, lenv)\
  cf_put_run(s, lenv, cf_black_termination, cf_black_make_up)

/* ------ Stream procedures ------ */
	  
/*
 * For the 2-D encoding modes, we leave the previous complete scan line
 * at the beginning of the buffer, and start the new data after it.
 */

/* Initialize CCITTFaxEncode filter */
void
s_CFE_init(register stream *s, CCITTFax_state *pcfs)
{	s->cfs = *pcfs;
	if ( s->cfs.K != 0 )
	{	/* Clear the initial reference line for 2-D encoding. */
		s->cptr = s->cbuf + s->cfs.raster - 1;
		memset(s->cbuf, (s->cfs.BlackIs1 ? 0xff : 0), s->cfs.raster);
		s->cfs.k_left = -1;
	}
	s->cfs.k_left = max(s->cfs.K, 0);
	s->cfs.bits = 0;
	s->cfs.bits_left = bits_size;
}

/* Flush the buffer */
private int cf_encode_1d(P2(stream *, byte *));
private int cf_encode_2d(P3(stream *, byte *, byte *));
private int
s_CFE_write_buf(register stream *s)
{	int raster = s->cfs.raster;
	byte *prev = s->cbuf;
	byte *p = (s->cfs.K ? prev + raster : prev);
	byte *limit = s->cptr;
	byte end_mask = 1 << (-s->cfs.Columns & 7);
	uint count;
	/* Do complete scan lines. */
	while ( (count = limit - p + 1) >= raster )
	{	/* Ensure that the scan line ends with a polarity change. */
		/* This may involve saving and restoring one byte beyond */
		/* the scan line. */
		byte *next = p + raster;
		byte save_next = *next;
		int code;
		if ( end_mask == 1 )		/* set following byte */
			*next = (next[-1] & 1) - 1;
		else if ( next[-1] & end_mask )	/* clear lower bits */
			next[-1] &= -end_mask;
		else				/* set lower bits */
			next[-1] |= end_mask - 1;
		if ( s->cfs.EncodedByteAlign )
			s->cfs.bits_left &= ~7;
		if ( s->cfs.K > 0 )
		{	/* Group 3, mixed encoding */
			if ( --(s->cfs.k_left) )
			{	/* Use 2-D encoding */
				if ( s->cfs.EndOfLine )
					cf_put_code(s, &cf2_run_eol_2d);
				code = cf_encode_2d(s, p, prev);
			}
			else
			{	/* Use 1-D encoding */
				if ( s->cfs.EndOfLine )
					cf_put_code(s, &cf2_run_eol_1d);
				code = cf_encode_1d(s, p);
				s->cfs.k_left = s->cfs.K;
			}
		}
		else
		{	/* Uniform encoding */
			if ( s->cfs.EndOfLine )
				cf_put_code(s, &cf_run_eol);
			code = (s->cfs.K == 0 ? cf_encode_1d(s, p) :
				cf_encode_2d(s, p, prev));
		}
		*next = save_next;
		if ( code )
		{	s->end_status = code;
			break;
		}
		prev = p;
		p = next;
	}
	/* Move any remaining partial scan line (and, if 2-D encoding */
	/* is a possibility, the last full scan line) to the beginning */
	/* of the buffer. */
	if ( s->cfs.K )
		count += raster, p -= raster;
	memcpy(s->cbuf, p, count);
	s->cptr = s->cbuf - 1 + count;
	return 0;
}

/* Encode a 1-D scan line. */
private int
cf_encode_1d(stream *s, register byte *p)
{	byte invert = (s->cfs.BlackIs1 ? 0 : 0xff);
	register uint count;
	uint end_count = s->cfs.Columns & 7;
	register uint data = *p++ ^ invert;
	for ( count = s->cfs.raster << 3; count != end_count; )
	{	int white, black;
		/* Parse a white run. */
		white = count;
		skip_white_pixels(data, p, count, invert, w0);
		white -= count;
		cf_put_white_run(s, white);
		if ( count == end_count ) break;
		/* Parse a black run. */
		black = count;
		skip_black_pixels(data, p, count, invert, b0, b1);
		black -= count;
		cf_put_black_run(s, black);
	}
	return 0;
}

/* Encode a 2-D scan line. */
private int
cf_encode_2d(stream *s, byte *p, byte *prev_p)
{	byte invert = (s->cfs.BlackIs1 ? 0 : 0xff);
	byte invert_white = invert;
	register uint count;
	uint end_count = s->cfs.Columns & 7;
	register uint data = *p++ ^ invert;
	int dist = prev_p - p;
	for ( count = s->cfs.raster << 3; count != end_count; )
	{	/* If invert == invert_white, white and black have their */
		/* correct meanings; if invert == ~invert_white, */
		/* black and white are interchanged. */
		uint a0, a1, b1;
		uint prev_count = count;
		byte prev_data;
		int diff;
		static const byte count_bit[8] =
			{ 0x80, 1, 2, 4, 8, 0x10, 0x20, 0x40 };
		prev_p = p + dist;
		prev_data = prev_p[-1] ^ invert;
		/* Find the a1 and b1 transitions. */
		a0 = count;
		skip_white_pixels(data, p, count, invert, w00);
		a1 = count;
		if ( (prev_data & count_bit[prev_count & 7]) )
		{	/* Look for changing white first. */
			skip_black_pixels(prev_data, prev_p, prev_count, invert, b01, b11);
		}
		if ( prev_count != end_count )
		{	skip_white_pixels(prev_data, prev_p, prev_count, invert, w01);
		}
		b1 = prev_count;
		/* In all the comparisons below, remember that count */
		/* runs downward, not upward, so the comparisons are */
		/* reversed. */
		if ( b1 >= a1 + 2 )
		{	/* Could be a pass mode.  Find b2. */
			if ( prev_count != end_count )
			{	skip_black_pixels(prev_data, prev_p,
						 prev_count, invert, b02, b12);
			}
			if ( prev_count > a1 )
			{	/* Use pass mode. */
				cf_put_code(s, &cf2_run_pass);
				count = prev_count;
				p = prev_p - dist;
				data = p[-1] ^ invert;
				continue;
			}
		}
		/* Check for vertical coding. */
		diff = a1 - b1;		/* i.e., logical b1 - a1 */
		if ( diff <= 3 && diff >= -3 )
		{	/* Use vertical coding. */
			cf_put_code(s, &cf2_run_vertical[diff + 3]);
			invert = ~invert;	/* a1 polarity changes */
			continue;
		}
		/* No luck, use horizontal coding. */
		cf_put_code(s, &cf2_run_horizontal);
		if ( count != end_count )
		{	skip_black_pixels(data, p, count, invert, b03, b13);	/* find a2 */
		}
		a0 -= a1;
		a1 -= count;
		if ( invert == invert_white )
		{	cf_put_white_run(s, a0);
			cf_put_black_run(s, a1);
		}
		else
		{	cf_put_black_run(s, a0);
			cf_put_white_run(s, a1);
		}
	}
	return 0;
}

/* Close the stream */
private int
s_CFE_close(register stream *s)
{	int code = s_CFE_write_buf(s);
	if ( code == ERRC ) return code;
	if ( s->cfs.EndOfBlock )
	{	int i = (s->cfs.K < 0 ? 2 : 6);
		const cfe_run _ds *rp =
			(s->cfs.K > 0 ? &cf2_run_eol_1d : &cf_run_eol);
		while ( --i >= 0 )
			cf_put_code(s, rp);
	}
	/* Force out the last byte or bytes. */
	while ( s->cfs.bits_left < bits_size )
	{	stream *strm = s->strm;
		sputc(strm, s->cfs.bits >> (bits_size - 8));
		s->cfs.bits <<= 8;
		s->cfs.bits_left += 8;
	}
	return s_std_close(s);
}

/* Stream procedures */
const stream_procs s_CFE_procs =
   {	s_std_noavailable, NULL, s_filter_write_flush, s_CFE_close,
	NULL, s_CFE_write_buf
   };
