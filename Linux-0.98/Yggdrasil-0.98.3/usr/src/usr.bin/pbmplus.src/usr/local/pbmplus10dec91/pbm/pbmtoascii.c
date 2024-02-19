/* pbmtoascii.c - read a portable bitmap and produce ASCII graphics
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

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    bit** bits;
    register bit* bP;
    register bit* b1P;
    int rows, cols, row, lastcol;
    register int col;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    bits = pbm_readpbm( ifp, &cols, &rows );

    pm_close( ifp );
    
    /* Write out rows by twos. */
    for ( row = 0; row < rows; row += 2 )
	{
	/* Find end of lines. */
	for ( lastcol = cols-1; lastcol > 0; --lastcol )
	    {
	    if ( bits[row][lastcol] == PBM_BLACK )
		break;
	    if ( row+1 < rows && bits[row+1][lastcol] == PBM_BLACK )
		break;
	    }
        for ( col = 0, bP = bits[row], b1P = bits[row+1]; col <= lastcol; ++col, ++bP, ++b1P )
	    {
	    if ( *bP == PBM_WHITE )
		{
		if ( row+1 >= rows || *b1P == PBM_WHITE )
		    putchar( ' ' );
		else
		    putchar( 'o' );
		}
	    else
		{
		if ( row+1 >= rows || *b1P == PBM_WHITE )
		    putchar( '"' );
		else
		    putchar( '$' );
		}
	    }
	putchar( '\n' );
        }

    exit( 0 );
    }
