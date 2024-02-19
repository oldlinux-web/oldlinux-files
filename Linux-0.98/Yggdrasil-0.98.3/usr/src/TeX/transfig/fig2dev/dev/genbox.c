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

/* 
 *	genbox : Empty box driver for fig2dev translator
 *
*/
#include <stdio.h>
#include "object.h"
#include "fig2dev.h"

void genbox_option(opt, optarg)
char opt, *optarg;
{
  	switch (opt) {

	case 's':
	case 'f':
	case 'm':
	case 'L':
		break;

 	default:
		put_msg(Err_badarg, opt, "box");
		exit(1);
	}
}

void genbox_start(objects)
F_compound	*objects;
{
	double ppi;

	if (0 == (ppi = (double)objects->nwcorner.x)) {
	    fprintf(stderr, "Resolution is zero!! default to 80 ppi\n");
	    ppi = 80.0;
	    }

	/* draw box */
        fprintf(tfp, "\\makebox[%.3fin]{\\rule{0in}{%.3fin}}\n",
		(urx-llx)*mag/ppi, (ury-lly)*mag/ppi);
	}

struct driver dev_box = {
	genbox_option,
	genbox_start,
	gendev_null,
	gendev_null,
	gendev_null,
	gendev_null,
	gendev_null,
	gendev_null,
	INCLUDE_TEXT
};
