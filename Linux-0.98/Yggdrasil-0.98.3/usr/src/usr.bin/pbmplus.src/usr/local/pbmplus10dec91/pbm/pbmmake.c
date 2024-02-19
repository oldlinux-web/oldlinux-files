/* pbmmake.c - create a blank bitmap of a specified size
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    register bit color, gray, rowcolor;
    bit* bitrow;
    register bit* bP;
    int argn, rows, cols, row, col;
    char* usage = "[-white|-black|-gray] <width> <height>";

    pbm_init( &argc, argv );

    argn = 1;
    color = PBM_WHITE;
    gray = 0;

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-white", 2 ) )
	    color = PBM_WHITE;
	else if ( pm_keymatch( argv[argn], "-black", 2 ) )
	    color = PBM_BLACK;
	else if ( pm_keymatch( argv[argn], "-gray", 2 ) ||
	          pm_keymatch( argv[argn], "-grey", 2 ) )
	    gray = 1;
	else
	    pm_usage( usage );
	++argn;
	}

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

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    if ( gray )
	rowcolor = PBM_WHITE;  /* arbitrarily make the corner white */
    for ( row = 0; row < rows; ++row )
	{
	if ( gray )
	    {
	    color = rowcolor;
	    rowcolor = 1 - rowcolor;
	    }
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    {
	    *bP = color;
	    if ( gray )
		color = 1 - color;
	    }
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }
