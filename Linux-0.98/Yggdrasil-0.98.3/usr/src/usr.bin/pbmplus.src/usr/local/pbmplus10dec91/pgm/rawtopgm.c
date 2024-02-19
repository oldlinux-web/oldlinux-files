/* rawtopgm.c - convert raw grayscale bytes into a portable graymap
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
    char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    int argn, headerskip, row, i;
    float rowskip, toskip;
    register int col, val;
    int rows, cols;
    char* usage = "[-headerskip N] [-rowskip N] <width> <height> [rawfile]";
    double atof();

    pgm_init( &argc, argv );

    argn = 1;
    headerskip = 0;
    rowskip = 0.0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-headerskip", 2 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    headerskip = atoi( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-rowskip", 2 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    rowskip = atof( argv[argn] );
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn + 2 > argc )
	pm_usage( usage );

    cols = atoi( argv[argn++] );
    rows = atoi( argv[argn++] );
    if ( cols <= 0 || rows <= 0 )
	pm_usage( usage );

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pgm_writepgminit( stdout, cols, rows, (gray) 255, 0 );
    grayrow = pgm_allocrow( cols );
    for ( i = 0; i < headerskip; ++i )
	{
	val = getc( ifp );
	if ( val == EOF )
	    pm_error( "EOF / read error" );
	}
    toskip = 0.00001;
    for ( row = 0; row < rows; ++row)
	{
	for ( col = 0, gP = grayrow; col < cols; ++col )
	    {
	    val = getc( ifp );
	    if ( val == EOF )
		pm_error( "EOF / read error" );
	    *gP++ = val;
	    }
	for ( toskip += rowskip; toskip >= 1.0; toskip -= 1.0 )
	    {
	    val = getc( ifp );
	    if ( val == EOF )
		pm_error( "EOF / read error" );
	    }
	pgm_writepgmrow( stdout, grayrow, cols, (gray) 255, 0 );
	}
    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
