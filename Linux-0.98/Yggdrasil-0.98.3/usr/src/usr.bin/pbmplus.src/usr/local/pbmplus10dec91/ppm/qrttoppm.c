/* qrttoppm.c - read a QRT ray-tracer output file and produce a portable pixmap
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

#include "ppm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    register pixel* pixelrow;
    int rows, cols, row, col;
    pixval maxval;
    unsigned char* buf;

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[qrtfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    /* Read in the QRT file.  First the header. */
    cols = getc( ifp );
    cols += getc( ifp ) << 8;
    rows = getc( ifp );
    rows += getc( ifp ) << 8;

    if ( cols <= 0 || rows <= 0 )
	pm_error( "invalid size: %d %d", cols, rows );
    maxval = 255;

    ppm_writeppminit( stdout, cols, rows, maxval, 0 );
    pixelrow = ppm_allocrow( cols );
    buf = (unsigned char *) malloc( 3 * cols );
    if ( buf == (unsigned char *) 0 )
	pm_error( "out of memory" );

    for ( row = 0; row < rows; row++ )
	{
        (void) getc( ifp );	/* discard */
        (void) getc( ifp );	/* linenum */
	if ( fread( buf, 3 * cols, 1, ifp ) != 1 )
	    pm_error( "EOF / read error" );
	for ( col = 0; col < cols; col++ )
	    PPM_ASSIGN(
		pixelrow[col], buf[col], buf[cols + col], buf[2 * cols + col] );
	ppm_writeppmrow( stdout, pixelrow, cols, maxval, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
