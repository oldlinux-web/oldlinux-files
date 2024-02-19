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

static char		*psfontnames[] = {
			"Times-Roman", "Times-Roman",	/* default */
			"Times-Roman",			/* roman */
			"Times-Bold",			/* bold */
			"Times-Italic",			/* italic */
			"Helvetica",			/* sans serif */
			"Courier"			/* typewriter */
		};

#define PS_FONTNAMES(T)	\
  	(((v2_flag&&!v21_flag) || psfont_text(T)) ? PSfontnames : psfontnames)

#define PSFONT(T) \
 ((T->font) <= MAXFONT(T) ? PS_FONTNAMES(T)[T->font+1] : PS_FONTNAMES(T)[0])

#define PSFONTMAG(T)	(((T->size) > 0 ? \
				((T->size) <= ULIMIT_FONT_SIZE ? \
			 		(T->size) : \
					ULIMIT_FONT_SIZE) : \
				font_size)/(rigid_text(T) ? mag : 1.0))
