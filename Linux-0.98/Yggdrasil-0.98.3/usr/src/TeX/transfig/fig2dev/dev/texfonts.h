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

static char		*texfontnames[] = {
			"rm", "rm",		/* default */
			"rm",			/* roman */
			"bf",			/* bold */
			"it",			/* italic */
			"sf", 			/* sans serif */
			"tt"			/* typewriter */
		};

/* The selection of font names may be site dependent.
 * Not all fonts are preloaded at all sizes.
 */

static char		*texfontsizes[] = {
			"elv", "elv",		/* default */
			"fiv", "fiv", "fiv", "fiv", 	/* small fonts */
			"fiv",			/* five point font */
			"six", "sev", "egt",	/* etc */
			"nin", "ten", "elv",
			"twl", "twl", "frtn",	
			"frtn", "frtn", "svtn",
			"svtn", "svtn", "twty",
			"twty", "twty", "twty", "twty", "twfv",
                        "twfv", "twfv", "twfv", "twentynine",
                        "twentynine", "twentynine", "twentynine", "twentynine",
                        "thirtyfour", "thirtyfour", "thirtyfour", "thirtyfour",
                        "thirtyfour", "thirtyfour", "thirtyfour", "fortyone",
                        "fortyone", "fortyone"
  			};

static char		*texFontsizes[] = {
 			"Elv", "Elv",		/* default */
 			"Fiv", "Fiv", "Fiv", "Fiv", 	/* small fonts */
 			"Fiv",			/* five point font */
 			"Six", "Sev", "Egt",	/* etc */
 			"Nin", "Ten", "Elv",
 			"Twl", "Twl", "Frtn",	
 			"Frtn", "Frtn", "Svtn",
 			"Svtn", "Svtn", "Twty",
 			"Twty", "Twty", "Twty", "Twty", "Twfv",
                        "Twfv", "Twfv", "Twfv", "Twentynine",
                        "Twentynine", "Twentynine", "Twentynine", "Twentynine",
                        "Thirtyfour", "Thirtyfour", "Thirtyfour", "Thirtyfour",
                        "Thirtyfour", "Thirtyfour", "Thirtyfour", "Fortyone",
                        "Fortyone", "Fortyone"
  			};

#define MAXFONTSIZE 	42

static int capfonts = 0;

#define TEXFONTSIZES	(capfonts ? texFontsizes : texfontsizes)
#define TEXFONT(F)	(texfontnames[((F) <= MAX_FONT) ? (F)+1 : MAX_FONT])
#define TEXFONTSIZE(S)	(TEXFONTSIZES[((S) <= MAXFONTSIZE) ? round(S)+1\
				      				: MAXFONTSIZE])
#define TEXFONTMAG(T)	TEXFONTSIZE(T->size*(rigid_text(T) ? 1.0 : mag))

