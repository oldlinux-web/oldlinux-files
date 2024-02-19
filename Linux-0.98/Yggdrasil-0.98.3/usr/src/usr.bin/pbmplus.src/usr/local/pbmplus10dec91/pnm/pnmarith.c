/* pnmarith.c - perform arithmetic on two portable anymaps
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
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
    xelval maxval1, maxval2, maxval3;
    int argn, rows1, cols1, format1, rows2, cols2, format2, format3, row, col;
    char function;
    char* usage = "-add|-subtract|-multiply pnmfile1 pnmfile2";

    pnm_init( &argc, argv );

    argn = 1;
    function = ' ';

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-add", 2 ) )
	    function = '+';
	else if ( pm_keymatch( argv[argn], "-subtract", 2 ) )
	    function = '-';
	else if ( pm_keymatch( argv[argn], "-multiply", 2 ) )
	    function = '*';
	else
	    pm_usage( usage );
	++argn;
	}

    if ( function == ' ' )
	pm_usage( usage );

    if ( argn == argc )
	pm_usage( usage );
    ifp1 = pm_openr( argv[argn] );
    ++argn;

    if ( argn == argc )
	pm_usage( usage );
    ifp2 = pm_openr( argv[argn] );
    ++argn;

    if ( argn != argc )
	pm_usage( usage );

    pnm_readpnminit( ifp1, &cols1, &rows1, &maxval1, &format1 );
    xelrow1 = pnm_allocrow( cols1 );
    pnm_readpnminit( ifp2, &cols2, &rows2, &maxval2, &format2 );
    if ( cols2 != cols1 || rows2 != rows1 )
	pm_error(
	    "the two anymaps must be the same width and height" );
    xelrow2 = pnm_allocrow( cols1 );

    maxval3 = max( maxval1, maxval2 );
    format3 = max( PNM_FORMAT_TYPE(format1), PNM_FORMAT_TYPE(format2) );
    if ( PNM_FORMAT_TYPE(format1) != format3 ||
	 PNM_FORMAT_TYPE(format2) != format3 )
	{
	switch ( PNM_FORMAT_TYPE(format3) )
	    {
	    case PPM_TYPE:
	    if ( PNM_FORMAT_TYPE(format1) != format3 )
		pm_message( "promoting first file to PPM" );
	    if ( PNM_FORMAT_TYPE(format2) != format3 )
		pm_message( "promoting second file to PPM" );
	    break;
	    case PGM_TYPE:
	    if ( PNM_FORMAT_TYPE(format1) != format3 )
		pm_message( "promoting first file to PGM" );
	    if ( PNM_FORMAT_TYPE(format2) != format3 )
		pm_message( "promoting second file to PGM" );
	    break;
	    }
	}

    pnm_writepnminit( stdout, cols1, rows1, maxval3, format3, 0 );
    for ( row = 0; row < rows1; ++row )
	{
	pnm_readpnmrow( ifp1, xelrow1, cols1, maxval1, format1 );
	if ( maxval1 != maxval3 || PNM_FORMAT_TYPE(format1) != format3 )
	    pnm_promoteformatrow(
		xelrow1, cols1, maxval1, format1, maxval3, format3 );

	pnm_readpnmrow( ifp2, xelrow2, cols1, maxval2, format2 );
	if ( maxval2 != maxval3 || PNM_FORMAT_TYPE(format2) != format3 )
	    pnm_promoteformatrow(
		xelrow2, cols1, maxval2, format2, maxval3, format3 );

        for ( col = 0, x1P = xelrow1, x2P = xelrow2;
	      col < cols1; ++col, ++x1P, ++x2P )
	    {
	    switch ( PNM_FORMAT_TYPE(format3) )
		{
		case PPM_TYPE:
		{
		int r1, g1, b1, r2, g2, b2;

		r1 = PPM_GETR( *x1P );
		g1 = PPM_GETG( *x1P );
		b1 = PPM_GETB( *x1P );
		r2 = PPM_GETR( *x2P );
		g2 = PPM_GETG( *x2P );
		b2 = PPM_GETB( *x2P );
		switch ( function )
		    {
		    case '+':
		    r1 += r2;
		    g1 += g2;
		    b1 += b2;
		    break;

		    case '-':
		    r1 -= r2;
		    g1 -= g2;
		    b1 -= b2;
		    break;

		    case '*':
		    r1 = r1 * r2 / maxval3;
		    g1 = g1 * g2 / maxval3;
		    b1 = b1 * b2 / maxval3;
		    break;

		    default:
		    pm_error( "can't happen" );
		    }
		if ( r1 < 0 ) r1 = 0;
		else if ( r1 > maxval3 ) r1 = maxval3;
		if ( g1 < 0 ) g1 = 0;
		else if ( g1 > maxval3 ) g1 = maxval3;
		if ( b1 < 0 ) b1 = 0;
		else if ( b1 > maxval3 ) b1 = maxval3;
		PPM_ASSIGN( *x1P, r1, g1, b1 );
		}
		break;

		default:
		{
		int g1, g2;

		g1 = PNM_GET1( *x1P );
		g2 = PNM_GET1( *x2P );
		switch ( function )
		    {
		    case '+':
		    g1 += g2;
		    break;

		    case '-':
		    g1 -= g2;
		    break;

		    case '*':
		    g1 = g1 * g2 / maxval3;
		    break;

		    default:
		    pm_error( "can't happen" );
		    }
		if ( g1 < 0 ) g1 = 0;
		else if ( g1 > maxval3 ) g1 = maxval3;
		PNM_ASSIGN1( *x1P, g1 );
		}
		break;
		}
	    }
	pnm_writepnmrow( stdout, xelrow1, cols1, maxval3, format3, 0 );
	}

    pm_close( ifp1 );
    pm_close( ifp2 );
    pm_close( stdout );

    exit( 0 );
    }
