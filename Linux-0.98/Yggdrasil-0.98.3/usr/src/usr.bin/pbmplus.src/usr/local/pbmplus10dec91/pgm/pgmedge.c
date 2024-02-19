/* pgmedge.c - edge-detect a portable graymap
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pgm.h"
#include <math.h>

void
main( argc, argv )
int argc;
char *argv[];
    {
    FILE *ifp;
    gray *row0, *row1, *row2, *tmprow, *orow;
    int argn, rows, cols, format, row;
    register int col;
    gray maxval;
    double sum1, sum2, sum;
    char *usage = "[pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	argn++;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pgm_pbmmaxval = 255;	/* use larger value for better results */

    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    if ( cols < 3 || rows < 3 )
	pm_error( "the image is too small" );

    row0 = pgm_allocrow( cols );
    row1 = pgm_allocrow( cols );
    row2 = pgm_allocrow( cols );
    orow = pgm_allocrow( cols );

    pgm_writepgminit( stdout, cols, rows, maxval, 0 );

    /* Read in the first two rows. */
    pgm_readpgmrow( ifp, row0, cols, maxval, format );
    pgm_readpgmrow( ifp, row1, cols, maxval, format );

    /* Write out the first row, all zeros. */
    for ( col = 0; col < cols; ++col )
	orow[col] = 0;
    pgm_writepgmrow( stdout, orow, cols, maxval, 0 );

    /* Now the rest of the image -- read in the next row, and write
    ** write out the current row.
    */
    for ( row = 1; row < rows - 1; row++ )
	{
	pgm_readpgmrow( ifp, row2, cols, maxval, format );

	for ( col = 1; col < cols - 1; col++ )
	    {
	    sum1 = (double) row0[col+1] - (double) row0[col-1] +
		   2.0 * ( (double) row1[col+1] - (double) row1[col-1] ) +
		   (double) row2[col+1] - (double) row2[col-1];
	    sum2 = ( (double) row2[col-1] + 2.0 * (double) row2[col] +
		     (double) row2[col+1] ) -
		   ( (double) row0[col-1] + 2.0 * (double) row0[col] +
		     (double) row0[col+1] );
	    sum = sqrt( sum1 * sum1 + sum2 * sum2 );
	    sum /= 1.8;		/* arbitrary scaling factor */
	    if ( sum > maxval ) sum = maxval;
	    orow[col] = sum;
	    }

	/* Write out a row. */
	pgm_writepgmrow( stdout, orow, cols, maxval, 0 );

	/* Rotate rows. */
	tmprow = row0;
	row0 = row1;
	row1 = row2;
	row2 = tmprow;
	}
    pm_close( ifp );

    /* And write the last row, zeros again. */
    for ( col = 0; col < cols; ++col )
	orow[col] = 0;
    pgm_writepgmrow( stdout, orow, cols, maxval, 0 );

    pm_close( stdout );

    exit( 0 );
    }
