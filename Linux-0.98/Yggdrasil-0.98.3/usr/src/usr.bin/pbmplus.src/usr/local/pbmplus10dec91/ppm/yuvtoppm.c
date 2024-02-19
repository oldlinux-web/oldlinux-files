/* yuvtoppm.c - convert Abekas YUV bytes into a portable pixmap
**
** by Marc Boucher
** Internet: marc@PostImage.COM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69
**
** Uses integer arithmetic rather than floating point for better performance
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

/* x must be signed for the following to work correctly */
#define limit(x) (((x>0xffffff)?0xff0000:((x<=0xffff)?0:x&0xff0000))>>16)

void
main(argc, argv)
	char          **argv;
{
	FILE           *ifp;
	pixel          *pixrow;
	int             argn, rows, cols, row, i;
	char           *usage = "<width> <height> [yuvfile]";
	long  *yuvbuf;

	ppm_init(&argc, argv);

	argn = 1;

	if (argn + 2 > argc)
		pm_usage(usage);

	cols = atoi(argv[argn++]);
	rows = atoi(argv[argn++]);
	if (cols <= 0 || rows <= 0)
		pm_usage(usage);

	if (argn < argc) {
		ifp = pm_openr(argv[argn]);
		++argn;
	} else
		ifp = stdin;

	if (argn != argc)
		pm_usage(usage);

	if (255 > PGM_MAXMAXVAL)
		pm_error(
      "maxval of 255 is too large - try recompiling with a larger pixval type");

	ppm_writeppminit(stdout, cols, rows, (pixval) 255, 0);
	pixrow = ppm_allocrow(cols);
	yuvbuf = (long *) pm_allocrow(cols, 2);

	for (row = 0; row < rows; ++row) {
		long   tmp, y, u, v, y1, r, g, b, *yuvptr;
		register pixel *pP;
		register int    col;

		fread(yuvbuf, cols * 2, 1, ifp);

		for (col = 0, pP = pixrow, yuvptr = yuvbuf; col < cols; col += 2) {
			tmp = *yuvptr++;
			u = (0xff & (tmp >> 24)) - 128;
			y = ((0xff & (tmp >> 16)) - 16);
			if (y < 0) y = 0;

			v = (0xff & (tmp >> 8)) - 128;
			y1 = ((0xff & tmp) - 16);
			if (y1 < 0) y1 = 0;

			r = 104635 * v;
			g = -25690 * u + -53294 * v;
			b = 132278 * u;

			y*=76310; y1*=76310;

			PPM_ASSIGN(*pP, limit(r+y), limit(g+y), limit(b+y));
			pP++;
			PPM_ASSIGN(*pP, limit(r+y1), limit(g+y1), limit(b+y1));
			pP++;
		}
		ppm_writeppmrow(stdout, pixrow, cols, (pixval) 255, 0);
	}
	pm_close(ifp);
	pm_close(stdout);

	exit(0);
}
