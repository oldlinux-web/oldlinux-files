/* ppmtorgb3.c - separate a portable pixmap into three portable graymaps
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
#include "pgm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    FILE* redfile;
    FILE* grnfile;
    FILE* blufile;
    char* basename;
    char filename[100];
    char* cp;
    pixel* pixelrow;
    register pixel* pP;
    gray* grayrow;
    register gray* gP;
    int rows, cols, format, row;
    register int col;
    pixval maxval;

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[ppmfile]" );

    if ( argc == 2 )
	{
	ifp = pm_openr( argv[1] );
	basename = argv[1];
	cp = rindex( basename, '.' );
	if ( cp != NULL )
	    *cp = '\0';
	}
    else
	{
	ifp = stdin;
	basename = "noname";
	}

    ppm_readppminit( ifp, &cols, &rows, &maxval, &format );
    pixelrow = ppm_allocrow( cols );
    (void) strcpy( filename, basename );
    (void) strcat( filename, ".red" );
    redfile = pm_openw( filename );
    pgm_writepgminit( redfile, cols, rows, (gray) maxval, 0 );
    (void) strcpy( filename, basename );
    (void) strcat( filename, ".grn" );
    grnfile = pm_openw( filename );
    pgm_writepgminit( grnfile, cols, rows, (gray) maxval, 0 );
    (void) strcpy( filename, basename );
    (void) strcat( filename, ".blu" );
    blufile = pm_openw( filename );
    pgm_writepgminit( blufile, cols, rows, (gray) maxval, 0 );
    grayrow = pgm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	ppm_readppmrow( ifp, pixelrow, cols, maxval, format );

	for ( col = 0, pP = pixelrow, gP = grayrow; col < cols;
	      ++col, ++pP, ++gP )
	    *gP = (gray) PPM_GETR( *pP );
	pgm_writepgmrow( redfile, grayrow, cols, maxval, 0 );

	for ( col = 0, pP = pixelrow, gP = grayrow; col < cols;
	      ++col, ++pP, ++gP )
	    *gP = (gray) PPM_GETG( *pP );
	pgm_writepgmrow( grnfile, grayrow, cols, maxval, 0 );

	for ( col = 0, pP = pixelrow, gP = grayrow; col < cols;
	      ++col, ++pP, ++gP )
	    *gP = (gray) PPM_GETB( *pP );
	pgm_writepgmrow( blufile, grayrow, cols, maxval, 0 );
	}

    pm_close( ifp );
    pm_close( redfile );
    pm_close( blufile );
    pm_close( grnfile );

    exit( 0 );
    }
