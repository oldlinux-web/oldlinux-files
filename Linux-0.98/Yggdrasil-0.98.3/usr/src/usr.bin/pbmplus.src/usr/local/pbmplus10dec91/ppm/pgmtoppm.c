/* pgmtoppm.c - colorize a portable graymap into a portable pixmap
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
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    pixel p;
    pixel* pixelrow;
    register pixel* pP;
    pixel** mappixels;
    int argn, rows, cols, format, maprows, mapcols, mapmaxcolor, row;
    register int col;
    gray maxval;
    pixval mapmaxval;
    char* color0;
    char* color1;
    pixval red0, grn0, blu0, red1, grn1, blu1;
    char* usage = "<colorspec> [pgmfile]\n                 <colorspec1>,<colorspec2> [pgmfile]\n                 -map mapfile [pgmfile]";

    ppm_init( &argc, argv );

    argn = 1;
    mappixels = (pixel**) 0;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-map", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc )
		pm_usage( usage );
	    ifp = pm_openr( argv[argn] );
	    mappixels = ppm_readppm( ifp, &mapcols, &maprows, &mapmaxval );
	    pm_close( ifp );
	    mapmaxcolor = maprows * mapcols - 1;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( mappixels == (pixel**) 0 )
	{
	if ( argn == argc )
	    pm_usage( usage );
	color0 = argv[argn];
	++argn;
	}

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    grayrow = pgm_allocrow( cols );
    if ( mappixels == (pixel**) 0 )
	ppm_writeppminit( stdout, cols, rows, (pixval) maxval, 0 );
    else
	ppm_writeppminit( stdout, cols, rows, mapmaxval, 0 );
    pixelrow = ppm_allocrow( cols );

    if ( mappixels == (pixel**) 0 )
	{
	color1 = index( color0, '-' );
	if ( color1 == 0 )
	    {
	    color1 = color0;
	    red0 = 0;
	    grn0 = 0;
	    blu0 = 0;
	    }
	else
	    {
	    *color1 = '\0';
	    ++color1;
	    p = ppm_parsecolor( color0, (pixval) maxval );
	    red0 = PPM_GETR( p );
	    grn0 = PPM_GETG( p );
	    blu0 = PPM_GETB( p );
	    }
	p = ppm_parsecolor( color1, (pixval) maxval );
	red1 = PPM_GETR( p );
	grn1 = PPM_GETG( p );
	blu1 = PPM_GETB( p );
	}

    for ( row = 0; row < rows; ++row )
	{
	pgm_readpgmrow( ifp, grayrow, cols, maxval, format );

	if ( mappixels == (pixel**) 0 )
	    {
	    for ( col = 0, gP = grayrow, pP = pixelrow;
		  col < cols;
		  ++col, ++gP, ++pP )
		PPM_ASSIGN(
		    *pP,
		    ( red0 * ( maxval - *gP ) + red1 * *gP ) / maxval,
		    ( grn0 * ( maxval - *gP ) + grn1 * *gP ) / maxval,
		    ( blu0 * ( maxval - *gP ) + blu1 * *gP ) / maxval );

	    }
	else
	    {
	    register int c;

	    for ( col = 0, gP = grayrow, pP = pixelrow;
		  col < cols;
		  ++col, ++gP, ++pP )
		{
		if ( maxval == mapmaxcolor )
		    c = *gP;
		else
		    c = *gP * mapmaxcolor / maxval;
		*pP = mappixels[c / mapcols][c % mapcols];
		}
	    }

	ppm_writeppmrow( stdout, pixelrow, cols, (pixval) maxval, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
