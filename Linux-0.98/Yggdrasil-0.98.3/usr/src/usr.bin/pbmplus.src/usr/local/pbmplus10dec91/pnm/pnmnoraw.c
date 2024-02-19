/* pnmnoraw.c - force a portable anymap into ASCII format
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

#include "pnm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xelval maxval;
    register xel* xelrow;
    int rows, cols, format, row;

    pnm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pnmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );
    pnm_writepnminit( stdout, cols, rows, maxval, format, 1 );
    xelrow = pnm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	pnm_writepnmrow( stdout, xelrow, cols, maxval, format, 1 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
