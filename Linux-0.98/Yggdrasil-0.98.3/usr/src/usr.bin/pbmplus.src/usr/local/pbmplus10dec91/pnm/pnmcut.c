/* pnmcut.c - cut a rectangle out of a portable anymap
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

#include "pnm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    register xel* xelrow;
    xelval maxval;
    int rows, cols, format, x, y, width, height, row;
    char* usage = "x y width height [pnmfile]";

    pnm_init( &argc, argv );

    if ( argc < 5 || argc > 6 )
	pm_usage( usage );

    if ( sscanf( argv[1], "%d", &x ) != 1 )
	pm_usage( usage );
    if ( sscanf( argv[2], "%d", &y ) != 1 )
	pm_usage( usage );
    if ( sscanf( argv[3], "%d", &width ) != 1 )
	pm_usage( usage );
    if ( sscanf( argv[4], "%d", &height ) != 1 )
	pm_usage( usage );

    if ( width < 1 )
	pm_error( "width is less than 1" );
    if ( height < 1 )
	pm_error( "height is less than 1" );

    if ( argc == 6 )
	ifp = pm_openr( argv[5] );
    else
	ifp = stdin;

    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );
    xelrow = pnm_allocrow( cols );

    if ( x <= -cols )
	pm_error(
	    "x is too negative -- the anymap has only %d cols", cols );
    else if ( x >= cols )
	pm_error(
	    "x is too large -- the anymap has only %d cols", cols );
    if ( y <= -rows )
	pm_error(
	    "y is too negative -- the anymap has only %d rows", rows );
    else if ( y >= rows )
	pm_error(
	    "y is too large -- the anymap has only %d rows", rows );

    if ( x < 0 )
	x += cols;
    if ( y < 0 )
	y += rows;

    if ( x + width > cols )
	pm_error(
	    "x + width is too large by %d xels", x + width - cols );
    if ( y + height > rows )
	pm_error(
	    "y + height is too large by %d xels", y + height - rows );

    pnm_writepnminit( stdout, width, height, maxval, format, 0 );
    for ( row = 0; row < y + height; row++ )
	{
	pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	if ( row >= y )
	    pnm_writepnmrow( stdout, &(xelrow[x]), width, maxval, format, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
