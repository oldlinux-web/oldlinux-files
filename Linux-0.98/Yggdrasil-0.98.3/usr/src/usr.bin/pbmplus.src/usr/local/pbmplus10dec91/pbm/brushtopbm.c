/* brushtopbm.c - read a doodle brush file and write a portable bitmap
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

static void getinit ARGS(( FILE* file, int* colsP, int* rowsP ));
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

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[brushfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    /* Compute padding to round cols up to the next multiple of 16. */
    padright = ( ( cols + 15 ) / 16 ) * 16 - cols;

    for ( row = 0; row < rows; ++row )
	{
	/* Get data. */
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    *bP = getbit( ifp );
	/* Discard line padding. */
        for ( col = 0; col < padright; ++col )
	    (void) getbit( ifp );
	/* Write row. */
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );
    
    exit( 0 );
    }


static int item, bitsperitem, bitshift;

static void
getinit( file, colsP, rowsP )
    FILE* file;
    int* colsP;
    int* rowsP;
    {
    int i;

    if ( getc( file ) != 1 )
	pm_error( "bad magic number 1" );
    if ( getc( file ) != 0 )
	pm_error( "bad magic number 2" );
    *colsP = getc( file ) << 8;
    *colsP += getc( file );
    *rowsP = getc( file ) << 8;
    *rowsP += getc( file );
    bitsperitem = 8;

    /* Junk rest of header. */
    for ( i = 0; i < 10; ++i )  /* 10 is just a guess at the header size */
	(void) getc( file );
    }

static bit
getbit( file )
    FILE* file;
    {
    bit b;

    if ( bitsperitem == 8 )
	{
	item = getc( file );
	bitsperitem = 0;
	bitshift = 7;
	}
    ++bitsperitem;
    b = ( ( item >> bitshift) & 1 ) ? PBM_WHITE : PBM_BLACK;
    --bitshift;
    return b;
    }
