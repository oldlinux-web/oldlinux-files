/* pgmenhance.c - edge-enhance a portable graymap
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pgm.h"

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray* prevrow;
    gray* thisrow;
    gray* nextrow;
    gray* temprow;
    gray* newrow;
    register gray* ngP;
    int argn, n, rows, cols, row, col;
    float phi, omphi;
    gray maxval;
    long sum, newval;
    int format;
    char* usage = "[-N] [pgmfile]  ( 1 <= N <= 9, default = 9 )";

    pgm_init( &argc, argv );

    argn = 1;
    n = 9;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( sscanf( &(argv[argn][1]), "%d", &n ) != 1 )
	    pm_usage( usage );
	if ( n < 1 || n > 9 )
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pgm_pbmmaxval = 255;	/* use a larger value for better (?) results */
    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    prevrow = pgm_allocrow( cols );
    thisrow = pgm_allocrow( cols );
    nextrow = pgm_allocrow( cols );

    pgm_writepgminit( stdout, cols, rows, maxval, 0 );
    newrow = pgm_allocrow( cols );

    /* The edge enhancing technique is taken from Philip R. Thompson's "xim"
    ** program, which in turn took it from section 6 of "Digital Halftones by
    ** Dot Diffusion", D. E. Knuth, ACM Transaction on Graphics Vol. 6, No. 4,
    ** October 1987, which in turn got it from two 1976 papers by J. F. Jarvis
    ** et. al.
    */
    phi = n / 10.0;
    omphi = 1.0 - phi;

    /* First row. */
    pgm_readpgmrow( ifp, thisrow, cols, maxval, format );
    pgm_writepgmrow( stdout, thisrow, cols, maxval, 0 );
    pgm_readpgmrow( ifp, nextrow, cols, maxval, format );

    /* Other rows. */
    for ( row = 1; row < rows - 1; row++ )
	{
	temprow = prevrow;
	prevrow = thisrow;
	thisrow = nextrow;
	nextrow = temprow;
	pgm_readpgmrow( ifp, nextrow, cols, maxval, format );

	ngP = newrow;
	*ngP = thisrow[0];
	ngP++;
	for ( col = 1; col < cols - 1; col++, ngP++ )
	    {
	    /* Compute the sum of the neighborhood. */
	    sum =
		(long) prevrow[col-1] + (long) prevrow[col] +
		(long) prevrow[col+1] +
		(long) thisrow[col-1] + (long) thisrow[col] +
		(long) thisrow[col+1] +
		(long) nextrow[col-1] + (long) nextrow[col] +
		(long) nextrow[col+1];
	    /* Now figure new value. */
	    newval = ( ( thisrow[col] - phi * sum / 9 ) / omphi + 0.5 );
	    if ( newval < 0 )
		*ngP = 0;
	    else if ( newval > maxval )
		*ngP = maxval;
	    else
		*ngP = newval;
	    }
	*ngP = thisrow[cols - 1];
	pgm_writepgmrow( stdout, newrow, cols, maxval, 0 );
	}
    pm_close( ifp );

    /* Last row. */
    pgm_writepgmrow( stdout, nextrow, cols, maxval, 0 );

    pm_close( stdout );

    exit( 0 );
    }
