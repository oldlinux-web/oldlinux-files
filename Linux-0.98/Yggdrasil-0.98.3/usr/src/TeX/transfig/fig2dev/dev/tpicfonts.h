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
			"rm",			/* default */
			"rm",			/* roman */
			"bf",			/* bold */
			"it",			/* italic */
			"sf", 			/* sans serif */
			"sl",			/* slanted */
			"tt",			/* typewriter */
			/* Virtual fonts from here on */
			"avant",
			"avantcsc",
			"avantd",
			"avantdi",
			"avanti",
			"bookd",
			"bookdi",
			"bookl",
			"booklcsc",
			"bookli",
			"chanc",
			"cour",
			"courb",
			"courbi",
			"couri",
			"helv",
			"helvb",
			"helvbi",
			"helvc",
			"helvcb",
			"helvcbi",
			"helvci",
			"helvcsc",
			"helvi",
			"pal",
			"palb",
			"palbi",
			"palbu",
			"palc",
			"palcsc",
			"pali",
			"palsl",
			"palu",
			"palx",
			"times",
			"timesb",
			"timesbi",
			"timesc",
			"timescsc",
			"timesi",
			"timessl",
			"timesx"
		};

#define	MAX_TPICFONT	48

/* The selection of font names may be site dependent.
 * Not all fonts are preloaded at all sizes.
 */

static char		*texfontsizes[] = {
			"ten",			/* default */
			"fiv", "fiv", "fiv", "fiv", 	/* small fonts */
			"fiv",			/* five point font */
			"six", "sev", "egt",	/* etc */
			"nin", "ten", "elv",
			"twl", "twl", "frtn",	
			"frtn", "frtn", "svtn",
			"svtn", "svtn", "twty",
			"twty", "twty", "twty", "twty", "twfv"
			};

static int		TeXfontsizes[] = {
			10,		/* default */
			5, 5, 5, 5, 	/* small fonts */
			5,			/* five point font */
			6, 7, 8,	/* etc */
			9, 10, 11,
			12, 12, 14,	
			14, 14, 17,
			17, 17, 20,
			20, 20, 20, 20, 25
			};


#define MAXFONTSIZE 25

#define TEXFONT(F)	(texfontnames[((F) <= MAX_TPICFONT) ? (F) : MAX_TPICFONT])
#define TEXFONTSIZE(S)	(texfontsizes[((S) <= MAXFONTSIZE) ? (S) : MAXFONTSIZE])
#define TEXFONTMAG(T)	TEXFONTSIZE((int)(T->size*(rigid_text(T) ? 1.0 : mag)))
#define TEXFONTSIZEINT(S)	(TeXfontsizes[((S) <= MAXFONTSIZE) ? (S) : MAXFONTSIZE])
#define TEXFONTMAGINT(T)	TEXFONTSIZEINT((int)(T->size*(rigid_text(T) ? 1.0 : mag)))

static char	*texture_patterns[] = {
	"8 0 8 0 4 1 3 e 0 8 0 8 1 4 e 3",	/* scales */
	"f f 8 0 8 0 8 0 f f 0 8 0 8 0 8",	/* bricks */
	"8 1 4 2 2 4 1 8 8 1 4 2 2 4 1 8",	/* waves */
	"8 0 4 0 2 0 1 0 0 8 0 4 0 2 0 1",	/* light backslash alternating */
	"e 0 7 0 3 8 1 c 0 e 0 7 8 3 c 1",	/* heavy backslash alternating */
	"7 7 b b d d e e 7 7 b b d d e e",	/* heavy backslash */
	"8 8 4 4 2 2 1 1 8 8 4 4 2 2 1 1",	/* light backslash */
	"9 9 c c 6 6 3 3 9 9 c c 6 6 3 3",	/* medium backslash */
	"2 0 4 0 8 0 0 0 0 8 0 4 0 2 0 0",	/* light hash */
	"f f 0 0 f f 0 0 f f 0 0 f f 0 0",	/* horizontal lines */
	"f f 0 0 0 0 0 0 f f 0 0 0 0 0 0",	/* spaced horizontal lines */
	"c c 0 0 0 0 0 0 3 3 0 0 0 0 0 0",	/* spaced horizontal dashed lines */
	"f 0 f 0 f 0 f 0 0 f 0 f 0 f 0 f",	/* chessboard */
	"f f 8 8 8 8 8 8 f f 8 8 8 8 8 8",	/* light meshed lines */
	"a a 4 4 a a 1 1 a a 4 4 a a 1 1",	/* hashed dotted lines */
	"0 1 0 2 0 4 0 8 1 0 2 0 4 0 8 0",	/* spaced light frontslash */
	"8 3 0 7 0 e 1 c 3 8 7 0 e 0 c 1",	/* spaced heavy frontslash */
	"e e d d b b 7 7 e e d d b b 7 7",	/* heavy frontslash */
	"1 1 2 2 4 4 8 8 1 1 2 2 4 4 8 8",	/* light frontslash */
	"3 3 6 6 c c 9 9 3 3 6 6 c c 9 9",	/* medium frontslash */
	"4 0 a 0 0 0 0 0 0 4 0 a 0 0 0 0",	/* wallpaper - birds */
	"a a a a a a a a a a a a a a a a",	/* vertical lines */
	"8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8",	/* spaced vertical lines */
	"0 1 0 1 1 0 1 0 0 1 0 1 1 0 1 0",	/* vertical dashed lines */
	"0 0 0 8 1 4 2 a 5 5 2 a 1 4 0 8",	/* hashed diamonds */
	"f f 8 0 8 0 8 0 8 0 8 0 8 0 8 0",	/* spaced meshed lines */
	"8 2 4 4 2 8 1 0 2 8 4 4 8 2 0 1"	/* hashed dotted lines */
};

#define	MAXPATTERNS	27
