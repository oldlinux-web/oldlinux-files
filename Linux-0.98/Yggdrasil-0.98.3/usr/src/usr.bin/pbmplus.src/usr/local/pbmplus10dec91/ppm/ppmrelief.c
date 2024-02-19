/* ppmrelief.c - generate a relief map of a portable pixmap
**
** Copyright (C) 1990 by Wilson H. Bent, Jr.
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

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel** inputbuf;
    pixel* outputrow;
    int argn, rows, cols, format, row, rowa, rowb;
    register int col;
    pixval maxval, mv2, r, g, b;
    char* usage = "[ppmfile]";

    ppm_init( &argc, argv );

    argn = 1;

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    ppm_pbmmaxval = PPM_MAXMAXVAL;	/* use larger value for better results */

    ppm_readppminit( ifp, &cols, &rows, &maxval, &format );
    mv2 = maxval / 2;

    /* Allocate space for 3 input rows, plus an output row. */
    inputbuf = ppm_allocarray( cols, 3 );
    outputrow = ppm_allocrow( cols );

    ppm_writeppminit( stdout, cols, rows, maxval, 0 );

    /* Read in the first two rows. */
    ppm_readppmrow( ifp, inputbuf[0], cols, maxval, format );
    ppm_readppmrow( ifp, inputbuf[1], cols, maxval, format );

    /* Write out the first row, all zeros. */
    for ( col = 0; col < cols; ++col )
        PPM_ASSIGN( outputrow[col], 0, 0, 0 );
    ppm_writeppmrow( stdout, outputrow, cols, maxval, 0 );

    /* Now the rest of the image - read in the 3rd row of inputbuf,
    ** and convolve with the first row into the output buffer.
    */
    for ( row = 2 ; row < rows; ++row )
	{
	rowa = row % 3;
	rowb = (row + 2) % 3;
	ppm_readppmrow( ifp, inputbuf[rowa], cols, maxval, format );

	for ( col = 0; col < cols - 2; ++col )
	    {
	    r = PPM_GETR( inputbuf[rowa][col] ) +
		( mv2 - PPM_GETR( inputbuf[rowb][col + 2] ) );
	    g = PPM_GETG( inputbuf[rowa][col] ) +
		( mv2 - PPM_GETG( inputbuf[rowb][col + 2] ) );
	    b = PPM_GETB( inputbuf[rowa][col] ) +
		( mv2 - PPM_GETB( inputbuf[rowb][col + 2] ) );
	    PPM_ASSIGN( outputrow[col + 1], r, g, b );
	    }
	ppm_writeppmrow( stdout, outputrow, cols, maxval, 0 );
	}

    /* And write the last row, zeros again. */
    for ( col = 0; col < cols; ++col )
        PPM_ASSIGN( outputrow[col], 0, 0, 0 );
    ppm_writeppmrow( stdout, outputrow, cols, maxval, 0 );

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
