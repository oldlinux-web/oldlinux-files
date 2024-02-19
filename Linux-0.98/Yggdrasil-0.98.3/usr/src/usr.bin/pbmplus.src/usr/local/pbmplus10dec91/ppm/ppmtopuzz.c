/* ppmtopuzz.c - read a portable pixmap and write an X11 "puzzle" file
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
#include "ppmcmap.h"

#define MAXVAL 255
#define MAXCOLORS 256

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel** pixels;
    register pixel* pP;
    colorhist_vector chv;
    colorhash_table cht;
    int rows, cols, row, colors, i;
    register int col;
    pixval maxval;

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[ppmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );

    pm_message( "computing colormap..." );
    chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors );
    if ( chv == (colorhist_vector) 0 )
	{
	pm_message(
	    "too many colors - try doing a 'ppmquant %d'", MAXCOLORS );
	exit( 1 );
	}
    pm_message( "%d colors found", colors );

    /* Write puzzle header. */
    (void) pm_writebiglong( stdout, cols );
    (void) pm_writebiglong( stdout, rows );
    (void) putchar( (unsigned char) colors );
    if ( maxval > MAXVAL )
	pm_message(
	    "maxval is not %d - automatically rescaling colors", MAXVAL );
    for ( i = 0; i < colors; ++i )
	{
	pixel p;

	p = chv[i].color;
	if ( maxval != MAXVAL )
	    PPM_DEPTH( p, p, maxval, MAXVAL );
	(void) putchar( (unsigned char) PPM_GETR( p ) );
	(void) putchar( (unsigned char) PPM_GETG( p ) );
	(void) putchar( (unsigned char) PPM_GETB( p ) );
	}

    /* Convert color vector to color hash table, for fast lookup. */
    cht = ppm_colorhisttocolorhash( chv, colors );
    ppm_freecolorhist( chv );

    /* And write out the data. */
    for ( row = 0; row < rows; ++row )
	{
	for ( col = 0, pP = pixels[row]; col < cols; ++col, ++pP )
	    {
	    register int color;

	    color = ppm_lookupcolor( cht, pP );
	    if ( color == -1 )
		pm_error(
		    "color not found?!?  row=%d col=%d  r=%d g=%d b=%d",
		    row, col, PPM_GETR(*pP), PPM_GETG(*pP), PPM_GETB(*pP) );
	    (void) putchar( (unsigned char) color );
	    }
	}

    exit( 0 );
    }
