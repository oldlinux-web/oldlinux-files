/* mgrtopbm.c - read a MGR bitmap and produce a portable bitmap
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

static void getinit ARGS(( FILE* file, int* colsP, int* rowsP, int* depthP, int* padrightP ));
static bit getbit ARGS(( FILE* file ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, depth, padright, row, col;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[mgrfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows, &depth, &padright );
    if ( depth != 1 )
	pm_error( "MGR file has depth of %d, must be 1", depth );

    pbm_writepbminit( stdout, cols, rows, 0 );
    bitrow = pbm_allocrow( cols );

    for ( row = 0; row < rows; row++ )
	{
	/* Get data, bit-reversed within each byte. */
        for ( col = 0, bP = bitrow; col < cols; col++, bP++ )
	    *bP = getbit( ifp );
	/* Discard line padding */
        for ( col = 0; col < padright; col ++ )
	    (void) getbit( ifp );
	pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }


static unsigned char item;
static int bitsperitem, bitshift;

static void
getinit( file, colsP, rowsP, depthP, padrightP )
    FILE* file;
    int* colsP;
    int* rowsP;
    int* depthP;
    int* padrightP;
    {
    struct b_header head;
    int pad;

    if ( fread( &head, sizeof(struct old_b_header), 1, file ) != 1 )
	pm_perror( "reading header" );
    if ( head.magic[0] == 'y' && head.magic[1] == 'z' )
	{ /* new style bitmap */
	if ( fread( &head.depth, sizeof(head) - sizeof(struct old_b_header), 1, file ) != 1 )
	    pm_perror( "reading rest of header" );
	*depthP = (int) head.depth - ' ';
	pad = 8;
	}
    else if ( head.magic[0] == 'x' && head.magic[1] == 'z' )
	{ /* old style bitmap with 32-bit padding */
	*depthP = 1;
	pad = 32;
	}
    else if ( head.magic[0] == 'z' && head.magic[1] == 'z' )
	{ /* old style bitmap with 16-bit padding */
	*depthP = 1;
	pad = 16;
	}
    else if ( head.magic[0] == 'z' && head.magic[1] == 'y' )
	{ /* old style 8-bit pixmap with 16-bit padding */
	*depthP = 8;
	pad = 16;
	}
    else
	pm_error(
	    "bad magic chars in MGR file: '%c%c'",
	    head.magic[0], head.magic[1] );
    *colsP = ( ( (int) head.h_wide - ' ' ) << 6 ) + ( (int) head.l_wide - ' ' );
    *rowsP = ( ( (int) head.h_high - ' ' ) << 6 ) + ( (int) head.l_high - ' ' );
    *padrightP = ( ( *colsP + pad - 1 ) / pad ) * pad - *colsP;

    bitsperitem = 8;
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
    bitsperitem++;
    b = ( ( item >> bitshift) & 1 ) ? PBM_BLACK : PBM_WHITE;
    bitshift--;
    return b;
    }
