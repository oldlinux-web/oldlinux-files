/* pgmhist.c - print a histogram of the values in a portable graymap
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

void
main( argc, argv )
int argc;
char *argv[];
    {
    FILE *ifp;
    gray maxval, *grayrow;
    register gray *gP;
    int argn, rows, cols, format, row;
    int i, *hist, *rcount, count, size;
    register int col;
    char *usage = "[pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	argn++;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    grayrow = pgm_allocrow( cols );

    /* Build histogram. */
    hist = (int *) malloc( ( maxval + 1 ) * sizeof(int) );
    rcount = (int *) malloc( ( maxval + 1 ) * sizeof(int) );
    if ( hist == (int *) 0 || rcount == (int *) 0 )
	pm_error( "out of memory" );
    for ( i = 0; i <= maxval; i++ )
	hist[i] = 0;
    for ( row = 0; row < rows; row++ )
	{
	pgm_readpgmrow( ifp, grayrow, cols, maxval, format );
        for ( col = 0, gP = grayrow; col < cols; col++, gP++ )
	    hist[(int) *gP]++;
	}

    pm_close( ifp );

    /* Compute count-down */
    count = 0;
    for ( i = maxval; i >= 0; i-- )
	{
	count += hist[i];
	rcount[i] = count;
	}

    /* And print it. */
    printf( "value\tcount\tb%%\tw%%\n" );
    printf( "-----\t-----\t--\t--\n" );
    count = 0;
    size = rows * cols;
    for ( i = 0; i <= maxval; i++ )
	if ( hist[i] > 0 )
	    {
	    count += hist[i];
	    printf(
		"%d\t%d\t%5.3g%%\t%5.3g%%\n", i, hist[i],
		(float) count * 100.0 / size, (float) rcount[i] * 100.0 / size );
	    }

    exit( 0 );
    }
