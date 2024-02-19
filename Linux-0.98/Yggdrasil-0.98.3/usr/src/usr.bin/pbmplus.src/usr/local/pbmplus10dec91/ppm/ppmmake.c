/* ppmmake.c - create a pixmap of a specified color and size
**
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

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    pixel color;
    pixel* pixrow;
    register pixel* pP;
    int argn, rows, cols, row;
    register int col;
    char* usage = "<color> <width> <height>";

    ppm_init( &argc, argv );

    argn = 1;

    if ( argn == argc )
	pm_usage( usage );
    color = ppm_parsecolor( argv[argn], PPM_MAXMAXVAL );
    ++argn;
    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &cols ) != 1 )
	pm_usage( usage );
    ++argn;
    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &rows ) != 1 )
	pm_usage( usage );
    ++argn;

    if ( argn != argc )
	pm_usage( usage );

    ppm_writeppminit( stdout, cols, rows, PPM_MAXMAXVAL, 0 );
    pixrow = ppm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
        for ( col = 0, pP = pixrow; col < cols; ++col, ++pP )
	    *pP = color;
	ppm_writeppmrow( stdout, pixrow, cols, PPM_MAXMAXVAL, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }
