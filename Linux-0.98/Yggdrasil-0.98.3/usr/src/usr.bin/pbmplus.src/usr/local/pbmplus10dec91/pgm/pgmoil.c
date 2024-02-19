/* pgmoil.c - read a portable graymap and turn into an oil painting
**
** Copyright (C) 1990 by Wilson Bent (whb@hoh-2.att.com)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "pgm.h"

static int hist[PGM_MAXMAXVAL+1];

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    gray midval, maxval;
    gray** grays;
    gray* grayrow;
    int argn, rows, n, cols, row;
    register int col, drow, dcol;
    int i;
    char* usage = "[-n <n>] [pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;
    n = 3;		/* DEFAULT VALUE */

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' )
	{
	if ( argv[argn][1] == 'n' )
	    {
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &n ) != 1 )
		pm_usage( usage );
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    grays = pgm_readpgm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );

    pgm_writepgminit( stdout, cols, rows, maxval, 0 );
    grayrow = pgm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	for ( col = 0; col < cols; ++col )
	    {
	    for ( i = 0; i <= maxval; ++i )
		hist[i] = 0;

	    for ( drow = row - n; drow <= row + n; ++drow )
		if ( drow >= 0 && drow < rows )
		    for ( dcol = col - n; dcol <= col + n; ++dcol )
			if ( dcol >= 0 && dcol < cols )
			    ++hist[ (int) (grays[drow][dcol]) ];

	    for ( drow = dcol = 0; dcol < maxval; ++dcol )
		if ( hist[dcol] > drow )
		    {
		    drow = hist[dcol];
		    midval = dcol;
		    }
	    grayrow[col] = midval;
	    }
	pgm_writepgmrow( stdout, grayrow, cols, maxval, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }
