/* pnmenlarge.c - read a portable anymap and enlarge it N times
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

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel* xelrow;
    xel* newxelrow;
    register xel *xP;
    register xel *nxP;
    int argn, n, rows, cols, format, row;
    register int col, subrow, subcol;
    xelval maxval;
    char* usage = "N [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;

    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &n ) != 1 )
	pm_usage( usage );
    if ( n < 2 )
	pm_error( "N must be greater than 1" );
    ++argn;

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
    xelrow = pnm_allocrow( cols );
    pnm_writepnminit( stdout, cols * n, rows * n, maxval, format, 0 );
    newxelrow = pnm_allocrow( cols * n );

    for ( row = 0; row < rows; ++row )
	{
	pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	for ( subrow = 0; subrow < n; ++subrow )
	    {
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		for ( subcol = 0, nxP = &(newxelrow[col * n]);
		      subcol < n; ++subcol, ++nxP )
		    *nxP = *xP;
		}
	    pnm_writepnmrow( stdout, newxelrow, cols * n, maxval, format, 0 );
	    }
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
