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
 *	genpstex.c : psTeX and psTeX_t drivers for fig2dev
 *
 *	Author: Jose Alberto Fernandez R /Maryland CP 9/90
 * 	It uses the LaTeX and PostScript drivers to generate 
 *      LaTeX processed text for a Postscript figure.
 *
 * The pstex_t driver is like a latex driver that only translates 
 * text defined in the defaul font.
 *
 * The pstex driver is like a PostScript driver that translates 
 * everything except for text in the default font.
 *
 * The option '-p file' added to the pstex_t translator specifies
 * the name of the PostScript file to be called in the psfig macro.
 * If not set or its value is null then no PS file will be inserted.
 *
 * Jose Alberto.
 */

#if defined(hpux) || defined(SYSV)
#include <sys/types.h>
#endif
#include <sys/file.h>
#include <stdio.h>
#include <math.h>
#include "object.h"
#include "fig2dev.h"
#include "texfonts.h"

extern char *strchr();
#ifndef fabs
extern double fabs();
#endif
#ifndef sin
extern double sin();
#endif
#ifndef cos
extern double cos();
#endif
#ifndef acos
extern double acos();
#endif
#ifndef atan
extern double atan();
#endif
extern double rad2deg;

#ifdef hpux
#define rint(a) floor((a)+0.5)     /* close enough? */
#endif

#ifdef gould
#define rint(a) floor((a)+0.5)     /* close enough? */
#endif

extern void genlatex_start (),
	gendev_null (),
	genlatex_end (),
     	genps_option (),
	genps_start (),
	genps_arc (),
	genps_ellipse (),
	genps_line (),
	genps_spline (),
	genps_end (),
        genlatex_option (),
        genlatex_text (),
        genps_text ();

static char pstex_file[1000] = "";

void genpstex_t_option(opt, optarg)
char opt, *optarg;
{
       if (opt == 'p') strcpy(pstex_file, optarg);
       else genlatex_option(opt, optarg);
}


void genpstex_t_start(objects)
F_compound	*objects;
{
	/* Put PostScript Image if any*/
        if (pstex_file[0] != '\0')
        {
              fprintf(tfp, "\\begin{picture}(0,0)%%\n");
              fprintf(tfp, "\\special{psfile=%s}%%\n",pstex_file);
              fprintf(tfp, "\\end{picture}%%\n");
	}
        genlatex_start(objects);

}

void genpstex_t_text(t)
F_text	*t;
{

	if (!special_text(t))
	  gendev_null(t);
	else genlatex_text(t);
}

void genpstex_text(t)
F_text	*t;
{

	if (!special_text(t))
	  genps_text(t);
	else gendev_null(t);
}

void genpstex_option(opt, optarg)
char opt, *optarg;
{
       if (opt != 'p') genlatex_option(opt, optarg);
}

struct driver dev_pstex_t = {
     	genpstex_t_option,
	genpstex_t_start,
	gendev_null,
	gendev_null,
	gendev_null,
	gendev_null,
	genpstex_t_text,
	genlatex_end,
	EXCLUDE_TEXT
};

struct driver dev_pstex = {
     	genpstex_option,
	genps_start,
	genps_arc,
	genps_ellipse,
	genps_line,
	genps_spline,
	genpstex_text,
	genps_end,
	INCLUDE_TEXT
};


