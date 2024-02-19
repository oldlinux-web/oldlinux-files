/* cmuwmtopbm.c - read a CMU window manager bitmap and produce a portable bitmap
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

static void getinit ARGS(( FILE* file, int* colsP, int* rowsP, short* depthP, int* padrightP ));
static bit getbit ARGS(( FILE* file ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, padright, row, col;
    short depth;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[cmuwmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows, &depth, &padright );
    if ( depth != 1 )
	pm_error(
	    "CMU window manager file has depth of %d, must be 1",
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

static int item, bitsperitem, bitshift;

static void
getinit( file, colsP, rowsP, depthP, padrightP )
    FILE* file;
    int* colsP;
    int* rowsP;
    short* depthP;
    int* padrightP;
    {
    long l;

    if ( pm_readbiglong( file, &l ) == -1 )
	pm_error( "EOF / read error" );
    if ( l != CMUWM_MAGIC )
	pm_error( "bad magic number in CMU window manager file" );
    if ( pm_readbiglong( file, &l ) == -1 )
	pm_error( "EOF / read error" );
    *colsP = (int) l;
    if ( pm_readbiglong( file, &l ) == -1 )
	pm_error( "EOF / read error" );
    *rowsP = (int) l;
    if ( pm_readbigshort( file, depthP ) == -1 )
	pm_error( "EOF / read error" );
    *padrightP = ( ( *colsP + 7 ) / 8 ) * 8 - *colsP;

    bitsperitem = 0;
    }

static bit
getbit( file )
    FILE* file;
    {
    bit b;

    if ( bitsperitem == 0 )
	{
	item = getc( file );
	if ( item == EOF )
	    pm_error( "EOF / read error" );
	bitsperitem = 8;
	bitshift = 7;
	}
    b = ( ( item >> bitshift) & 1 ) ? PBM_WHITE : PBM_BLACK;
    --bitsperitem;
    --bitshift;
    return b;
    }
