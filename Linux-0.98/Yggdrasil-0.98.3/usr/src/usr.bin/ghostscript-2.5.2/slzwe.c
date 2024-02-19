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

/* slzwe.c */
/* LZW encoding filter */
#include "stdio_.h"	/* includes std.h */
#include "gdebug.h"
#include "stream.h"

/* Imported procedures */
extern int s_filter_write_flush(P1(stream *));

/********************************************************/
/* LZW routines are based on:				*/
/* Dr. Dobbs Journal --- Oct. 1989. 			*/
/* Article on LZW Data Compression by Mark R. Nelson 	*/
/********************************************************/

/*
 * This code implements enhancements to the LZW algorithm.
 *
 * At any moment during the encoding process, let S be the width of the
 * output code in bits.  Let N = 1 << S.  Let M be the next code to be
 * assigned; we know that N / 2 <= M < N.  The only possible codes that
 * can appear in the output are 0 .. M-1.  Therefore, we can encode some
 * of these with only S-1 bits.  Specifically, let D = N - M.  Then to
 * output the code C (0 <= C < M):
 *	If C < D, output C in S-1 bits.
 *	if D <= C < N / 2, output C * 2 in S bits.
 *	Otherwise (N / 2 <= C < M), output (C + N / 2 - M) * 2 + 1 in S bits.
 */

/* Define the special codes */
#define code_reset 256
#define code_eod 257
#define code_0 258			/* first assignable code */

/* ------ LZWEncode filter ------ */

typedef struct lzw_encode_s {
	byte datum;			/* last byte of this code */
	unsigned mark : 4;		/* (only need 1 bit) */
	unsigned prefix : 12;		/* code for prefix of this code */
} lzw_encode;

#define encode_max 3000		/* max # of codes, must be */
					/* > code_0 and <= 4095 */
#define hash_size (encode_max  + encode_max / 4)

typedef struct lzw_encode_table_s {
	lzw_encode encode[encode_max];
	ushort hashed[hash_size];
} lzw_encode_table;

/* Hashing function */
#define encode_hash(code, chr)\
  ((uint)((code) * 59 + (chr) * ((hash_size / 256) | 1)) % hash_size)

/* Export the table size */
const uint s_LZWE_table_sizeof = sizeof(lzw_encode_table);

/* Internal routine to put a code onto the target stream. */
/* Let S = s->lzws.code_size, M = s->lzws.next_code, N = 1 << M. */
/* Relevant invariants: 9 <= S <= 12; N / 2 <= M < N; 0 <= code < M; */
/* 1 <= s->lzws.bits_left <= 8; only the rightmost (8 - s->lzws.bits_left) */
/* bits of s->lzws.bits contain valid data. */
private void
lzw_put_code(register stream *s, uint code)
{	byte cb;
	uint rep = code, size = s->lzws.code_size;
	stream *strm = s->strm;
	if ( s->lzws.enhanced )
	   {	lzw_encode *encode = s->lzws.encode_table->encode;
		uint mcode = code;
		lzw_encode *ep;
		uint N = 1 << size, M = s->lzws.next_code;
		/* Mark this code and all its prefixes. */
		while ( !(ep = encode + mcode)->mark )
		   {	ep->mark = 1;
			mcode = ep->prefix;
		   }
		/* See if we can represent the code in S-1 bits. */
		if ( code < N - M )
			--size;		/* yes */
		else if ( code < N / 2 )
			rep <<= 1;	/* no, trailing 0 bit */
		else			/* no, trailing 1 bit */
			rep = (code + N / 2 - M) * 2 + 1;
	   }
	cb = (s->lzws.bits << s->lzws.bits_left) +
		(rep >> (size - s->lzws.bits_left));
#ifdef DEBUG
if ( gs_debug['w'] )
   {	dprintf2("[w]writing 0x%x,%d", code, s->lzws.code_size);
	if ( s->lzws.enhanced ) dprintf2(" -> 0x%x,%d", rep, size);
	dputc('\n');
   }
#endif
	sputc(strm, cb);
	if ( (s->lzws.bits_left += 8 - size) <= 0 )
	   {	const byte cb1 = rep >> -s->lzws.bits_left;
		sputc(strm, cb1);
		s->lzws.bits_left += 8;
	   }
	s->lzws.bits = rep;
}

/* Internal routine to reset the encoding table */
private void
lzw_reset_encode(stream *s)
{	register int c;
	lzw_encode_table *table = s->lzws.encode_table;
	lzw_put_code(s, code_reset);
	s->lzws.next_code = code_0;
	s->lzws.code_size = 9;
	s->lzws.prev_code = code_eod;
	for ( c = 0; c < hash_size; c++ )
		table->hashed[c] = code_eod;
	for ( c = 0; c < 256; c++ )
	   {	lzw_encode *ec = &table->encode[c];
		register ushort *tc = &table->hashed[encode_hash(code_eod, c)];
		while ( *tc != code_eod )
		  if ( ++tc == &table->hashed[hash_size] )
		    tc = &table->hashed[0];
		*tc = c;
		ec->datum = c, ec->prefix = code_eod, ec->mark = 1;
	   }
	table->encode[code_eod].prefix = code_reset;	/* guarantee no match */
	table->encode[code_eod].mark = 1;
	table->encode[code_reset].mark = 1;
}

/* Initialize LZWEncode filter */
void
s_LZWE_init(register stream *s, lzw_encode_table *table, int enhanced)
{	s->lzws.bits_left = 8;
	s->lzws.encode_table = table;
	s->lzws.code_size = 9;		/* needed for reset code */
	s->lzws.next_code = code_0;		/* ditto */
	s->lzws.enhanced = enhanced;
	lzw_reset_encode(s);
}

/* Flush the buffer */
private int
s_LZWE_write_buf(register stream *s)
{	register byte *p = s->cbuf;
	byte *limit = s->cptr;
	int code = s->lzws.prev_code;
	lzw_encode_table *table = s->lzws.encode_table;
	ushort *table_end = &table->hashed[hash_size];
	int limit_code;
#define set_limit_code()\
  limit_code = (1 << s->lzws.code_size) - 1;\
  if ( limit_code > encode_max ) limit_code = encode_max
	set_limit_code();
	while ( p <= limit )
	   {	byte c = *p;
		ushort *tp;
		for ( tp = &table->hashed[encode_hash(code, c)]; ; )
		   {	lzw_encode *ep = &table->encode[*tp];
			if ( ep->prefix == code && ep->datum == c )
			   {	code = *tp;
				p++;
				break;
			   }
			else if ( *tp != code_eod )
			   {	if ( ++tp == table_end )
				  tp = &table->hashed[0]; /* wrap around */
			   }
			else
			   {	/* end of recognized sequence */
				lzw_put_code(s, code);
				if ( s->lzws.next_code == limit_code )
				   {	/* Reached power of 2 or limit. */
					/* Determine which. */
					if ( s->lzws.next_code == encode_max )
					   {	lzw_reset_encode(s);
						set_limit_code();
						goto cx;
					   }
					s->lzws.code_size++;
					set_limit_code();
				   }
#ifdef DEBUG
if ( gs_debug['w'] )
				dprintf3("[w]encoding 0x%x=0x%x+%c\n",
				         s->lzws.next_code, code, c);
#endif
				*tp = s->lzws.next_code++;
				ep = &table->encode[*tp];
				ep->datum = c;
				ep->prefix = code;
				ep->mark = 0;
cx:				code = code_eod;
				break;
			   }
		   }
	   }
	s->lzws.prev_code = code;
	s->cptr = s->cbuf - 1;
	return 0;
}

/* Close the stream */
private int
s_LZWE_close(register stream *s)
{	(*s->procs.write_buf)(s);
	if ( s->lzws.prev_code != code_eod )
	   {	lzw_put_code(s, s->lzws.prev_code);	/* put out final code */
		/* Adjust next_code for the enhanced case. */
		s->lzws.next_code++;
	   }
	lzw_put_code(s, code_eod);
	if ( s->lzws.bits_left < 8 )
	  sputc(s->strm, s->lzws.bits << s->lzws.bits_left);	/* final byte */
	return s_std_close(s);
}

/* Stream procedures */
const stream_procs s_LZWE_procs =
   {	s_std_noavailable, NULL, s_filter_write_flush, s_LZWE_close,
	NULL, s_LZWE_write_buf
   };
