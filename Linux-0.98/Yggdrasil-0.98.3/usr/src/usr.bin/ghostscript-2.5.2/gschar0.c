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

/* gschar0.c */
/* Composite font decoding for Ghostscript library */
/****** DOESN'T WORK. DON'T TRY TO USE IT. ******/
#include "gx.h"
#include "gserrors.h"
#include "gxfixed.h"
#include "gxmatrix.h"			/* for gzstate.h */
#include "gzstate.h"			/* must precede gzdevice */
#include "gzdevice.h"			/* must precede gxchar */
#include "gxdevmem.h"
#include "gxchar.h"
#include "gxfont.h"

/* Select the appropriate descendant of a font. */
/* Free variables: fdepth, index, p. */
#define select_descendant(pfont, pdata, n)\
  if ( (n) >= pdata->encoding_size )\
    return_error(gs_error_rangecheck);\
  if ( fdepth >= max_font_depth - 1 )\
    return_error(gs_error_invalidfont);\
  pfont = pdata->FDepVector[pdata->Encoding[n]];\
  penum->fstack[penum->fdepth = ++fdepth] = pfont;\
  penum->index = index = p - penum->str

/* Define a structure for a (character code, font index) pair. */
typedef struct {
	uint fidx;
	char_code chr;
} mapped_char;

/* Forward declarations */
private int map_SubsVector(P4(const byte *, int, gs_type0_data *, mapped_char *));

/* Get the next character (to penum->chr) and font (to penum->pfont)
/* from a composite string. */
/* If we run off the end of the string in the middle of a */
/* multi-byte sequence, return gs_error_rangecheck. */
/* If the string is empty, return 1.  Otherwise, return 0. */
int
gs_type0_next(gs_show_enum *penum)
{	uint index = penum->index;
	const byte *p = penum->str + index;
	uint size = penum->size;
	int fdepth = 0;
	gs_font *pfont = penum->fstack[0];
	gs_type0_data *pdata;
	mapped_char mc;
	int code;
	if ( size == index ) return 1;
#define need_left(n)\
  if ( size - index < n ) return_error(gs_error_rangecheck)
top:	pdata = &pfont->data.type0_data;
top1:	switch ( pdata->FMapType )
	   {
	default:			/* can't happen */
		return_error(gs_error_invalidfont);

		/* ------ Non-modal mappings ------ */

	case fmap_8_8:
		need_left(2);
		mc.fidx = *p++;
		mc.chr = *p++;
		break;

	case fmap_1_7:
		mc.fidx = *p >> 7;
		mc.chr = *p++ & 0x7f;
		break;

	case fmap_9_7:
		need_left(2);
		mc.fidx = ((uint)*p << 1) + (p[1] >> 7);
		mc.chr = p[1] & 0x7f;
		p += 2;
		break;

	case fmap_SubsVector:
	{	int width = pdata->subs_width;
		need_left(width);
		code = map_SubsVector(p, width, pdata, &mc);
		if ( code < 0 ) return code;
		break;
	}

		/* ------ Modal mappings ------ */

	case fmap_escape:
		if ( *p != pdata->EscChar ) goto same_mode;
		need_left(2);
		mc.fidx = p[1]; p += 2; goto cmode;

	case fmap_double_escape:
		if ( *p != pdata->EscChar ) goto same_mode;
		need_left(2);
		if ( p[1] == *p )
		   {	need_left(3);
			mc.fidx = p[2] + 256; p += 3; goto cmode;
		   }
		mc.fidx = p[1]; p += 2; goto cmode;
		/* Enter a mode. */
cmode:		penum->fdepth = 1;
		break;

	case fmap_shift:
		if ( *p == pdata->ShiftIn ) mc.fidx = 0;
		else if ( *p == pdata->ShiftOut ) mc.fidx = 1;
		else goto same_mode;
		select_descendant(pfont, pdata, mc.fidx);
		goto top;

		/* Stay in the same mode. */
same_mode:	for ( ; ; )
		{	pfont = penum->fstack[++fdepth];
			if ( pfont->FontType != ft_composite) break;
			pdata = &pfont->data.type0_data;
			if ( pdata->FMapType != fmap_escape )
				goto top1;	/* must be non-modal */
		}
/******		penum->chr = *p++;	******/
		penum->index = index + 1;
		return 0;

	   }

	/* Control continues here for non-modal mappings. */
remap:	select_descendant(pfont, pdata, mc.fidx);
	/****** WHERE DOES THE FOLLOWING COMMENT BELONG?? ******/
	/* We pre-checked the encoding vector, so we know that */
	/* fidx is now a legal subscript for FDepVector. */
	if ( pfont->FontType == ft_composite )	/* must be non-modal */
	{	pdata = &pfont->data.type0_data;
		switch ( pdata->FMapType )
		{

		case fmap_8_8:
			need_left(1);
			mc.fidx = mc.chr;
			mc.chr = *p++;
			break;

		case fmap_1_7:
		case fmap_9_7:
			need_left(1);
			mc.fidx = mc.chr;
			mc.chr = *p++ & 0x7f;
			break;

		case fmap_SubsVector:
		{	int width = pdata->subs_width;
			/* It isn't clear what to use for the input */
			/* to the mapping algorithm.... */
			byte c[4];
			int i = width;
			while ( --i >= 0 )
				c[i] = (byte)mc.chr, mc.chr >>= 8;
			code = map_SubsVector(c, width, pdata, &mc);
			if ( code < 0 ) return code;
			break;
		}

		default:		/* can't happen */
			return_error(gs_error_invalidfont);
		}
		goto remap;
	}
	penum->pfont = pfont;
/******	penum->chr = mc.chr;	******/
	return 0;
}

/* ------ Internal routines ------ */

/* Get the font index for a character from a SubsVector. */
private int
map_SubsVector(const byte *p, int width, gs_type0_data *pdata, mapped_char *pmc)
{	uint subs_count = pdata->subs_size;
	const byte *psv = pdata->SubsVector;
#define subs_loop(subs_elt, width)\
  while ( subs_count != 0 && chr >= (schr = subs_elt) )\
    subs_count--, chr -= schr, psv += width;\
  pmc->chr = chr; break
	switch ( width )
	{
	default:		/* can't happen */
		return_error(gs_error_invalidfont);
	case 1:
	   {	byte chr = *p, schr;
		subs_loop(*psv, 1);
	   }
	case 2:
#define w2(p) (((ushort)*p << 8) + p[1])
	   {	ushort chr = w2(p), schr;
		subs_loop(w2(psv), 2);
	   }
#undef w2
	case 3:
#define w3(p) (((ulong)*p << 16) + ((uint)p[1] << 8) + p[2])
	   {	ulong chr = w3(p), schr;
		subs_loop(w3(psv), 3);
	   }
#undef w3
	case 4:
#define w4(p)\
  (((ulong)*p << 24) + ((ulong)p[1] << 16) + ((uint)p[2] << 8) + p[3])
	   {	ulong chr = w4(p), schr;
		subs_loop(w4(psv), 4);
	   }
#undef w4
#undef subs_loop
	}
	pmc->fidx = pdata->subs_size - subs_count;
	return 0;
}
