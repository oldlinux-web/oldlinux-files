/* ppmtoyuv.c - convert a portable pixmap into an Abekas YUV file
**
** by Marc Boucher
** Internet: marc@PostImage.COM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69.
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

void
main(argc, argv)
char **argv;
{
	FILE *ifp;
	pixel          *pixelrow;
	register pixel *pP;
	int             rows, cols, format, row;
	register int    col;
	pixval          maxval;
	unsigned long   y1, y2=0, u=0, v=0, u0=0, u1, u2, v0=0, v1, v2;
	unsigned char  *yuvbuf;

	ppm_init(&argc, argv);

	if (argc > 2) pm_usage("[ppmfile]");

	if (argc == 2) ifp = pm_openr(argv[1]);
	else ifp = stdin;

	ppm_readppminit(ifp, &cols, &rows, &maxval, &format);
	pixelrow = ((pixel*) pm_allocrow( cols, sizeof(pixel) ));
	yuvbuf = (unsigned char *) pm_allocrow( cols, 2 );

	for (row = 0; row < rows; ++row) {
		unsigned char *yuvptr;

		ppm_readppmrow(ifp, pixelrow, cols, maxval, format);

		for (col = 0, pP = pixelrow, yuvptr=yuvbuf; col < cols; col += 2, ++pP) {
			pixval r, g, b;

			/* first pixel gives Y and 0.5 of chroma */
			r = PPM_GETR(*pP);
			g = PPM_GETG(*pP);
			b = PPM_GETB(*pP);

			y1 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y2);
			u1 = -4853 * r - 9530 * g + 14383 * b;
			v1 = 14386 * r - 12046 * g - 2340 * b;

			pP++;
			/* second pixel just yields a Y and 0.25 U, 0.25 V */
			r = PPM_GETR(*pP);
			g = PPM_GETG(*pP);
			b = PPM_GETB(*pP);

			y2 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y1);
			u2 = -2426 * r - 4765 * g + 7191 * b;
			v2 = 7193 * r - 6023 * g - 1170 * b;

			/* filter the chroma */
			u = u0 + u1 + u2 + (0xffff & u);
			v = v0 + v1 + v2 + (0xffff & v);

			u0 = u2;
			v0 = v2;

			*yuvptr++ = (u >> 16) + 128;
			*yuvptr++ = (y1 >> 16) + 16;
			*yuvptr++ = (v >> 16) + 128;
			*yuvptr++ = (y2 >> 16) + 16;
		}
		fwrite(yuvbuf, cols*2, 1, stdout);
	}

	pm_close(ifp);

	exit(0);
}
