/* spctoppm.c - read a compressed Spectrum file and produce a portable pixmap
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

static void DoBitmap ARGS(( FILE* ifp ));
static void DoChar ARGS(( int n, char c ));
static void DoColormap ARGS(( FILE* ifp ));

static char screen[ROWS*COLS/2];
static short sscreen[ROWS*COLS/4];
static pixel pal[ROWS][48];
static long colormap_length, bitmap_length;

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    char c1, c2;
    pixel* pixelrow;
    register pixel* pP;
    int row, col;

    ppm_init( &argc, argv );

    /* Check args. */
    if ( argc > 2 )
	pm_usage( "[spcfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    /* Check SPC file header. */
    c1 = getc( ifp );
    c2 = getc( ifp );

    if ( ( c1 != 'S' ) || ( c2 != 'P' ) )
	pm_error( "not a Spectrum picture" );

    /* Skip reserved bytes. */
    getc( ifp );
    getc( ifp );

    /* Get length of bitmap data. */
    (void) pm_readbiglong( ifp, &bitmap_length );

    /* and colormap */
    (void) pm_readbiglong( ifp, &colormap_length );

    /* Process bitmap. */
    DoBitmap( ifp );

    /* Process colormap. */
    DoColormap( ifp );

    pm_close( ifp );

    /* Write the PPM file. */
    ppm_writeppminit( stdout, COLS, ROWS, (pixval) MAXVAL, 0 );
    pixelrow = ppm_allocrow( COLS );

    for ( row = 0; row < ROWS; ++row )
	{
	for ( col = 0, pP = pixelrow; col < COLS; ++col, ++pP )
	    {
	    int c, ind, b, plane, x1;

	    /* Compute pixel value. */
	    ind = ( 80 * row ) + ( ( col >> 4 ) << 2 );
	    b = 0x8000 >> (col & 0xf);
	    c = 0;
	    for ( plane = 0; plane < 4; ++plane )
		if ( b & sscreen[ind+plane] )
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

static void
DoBitmap( ifp )
    FILE* ifp;
    {
    int i;
    long count, data;
    char h, c;

    /* Zero out first scan line. */
    for ( i = 0; i < 160; ++i )
	screen[i] = 0;

    /* 'count' counts number of input bytes. */
    count = 0;

    /* 'data' counts just data bytes. */
    data = 0;

    while ( count < bitmap_length )
	{
	/* Get next record header. */
	h = getc( ifp );
	++count;

	if ( ( h >= 0 ) && ( count < bitmap_length ) )
	    {
	    for ( i = 0; i <= h; ++i )
		{
		c = getc( ifp );
		++count;
		DoChar( data, c );
		++data;
		}
	    }
	else if ( ( h < 0 ) && ( count < bitmap_length ) )
	    {
	    c = getc( ifp );
	    ++count;

	    for ( i = 0; i < ( 2 - h ); ++i )
		{
		DoChar( data, c );
		++data;
		}
	    }
    }

    /* Convert the char version of the screen to short. */
    for ( i = 0; i < ROWS*COLS/4; ++i )
	sscreen[i] = ( screen[i<<1] << 8 ) + ( 0xff & screen[(i<<1)+1] );
    }

#if __STDC__
static void
DoChar( int n, char c )
#else /*__STDC__*/
static void
DoChar( n, c )
    int n;
    char c;
#endif /*__STDC__*/
    {
    int i;

    /* Compute screen index. */
    i = 160 + 2 * ( n / 7960 ) + 8 * ( ( n % 7960 ) / 2 ) + ( n & 1 );
    screen[i] = c;
    }

static void
DoColormap( ifp )
    FILE* ifp;
    {
    int i, j, b;
    short mask;

    /* Clear first three palettes. */
    for ( j = 0; j < 48; ++j )
	PPM_ASSIGN( pal[0][j], 0, 0, 0 );

    /* Read the palettes. */
    for ( i = 1; i < ROWS; ++i )
	for ( j = 0; j < 3; ++j )
	    {
	    (void) pm_readbigshort( ifp, &mask );
	    for ( b = 0; b < 15; ++b )
		if ( mask & ( 1 << b ) )
		    {
		    short k;
		    (void) pm_readbigshort( ifp, &k );
		    PPM_ASSIGN( pal[i][(j*16)+b],
			( k & 0x700 ) >> 8,
			( k & 0x070 ) >> 4,
			( k & 0x007 ) );
		    }
	    }
    }
