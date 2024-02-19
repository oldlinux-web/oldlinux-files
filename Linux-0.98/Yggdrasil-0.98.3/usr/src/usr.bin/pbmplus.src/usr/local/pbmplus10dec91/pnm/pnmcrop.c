/* pnmcrop.c - crop a portable anymap
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

#include "pnm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel** xels;
    register xel* xP;
    xel background;
    xelval maxval;
    int argn, format, backdefault, backblack;
    int rows, cols, row, col, newrows, newcols;
    int top, bottom, left, right;
    char* usage = "[-white|-black] [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    backdefault = 1;

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-white", 2 ) )
	    {
	    backdefault = 0;
	    backblack = 0;
	    }
	else if ( pm_keymatch( argv[argn], "-black", 2 ) )
	    {
	    backdefault = 0;
	    backblack = 1;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    xels = pnm_readpnm( ifp, &cols, &rows, &maxval, &format );
    pm_close( ifp );

    if ( backdefault )
	background = pnm_backgroundxel( xels, cols, rows, maxval, format );
    else
	if ( backblack )
	    background = pnm_blackxel( maxval, format );
	else
	    background = pnm_whitexel( maxval, format );

    /* Find first non-background line. */
    for ( top = 0; top < rows; top++ )
	for ( col = 0, xP = xels[top]; col < cols; col++, xP++ )
	    if ( ! PNM_EQUAL( *xP, background ) )
		goto gottop;
gottop:

    /* Find last non-background line. */
    for ( bottom = rows - 1; bottom >= top; bottom-- )
	for ( col = 0, xP = xels[bottom]; col < cols; col++, xP++ )
	    if ( ! PNM_EQUAL( *xP, background ) )
		goto gotbottom;
gotbottom:

    /* Find first non-background column.  To avoid massive paging on
    ** large anymaps, we use a different loop than the above two cases. */
    left = cols - 1;
    for ( row = top; row <= bottom; row++ )
	{
	int thisleft;

	for ( thisleft = 0; thisleft < left; thisleft++ )
	    if ( ! PNM_EQUAL( xels[row][thisleft], background ) )
		{
		left = thisleft;
		break;
		}
	}

    /* Find last non-background column.  Again, use row-major loop. */
    right = left + 1;
    for ( row = top; row <= bottom; row++ )
	{
	int thisright;

	for ( thisright = cols - 1; thisright > right; thisright-- )
	    if ( ! PNM_EQUAL( xels[row][thisright], background ) )
		{
		right = thisright;
		break;
		}
	}

#define ending(n) (((n) > 1) ? "s" : "")
    if ( top > 0 )
	pm_message( "cropping %d row%s off the top", top, ending(top) );
    if ( bottom < rows - 1 )
	pm_message( "cropping %d row%s off the bottom", rows-1-bottom, ending(rows-1-bottom) );
    if ( left > 0 )
	pm_message( "cropping %d col%s off the left", left, ending(left) );
    if ( right < cols - 1 )
	pm_message( "cropping %d col%s off the right", cols-1-right, ending(cols-1-right) );

    /* Now write out the new anymap. */
    newcols = right - left + 1;
    newrows = bottom - top + 1;
    pnm_writepnminit( stdout, newcols, newrows, maxval, format, 0 );
    for ( row = top; row <= bottom; row++ )
	pnm_writepnmrow( stdout, &(xels[row][left]), newcols, maxval, format, 0 );
    pm_close( stdout );

    exit( 0 );
    }
