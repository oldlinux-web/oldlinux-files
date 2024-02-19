/* ppmtoilbm.c - read a portable pixmap and produce an IFF ILBM file
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

#include "ppm.h"
#include "ppmcmap.h"
#include "ilbm.h"

#define MAX_COLORS 256
#define MAX_HAMIF_COLORS 32

static int colorstobpp ARGS(( int colors ));
static void put_chunk_head ARGS(( char* str, long size ));
static void put_fourchars ARGS(( char* str ));
static void put_big_short ARGS(( short s ));
static void put_big_long ARGS(( long l ));
static void put_byte ARGS(( unsigned char b ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, rows, cols, colors, i, hamif, ham, nPlanes, formsize, cmapsize;
    pixel** pixels;
    pixel* pP;
    int row, col, plane;
    pixval maxval;
    colorhash_table cht;
    colorhist_vector chv;
    unsigned char* raw_rowbuf;
    unsigned char* coded_rowbuf;
    char* usage = "[-hamif] [ppmfile]";

    ppm_init( &argc, argv );

    argn = 1;
    hamif = 0;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-hamif", 2 ) )
	    hamif = 1;
	else if ( pm_keymatch( argv[argn], "-nohamif", 2 ) )
	    hamif = 0;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );

    pm_close( ifp );

    /* Figure out the colormap. */
    pm_message( "computing colormap..." );
    chv = ppm_computecolorhist( pixels, cols, rows, MAX_COLORS, &colors );
    if ( hamif && chv == (colorhist_vector) 0 )
	{
	pm_message(
	    "too many colors - proceeding to write a HAM file" );
        pm_message(
            "if you want a non-HAM file, try doing a 'ppmquant %d'",
            MAX_HAMIF_COLORS );
        ham = 1;
        colors = 16;
        nPlanes = 6;
	}
    else if ( chv == (colorhist_vector) 0 )
	pm_error( "too many colors - try doing a ppmquant" );
    else if ( hamif && colors > MAX_HAMIF_COLORS )
	{
	pm_message(
	  "%d colors found - proceeding to write a HAM file", colors );
	pm_message(
	    "if you want a non-HAM file, try doing a 'ppmquant %d'",
	    MAX_HAMIF_COLORS );
	ham = 1;
	colors = 16;
	nPlanes = 6;
	}
    else
	{
	pm_message( "%d colors found", colors );
	ham = 0;
	nPlanes = colorstobpp( colors );
	/* Make a hash table for fast color lookup. */
	cht = ppm_colorhisttocolorhash( chv, colors );
	}

    /* Start off the ILBM with a FORM ILBM. */
    cmapsize = colors * 3;
    if ( odd( colors ) )
	cmapsize++;					/* pad CMAP to word */
    formsize =
	4 +						/* ILBM */
	4 + 4 + 20 +					/* BMHD size header*/
	4 + 4 + cmapsize +				/* CMAP size colors */
	4 + 4 + 4 +					/* CAMG size val */
	4 + 4 + rows * nPlanes * RowBytes(cols);	/* BODY size data */
    put_chunk_head( "FORM", formsize );
    put_fourchars( "ILBM" );

    /* Write out the BMHD. */
    put_chunk_head( "BMHD", 20 );
    put_big_short( cols );
    put_big_short( rows );
    put_big_short( 0 );		/* x */
    put_big_short( 0 );		/* y */
    put_byte( nPlanes );
    put_byte( mskNone );
    put_byte( cmpNone );
    put_byte( 0 );		/* pad1 */
    put_big_short( 0 );		/* transparentColor */
    put_byte( 10 );		/* xAsp */
    put_byte( 10 );		/* yAsp */
    put_big_short( cols );	/* pageWidth */
    put_big_short( rows );	/* pageHeight */

    /* Write out the CMAP. */
    if ( maxval > 255 )
	pm_message( "maxval is not 255 - automatically rescaling colors" );
    put_chunk_head( "CMAP", cmapsize );
    for ( i = 0; i < colors; i++ )
	{
	if ( ham )
	    { /* Grayscale colormap for HAM. */
	    put_byte( i << 4 );
	    put_byte( i << 4 );
	    put_byte( i << 4 );
	    }
	else if ( maxval == 255 )
	    {
	    put_byte( PPM_GETR( chv[i].color ) );
	    put_byte( PPM_GETG( chv[i].color ) );
	    put_byte( PPM_GETB( chv[i].color ) );
	    }
	else
	    {
	    put_byte( (char) ( (int) PPM_GETR(chv[i].color) * 255 / maxval ) );
	    put_byte( (char) ( (int) PPM_GETG(chv[i].color) * 255 / maxval ) );
	    put_byte( (char) ( (int) PPM_GETB(chv[i].color) * 255 / maxval ) );
	    }
	}
    if ( odd( colors ) )
	put_byte( 0 );		/* Pad to word. */

    /* Write out the CAMG. */
    put_chunk_head( "CAMG", 4 );
    if ( ham )
	put_big_long( vmHAM );
    else
	put_big_long( 0 );

    /* And finally, encode and write out the BODY. */
    raw_rowbuf = (unsigned char *) malloc( cols );
    coded_rowbuf = (unsigned char *) malloc( RowBytes(cols) );
    if ( raw_rowbuf == 0 || coded_rowbuf == 0 )
	pm_error( "out of memory" );
    put_chunk_head( "BODY", rows * nPlanes * RowBytes(cols) );
    for ( row = 0; row < rows; row++ )
	{
	/* Go from pixels to raw bytes. */
	if ( ham )
	    { /* HAM mode. */
	    register int noprev, pr, pg, pb, r, g, b, l;

	    noprev = 1;
	    for ( col = 0, pP = pixels[row]; col < cols; col++, pP++ )
		{
		r = PPM_GETR( *pP );
		g = PPM_GETG( *pP );
		b = PPM_GETB( *pP );
		l = PPM_LUMIN( *pP );
		if ( maxval != 15 )
		    {
		    r = r * 15 / maxval;
		    g = g * 15 / maxval;
		    b = b * 15 / maxval;
		    l = l * 15 / maxval;
		    }
		if ( noprev )
		    { /* No previous pixels, gotta use the gray option. */
		    raw_rowbuf[col] = l;
		    pr = pg = pb = l;
		    noprev = 0;
		    }
		else
		    {
		    register int dred, dgreen, dblue, dgray;

		    /* Compute distances for the four options. */
		    dred = abs( g - pg ) + abs( b - pb );
		    dgreen = abs( r - pr ) + abs( b - pb );
		    dblue = abs( r - pr ) + abs( g - pg );
		    dgray = abs( r - l ) + abs( g - l ) + abs( b - l );

		    if ( dgray < dred && dgray < dgreen && dgray < dblue )
			{
			raw_rowbuf[col] = l;
			pr = pg = pb = l;
			}
		    else if ( dblue < dred && dblue < dgreen )
			{
			raw_rowbuf[col] = ( 1 << 4 ) + b;
			pb = b;
			}
		    else if ( dred < dgreen )
			{
			raw_rowbuf[col] = ( 2 << 4 ) + r;
			pr = r;
			}
		    else
			{
			raw_rowbuf[col] = ( 3 << 4 ) + g;
			pg = g;
			}
		    }
		}
	    }
	else
	    /* Non-HAM. */
	    for ( col = 0, pP = pixels[row]; col < cols; col++, pP++ )
		raw_rowbuf[col] = ppm_lookupcolor( cht, pP );

	/* Encode and write raw bytes in plane-interleaved form. */
	for ( plane = 0; plane < nPlanes; plane++ )
	    {
	    int mask;
	    unsigned char *cp;

	    mask = 1 << plane;
	    cp = coded_rowbuf;
	    *cp = 0;
	    for ( col = 0; col < cols; col++ )
		{
		int b;

		b = ( raw_rowbuf[col] & mask ) ? 1 : 0;
		*cp |= b << ( 7 - col % 8 );
		if ( col % 8 == 7 )
		    {
		    cp++;
		    *cp = 0;
		    }
		}
	    fwrite( coded_rowbuf, RowBytes(cols), 1, stdout );
	    }
	}

    exit( 0 );
    }

static int
colorstobpp( colors )
    int colors;
    {
    int bpp;

    if ( colors <= 2 )
	bpp = 1;
    else if ( colors <= 4 )
	bpp = 2;
    else if ( colors <= 8 )
	bpp = 3;
    else if ( colors <= 16 )
	bpp = 4;
    else if ( colors <= 32 )
	bpp = 5;
    else if ( colors <= 64 )
	bpp = 6;
    else if ( colors <= 128 )
	bpp = 7;
    else if ( colors <= 256 )
	bpp = 8;
    else
	pm_error( "can't happen" );

    return bpp;
    }

static void
put_chunk_head( str, size )
    char* str;
    long size;
    {
    put_fourchars( str );
    put_big_long( size );
    }

static void
put_fourchars( str )
char* str;
    {
    fputs( str, stdout );
    }

#if __STDC__
static void
put_big_short( short s )
#else /*__STDC__*/
static void
put_big_short( s )
    short s;
#endif /*__STDC__*/
    {
    if ( pm_writebigshort( stdout, s ) == -1 )
	pm_error( "write error" );
    }

#if __STDC__
static void
put_big_long( long l )
#else /*__STDC__*/
static void
put_big_long( l )
    long l;
#endif /*__STDC__*/
    {
    if ( pm_writebiglong( stdout, l ) == -1 )
	pm_error( "write error" );
    }

#if __STDC__
static void
put_byte( unsigned char b )
#else /*__STDC__*/
static void
put_byte( b )
    unsigned char b;
#endif /*__STDC__*/
    {
    (void) putc( b, stdout );
    }
