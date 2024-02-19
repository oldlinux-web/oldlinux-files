/* pnmpaste.c - paste a rectangle into a portable anymap
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
    FILE* ifp1;
    FILE* ifp2;
    register xel* xelrow1;
    register xel* xelrow2;
    register xel* x1P;
    register xel* x2P;
    xelval maxval1, maxval2, newmaxval;
    int argn, rows1, cols1, format1, x, y;
    int rows2, cols2, format2, newformat, row;
    register int col;
    char function;
    char* usage = "[-replace|-or|-and|-xor] frompnmfile x y [intopnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    function = 'r';

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-replace", 2 ) )
	    function = 'r';
	else if ( pm_keymatch( argv[argn], "-or", 2 ) )
	    function = 'o';
	else if ( pm_keymatch( argv[argn], "-and", 2 ) )
	    function = 'a';
	else if ( pm_keymatch( argv[argn], "-xor", 2 ) )
	    function = 'x';
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn == argc )
	pm_usage( usage );
    ifp1 = pm_openr( argv[argn] );
    ++argn;

    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &x ) != 1 )
	pm_usage( usage );
    ++argn;
    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &y ) != 1 )
	pm_usage( usage );
    ++argn;

    if ( argn != argc )
	{
	ifp2 = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp2 = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pnm_readpnminit( ifp1, &cols1, &rows1, &maxval1, &format1 );
    xelrow1 = pnm_allocrow(cols1);
    pnm_readpnminit( ifp2, &cols2, &rows2, &maxval2, &format2 );
    xelrow2 = pnm_allocrow(cols2);

    if ( x <= -cols2 )
	pm_error(
	    "x is too negative -- the second anymap has only %d cols",
	    cols2 );
    else if ( x >= cols2 )
	pm_error(
	    "x is too large -- the second anymap has only %d cols",
	    cols2 );
    if ( y <= -rows2 )
	pm_error(
	    "y is too negative -- the second anymap has only %d rows",
	    rows2 );
    else if ( y >= rows2 )
	pm_error(
	    "y is too large -- the second anymap has only %d rows",
	    rows2 );

    if ( x < 0 )
	x += cols2;
    if ( y < 0 )
	y += rows2;

    if ( x + cols1 > cols2 )
	pm_error( "x + width is too large by %d pixels", x + cols1 - cols2 );
    if ( y + rows1 > rows2 )
	pm_error( "y + height is too large by %d pixels", y + rows1 - rows2 );

    newformat = max( PNM_FORMAT_TYPE(format1), PNM_FORMAT_TYPE(format2) );
    newmaxval = max( maxval1, maxval2 );

    if ( function != 'r' && newformat != PBM_TYPE )
	pm_error( "no logical operations allowed for non-bitmaps" );

    pnm_writepnminit( stdout, cols2, rows2, newmaxval, newformat, 0 );

    for ( row = 0; row < rows2; ++row )
	{
	pnm_readpnmrow( ifp2, xelrow2, cols2, maxval2, format2 );
	pnm_promoteformatrow( xelrow2, cols2, maxval2, format2,
	    newmaxval, newformat );

	if ( row >= y && row < y + rows1 )
	    {
	    pnm_readpnmrow( ifp1, xelrow1, cols1, maxval1, format1 );
	    pnm_promoteformatrow( xelrow1, cols1, maxval1, format1,
		newmaxval, newformat );
	    for ( col = 0, x1P = xelrow1, x2P = &(xelrow2[x]);
		  col < cols1; ++col, ++x1P, ++x2P )
		{
		register xelval b1, b2;

		switch ( function )
		    {
		    case 'r':
		    *x2P = *x1P;
		    break;

		    case 'o':
		    b1 = PNM_GET1( *x1P );
		    b2 = PNM_GET1( *x2P );
		    if ( b1 != 0 || b2 != 0 )
			PNM_ASSIGN1( *x2P, newmaxval );
		    else
			PNM_ASSIGN1( *x2P, 0 );
		    break;

		    case 'a':
		    b1 = PNM_GET1( *x1P );
		    b2 = PNM_GET1( *x2P );
		    if ( b1 != 0 && b2 != 0 )
			PNM_ASSIGN1( *x2P, newmaxval );
		    else
			PNM_ASSIGN1( *x2P, 0 );
		    break;

		    case 'x':
		    b1 = PNM_GET1( *x1P );
		    b2 = PNM_GET1( *x2P );
		    if ( ( b1 != 0 && b2 == 0 ) || ( b1 == 0 && b2 != 0 ) )
			PNM_ASSIGN1( *x2P, newmaxval );
		    else
			PNM_ASSIGN1( *x2P, 0 );
		    break;

		    default:
		    pm_error( "can't happen" );
		    }
		}
	    }

	pnm_writepnmrow( stdout, xelrow2, cols2, newmaxval, newformat, 0 );
	}
    
    pm_close( ifp1 );
    pm_close( ifp2 );
    pm_close( stdout );

    exit( 0 );
    }
