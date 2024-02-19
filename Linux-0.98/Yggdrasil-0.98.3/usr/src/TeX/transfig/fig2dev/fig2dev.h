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

#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#ifndef NeXT
#define	strchr	index
#define	strrchr	rindex
#endif
#endif

#define round(x)	((int) ((x) + ((x >= 0)? 0.5: -0.5)))

/* 
 * Device driver interface structure
 */
struct driver {
 	void (*option)();	/* interpret driver-specific options */
  	void (*start)();	/* output file header */
	void (*arc)();		/* object generators */
	void (*ellipse)();
	void (*line)();
	void (*spline)();
	void (*text)();
	void (*end)();		/* output file trailer */
  	int text_include;	/* include text length in bounding box */
#define INCLUDE_TEXT 1
#define EXCLUDE_TEXT 0
};

extern char *strchr();

extern char Err_badarg[];
extern char Err_incomp[];
extern char Err_mem[];

extern char *PSfontnames[];

extern char	*prog, *from;
extern int	font_size;
extern double	mag;
extern FILE	*tfp;

extern int llx, lly, urx, ury, coord_system;

extern void gendev_null();
