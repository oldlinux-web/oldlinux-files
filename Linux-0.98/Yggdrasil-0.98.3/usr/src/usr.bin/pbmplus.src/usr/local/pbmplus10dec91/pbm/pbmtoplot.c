/* pbmtoplot.c - read a portable bitmap and produce a UNIX-format plot file.
**
** Copyright (C) 1990 by Arthur David Olson.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "pbm.h"

static void
puttwo( i )
    int i;
    {
    (void) putchar(i);
    (void) putchar(i >> 8);
    }

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    register bit** bits;
    register int row, col, scol;
    int	rows, cols;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    ifp = (argc == 2) ? pm_openr( argv[1] ) : stdin;

    bits = pbm_readpbm( ifp, &cols, &rows );

    pm_close( ifp );

    (void) putchar( 's' );
    puttwo( 0 );
    puttwo( 0 );
    puttwo( rows - 1 );
    puttwo( cols - 1 );
    for ( row = 0; row < rows; ++row )
	{
	for ( col = 0; col < cols; ++col )
	    {
	    if ( bits[row][col] == PBM_WHITE )
		continue;
	    scol = col;
	    while ( ++col < cols && bits[row][col] == PBM_BLACK )
		; /* nothing */
	    --col;
	    if ( col == scol )
		(void) putchar( 'p' );
	    else
		{
		(void) putchar( 'l' );
		puttwo( scol );
		puttwo( rows - 1 - row );
		}
	    puttwo( col );
	    puttwo( rows - 1 - row );
	    }
	}

    exit( 0 );
    }
