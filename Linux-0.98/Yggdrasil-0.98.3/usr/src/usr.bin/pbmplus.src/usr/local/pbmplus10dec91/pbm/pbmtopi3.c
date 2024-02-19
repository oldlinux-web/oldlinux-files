/* pbmtopi3.c - read a portable bitmap and produce a Atari Degas .pi3 file
**
** Module created from other pbmplus tools by David Beckemeyer.
**
** Copyright (C) 1988 by David Beckemeyer and Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "pbm.h"

static void putinit ARGS(( void ));
static void putbit ARGS(( bit b ));
static void putrest ARGS(( void ));
static void putitem ARGS(( void ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, format, padright, row, col;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pbm_readpbminit( ifp, &cols, &rows, &format );
    if (cols > 640)
	cols = 640;
    if (rows > 400)
	rows = 400;
    bitrow = pbm_allocrow( cols );
    
    /* Compute padding to round cols up to 640 */
    padright = 640 - cols;

    putinit( );
    for ( row = 0; row < rows; ++row )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    putbit( *bP );
	for ( col = 0; col < padright; ++col )
	    putbit( 0 );
        }
    while (row++ < 400)
	for ( col = 0; col < 640; ++col)
	    putbit( 0 );

    pm_close( ifp );

    putrest( );

    exit( 0 );
    }

static short item;
static short bitsperitem, bitshift;

static void
putinit( )
    {
    struct degasHDR {
	short res;
	short pal[16];
    } hdr;

    hdr.res = 2;
    hdr.pal[0] = 0x0777;
    hdr.pal[1] = 0x0700;
    fwrite( &hdr, sizeof(hdr), 1, stdout );
    item = 0;
    bitsperitem = 0;
    bitshift = 15;
    }

#if __STDC__
static void
putbit( bit b )
#else /*__STDC__*/
static void
putbit( b )
    bit b;
#endif /*__STDC__*/
    {
    if ( bitsperitem == 16 )
	putitem( );
    ++bitsperitem;
    if ( b == PBM_BLACK )
	item += 1 << bitshift;
    --bitshift;
    }

static void
putrest( )
    {
    if ( bitsperitem > 0 )
	putitem( );
    }

static void
putitem( )
    {
    fwrite( &item, sizeof(item), 1, stdout );
    item = 0;
    bitsperitem = 0;
    bitshift = 15;
    }
