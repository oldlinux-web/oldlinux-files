/* pbmtoptx.c - read a portable bitmap and produce a Printronix printer file
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

static void putinit ARGS(( void ));
static void putbit ARGS(( bit b ));
static void putrest ARGS(( void ));
static void putitem ARGS(( void ));

void main( argc, argv )
int argc;
char *argv[];
    {
    FILE *ifp;
    register bit *bitrow, *bP;
    int rows, cols, format, row, col;
    char *usage = "[pbmfile]";

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( usage );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pbm_readpbminit( ifp, &cols, &rows, &format );
    bitrow = pbm_allocrow( cols );

    putinit( );
    for ( row = 0; row < rows; row++ )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );
        for ( col = 0, bP = bitrow; col < cols; col++, bP++ )
	    putbit( *bP );
	putrest( );
	putchar( 5 );
	putchar( '\n' );
        }

    pm_close( ifp );
    
    exit( 0 );
    }

static char item;
static int bitsperitem, bitshift;

static void
putinit( )
    {
    bitsperitem = 0;
    item = 64;
    bitshift = 0;
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
    if ( bitsperitem == 6 )
	putitem( );
    if ( b == PBM_BLACK )
	item += 1 << bitshift;
    bitsperitem++;
    bitshift++;
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
    putchar( item );
    bitsperitem = 0;
    item = 64;
    bitshift = 0;
    }
