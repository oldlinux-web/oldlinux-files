/* ppmtopi1.c - read a portable pixmap and write a Degas PI1 file
**
** Copyright (C) 1991 by Steve Belczyk and Jef Poskanzer.
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

#define COLS 320
#define ROWS 200
#define MAXVAL 7
#define MAXCOLORS 16

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel** pixels;
    register pixel *pP;
    colorhist_vector chv;
    colorhash_table cht;
    int rows, cols, row, colors, i;
    register int col;
    pixval maxval;
    short screen[ROWS*COLS/4];	/* simulate the ST's video RAM */

    ppm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[ppmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );
    if ( (cols > COLS) || (rows > ROWS) )
	pm_error( "image is larger than %dx%d - sorry", COLS, ROWS );

    pm_message( "computing colormap..." );
    chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors );
    if ( chv == (colorhist_vector) 0 )
	{
	pm_message(
	    "too many colors - try doing a 'ppmquant %d'", MAXCOLORS );
	exit( 1 );
	}
    pm_message( "%d colors found", colors );

    /* Write PI1 header - resolution and palette. */
    (void) pm_writebigshort( stdout, (short) 0 );	/* low resolution */
    for ( i = 0; i < 16; ++i )
	{
	short w;

	if ( i < colors )
	    {
	    pixel p;

	    p = chv[i].color;
	    if ( maxval != MAXVAL )
		PPM_DEPTH( p, p, maxval, MAXVAL );
	    w  = ( (int) PPM_GETR( p ) ) << 8;
	    w |= ( (int) PPM_GETG( p ) ) << 4;
	    w |= ( (int) PPM_GETB( p ) );
	    }
	else
	    w = 0;
	(void) pm_writebigshort( stdout, w );
	}
    if ( maxval > MAXVAL )
	pm_message(
	    "maxval is not %d - automatically rescaling colors", MAXVAL );

    /* Convert color vector to color hash table, for fast lookup. */
    cht = ppm_colorhisttocolorhash( chv, colors );
    ppm_freecolorhist( chv );

    /* Clear the screen buffer. */
    for ( i = 0; i < ROWS*COLS/4; ++i )
	screen[i] = 0;

    /* Convert. */
    for ( row = 0; row < rows; ++row )
	{
	for ( col = 0, pP = pixels[row]; col < cols; ++col, ++pP )
	    {
	    register int color, ind, b, plane;

	    color = ppm_lookupcolor( cht, pP );
	    if ( color == -1 )
		pm_error(
		    "color not found?!?  row=%d col=%d  r=%d g=%d b=%d",
		    row, col, PPM_GETR(*pP), PPM_GETG(*pP), PPM_GETB(*pP) );
	    ind = 80 * row + ( ( col >> 4 ) << 2 );
	    b = 0x8000 >> (col & 0xf);
	    for ( plane = 0; plane < 4; ++plane )
		if ( color & (1 << plane) )
		    screen[ind+plane] |= b;
	    }
	}

    /* And write out the screen buffer. */
    for ( i = 0; i < ROWS*COLS/4; ++i )
	(void) pm_writebigshort( stdout, screen[i] );

    exit( 0 );
    }
