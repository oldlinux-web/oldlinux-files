/* pnmflip.c - perform one or more flip operations on a portable anymap
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

#include "pnm.h"

static void leftright ARGS(( int* aP, int* bP, int* cP, int* dP, int* eP, int* fP ));
static void topbottom ARGS(( int* aP, int* bP, int* cP, int* dP, int* eP, int* fP ));
static void transpose ARGS(( int* aP, int* bP, int* cP, int* dP, int* eP, int* fP ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, cols, rows, format, newrows, newcols;
    int a, b, c, d, e, f;
    register int row, col, newrow, newcol;
    xelval maxval;
    void leftright(), topbottom(), transpose();
    char* usage = "[-leftright|-lr] [-topbottom|-tb] [-transpose|-xy]\n            [-rotate90|-r90|-ccw] [-rotate270|r270|-cw]\n            [-rotate180|-r180] [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;

    /* Just check the validity of arguments here. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-lr", 2 ) ||
	     pm_keymatch( argv[argn], "-leftright", 2 ) )
	    { }
	else if ( pm_keymatch( argv[argn], "-tb", 3 ) ||
	     pm_keymatch( argv[argn], "-topbottom", 3 ) )
	    { }
	else if ( pm_keymatch( argv[argn], "-xy", 2 ) ||
	     pm_keymatch( argv[argn], "-transpose", 3 ) )
	    { }
	else if ( pm_keymatch( argv[argn], "-r90", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate90", 8 ) ||
	     pm_keymatch( argv[argn], "-ccw", 3 ) )
	    { }
	else if ( pm_keymatch( argv[argn], "-r270", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate270", 8 ) ||
	     pm_keymatch( argv[argn], "-cw", 3 ) )
	    { }
	else if ( pm_keymatch( argv[argn], "-r180", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate180", 8 ) )
	    { }
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

    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );

    /* Now go through the flags again, this time accumulating transforms. */
    a = 1; b = 0;
    c = 0; d = 1;
    e = 0; f = 0;
    argn = 1;
    while ( argn < argc && argv[argn][0] == '-' )
	{
	if ( pm_keymatch( argv[argn], "-lr", 2 ) ||
	     pm_keymatch( argv[argn], "-leftright", 2 ) )
	    leftright( &a, &b, &c, &d, &e, &f );
	else if ( pm_keymatch( argv[argn], "-tb", 3 ) ||
	     pm_keymatch( argv[argn], "-topbottom", 3 ) )
	    topbottom( &a, &b, &c, &d, &e, &f );
	else if ( pm_keymatch( argv[argn], "-xy", 2 ) ||
	     pm_keymatch( argv[argn], "-transpose", 3 ) )
	    transpose( &a, &b, &c, &d, &e, &f );
	else if ( pm_keymatch( argv[argn], "-r90", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate90", 8 ) ||
	     pm_keymatch( argv[argn], "-ccw", 3 ) )
	    {
	    transpose( &a, &b, &c, &d, &e, &f );
	    topbottom( &a, &b, &c, &d, &e, &f );
	    }
	else if ( pm_keymatch( argv[argn], "-r270", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate270", 8 ) ||
	     pm_keymatch( argv[argn], "-cw", 3 ) )
	    {
	    transpose( &a, &b, &c, &d, &e, &f );
	    leftright( &a, &b, &c, &d, &e, &f );
	    }
	else if ( pm_keymatch( argv[argn], "-r180", 3 ) ||
	     pm_keymatch( argv[argn], "-rotate180", 8 ) )
	    {
	    leftright( &a, &b, &c, &d, &e, &f );
	    topbottom( &a, &b, &c, &d, &e, &f );
	    }
	else
	    pm_error( "shouldn't happen!" );
	++argn;
	}

    /* Okay, we've got a matrix. */
    newcols = abs( a ) * cols + abs( c ) * rows;
    newrows = abs( b ) * cols + abs( d ) * rows;

    if ( b == 0 && d == 1 && f == 0 )
	{
	/* In this case newrow is always equal to row, so we can do the
	** transform line by line and avoid in-memory buffering altogether.
	*/
	register xel* xelrow;
	register xel* newxelrow;
	register xel* xP;

	xelrow = pnm_allocrow( cols );
	newxelrow = pnm_allocrow( newcols );
	pnm_writepnminit( stdout, newcols, newrows, maxval, format, 0 );

	for ( row = 0; row < rows; ++row )
	    {
	    pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		/* Transform a point:
		**
		**            [ a b 0 ]
		**  [ x y 1 ] [ c d 0 ] = [ x2 y2 1 ]
		**            [ e f 1 ]
		*/
		newcol = a * col + c * row + e * ( newcols - 1 );
		newxelrow[newcol] = *xP;
		}
	    pnm_writepnmrow( stdout, newxelrow, newcols, maxval, format, 0 );
	    }
	}
#ifdef notdef
    else if ( a == 0 && b != 0 && c != 0 && d == 0 )
	{
	/* This case is likely to thrash.  Can't think of any fix at the
	** moment, though.
	*/
	}
#endif /* notdef */
    else
	{
	/* Generic case.  Read in the anymap a line at a time and transform
	** it into an in-memory array.
	*/
	register xel* xelrow;
	register xel** newxels;
	register xel* xP;

	xelrow = pnm_allocrow( cols );
	newxels = pnm_allocarray( newcols, newrows );

	for ( row = 0; row < rows; ++row )
	    {
	    pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		/* Transform a point:
		**
		**            [ a b 0 ]
		**  [ x y 1 ] [ c d 0 ] = [ x2 y2 1 ]
		**            [ e f 1 ]
		*/
		newcol = a * col + c * row + e * ( newcols - 1 );
		newrow = b * col + d * row + f * ( newrows - 1 );
		newxels[newrow][newcol] = *xP;
		}
	    }

	pnm_writepnm( stdout, newxels, newcols, newrows, maxval, format, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static void
leftright( aP, bP, cP, dP, eP, fP )
    int* aP;
    int* bP;
    int* cP;
    int* dP;
    int* eP;
    int* fP;
    {
    *aP = - *aP;
    *cP = - *cP;
    *eP = - *eP + 1;
    }

static void
topbottom( aP, bP, cP, dP, eP, fP )
    int* aP;
    int* bP;
    int* cP;
    int* dP;
    int* eP;
    int* fP;
    {
    *bP = - *bP;
    *dP = - *dP;
    *fP = - *fP + 1;
    }

static void
transpose( aP, bP, cP, dP, eP, fP )
    int* aP;
    int* bP;
    int* cP;
    int* dP;
    int* eP;
    int* fP;
    {
    register int t;

    t = *aP;
    *aP = *bP;
    *bP = t;
    t = *cP;
    *cP = *dP;
    *dP = t;
    t = *eP;
    *eP = *fP;
    *fP = t;
    }
