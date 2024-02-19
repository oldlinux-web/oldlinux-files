/* gouldtoppm.c - read GOULD imaging system files and write a portable pixmap
**
** Copyright (C) 1990  Stephen P. Lesniewski
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "ppm.h"

#define MAXVAL 255

static void getgouldheader ARGS(( FILE *infile, unsigned long *cols, unsigned long *nlines, unsigned long *bytesperline, unsigned long *bitsperpixel, unsigned long *NB ));

void
main( argc, argv )
int argc;
char *argv[];
{
	FILE *ifp;
	pixel *pixrow;

	unsigned long cols, nlines, bytesperline, bitsperpixel, NB, x, y;
	unsigned char pbuffer[4];

	int color_type;

	ppm_init( &argc, argv );

	if ( argc > 2 )
		pm_usage( "[gouldfile]" );

	if ( argc == 2 )
		ifp = pm_openr( argv[1] );
	else
		ifp = stdin;

	getgouldheader( ifp, &cols, &nlines, &bytesperline, &bitsperpixel, &NB);

	ppm_writeppminit(stdout, cols, nlines, MAXVAL, 0);

	color_type = bitsperpixel/8;
	if (color_type == 0) color_type = NB;

	pixrow = ppm_allocrow(cols);

	for (y = 0; y < nlines; ++y)
	{
		for (x = 0; x < cols; ++x)
		{
			switch (color_type)
			{
			case 0:
				pm_error("incorrect color type" );

			case 1:
				fread(pbuffer, 1, 1, ifp);
				PPM_ASSIGN( pixrow[x], pbuffer[0], pbuffer[0], pbuffer[0]);
				break;

			case 2:
				fread(pbuffer, 2, 1, ifp);
				PPM_ASSIGN( pixrow[x], pbuffer[0], pbuffer[1], pbuffer[1]);
				break;

			case 3:
				fread(pbuffer, 3, 1, ifp);
				PPM_ASSIGN( pixrow[x], pbuffer[0], pbuffer[1], pbuffer[2]);
				break;

			default :
				fread(pbuffer, 3, 1, ifp);
				PPM_ASSIGN( pixrow[x], pbuffer[0], pbuffer[1], pbuffer[2]);
				break;

			} /* switch */
		}
		ppm_writeppmrow(stdout, pixrow, cols, MAXVAL, 0);
	}

	pm_close(ifp);
	pm_close(stdout);

	exit(0);
}

static void
getgouldheader( infile, cols, nlines, bytesperline, bitsperpixel, NB)
FILE *infile;
unsigned long *cols, *nlines, *bytesperline, *bitsperpixel, *NB;
{
	unsigned long nlines_new, bytesperline_new, numheaderrec;
	unsigned char headerblk[512];
	int i;

	if (fread(headerblk, 512, 1, infile) == 0)
		pm_error("cannot read gould header" );

	*nlines       = (headerblk[3]<<8) | headerblk[2];
	*bytesperline = (headerblk[5]<<8) | headerblk[4];
	*bitsperpixel = (headerblk[7]<<8) | headerblk[6];
	numheaderrec  = (headerblk[9]<<8) | headerblk[8];
	nlines_new    = (headerblk[15]<<24)| (headerblk[14]<<16) | (headerblk[13]<<8) | (headerblk[12]);
	bytesperline_new = (headerblk[19]<<24)| (headerblk[18]<<16) | (headerblk[17]<<8) | (headerblk[16]);
	*NB           = (headerblk[23]<<24)| (headerblk[22]<<16) | (headerblk[21]<<8) | (headerblk[20]);

	if (numheaderrec > 1)
		for (i = 1 ; i <numheaderrec; ++i)
			if (fread(headerblk, 512, 1, infile) == 0)
				pm_error("cannot read gould header(2nd)" );

	if (*nlines==0) *nlines=nlines_new;
	if (*bytesperline==0) *bytesperline=bytesperline_new;

	*cols = (*bytesperline)*8 / (*bitsperpixel);
}
