/* pbmtocmuwm.c - read a portable bitmap and produce a CMU window manager bitmap
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
#include "cmuwm.h"

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
    for ( row = 0; row < rows; row++ )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );
        for ( col = 0, bP = bitrow; col < cols; col++, bP++ )
	    putbit( *bP );
	for ( col = 0; col < padright; col++ )
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
    if ( pm_writebiglong( stdout, CMUWM_MAGIC ) == -1 )
	pm_error( "write error" );
    if ( pm_writebiglong( stdout, cols ) == -1 )
	pm_error( "write error" );
    if ( pm_writebiglong( stdout, rows ) == -1 )
	pm_error( "write error" );
    if ( pm_writebigshort( stdout, (short) 1 ) == -1 )
	pm_error( "write error" );

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
    if ( b == PBM_WHITE )
	item += 1 << bitshift;
    bitsperitem++;
    bitshift--;
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
    (void) putc( item, stdout );
    item = 0;
    bitsperitem = 0;
    bitshift = 7;
    }
