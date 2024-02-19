/* pnmscale.c - read a portable anymap and scale it
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

#include "pnm.h"

#define SCALE 4096
#define HALFSCALE 2048

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel* xelrow;
    xel* tempxelrow;
    xel* newxelrow;
    register xel* xP;
    register xel* nxP;
    int argn, specxscale, specyscale, specxsize, specysize, specxysize;
    int rows, cols, format, newformat, rowsread, newrows, newcols;
    register int row, col, needtoreadrow;
    xelval maxval;
    float xscale, yscale;
    long sxscale, syscale;
    register long fracrowtofill, fracrowleft;
    long* rs;
    long* gs;
    long* bs;
    char* usage = "<s> [pnmfile]\n            -xsize|width|-ysize|-height <s> [pnmfile]\n            -xscale|-yscale <s> [pnmfile]\n            -xscale|-xsize|-width <s> -yscale|-ysize|-height <s> [pnmfile]\n            -xysize <x> <y> [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    specxscale = specyscale = specxsize = specysize = specxysize = 0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-xscale", 4 ) )
	    {
	    if ( specxscale )
		pm_error( "already specified an x scale" );
	    if ( specxsize )
		pm_error(
		    "only one of -xsize/-width and -xscale may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &xscale ) != 1 )
		pm_usage( usage );
	    if ( xscale <= 0.0 )
		pm_error( "x scale must be greater than 0" );
	    specxscale = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-yscale", 4 ) )
	    {
	    if ( specyscale )
		pm_error( "already specified a y scale" );
	    if ( specysize )
		pm_error(
		    "only one of -ysize/-height and -yscale may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &yscale ) != 1 )
		pm_usage( usage );
	    if ( yscale <= 0.0 )
		pm_error( "y scale must be greater than 0" );
	    specyscale = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-xsize", 4 ) ||
	          pm_keymatch( argv[argn], "-width", 2 ) )
	    {
	    if ( specxsize )
		pm_error( "already specified a width" );
	    if ( specxscale )
		pm_error(
		    "only one of -xscale and -xsize/-width may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &newcols ) != 1 )
		pm_usage( usage );
	    if ( newcols <= 0 )
		pm_error( "new width must be greater than 0" );
	    specxsize = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-ysize", 4 ) ||
	          pm_keymatch( argv[argn], "-height", 2 ) )
	    {
	    if ( specysize )
		pm_error( "already specified a height" );
	    if ( specyscale )
		pm_error(
		    "only one of -yscale and -ysize/-height may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &newrows ) != 1 )
		pm_usage( usage );
	    if ( newrows <= 0 )
		pm_error( "new height must be greater than 0" );
	    specysize = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-xysize", 3 ) )
	    {
	    if ( specxsize || specysize || specxscale || specyscale )
		pm_error( "can't use -xysize with any other specifiers" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &newcols ) != 1 )
		pm_usage( usage );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &newrows ) != 1 )
		pm_usage( usage );
	    if ( newcols <= 0 || newrows <= 0 )
		pm_error( "new width and height must be greater than 0" );
	    specxsize = 1;
	    specysize = 1;
	    specxysize = 1;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( ! ( specxscale || specyscale || specxsize || specysize ) )
	{
	/* No flags specified, so a single scale factor is required. */
	if ( argn == argc )
	    pm_usage( usage );
	if ( sscanf( argv[argn], "%f", &xscale ) != 1 )
	    pm_usage( usage );
	if ( xscale <= 0.0 )
	    pm_error( "scale must be greater than 0" );
	++argn;
	yscale = xscale;
	specxscale = specyscale = 1;
	}

    /* Now get input file. */
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

    /* Promote PBM files to PGM. */
    if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
	{
        newformat = PGM_TYPE;
	pm_message( "promoting from PBM to PGM" );
	}
    else
        newformat = format;

    /* Compute all sizes and scales. */
    if ( specxysize )
	if ( (float) newcols / (float) newrows > (float) cols / (float) rows )
	    specxsize = 0;
	else
	    specysize = 0;

    if ( specxsize )
	xscale = (float) newcols / (float) cols;
    else if ( specxscale )
	newcols = cols * xscale + 0.999;

    if ( specysize )
	yscale = (float) newrows / (float) rows;
    else if ( specyscale )
	newrows = rows * yscale + 0.999;
    else
	if ( specxsize )
	    {
	    yscale = xscale;
	    newrows = rows * yscale + 0.999;
	    }
	else
	    {
	    yscale = 1.0;
	    newrows = rows;
	    }
    
    if ( ! ( specxsize || specxscale ) )
	if ( specysize )
	    {
	    xscale = yscale;
	    newcols = cols * xscale + 0.999;
	    }
	else
	    {
	    xscale = 1.0;
	    newcols = cols;
	    }

    sxscale = xscale * SCALE;
    syscale = yscale * SCALE;

    xelrow = pnm_allocrow( cols );
    if ( newrows == rows )	/* shortcut Y scaling if possible */
	tempxelrow = xelrow;
    else
	tempxelrow = pnm_allocrow( cols );
    rs = (long*) pm_allocrow( cols, sizeof(long) );
    gs = (long*) pm_allocrow( cols, sizeof(long) );
    bs = (long*) pm_allocrow( cols, sizeof(long) );
    rowsread = 0;
    fracrowleft = syscale;
    needtoreadrow = 1;
    for ( col = 0; col < cols; ++col )
	rs[col] = gs[col] = bs[col] = HALFSCALE;
    fracrowtofill = SCALE;

    pnm_writepnminit( stdout, newcols, newrows, maxval, newformat, 0 );
    newxelrow = pnm_allocrow( newcols );

    for ( row = 0; row < newrows; ++row )
	{
	/* First scale Y from xelrow into tempxelrow. */
	if ( newrows == rows )	/* shortcut Y scaling if possible */
	    {
	    pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
	    }
	else
	    {
	    while ( fracrowleft < fracrowtofill )
		{
		if ( needtoreadrow )
		    if ( rowsread < rows )
			{
			pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
			++rowsread;
			/* needtoreadrow = 0; */
			}
                switch ( PNM_FORMAT_TYPE(format) )
                    {
                    case PPM_TYPE:
		    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
			{
			rs[col] += fracrowleft * PPM_GETR( *xP );
			gs[col] += fracrowleft * PPM_GETG( *xP );
			bs[col] += fracrowleft * PPM_GETB( *xP );
			}
                    break;

                    default:
		    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
			gs[col] += fracrowleft * PNM_GET1( *xP );
                    break;
                    }
		fracrowtofill -= fracrowleft;
		fracrowleft = syscale;
		needtoreadrow = 1;
		}
	    /* Now fracrowleft is >= fracrowtofill, so we can produce a row. */
	    if ( needtoreadrow )
		if ( rowsread < rows )
		    {
		    pnm_readpnmrow( ifp, xelrow, cols, maxval, format );
		    ++rowsread;
		    needtoreadrow = 0;
		    }
	    switch ( PNM_FORMAT_TYPE(format) )
		{
		case PPM_TYPE:
		for ( col = 0, xP = xelrow, nxP = tempxelrow;
		      col < cols; ++col, ++xP, ++nxP )
		    {
		    register long r, g, b;

		    r = rs[col] + fracrowtofill * PPM_GETR( *xP );
		    g = gs[col] + fracrowtofill * PPM_GETG( *xP );
		    b = bs[col] + fracrowtofill * PPM_GETB( *xP );
		    r /= SCALE;
		    if ( r > maxval ) r = maxval;
		    g /= SCALE;
		    if ( g > maxval ) g = maxval;
		    b /= SCALE;
		    if ( b > maxval ) b = maxval;
		    PPM_ASSIGN( *nxP, r, g, b );
		    rs[col] = gs[col] = bs[col] = HALFSCALE;
		    }
		break;

		default:
		for ( col = 0, xP = xelrow, nxP = tempxelrow;
		      col < cols; ++col, ++xP, ++nxP )
		    {
		    register long g;

		    g = gs[col] + fracrowtofill * PNM_GET1( *xP );
		    g /= SCALE;
		    if ( g > maxval ) g = maxval;
		    PNM_ASSIGN1( *nxP, g );
		    gs[col] = HALFSCALE;
		    }
		break;
		}
	    fracrowleft -= fracrowtofill;
	    if ( fracrowleft == 0 )
		{
		fracrowleft = syscale;
		needtoreadrow = 1;
		}
	    fracrowtofill = SCALE;
	    }

	/* Now scale X from tempxelrow into newxelrow and write it out. */
	if ( newcols == cols )	/* shortcut X scaling if possible */
	    pnm_writepnmrow( stdout, tempxelrow, newcols, maxval, newformat, 0 );
	else
	    {
	    register long r, g, b;
	    register long fraccoltofill, fraccolleft;
	    register int needcol;

	    nxP = newxelrow;
	    fraccoltofill = SCALE;
	    r = g = b = HALFSCALE;
	    needcol = 0;
	    for ( col = 0, xP = tempxelrow; col < cols; ++col, ++xP )
		{
		fraccolleft = sxscale;
		while ( fraccolleft >= fraccoltofill )
		    {
		    if ( needcol )
			{
			++nxP;
			r = g = b = HALFSCALE;
			}
		    switch ( PNM_FORMAT_TYPE(format) )
			{
			case PPM_TYPE:
			r += fraccoltofill * PPM_GETR( *xP );
			g += fraccoltofill * PPM_GETG( *xP );
			b += fraccoltofill * PPM_GETB( *xP );
			r /= SCALE;
			if ( r > maxval ) r = maxval;
			g /= SCALE;
			if ( g > maxval ) g = maxval;
			b /= SCALE;
			if ( b > maxval ) b = maxval;
			PPM_ASSIGN( *nxP, r, g, b );
			break;

			default:
			g += fraccoltofill * PNM_GET1( *xP );
			g /= SCALE;
			if ( g > maxval ) g = maxval;
			PNM_ASSIGN1( *nxP, g );
			break;
			}
		    fraccolleft -= fraccoltofill;
		    fraccoltofill = SCALE;
		    needcol = 1;
		    }
		if ( fraccolleft > 0 )
		    {
		    if ( needcol )
			{
			++nxP;
			r = g = b = HALFSCALE;
			needcol = 0;
			}
		    switch ( PNM_FORMAT_TYPE(format) )
			{
			case PPM_TYPE:
			r += fraccolleft * PPM_GETR( *xP );
			g += fraccolleft * PPM_GETG( *xP );
			b += fraccolleft * PPM_GETB( *xP );
			break;

			default:
			g += fraccolleft * PNM_GET1( *xP );
			break;
			}
		    fraccoltofill -= fraccolleft;
		    }
		}
	    if ( fraccoltofill > 0 )
		{
		--xP;
		switch ( PNM_FORMAT_TYPE(format) )
		    {
		    case PPM_TYPE:
		    r += fraccoltofill * PPM_GETR( *xP );
		    g += fraccoltofill * PPM_GETG( *xP );
		    b += fraccoltofill * PPM_GETB( *xP );
		    break;

		    default:
		    g += fraccoltofill * PNM_GET1( *xP );
		    break;
		    }
		}
	    if ( ! needcol )
		{
                switch ( PNM_FORMAT_TYPE(format) )
                    {
                    case PPM_TYPE:
		    r /= SCALE;
		    if ( r > maxval ) r = maxval;
		    g /= SCALE;
		    if ( g > maxval ) g = maxval;
		    b /= SCALE;
		    if ( b > maxval ) b = maxval;
		    PPM_ASSIGN( *nxP, r, g, b );
                    break;

                    default:
		    g /= SCALE;
		    if ( g > maxval ) g = maxval;
		    PNM_ASSIGN1( *nxP, g );
                    break;
                    }
		}
	    pnm_writepnmrow( stdout, newxelrow, newcols, maxval, newformat, 0 );
	    }
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
