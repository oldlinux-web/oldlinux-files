/*
** tifftopnm.c - converts a Tagged Image File to a portable anymap
**
** Derived by Jef Poskanzer from tif2ras.c, which is:
**
** Copyright (c) 1990 by Sun Microsystems, Inc.
**
** Author: Patrick J. Naughton
** naughton@wind.sun.com
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted,
** provided that the above copyright notice appear in all copies and that
** both that copyright notice and this permission notice appear in
** supporting documentation.
**
** This file is provided AS IS with no warranties of any kind.  The author
** shall have no liability with respect to the infringement of copyrights,
** trade secrets or any patents by this file or any part thereof.  In no
** event will the author be liable for any lost revenue or profits or
** other special, indirect and consequential damages.
*/

#include "pnm.h"
#include <tiffioP.h>

#define MAXCOLORS 1024

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    int argn, cols, rows, grayscale, format;
    int numcolors;
    register TIFF* tif;
    int row, i;
    register int col;
    u_char* buf;
    register u_char* inP;
    int maxval;
    xel* xelrow;
    register xel* xP;
    xel colormap[MAXCOLORS];
    int headerdump;
    register u_char sample;
    register int bitsleft;
    unsigned short bps, spp, photomet;
    unsigned short* redcolormap;
    unsigned short* greencolormap;
    unsigned short* bluecolormap;
    char* usage = "[-headerdump] [tifffile]";

    pnm_init( &argc, argv );

    argn = 1;
    headerdump = 0;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-headerdump", 2 ) )
	    headerdump = 1;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	tif = TIFFOpen( argv[argn], "r" );
	if ( tif == NULL )
	    pm_error( "error opening TIFF file %s", argv[argn] );
	++argn;
	}
    else
	{
	tif = TIFFFdOpen( 0, "Standard Input", "r" );
	if ( tif == NULL )
	    pm_error( "error opening standard input as TIFF file" );
	}

    if ( argn != argc )
	pm_usage( usage );

    if ( headerdump )
	TIFFPrintDirectory( tif, stderr, TIFFPRINT_NONE );

    if ( ! TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &bps ) )
	bps = 1;
    if ( ! TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &spp ) )
	spp = 1;
    if ( ! TIFFGetField( tif, TIFFTAG_PHOTOMETRIC, &photomet ) )
	pm_error( "error getting photometric" );

    switch ( spp )
	{
	case 1:
	case 3:
	case 4:
	break;

	default:
	pm_error(
	    "can only handle 1-channel gray scale or 1- or 3-channel color" );
	}

    (void) TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &cols );
    (void) TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &rows );

    if ( headerdump )
	{
	pm_message( "%dx%dx%d image", cols, rows, bps * spp );
	pm_message( "%d bits/sample, %d samples/pixel", bps, spp );
	}

    maxval = ( 1 << bps ) - 1;
    if ( maxval == 1 && spp == 1 )
	{
	if ( headerdump )
	    pm_message("monochrome" );
	grayscale = 1;
	}
    else
	{
	switch ( photomet )
	    {
	    case PHOTOMETRIC_MINISBLACK:
	    if ( headerdump )
		pm_message( "%d graylevels (min=black)", maxval + 1 );
	    grayscale = 1;
	    break;

	    case PHOTOMETRIC_MINISWHITE:
	    if ( headerdump )
		pm_message( "%d graylevels (min=white)", maxval + 1 );
	    grayscale = 1;
	    break;

	    case PHOTOMETRIC_PALETTE:
	    if ( headerdump )
		pm_message( "colormapped" );
	    if ( ! TIFFGetField( tif, TIFFTAG_COLORMAP, &redcolormap, &greencolormap, &bluecolormap ) )
		pm_error( "error getting colormaps" );
	    numcolors = maxval + 1;
	    if ( numcolors > MAXCOLORS )
		pm_error( "too many colors" );
	    maxval = PNM_MAXMAXVAL;
	    grayscale = 0;
	    for ( i = 0; i < numcolors; ++i )
		{
		register xelval r, g, b;
		r = (long) redcolormap[i] * PNM_MAXMAXVAL / 65535L;
		g = (long) greencolormap[i] * PNM_MAXMAXVAL / 65535L;
		b = (long) bluecolormap[i] * PNM_MAXMAXVAL / 65535L;
		PPM_ASSIGN( colormap[i], r, g, b );
		}
	    break;

	    case PHOTOMETRIC_RGB:
	    if ( headerdump )
		pm_message( "truecolor" );
	    grayscale = 0;
	    break;

	    case PHOTOMETRIC_MASK:
	    pm_error( "don't know how to handle PHOTOMETRIC_MASK" );

	    case PHOTOMETRIC_DEPTH:
	    pm_error( "don't know how to handle PHOTOMETRIC_DEPTH" );

	    default:
	    pm_error( "unknown photometric: %d", photomet );
	    }
	}
    if ( maxval > PNM_MAXMAXVAL )
	pm_error(
"bits/sample is too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS" );


    if ( grayscale )
	{
	if ( maxval == 1 )
	    {
	    format = PBM_TYPE;
	    pm_message( "writing PBM file" );
	    }
	else
	    {
	    format = PGM_TYPE;
	    pm_message( "writing PGM file" );
	    }
	}
    else
	{
	format = PPM_TYPE;
	pm_message( "writing PPM file" );
	}

    buf = (u_char*) malloc(TIFFScanlineSize(tif));
    if ( buf == NULL )
	pm_error( "can't allocate memory for scanline buffer" );
    pnm_writepnminit( stdout, cols, rows, (xelval) maxval, format, 0 );
    xelrow = pnm_allocrow( cols );

#define NEXTSAMPLE \
    { \
    if ( bitsleft == 0 ) \
	{ \
	++inP; \
	bitsleft = 8; \
	} \
    bitsleft -= bps; \
    sample = ( *inP >> bitsleft ) & maxval; \
    }

    for ( row = 0; row < rows; ++row )
	{
	if ( TIFFReadScanline( tif, buf, row, 0 ) < 0 )
	    pm_error( "bad data read on line %d", row );
	inP = buf;
	bitsleft = 8;
	xP = xelrow;

	switch ( photomet )
	    {
	    case PHOTOMETRIC_MINISBLACK:
	    for ( col = 0; col < cols; ++col, ++xP )
		{
		NEXTSAMPLE
		PNM_ASSIGN1( *xP, sample );
		}
	    break;

	    case PHOTOMETRIC_MINISWHITE:
	    for ( col = 0; col < cols; ++col, ++xP )
		{
		NEXTSAMPLE
		sample = maxval - sample;
		PNM_ASSIGN1( *xP, sample );
		}
	    break;

	    case PHOTOMETRIC_PALETTE:
	    for ( col = 0; col < cols; ++col, ++xP )
		{
		NEXTSAMPLE
		*xP = colormap[sample];
		}
	    break;

	    case PHOTOMETRIC_RGB:
	    for ( col = 0; col < cols; ++col, ++xP )
		{
		register xelval r, g, b;

		NEXTSAMPLE
		r = sample;
		NEXTSAMPLE
		g = sample;
		NEXTSAMPLE
		b = sample;
		if ( spp == 4 )
		    NEXTSAMPLE		/* skip alpha channel */
		PPM_ASSIGN( *xP, r, g, b );
		}
	    break;

	    default:
	    pm_error( "unknown photometric: %d", photomet );
	    }
	pnm_writepnmrow( stdout, xelrow, cols, (xelval) maxval, format, 0 );
	}

    pm_close( stdout );
    exit( 0 );
    }
