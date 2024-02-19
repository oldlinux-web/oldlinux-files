/* mtvtoppm.c - read an MTV ray-tracer output file and produce a portable pixmap
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
    pixel* pixelrow;
    register pixel* pP;
    int rows, cols, row, col;
    pixval maxval;
#define MAXLINE 500
    char line[MAXLINE];
    unsigned char buf[3];

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[mtvfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    /* Read in the MTV file.  First the header. */
    if ( fgets( line, MAXLINE, ifp ) == NULL )
	pm_error( "unable to read MTV file header" );
    if ( sscanf( line, "%d%d", &cols, &rows ) != 2 )
	pm_error( "unable to parse MTV file header" );

    if ( cols <= 0 || rows <= 0 )
	pm_error( "invalid size: %d %d", cols, rows );
    maxval = 255;

    ppm_writeppminit( stdout, cols, rows, maxval, 0 );
    pixelrow = ppm_allocrow( cols );

    for ( row = 0; row < rows; row++ )
	{
	for ( col = 0, pP = pixelrow; col < cols; col++, pP++ )
	    {
	    if ( fread( buf, sizeof(buf), 1, ifp ) != 1 )
		pm_error( "EOF / read error" );
	    PPM_ASSIGN( *pP, buf[0], buf[1], buf[2] );
	    }
	ppm_writeppmrow( stdout, pixelrow, cols, maxval, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
