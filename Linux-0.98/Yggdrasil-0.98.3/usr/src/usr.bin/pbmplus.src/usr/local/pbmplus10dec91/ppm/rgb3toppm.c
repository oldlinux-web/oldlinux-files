/* rgb3toppm - combine three portable graymaps into one portable pixmap
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
    FILE* rfd;
    FILE* gfd;
    FILE* bfd;
    gray* rrow;
    gray* rP;
    gray* grow;
    gray* gP;
    gray* brow;
    gray* bP;
    pixel* pixelrow;
    register pixel* pP;
    int rows, cols, trows, tcols, row, col;
    gray rmaxval, gmaxval, bmaxval;
    int rformat, gformat, bformat;
    pixval pmaxval;

    ppm_init( &argc, argv );

    if ( argc != 4 )
	pm_usage( "<red pgmfile> <green pgmfile> <blue pgmfile> " );

    rfd = pm_openr( argv[1] );
    gfd = pm_openr( argv[2] );
    bfd = pm_openr( argv[3] );

    pgm_readpgminit( rfd, &cols, &rows, &rmaxval, &rformat );
    pgm_readpgminit( gfd, &tcols, &trows, &gmaxval, &gformat );
    if ( tcols != cols || trows != rows )
	pm_error( "all three graymaps must be the same size" );
    pgm_readpgminit( bfd, &tcols, &trows, &bmaxval, &bformat );
    if ( tcols != cols || trows != rows )
	pm_error( "all three graymaps must be the same size" );

    pmaxval = rmaxval;
    if ( gmaxval > pmaxval ) pmaxval = gmaxval;
    if ( bmaxval > pmaxval ) pmaxval = bmaxval;
    rrow = pgm_allocrow( cols );
    grow = pgm_allocrow( cols );
    brow = pgm_allocrow( cols );

    ppm_writeppminit( stdout, cols, rows, pmaxval, 0 );
    pixelrow = ppm_allocrow( cols );

    for ( row = 0; row < rows; row++ )
	{
	pgm_readpgmrow( rfd, rrow, cols, rmaxval, rformat );
	pgm_readpgmrow( gfd, grow, cols, gmaxval, gformat );
	pgm_readpgmrow( bfd, brow, cols, bmaxval, bformat );

	for ( col = 0, rP = rrow, gP = grow, bP = brow, pP = pixelrow;
	      col < cols;
	      ++col, ++rP, ++gP, ++bP, ++pP )
	    {
	    if ( rmaxval != pmaxval ) *rP = (int) *rP * pmaxval / rmaxval;
	    if ( gmaxval != pmaxval ) *gP = (int) *gP * pmaxval / gmaxval;
	    if ( bmaxval != pmaxval ) *bP = (int) *bP * pmaxval / bmaxval;
	    PPM_ASSIGN( *pP, *rP, *gP, *bP );
	    }
	ppm_writeppmrow( stdout, pixelrow, cols, pmaxval, 0 );
	}

    pm_close( rfd );
    pm_close( gfd );
    pm_close( bfd );
    pm_close( stdout );

    exit( 0 );
    }
