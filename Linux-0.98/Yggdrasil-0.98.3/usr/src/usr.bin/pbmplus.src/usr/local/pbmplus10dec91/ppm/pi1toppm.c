/* pi1toppm.c - read a Degas PI1 file and produce a portable pixmap
**
** Copyright (C) 1991 by Steve Belczyk (seb3@gte.com) and Jef Poskanzer.
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
    pixel pal[16];			/* Degas palette */
    short screen[ROWS*COLS/4];		/* simulates the Atari's video RAM */
    short i;
    long j;
    pixel* pixelrow;
    register pixel* pP;
    int row, col;

    ppm_init( &argc, argv );

    /* Check args. */
    if ( argc > 2 )
	pm_usage( "[pi1file]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    /* Check resolution word */
    (void) pm_readbiglong( ifp, &j );
    if ( j != 0 )
	pm_error( "not a PI1 file" );

    /* Read the palette. */
    for ( i = 0; i < 16; ++i )
	{
	(void) pm_readbiglong( ifp, &j );
	PPM_ASSIGN( pal[i], 
	    ( j & 0x700 ) >> 8,
	    ( j & 0x070 ) >> 4,
	    ( j & 0x007 ) );
	}

    /* Read the screen data */
    for ( i = 0; i < ROWS*COLS/4; ++i )
	(void) pm_readbigshort( ifp, &screen[i] );

    pm_close( ifp );

    /* Ok, get set for writing PPM. */
    ppm_writeppminit( stdout, COLS, ROWS, (pixval) MAXVAL, 0 );
    pixelrow = ppm_allocrow( COLS );

    /* Now do the conversion. */
    for ( row = 0; row < ROWS; ++row )
	{
	for ( col = 0, pP = pixelrow; col < COLS; ++col, ++pP )
	    {
	    register int c, ind, b, plane;

	    ind = 80 * row + ( ( col >> 4 ) << 2 );
	    b = 0x8000 >> ( col & 0xf );
	    c = 0;
	    for ( plane = 0; plane < 4; ++plane )
		if ( b & screen[ind+plane] )
		    c |= (1 << plane);
	    *pP = pal[c];
	    }
	ppm_writeppmrow( stdout, pixelrow, COLS, (pixval) MAXVAL, 0 );
	}

    pm_close( stdout );

    exit( 0 );
    }
