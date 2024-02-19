/* pbmlife.c - read a portable bitmap and apply Conway's rules of Life to it
**
** Copyright (C) 1988,1 1991 by Jef Poskanzer.
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
    bit* prevrow;
    bit* thisrow;
    bit* nextrow;
    bit* temprow;
    register bit* newrow;
    int rows, cols, row;
    register int col, count;
    int format;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pbm_readpbminit( ifp, &cols, &rows, &format );
    prevrow = pbm_allocrow( cols );
    thisrow = pbm_allocrow( cols );
    nextrow = pbm_allocrow( cols );

    pbm_writepbminit( stdout, cols, rows, 0 );
    newrow = pbm_allocrow( cols );

    pbm_readpbmrow( ifp, nextrow, cols, format );

    for ( row = 0; row < rows; ++row )
	{
	temprow = prevrow;
	prevrow = thisrow;
	thisrow = nextrow;
	nextrow = temprow;
	if ( row < rows - 1 )
	    pbm_readpbmrow( ifp, nextrow, cols, format );

        for ( col = 0; col < cols; ++col )
	    {
	    /* Check the neighborhood, with an unrolled double loop. */
	    count = 0;
	    if ( row > 0 )
		{
		/* upper left */
		if ( col > 0 && prevrow[col - 1] == PBM_WHITE )
		    ++count;
		/* upper center */
		if ( prevrow[col] == PBM_WHITE )
		    ++count;
		/* upper right */
		if ( col < cols - 1 && prevrow[col + 1] == PBM_WHITE )
		    ++count;
		}
	    /* left */
	    if ( col > 0 && thisrow[col - 1] == PBM_WHITE )
		++count;
	    /* right */
	    if ( col < cols - 1 && thisrow[col + 1] == PBM_WHITE )
		++count;
	    if ( row < rows - 1 )
		{
		/* lower left */
		if ( col > 0 && nextrow[col - 1] == PBM_WHITE )
		    ++count;
		/* lower center */
		if ( nextrow[col] == PBM_WHITE )
		    ++count;
		/* lower right */
		if ( col < cols - 1 && nextrow[col + 1] == PBM_WHITE )
		    ++count;
		}

	    /* And compute the new value. */
	    if ( thisrow[col] == PBM_WHITE )
		if ( count == 2 || count == 3 )
		    newrow[col] = PBM_WHITE;
		else
		    newrow[col] = PBM_BLACK;
	    else
		if ( count == 3 )
		    newrow[col] = PBM_WHITE;
		else
		    newrow[col] = PBM_BLACK;
	    }
	pbm_writepbmrow( stdout, newrow, cols, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
