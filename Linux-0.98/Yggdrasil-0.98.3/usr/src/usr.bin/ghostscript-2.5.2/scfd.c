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

/* scfd.c */
/* CCITTFax decoding filter */
#include "stdio_.h"	/* includes std.h */
#include "memory_.h"
#include "gdebug.h"
#include "scf.h"
#include "stream.h"

/* Initialize CCITTFaxDecode filter */
void
s_CFD_init(register stream *s, CCITTFax_state *pcfs)
{	int raster = pcfs->raster;	/* s->cfs isn't set yet! */
	s->cfs = *pcfs;
	if ( s->cfs.K != 0 )
	{	/* Clear the initial reference line for 2-D encoding. */
		s->cptr = s->endptr = s->cbuf + raster * 3 - 1;
		s->cfs.prev_pos = raster * 2;
		memset(s->cbuf + raster * 2, (s->cfs.BlackIs1 ? 0 : 0xff),
		       raster);
	}
	s->cfs.k_left = min(s->cfs.K, 0);
	s->cfs.cbit = 0;
	s->cfs.bits = 0;
	s->cfs.bits_left = 0;
	s->cfs.uncomp_run = 0;
}

/* Declare the variables that hold the state of the input. */
#define cfd_declare_stream_state\
	stream *strm = s->strm;\
	register byte *inptr;\
	byte *inend
#define cfd_declare_state\
	cfd_declare_stream_state;\
	register byte *p;\
	int pbit;\
	uint bits;\
	int bits_left
/* Load the input state from the stream. */
#define cfd_load_stream_state()\
	inptr = sbufptr(strm), inend = sbufend(strm)
#define cfd_load_state()\
	cfd_load_stream_state(),\
	p = s->cptr, pbit = s->cfs.cbit,\
	bits = s->cfs.bits, bits_left = s->cfs.bits_left
/* Store the input state back in the stream. */
#define cfd_store_stream_state()\
	ssetbufptr(strm, inptr)
#define cfd_store_state()\
	cfd_store_stream_state(),\
	s->cptr = p, s->cfs.cbit = pbit,\
	s->cfs.bits = bits, s->cfs.bits_left = bits_left

/* Macros to get blocks of bits from the input stream. */
/* Invariants: 0 <= bits_left <= bits_size; */
/* bits [bits_left-1..0] contain valid data. */
#define bits_size (arch_sizeof_int * 8) /* avoid sizeof so is constant */
/* n must not be greater than 8. */
#define ensure_bits(n)\
  if ( bits_left < n ) more_bits()
#define more_bits_1()\
  { int c;\
    if ( inptr < inend ) c = *inptr++;\
    else { cfd_store_stream_state(); c = sgetc(strm); cfd_load_stream_state(); }\
    if ( c < 0 ) { s->end_status = c; goto out; }\
    bits = (bits << 8) + c, bits_left += 8;\
  }
#if bits_size == 16
#  define more_bits() more_bits_1()
#else				/* bits_size >= 32 */
#  define more_bits()\
  { if ( inend - inptr < 3 ) more_bits_1()\
    else\
    { bits = (bits << 24) + ((uint)inptr[0] << 16) + ((uint)inptr[1] << 8) + inptr[2];\
      bits_left += 24, inptr += 3;\
    }\
  }
#endif
#define peek_bits(n) ((bits >> (bits_left - (n))) & ((1 << (n)) - 1))
#define peek_var_bits(n) ((bits >> (bits_left - (n))) & peek_masks[n])
static const byte peek_masks[9] = { 0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };
#define skip_bits(n) bits_left -= (n)

/* Get a run from the stream. */
#define get_run(decode, initial_bits, runlen, str)\
{	const cfd_node _ds *np;\
	int clen;\
	ensure_bits(initial_bits);\
	np = &decode[peek_bits(initial_bits)];\
	if ( (clen = np->code_length) > initial_bits )\
	{	do_debug(uint init_bits = peek_bits(initial_bits));\
		clen -= initial_bits;\
		skip_bits(initial_bits);\
		ensure_bits(clen);\
		np = &decode[np->run_length + peek_var_bits(clen)];\
		if_debug4('W', "%s clen=%d xcode=0x%x rlen=%d\n", str,\
			  initial_bits + np->code_length,\
			  (init_bits << np->code_length) +\
				  peek_var_bits(np->code_length),\
			  np->run_length);\
		skip_bits(np->code_length);\
	}\
	else\
	{	if_debug4('W', "%s clen=%d code=0x%x rlen=%d\n", str,\
			  clen, peek_var_bits(clen), np->run_length);\
		skip_bits(clen);\
	}\
	runlen = np->run_length;\
}

/* Skip data bits for a white run. */
/* rlen is either less than 64, or a multiple of 64. */
#define skip_data(rlen, makeup_label)\
	if ( (pbit -= rlen) < 0 )\
	{	p -= pbit >> 3, pbit &= 7;\
		if ( rlen >= 64 ) goto makeup_label;\
	}

/* Invert data bits for a black run. */
/* If rlen >= 64, execute makeup_action: this is to handle */
/* makeup codes efficiently, since these are always a multiple of 64. */
#define invert_data(rlen, black_byte, makeup_action, d)\
	if ( rlen > pbit )\
	{	*p++ ^= (1 << pbit) - 1;\
		rlen -= pbit;\
		switch ( rlen >> 3 )\
		{\
		default:	/* original rlen >= 64 */\
d:			memset(p, black_byte, rlen >> 3);\
			p += rlen >> 3;\
			rlen &= 7;\
			if ( !rlen ) pbit = 0, p--;\
			else pbit = 8 - rlen, *p ^= 0xff << pbit;\
			makeup_action;\
			break;\
		case 7:		/* original rlen possibly >= 64 */\
			if ( rlen + pbit >= 64 ) goto d;\
			*p++ = black_byte;\
		case 6: *p++ = black_byte;\
		case 5: *p++ = black_byte;\
		case 4: *p++ = black_byte;\
		case 3: *p++ = black_byte;\
		case 2: *p++ = black_byte;\
		case 1: *p = black_byte;\
			rlen &= 7;\
			if ( !rlen ) { pbit = 0; break; }\
			p++;\
		case 0:			/* know rlen != 0 */\
			pbit = 8 - rlen;\
			*p ^= 0xff << pbit;\
		}\
	}\
	else\
		pbit -= rlen,\
		*p ^= ((1 << rlen) - 1) << pbit

/* Buffer refill for CCITTFaxDecode filter */
private int cf_decode_1d(P1(stream *));
private int cf_decode_2d(P2(stream *, byte *));
private int cf_decode_uncompressed(P1(stream *));
private int
s_CFD_read_buf(register stream *s)
{	cfd_declare_stream_state;
	int raster = s->cfs.raster;
	register byte *p = s->cbuf - 1;
	byte *prev = p + s->cfs.prev_pos;
	byte *limit = p + s->bsize;
	int k_left = s->cfs.k_left;
	cfd_load_stream_state();
	/* Do complete scan lines. */
	while ( limit - p >= raster )
	{	uint bits = s->cfs.bits;
		int bits_left = s->cfs.bits_left;
		int code;
		if ( s->cfs.EncodedByteAlign )
			bits_left &= ~7;
		/* Check for an initial EOL code. */
		/* In 16-bit environments, we can't look at */
		/* the entire EOL code simultaneously; */
		/* having two separate algorithms isn't worth the trouble. */
		ensure_bits(8);
		/* Read an extra byte, and put it back if needed. */
		if ( peek_bits(8) != 0 ) goto ne;
		skip_bits(8);
		more_bits_1();
		if ( peek_bits(run_eol_code_length-9) != 0 )
		{	/* Undo the more_bits_1() */
			bits >>= 8;
			bits_left -= 8;
			inptr--;	/*sputback(strm)*/
			goto ne;
		}
		/* Skip over padding and EOL. */
		while ( 1 )
		{	ensure_bits(1);
			if ( peek_bits(1) ) break;
			skip_bits(1);
		}
		skip_bits(1);
		/* If we are in a Group 3 mixed regime, */
		/* check the next bit for 1- vs. 2-D. */
		if ( s->cfs.K > 0 )
		{	ensure_bits(1);
			k_left = (peek_bits(1) ? 0 : 1);
			skip_bits(1);
		}
		goto rd;
ne:		if ( s->cfs.EndOfLine )
		{	/* EOL is required, but none is present. */
			s->end_status = ERRC;
			break;
		}
rd:		s->cptr = p;
		s->cfs.cbit = 0;		/* for now */
		s->cfs.bits = bits;
		s->cfs.bits_left = bits_left;
		cfd_store_stream_state();
		if ( k_left < 0 )
		{	if_debug0('w', "[w2]new row\n");
			code = cf_decode_2d(s, prev);
		}
		else if ( k_left == 0 )
		{	if_debug0('w', "[w1]new row\n");
			code = cf_decode_1d(s);
			k_left = s->cfs.K;
		}
		else
		{	if_debug1('w', "[w1]new 2-D row, %d left\n", k_left);
			code = cf_decode_2d(s, prev);
			k_left--;
		}
		cfd_load_stream_state();
		if ( code < 0 )
		{	s->end_status = ERRC;
			break;
		}
		prev = p;
		p += raster;
	}
out:	cfd_store_stream_state();
	s->cptr = s->cbuf - 1;
	s->endptr = p;
	s->cfs.k_left = k_left;
	s->cfs.prev_pos = prev - s->cptr;
	return 0;
}
/* Decode a 1-D scan line. */
private int
cf_decode_1d(stream *s)
{	cfd_declare_state;
	byte black_byte = (s->cfs.BlackIs1 ? 0xff : 0);
	int end_bit = -s->cfs.Columns & 7;
	int raster = s->cfs.raster;
	byte *stop;
	cfd_load_state();
	stop = p + raster;
#define p_at_stop() (p >= stop && (pbit <= end_bit || p > stop))
	memset(p + 1, ~black_byte, raster);
	while ( !p_at_stop() )
	{	int bcnt;
dw:		/* Decode a white run. */
		get_run(cf_white_decode, cfd_white_initial_bits, bcnt, "[w1]white");
		if ( bcnt < 0 )
				/* exceptional situation */
		  switch ( bcnt )
		{
		case run_uncompressed:	/* Uncompressed data. */
			cfd_store_state();
			bcnt = cf_decode_uncompressed(s);
			if ( bcnt < 0 )
			{	s->end_status = bcnt;
				goto out;
			}
			cfd_load_state();
			if ( bcnt ) goto db;
			else goto dw;
		/*case run_error:*/
		/*case run_zeros:*/	/* Premature end-of-line. */
		default:
			s->end_status = ERRC;
			goto out;
		}
		skip_data(bcnt, dw);
		if ( p_at_stop() ) break;
db:		/* Decode a black run. */
		get_run(cf_black_decode, cfd_black_initial_bits, bcnt, "[w1]black");
		if ( bcnt < 0 )
		{	/* All exceptional codes are invalid here. */
			/****** WRONG, uncompressed IS ALLOWED ******/
			s->end_status = ERRC;
			goto out;
		}
		/* Invert bits designated by black run. */
		invert_data(bcnt, black_byte, goto db, idb);
	}
	if ( p > stop || pbit < end_bit )
	{	s->end_status = ERRC;
		goto out;
	}
out:	cfd_store_state();
	return s->end_status;
}
/* Decode a 2-D scan line. */
private int
cf_decode_2d(stream *s, byte *prev_p)
{	cfd_declare_state;
	int dist;
	byte invert = (s->cfs.BlackIs1 ? 0 : 0xff);
	byte black_byte = ~invert;
	byte invert_white = invert;
	register int count;
	int end_count = -s->cfs.Columns & 7;
	uint raster = s->cfs.raster;
	byte *endptr;
	int init_count = raster << 3;
	int rlen;
	cfd_load_state();
	endptr = p + raster;
	memset(p + 1, invert, raster);
	endptr[1] = 0xa0;	/* a byte with some 0s and some 1s, */
				/* to ensure run scan will stop */
	dist = prev_p - p;
	for ( count = init_count; count > end_count; )
	{	/* If invert == invert_white, white and black have their */
		/* correct meanings; if invert == ~invert_white, */
		/* black and white are interchanged. */
		if_debug1('W', "[w2]%4d:\n", count);
#ifdef DEBUG
		/* Check the invariant between p, pbit, and count. */
		{	int pcount = (endptr - p) * 8 + pbit;
			if ( pcount != count )
				dprintf2("[w2]Error: count=%d pcount=%d\n",
					 count, pcount);
		}
#endif
		/* We could just use get_run here, but we can do better: */
		ensure_bits(3);
		switch( peek_bits(3) )
		{
		default/*4..7*/:			/* vertical(0) */
			skip_bits(1);
			rlen = countof(cf2_run_vertical) / 2;
			break;
		case 2:					/* vertical(+1) */
			skip_bits(3);
			rlen = countof(cf2_run_vertical) / 2 + 1;
			break;
		case 3:					/* vertical(-1) */
			skip_bits(3);
			rlen = countof(cf2_run_vertical) / 2 - 1;
			break;
		case 1:					/* horizontal */
			skip_bits(3);
			if ( invert == invert_white )
			{	/* White, then black. */
				int olen;
hww:				get_run(cf_white_decode,
					cfd_white_initial_bits, rlen,
					" white");
				if ( (count -= rlen) < end_count )
				{	s->end_status = ERRC;
					goto out;
				}
				skip_data(rlen, hww);
hwb:				get_run(cf_black_decode,
					cfd_black_initial_bits, olen,
					" black");
				if ( (count -= olen) < end_count )
				{	s->end_status = ERRC;
					goto out;
				}
				invert_data(olen, black_byte, goto hwb, ihwb);
			}
			else
			{	/* Black, then white. */
				int olen;
hbb:				get_run(cf_black_decode,
					cfd_black_initial_bits, rlen,
					" black");
				if ( (count -= rlen) < end_count )
				{	s->end_status = ERRC;
					goto out;
				}
				invert_data(rlen, black_byte, goto hbb, ihbb);
hbw:				get_run(cf_white_decode,
					cfd_white_initial_bits, olen,
					" white");
				if ( (count -= olen) < end_count )
				{	s->end_status = ERRC;
					goto out;
				}
				skip_data(olen, hbw);
			}
			continue;
		case 0:				/* everything else */
			get_run(cf_2d_decode, cfd_2d_initial_bits, rlen,
				"[w2]");
			/* rlen may be run2_pass, run_uncompressed, or */
			/* 0..countof(cf2_run_vertical)-1. */
			if ( rlen < 0 )
			  switch ( rlen )
			{
			case run2_pass:
				break;
			case run_uncompressed:
			{	int which;
				cfd_store_state();
				which = cf_decode_uncompressed(s);
				if ( which < 0 )
				{	s->end_status = which;
					goto out;
				}
				cfd_load_state();
				/****** ADJUST count ******/
				invert = (which ? ~invert_white : invert_white);
			}	continue;
			default:	/* run_error, run_zeros */
				s->end_status = ERRC;
				goto out;
			}
		}
		/* Interpreting the run requires scanning the */
		/* previous ('reference') line. */
		{	int prev_count = count;
			byte prev_data;
			int dlen;
			static const byte count_bit[8] =
				{ 0x80, 1, 2, 4, 8, 0x10, 0x20, 0x40 };
			prev_p = p + dist + 1;
			if ( !(count & 7) )
				prev_p++;	/* because of skip macros */
			prev_data = prev_p[-1] ^ invert;
			/* Find the b1 transition. */
			if ( (prev_data & count_bit[prev_count & 7]) &&
			     prev_count < init_count
			   )
			{	/* Look for changing white first. */
				skip_black_pixels(prev_data, prev_p,
						  prev_count, invert,
						  b01, b11);
				if ( prev_count < end_count ) /* overshot */
					prev_count = end_count;
				if_debug1('W', " b1 other=%d", prev_count);
			}
			if ( prev_count != end_count )
			{	skip_white_pixels(prev_data, prev_p,
						  prev_count, invert, w01);
				if ( prev_count < end_count ) /* overshot */
					prev_count = end_count;
				if_debug1('W', " b1 same=%d", prev_count);
			}
			/* b1 = prev_count; */
			if ( rlen == run2_pass )
			{	/* Pass mode.  Find b2. */
				if ( prev_count != end_count )
				{	skip_black_pixels(prev_data, prev_p,
							  prev_count, invert,
							  b02, b12);
					if ( prev_count < end_count ) /* overshot */
						prev_count = end_count;
				}
				/* b2 = prev_count; */
				if_debug2('W', " b2=%d, pass %d\n",
					  prev_count, count - prev_count);
			}
			else
			{	/* Vertical coding. */
				/* Remember that count counts *down*. */
				prev_count += rlen - 3;	/* a1 */
				if_debug2('W', " vertical %d -> %d\n",
					  3 - rlen, prev_count);
			}
			/* Now either invert or skip from count */
			/* to prev_count, and reset count. */
			if ( !invert )
			{	/* Skip data bits. */
				p = endptr - (prev_count >> 3);
				pbit = prev_count & 7;
			}
			else
			{	/* Invert data bits. */
				dlen = count - prev_count;
				invert_data(dlen, black_byte, 0, idd);
			}
			count = prev_count;
			if ( rlen >= 0 )	/* vertical mode */
				invert = ~invert;	/* polarity changes */
		}
	}
	if ( count < end_count )	/* overrun! */
		s->end_status = ERRC;
out:	cfd_store_state();
	return s->end_status;
}
/* Decode uncompressed data. */
/* (Not tested: no sample data available!) */
/****** DOESN'T CHECK FOR OVERFLOWING SCAN LINE ******/
private int
cf_decode_uncompressed(stream *s)
{	cfd_declare_state;
	const cfd_node _ds *np;
	int clen, rlen;
	cfd_load_state();
	while ( 1 )
	{	ensure_bits(cfd_uncompressed_initial_bits);
		np = &cf_uncompressed_decode[peek_bits(cfd_uncompressed_initial_bits)];
		clen = np->code_length;
		rlen = np->run_length;
		if ( clen > cfd_uncompressed_initial_bits )
		{	/* Must be an exit code. */
			break;
		}
		if ( rlen == cfd_uncompressed_initial_bits )
		{	/* Longest representable white run */
			if_debug1('W', "[wu]%d\n", rlen);
			if ( (pbit -= cfd_uncompressed_initial_bits) < 0 )
				pbit += 8, p++;
		}
		else
		{	if_debug1('W', "[wu]%d+1\n", rlen);
			if ( pbit -= rlen < 0 )
				pbit += 8, p++;
			*p ^= 1 << pbit;
		}
		skip_bits(clen);
	}
	clen -= cfd_uncompressed_initial_bits;
	skip_bits(cfd_uncompressed_initial_bits);
	ensure_bits(clen);
	np = &cf_uncompressed_decode[rlen + peek_var_bits(clen)];
	rlen = np->run_length;
	skip_bits(np->code_length);
	if_debug1('w', "[wu]exit %d\n", rlen);
	if ( rlen >= 0 )
	{	/* Valid exit code, rlen = 2 * run length + next polarity */
		if ( (pbit -= rlen >> 1) < 0 )
			pbit += 8, p++;
		rlen &= 1;
	}
out:		/******* WRONG ******/
	cfd_store_state();
	return rlen;
}

/* Stream procedures */
const stream_procs s_CFD_procs =
   {	s_std_noavailable, NULL, s_std_read_flush, s_std_close,
	s_CFD_read_buf, NULL
   };
