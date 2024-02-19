/* pnmgamma.c - perform gamma correction on a portable pixmap
**
** Copyright (C) 1991 by Bill Davidson and Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pnm.h"
#include <math.h>
#include <ctype.h>

static void buildgamma ARGS(( xelval table[], xelval maxval, double gamma ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel* xelrow;
    register xel* xP;
    xelval maxval;
    int argn, rows, cols, format, newformat, row;
    register int col;
    double rgamma, ggamma, bgamma;
    xelval* rtable;
    xelval* gtable;
    xelval* btable;
    char *usage = "<value> [pnmfile]\n\t\t<redvalue> <greenvalue> <bluevalue> [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;

    /* Parse gamma args. */
    if ( argc == 2 || argc == 3 )
	{
	rgamma = ggamma = bgamma = atof( argv[argn] );
	++argn;
	}
    else if ( argc == 4 || argc == 5 )
	{
	rgamma = atof( argv[argn] );
	++argn;
	ggamma = atof( argv[argn] );
	++argn;
	bgamma = atof( argv[argn] );
	++argn;
	}
    else
	pm_usage( usage );

    if ( rgamma <= 0.0 || ggamma <= 0.0 || bgamma <= 0.0 )
	pm_usage( usage );

    if ( argn != argc )
	{
        ifp = pm_openr( argv[argn] );
        ++argn;
	}
    else 
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pnm_pbmmaxval = PNM_MAXMAXVAL;  /* use larger value for better results */
    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );
    xelrow = pnm_allocrow( cols );

    /* Promote PBM files to PGM.  Not that it makes much sense to
    ** gamma-correct PBM files. */
    if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
	{
        newformat = PGM_TYPE;
	pm_message( "promoting to PGM" );
	}
    else
        newformat = format;

    if ( rgamma != ggamma || ggamma != bgamma )
	if ( PNM_FORMAT_TYPE(newformat) == PGM_TYPE )
	    {
	    newformat = PPM_TYPE;
	    pm_message( "promoting to PPM" );
	    }

    /* Allocate space for the tables. */
    rtable = (xelval*) malloc( (maxval+1) * sizeof(xelval) );
    gtable = (xelval*) malloc( (maxval+1) * sizeof(xelval) );
    btable = (xelval*) malloc( (maxval+1) * sizeof(xelval) );
    if ( rtable == 0 || gtable == 0 || btable == 0 )
	pm_error( "out of memory" );

    /* Build the gamma corection tables. */
    buildgamma( rtable, maxval, rgamma );
    buildgamma( gtable, maxval, ggamma );
    buildgamma( btable, maxval, bgamma );

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );
    for ( row = 0; row < rows; ++row )
	{
	pnm_readpnmrow( ifp, xelrow, cols, maxval, format );

	/* Promote to PPM if differing gammas were specified. */
	if ( rgamma != ggamma || ggamma != bgamma )
	    if ( PNM_FORMAT_TYPE(format) != PPM_TYPE &&
		 PNM_FORMAT_TYPE(newformat) == PPM_TYPE )
		pnm_promoteformatrow(
		    xelrow, cols, maxval, format, maxval, newformat );

	switch ( PNM_FORMAT_TYPE(newformat) )
	    {
	    case PPM_TYPE:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		register xelval r, g, b;

		r = PPM_GETR( *xP );
		g = PPM_GETG( *xP );
		b = PPM_GETB( *xP );
		r = rtable[r];
		g = gtable[g];
		b = btable[b];
		PPM_ASSIGN( *xP, r, g, b );
		}
	    break;

	    default:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		register xelval g;

		g = PNM_GET1( *xP );
		g = gtable[g];
		PNM_ASSIGN1( *xP, g );
		}
	    break;
	    }

	pnm_writepnmrow( stdout, xelrow, cols, maxval, newformat, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

/*
** Builds a gamma table of size maxval+1 for the given gamma value.
**
** This function depends on pow(3m).  If you don't have it, you can
** simulate it with '#define pow(x,y) exp((y)*log(x))' provided that
** you have the exponential function exp(3m) and the natural logarithm
** function log(3m).  I can't believe I actually remembered my log
** identities.
*/

#if __STDC__
static void
buildgamma( xelval table[], xelval maxval, double gamma )
#else /*__STDC__*/
static void
buildgamma( table, maxval, gamma )
    xelval table[], maxval;
    double gamma;
#endif /*__STDC__*/
    {
    register int i, v;
    double one_over_gamma, ind, q;

    one_over_gamma = 1.0 / gamma;
    q = (double) maxval;
    for ( i = 0 ; i <= (int) maxval; ++i )
	{
	ind = ( (double) i ) / q;
	v = ( q * pow( ind, one_over_gamma ) ) + 0.5;
	if ( v > (int) maxval )
	    v = maxval;
	table[i] = v;
	}
    }
