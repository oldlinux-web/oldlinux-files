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

/*  The selection of font names may be site dependent */

char		*picfontnames[] = {
			"R", "R",		/* default */
			"R",			/* roman */
			"B",			/* bold */
			"I",			/* italic */
			"H",			/* sans serif */
			"C"			/* typewriter */
		};
#define MAXFONTSIZE 108

#define PICFONT(F)	(picfontnames[((F) <= MAX_FONT) ? (F)+1 : MAX_FONT])
#define PICFONTSIZE(S)  ((S) > 0 ? \
				((S) <= MAXFONTSIZE ? \
					round(S) : \
					MAXFONTSIZE) : \
				font_size)
#define PICFONTMAG(T)	PICFONTSIZE((int)(T->size*(rigid_text(T) ? 1.0 : mag)))
