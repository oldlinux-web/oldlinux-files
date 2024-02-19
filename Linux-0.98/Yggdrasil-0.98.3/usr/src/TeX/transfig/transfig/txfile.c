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

#include <stdio.h>
#include "transfig.h"

static char *fontfile = "figfonts.tex";

/*
 * create appropriate .tex file
 */
texfile(tx, in, altfonts, arg_list)
FILE *tx;
char *in;
int  altfonts;
argument *arg_list;
{
  enum language to;
  argument *a, *arg_l;

  for (a = arglist; a; a = a->next) {
     to = a->tolang;

     /* see if we already have this language */
     for (arg_l = arglist; arg_l != a; arg_l = arg_l->next)
	if ( arg_l->tolang == to ) break;
	
     if ( arg_l == a )
	switch (to)
	{
	case box:
		fprintf(tx, "\\typeout{TransFig: null figures.}\n");
		break;

	case eepicemu:
		to = eepicemu;

	case eepic:
#ifdef eemulation
		to = eepicemu;
#endif

	case epic:
		fprintf(tx, "\\typeout{TransFig: figures in %s.}\n",
							lname[(int)to]);
		if (to == eepicemu || to == eepic)
			fprintf(tx, "\\documentstyle{epic}");
		fprintf(tx, "\\documentstyle{%s}\n", lname[(int)to]);
		break;

	case latex:
		fprintf(tx, "\\typeout{TransFig: figures in LaTeX.}\n");
		break;

	case pictex:
		fprintf(tx, "\\typeout{TransFig: figures in PiCTeX.}\n");
		fprintf(tx, "\\input{prepictex}\n");
		fprintf(tx, "\\input{pictex}\n");
		fprintf(tx, "\\input{postpictex}\n");
		break;

	case postscript:
		fprintf(tx, "\\typeout{TransFig: figures in PostScript.}\n");
		break;

	case pstex: 
		fprintf(tx, "\\typeout{TransFig: figure text in LaTeX.}\n");
		fprintf(tx, "\\typeout{TransFig: figures in PostScript.}\n");
		break;

	case psfig:
		fprintf(tx, "\\typeout{TransFig: figures in PostScript w/psfig.}\n");
		fprintf(tx, "\\documentstyle{psfig}\n");
		break;

	case textyl:
		fprintf(tx, "\\typeout{TransFig: figures in TeXtyl.}\n");
		break;

	case tpic:
		fprintf(tx, "\\typeout{TransFig: figures in tpic.}\n");
		break;

	default:
		fprintf(tx, "Unknown graphics language %s\n", lname[(int)to]);
		exit(1);
		break;

	}
  }
  if (altfonts==1) fprintf(tx, "\\input{%s}\n", fontfile);
  if (*in) fprintf(tx, "\\input{%s}\n", in);
}
