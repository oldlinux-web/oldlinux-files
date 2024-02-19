/* pbmtomgr.c - read a portable bitmap and produce a MGR bitmap
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

#include "pbm.h"
#include "mgr.h"

static void putinit ARGS(( int rows, int cols ));
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
    bitrow = pbm_allocrow( cols );
    
    /* Round cols up to the nearest multiple of 8. */
    padright = ( ( cols + 7 ) / 8 ) * 8 - cols;

    putinit( rows, cols );
    for ( row = 0; row < rows; ++row )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    putbit( *bP );
	for ( col = 0; col < padright; ++col )
	    putbit( 0 );
        }

    pm_close( ifp );

    putrest( );

    exit( 0 );
    }

static unsigned char item;
static int bitsperitem, bitshift;

static void
putinit( rows, cols )
    int rows, cols;
    {
    struct b_header head;

    head.magic[0] = 'y';
    head.magic[1] = 'z';
    head.h_wide = ( ( cols >> 6 ) & 0x3f ) + ' ';
    head.l_wide = ( cols & 0x3f ) + ' ';
    head.h_high = ( ( rows >> 6 ) & 0x3f ) + ' ';
    head.l_high = ( rows & 0x3f ) + ' ';
    head.depth = ( 1 & 0x3f ) + ' ';
    head._reserved = ' ';
    fwrite( &head, sizeof(head), 1, stdout );

    item = 0;
    bitsperitem = 0;
    bitshift = 7;
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
    if ( bitsperitem == 8 )
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
    bitshift = 7;
    }
