/* pnmshear.c - read a portable anymap and shear it by some angle
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
#include <math.h>
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif /*M_PI*/

#define SCALE 4096
#define HALFSCALE 2048

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel* xelrow;
    register xel* newxelrow;
    register xel* xP;
    register xel* nxP;
    xel bgxel, prevxel;
    int argn, rows, cols, format, newformat, newcols, row, col;
    xelval maxval;
    int antialias;
    float fangle, shearfac, new0;
    int intnew0;
    register long fracnew0, omfracnew0;
    char* usage = "[-noantialias] <angle> [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    antialias = 1;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' &&
	 ( argv[argn][1] < '0' || argv[argn][1] > '9' ) )
        {
        if ( pm_keymatch( argv[argn], "-antialias", 2 ) )
            antialias = 1;
        else if ( pm_keymatch( argv[argn], "-noantialias", 2 ) )
            antialias = 0;
        else
            pm_usage( usage );
        ++argn;
        }

    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%f", &fangle ) != 1 )
	pm_usage( usage );
    ++argn;
    if ( fangle <= -90.0 || fangle >= 90.0 )
	pm_error( "angle must be between -90 and 90" );
    fangle = fangle * M_PI / 180.0;	/* convert to radians */
    shearfac = tan( fangle );
    if ( shearfac < 0.0 )
	shearfac = -shearfac;

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

    /* Promote PBM files to PGM. */
    if ( antialias && PNM_FORMAT_TYPE(format) == PBM_TYPE )
	{
	newformat = PGM_TYPE;
	pm_message( "promoting from PBM to PGM - use -noantialias to avoid this" );
	}
    else
	newformat = format;

    newcols = rows * shearfac + cols + 0.999999;

    pnm_writepnminit( stdout, newcols, rows, maxval, newformat, 0 );
    newxelrow = pnm_allocrow( newcols );

    bgxel = pnm_backgroundxelrow( xelrow, cols, maxval, format );

    for ( row = 0; row < rows; ++row )
	{
	pnm_readpnmrow( ifp, xelrow, cols, maxval, format );

	if ( fangle > 0.0 )
	    new0 = row * shearfac;
	else
	    new0 = ( rows - row ) * shearfac;
	intnew0 = (int) new0;

	if ( antialias )
	    {
	    fracnew0 = ( new0 - intnew0 ) * SCALE;
	    omfracnew0 = SCALE - fracnew0;

	    for ( col = 0, nxP = newxelrow; col < newcols; ++col, ++nxP )
		*nxP = bgxel;

	    prevxel = bgxel;
	    for ( col = 0, nxP = &(newxelrow[intnew0]), xP = xelrow; col < cols; ++col, ++nxP, ++xP )
		{
		switch ( PNM_FORMAT_TYPE(format) )
		    {
		    case PPM_TYPE:
		    PPM_ASSIGN( *nxP,
			( fracnew0 * PPM_GETR(prevxel) + omfracnew0 * PPM_GETR(*xP) + HALFSCALE ) / SCALE,
			( fracnew0 * PPM_GETG(prevxel) + omfracnew0 * PPM_GETG(*xP) + HALFSCALE ) / SCALE,
			( fracnew0 * PPM_GETB(prevxel) + omfracnew0 * PPM_GETB(*xP) + HALFSCALE ) / SCALE );
		    break;

		    default:
		    PNM_ASSIGN1( *nxP,
			( fracnew0 * PNM_GET1(prevxel) + omfracnew0 * PNM_GET1(*xP) + HALFSCALE ) / SCALE );
		    break;
		    }
		prevxel = *xP;
		}
	    if ( fracnew0 > 0 )
		{
		nxP = &(newxelrow[intnew0 + cols]);
		switch ( PNM_FORMAT_TYPE(format) )
		    {
		    case PPM_TYPE:
		    PPM_ASSIGN( *nxP,
			( fracnew0 * PPM_GETR(prevxel) + omfracnew0 * PPM_GETR(bgxel) + HALFSCALE ) / SCALE,
			( fracnew0 * PPM_GETG(prevxel) + omfracnew0 * PPM_GETG(bgxel) + HALFSCALE ) / SCALE,
			( fracnew0 * PPM_GETB(prevxel) + omfracnew0 * PPM_GETB(bgxel) + HALFSCALE ) / SCALE );
		    break;

		    default:
		    PNM_ASSIGN1( *nxP,
			( fracnew0 * PNM_GET1(prevxel) + omfracnew0 * PNM_GET1(bgxel) + HALFSCALE ) / SCALE );
		    break;
		    }
		}
	    }
	else
	    {
	    for ( col = 0, nxP = newxelrow; col < intnew0; ++col, ++nxP )
		*nxP = bgxel;
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++nxP, ++xP )
		*nxP = *xP;
	    for ( col = intnew0 + cols; col < newcols; ++col, ++nxP )
		*nxP = bgxel;
	    }

	pnm_writepnmrow( stdout, newxelrow, newcols, maxval, newformat, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
