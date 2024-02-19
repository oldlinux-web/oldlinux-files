/* pbmtoeps.c - read a portable bitmap and produce Epson graphics
**
** Copyright (C) 1990 by John Tiller (tiller@galois.msfc.nasa.gov)
**			 and Jef Poskanzer.
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

static int mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit** bits;
    bit* bP[8];
    int rows, cols, row, col, lastcol;
    int idx;
    int val;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    bits = pbm_readpbm( ifp, &cols, &rows );

    pm_close( ifp );
    
    /* Change line spacing to 8/72 inches. */
    printf("\033A\010");
    /* Write out rows by eights. */
    for ( row = 0; row < rows; row += 8 )
	{
	/* Find end of lines. */
	for ( lastcol = cols-1; lastcol >= 0; --lastcol )
	    {
	    for ( idx = 0; idx < 8 && row+idx < rows; ++idx )
		if ( bits[row+idx][lastcol] == PBM_BLACK )
		    break;
	    if ( idx < 8 && row+idx < rows )
		break;
	    }
	for ( idx = 0; idx < 8 && row+idx < rows; ++idx )
	    bP[idx] = bits[row+idx];
	/* Put in plotter (1:1) graphics. */
	if ( lastcol >= 0 )
	    printf("\033*\005%c%c", (lastcol+1)%256, (lastcol+1)/256);
        for ( col = 0; col <= lastcol; ++col )
	    {
	    val = 0;
	    for ( idx = 0; idx < 8 && row+idx < rows; ++idx )
		if ( *bP[idx] == PBM_BLACK )
		    val |= mask[idx];
	    putchar( val );
	    for ( idx = 0; idx < 8 && row+idx < rows; ++idx )
		++bP[idx];
	    }
	putchar( '\n' );
        }
    putchar( '\f' );
    /* Restore normal line spacing. */
    printf("\033@");
    exit( 0 );
    }
