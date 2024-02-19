/* sputoppm.c - read an uncompressed Spectrum file and produce a portable pixmap
**
** Copyright (C) 1991 by Steve Belczyk and Jef Poskanzer
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

#define ROWS 200
#define COLS 320
#define MAXVAL 7

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel pal[ROWS][48];		/* Spectrum palettes, three per row */
    short screen[ROWS*COLS/4];		/* simulates the Atari's video RAM */
    int i, j;
    pixel* pixelrow;
    register pixel* pP;
    int row, col;

    ppm_init( &argc, argv );

    /* Check args. */
    if ( argc > 2 )
	pm_usage( "[spufile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    /* Read the SPU file */

    /* Read the screen data. */
    for ( i = 0; i < ROWS*COLS/4; ++i )
	(void) pm_readbigshort( ifp, &screen[i] );

    /* Clear the first palette line. */
    for ( j = 0; j < 48; ++j )
	PPM_ASSIGN( pal[0][j], 0, 0, 0 );

    /* Read the palettes. */
    for ( i = 1; i < ROWS; ++i )
	for ( j = 0; j < 48; ++j )
	    {
	    short k;
	    (void) pm_readbigshort( ifp, &k );
	    PPM_ASSIGN( pal[i][j],
		( k & 0x700 ) >> 8,
		( k & 0x070 ) >> 4,
		( k & 0x007 ) );
	    }

    pm_close( ifp );

    /* Ok, get set for writing PPM. */
    ppm_writeppminit( stdout, COLS, ROWS, (pixval) MAXVAL, 0 );
    pixelrow = ppm_allocrow( COLS );

    /* Now do the conversion. */
    for ( row = 0; row < ROWS; ++row )
	{
	for ( col = 0, pP = pixelrow; col < COLS; ++col, ++pP )
	    {
	    int c, ind, b, plane, x1;

	    /* Compute pixel value. */
	    ind = 80 * row + ( ( col >> 4 ) << 2 );
	    b = 0x8000 >> (col & 0xf);
	    c = 0;
	    for ( plane = 0; plane < 4; ++plane )
		if ( b & screen[ind+plane] )
		    c |= (1 << plane);

	    /* Compute palette index. */
	    x1 = 10 * c;
	    if ( c & 1 )
		x1 -= 5;
	    else
		++x1;
	    if ( ( col >= x1 ) && ( col < ( x1 + 160 ) ) )
		c += 16;
	    if ( col >= ( x1 + 160 ) )
		c += 32;

	    /* Store the proper color. */
	    *pP = pal[row][c];
	    }
	ppm_writeppmrow( stdout, pixelrow, COLS, (pixval) MAXVAL, 0 );
	}

    pm_close( stdout );

    exit( 0 );
    }
