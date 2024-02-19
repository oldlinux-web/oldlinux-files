/* pbmmask.c - create a mask bitmap from a portable bitmap
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

static void addflood ARGS(( int col, int row ));
static void flood ARGS(( void ));

static bit** bits;
static bit** mask;
static bit backcolor;
static int rows, cols;

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, expand, wcount;
    register int row, col;
    char* usage = "[-expand] [pbmfile]";

    pbm_init( &argc, argv );

    argn = 1;
    expand = 0;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-expand", 2 ) )
	    expand = 1;
	else if ( pm_keymatch( argv[argn], "-noexpand", 2 ) )
	    expand = 0;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn == argc )
	ifp = stdin;
    else
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}

    if ( argn != argc )
	pm_usage( usage );

    bits = pbm_readpbm( ifp, &cols, &rows );
    pm_close( ifp );
    mask = pbm_allocarray( cols, rows );

    /* Clear out the mask. */
    for ( row = 0; row < rows; ++row )
        for ( col = 0; col < cols; ++col )
	    mask[row][col] = PBM_BLACK;

    /* Figure out the background color, by counting along the edge. */
    wcount = 0;
    for ( row = 0; row < rows; ++row )
	{
	if ( bits[row][0] == PBM_WHITE )
	    ++wcount;
	if ( bits[row][cols - 1] == PBM_WHITE )
	    ++wcount;
	}
    for ( col = 1; col < cols - 1; ++col )
	{
	if ( bits[0][col] == PBM_WHITE )
	    ++wcount;
	if ( bits[rows - 1][col] == PBM_WHITE )
	    ++wcount;
	}
    if ( wcount >= rows + cols - 2 )
	backcolor = PBM_WHITE;
    else
	backcolor = PBM_BLACK;

    /* Flood the entire edge.  Probably the first call will be enough, but
    ** might as well be sure. */
    for ( col = cols - 3; col >= 2; col -= 2 )
	{
	addflood( col, rows - 1 );
	addflood( col, 0 );
	}
    for ( row = rows - 1; row >= 0; row -= 2 )
	{
	addflood( cols - 1, row );
	addflood( 0, row );
	}
    flood( );

    if ( ! expand )
	/* Done. */
	pbm_writepbm( stdout, mask, cols, rows, 0 );
    else
	{ /* Expand by one pixel. */
	register int srow, scol;
	bit** emask;

	emask = pbm_allocarray( cols, rows );

	for ( row = 0; row < rows; ++row )
	    for ( col = 0; col < cols; ++col )
		if ( mask[row][col] == PBM_BLACK )
		    emask[row][col] = PBM_BLACK;
		else
		    {
		    emask[row][col] = PBM_WHITE;
		    for ( srow = row - 1; srow <= row + 1; ++srow )
			for ( scol = col - 1; scol <= col + 1; ++scol )
			    if ( srow >= 0 && srow < rows &&
				 scol >= 0 && scol < cols &&
				 mask[srow][scol] == PBM_BLACK )
				{
				emask[row][col] = PBM_BLACK;
				break;
				}
		    }

	/* Done. */
	pbm_writepbm( stdout, emask, cols, rows, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }

static short* fcols;
static short* frows;
static int fstacksize = 0, fstackp = 0;

static void
addflood( col, row )
int col, row;
    {
    if ( bits[row][col] == backcolor && mask[row][col] == PBM_BLACK )
	{
	if ( fstackp >= fstacksize )
	    {
	    if ( fstacksize == 0 )
		{
		fstacksize = 1000;
		fcols = (short*) malloc( fstacksize * sizeof(short) );
		frows = (short*) malloc( fstacksize * sizeof(short) );
		if ( fcols == (short*) 0 || frows == (short*) 0 )
		    pm_error( "out of memory" );
		}
	    else
		{
		fstacksize *= 2;
		fcols = (short*) realloc(
		    (char*) fcols, fstacksize * sizeof(short) );
		frows = (short*) realloc(
		    (char*) frows, fstacksize * sizeof(short) );
		if ( fcols == (short*) 0 || frows == (short*) 0 )
		    pm_error( "out of memory" );
		}
	    }
	fcols[fstackp] = col;
	frows[fstackp] = row;
	++fstackp;
	}
    }

static void
flood( )
    {
    register int col, row, c;

    while ( fstackp > 0 )
	{
	--fstackp;
	col = fcols[fstackp];
	row = frows[fstackp];
	if ( bits[row][col] == backcolor && mask[row][col] == PBM_BLACK )
	    {
	    mask[row][col] = PBM_WHITE;
	    if ( row - 1 >= 0 )
		addflood( col, row - 1 );
	    if ( row + 1 < rows )
		addflood( col, row + 1 );
	    for ( c = col + 1; c < cols; ++c )
		{
		if ( bits[row][c] == backcolor && mask[row][c] == PBM_BLACK )
		    {
		    mask[row][c] = PBM_WHITE;
		    if ( row - 1 >= 0 && ( bits[row - 1][c - 1] != backcolor || mask[row - 1][c - 1] != PBM_BLACK ) )
			addflood( c, row - 1 );
		    if ( row + 1 < rows && ( bits[row + 1][c - 1] != backcolor || mask[row + 1][c - 1] != PBM_BLACK ) )
			addflood( c, row + 1 );
		    }
		else
		    break;
		}
	    for ( c = col - 1; c >= 0; --c )
		{
		if ( bits[row][c] == backcolor && mask[row][c] == PBM_BLACK )
		    {
		    mask[row][c] = PBM_WHITE;
		    if ( row - 1 >= 0 && ( bits[row - 1][c + 1] != backcolor || mask[row - 1][c + 1] != PBM_BLACK ) )
			addflood( c, row - 1 );
		    if ( row + 1 < rows && ( bits[row + 1][c + 1] != backcolor || mask[row + 1][c + 1] != PBM_BLACK ) )
			addflood( c, row + 1 );
		    }
		else
		    break;
		}
	    }
	}
    }
