/* ybmtopbm.c - read a file from Bennet Yee's 'xbm' program and write a pbm.
**
** Written by Jamie Zawinski based on code (C) 1988 by Jef Poskanzer.
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

static void getinit ARGS(( FILE* file, short* colsP, short* rowsP, short* depthP, short* padrightP ));
static bit getbit ARGS(( FILE* file ));

#define YBM_MAGIC  ( ( '!' << 8 ) | '!' )

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    short rows, cols, padright, row, col;
    short depth;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[ybmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows, &depth, &padright );
    if ( depth != 1 )
	pm_error(
	    "YBM file has depth of %d, must be 1",
	    (int) depth );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    for ( row = 0; row < rows; ++row )
	{
	/* Get data. */
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    *bP = getbit( ifp );
	/* Discard line padding */
        for ( col = 0; col < padright; ++col )
	    (void) getbit( ifp );
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static int item;
static int bitsperitem, bitshift;

static void
getinit( file, colsP, rowsP, depthP, padrightP )
    FILE* file;
    short* colsP;
    short* rowsP;
    short* depthP;
    short* padrightP;
    {
    short magic;

    if ( pm_readbigshort( file, &magic ) == -1 )
	pm_error( "EOF / read error" );
    if ( magic != YBM_MAGIC )
	pm_error( "bad magic number in YBM file" );
    if ( pm_readbigshort( file, colsP ) == -1 )
	pm_error( "EOF / read error" );
      if ( pm_readbigshort( file, rowsP ) == -1 )
	pm_error( "EOF / read error" );

    *depthP = 1;
    *padrightP = ( ( *colsP + 15 ) / 16 ) * 16 - *colsP;
    bitsperitem = 0;
    }

static bit
getbit( file )
    FILE* file;
    {
    bit b;

    if ( bitsperitem == 0 )
	{
	item = getc(file) | getc(file)<<8;
	if ( item == EOF )
	    pm_error( "EOF / read error" );
	bitsperitem = 16;
	bitshift = 0;
	}
    b = ( ( item >> bitshift) & 1 ) ? PBM_BLACK : PBM_WHITE;
    --bitsperitem;
    ++bitshift;
    return b;
    }
