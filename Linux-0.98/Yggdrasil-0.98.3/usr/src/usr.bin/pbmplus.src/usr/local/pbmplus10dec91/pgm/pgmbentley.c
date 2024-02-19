/* pgmbentley.c - read a portable graymap and smear it according to brightness
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

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    gray maxval;
    gray** gin;
    gray** gout;
    int argn, rows, cols, row;
    register int brow, col;
    char* usage = "[pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    gin = pgm_readpgm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );
    gout = pgm_allocarray( cols, rows );

#define N 4
    for ( row = 0; row < rows; ++row )
	for ( col = 0; col < cols; ++col )
	    {
	    brow = row + (int) (gin[row][col]) / N;
	    if ( brow >= rows )
		brow = rows - 1;
	    gout[brow][col] = gin[row][col];
	    }

    pgm_writepgm( stdout, gout, cols, rows, maxval, 0 );
    pm_close( stdout );
    pgm_freearray( gout, rows );

    exit( 0 );
    }
