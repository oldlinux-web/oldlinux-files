/*
** pnmtotiff.c - converts a portable anymap to a Tagged Image File
**
** Derived by Jef Poskanzer from ras2tif.c, which is:
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

#include "ppmcmap.h"
#define MAXCOLORS 256

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    int argn;
    char* inf = NULL;
    FILE* ifp;
    xel** xels;
    register xel* xP;
    colorhist_vector chv;
    colorhash_table cht;
    unsigned short red[MAXCOLORS], grn[MAXCOLORS], blu[MAXCOLORS];
    int cols, rows, format, row, colors, i;
    register int col;
    xelval maxval;
    int grayscale;
    TIFF* tif;
    long g3options;
    long rowsperstrip;
    unsigned short compression;
    unsigned short fillorder;
    unsigned short predictor;
    short photometric;
    short samplesperpixel;
    short bitspersample;
    int bytesperrow;
    unsigned char* buf;
    unsigned char* tP;
    char* usage = "[-none|-packbits|-lzw|-g3|-g4] [-msb2lsb|-lsb2msb] [-2d] [-fill] [-predictor n] [-rowsperstrip n] [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    compression = COMPRESSION_LZW;
    g3options = 0;
    fillorder = FILLORDER_MSB2LSB;
    predictor = 0;
    rowsperstrip = 0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-none", 2 ) )
	    compression = COMPRESSION_NONE;
	else if ( pm_keymatch( argv[argn], "-packbits", 3 ) ) 
	    compression = COMPRESSION_PACKBITS;
	else if ( pm_keymatch( argv[argn], "-lzw", 3 ) ) 
	    compression = COMPRESSION_LZW;
	else if ( pm_keymatch( argv[argn], "-g3", 3 ) ) 
	    compression = COMPRESSION_CCITTFAX3;
	else if ( pm_keymatch( argv[argn], "-g4", 3 ) ) 
	    compression = COMPRESSION_CCITTFAX4;
	else if ( pm_keymatch( argv[argn], "-msb2lsb", 3 ) )
	    fillorder = FILLORDER_MSB2LSB;
	else if ( pm_keymatch( argv[argn], "-lsb2msb", 3 ) )
	    fillorder = FILLORDER_LSB2MSB;
	else if ( pm_keymatch( argv[argn], "-2d", 2 ) )
	    g3options |= GROUP3OPT_2DENCODING;
	else if ( pm_keymatch( argv[argn], "-fill", 2 ) )
	    g3options |= GROUP3OPT_FILLBITS;
	else if ( pm_keymatch( argv[argn], "-predictor", 3) )
	    {
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%hu", &predictor ) != 1 )
		pm_usage( usage );
	    if ( predictor != 1 && predictor != 2 )
		pm_usage( usage );
	    }
	else if ( pm_keymatch( argv[argn], "-rowsperstrip", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc ||
		 sscanf( argv[argn], "%ld", &rowsperstrip ) != 1 )
		pm_usage( usage );
	    if ( rowsperstrip < 1 )
		pm_usage( usage );
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	inf = argv[argn];
	ifp = pm_openr( inf );
	++argn;
	}
    else
	{
	inf = "Standard Input";
	ifp = stdin;
	}

    if ( argn != argc )
	pm_usage( usage );

    xels = pnm_readpnm( ifp, &cols, &rows, &maxval, &format );
    pm_close( ifp );

    /* Check for grayscale. */
    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	pm_message( "computing colormap..." );
	chv = ppm_computecolorhist( xels, cols, rows, MAXCOLORS, &colors );
	if ( chv == (colorhist_vector) 0 )
	    {
	    pm_message(
		"Too many colors - proceeding to write a 24-bit RGB file." );
	    pm_message(
		"If you want an 8-bit palette file, try doing a 'ppmquant %d'.",
		MAXCOLORS );
	    grayscale = 0;
	    }
	else
	    {
	    pm_message( "%d colors found", colors );
	    grayscale = 1;
	    for ( i = 0; i < colors; ++i )
		{
		register xelval r, g, b;

		r = PPM_GETR( chv[i].color );
		g = PPM_GETG( chv[i].color );
		b = PPM_GETB( chv[i].color );
		if ( r != g || g != b )
		    {
		    grayscale = 0;
		    break;
		    }
		}
	    }
	break;

	default:
	chv = (colorhist_vector) 0;
	grayscale = 1;
	break;
	}

    /* Open output file. */
    tif = TIFFFdOpen( 1, "Standard Output", "w" );
    if ( tif == NULL )
	pm_error( "error opening standard output as TIFF file" );

    /* Figure out TIFF parameters. */
    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	if ( chv == (colorhist_vector) 0 )
	    {
	    samplesperpixel = 3;
	    bitspersample = 8;
	    photometric = PHOTOMETRIC_RGB;
	    bytesperrow = cols * 3;
	    }
	else if ( grayscale )
	    {
	    samplesperpixel = 1;
	    bitspersample = pm_maxvaltobits( maxval );
	    photometric = PHOTOMETRIC_MINISBLACK;
	    bytesperrow = ( cols + i - 1 ) / i;
	    }
	else
	    {
	    samplesperpixel = 1;
	    bitspersample = 8;
	    photometric = PHOTOMETRIC_PALETTE;
	    bytesperrow = cols;
	    }
	break;

	case PGM_TYPE:
	samplesperpixel = 1;
	bitspersample = pm_maxvaltobits( maxval );
	photometric = PHOTOMETRIC_MINISBLACK;
	i = 8 / bitspersample;
	bytesperrow = ( cols + i - 1 ) / i;
	break;

	default:
	samplesperpixel = 1;
	bitspersample = 1;
	photometric = PHOTOMETRIC_MINISBLACK;
	bytesperrow = ( cols + 7 ) / 8;
	break;
	}

    if ( rowsperstrip == 0 )
	rowsperstrip = ( 8 * 1024 ) / bytesperrow;
    buf = (unsigned char*) malloc( bytesperrow );
    if ( buf == (unsigned char*) 0 )
	pm_error( "can't allocate memory for row buffer" );

    /* Set TIFF parameters. */
    TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, cols );
    TIFFSetField( tif, TIFFTAG_IMAGELENGTH, rows );
    TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, bitspersample );
    TIFFSetField( tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
    TIFFSetField( tif, TIFFTAG_COMPRESSION, compression );
    if ( compression == COMPRESSION_CCITTFAX3 && g3options != 0 )
	TIFFSetField( tif, TIFFTAG_GROUP3OPTIONS, g3options );
    if ( compression == COMPRESSION_LZW && predictor != 0 )
	TIFFSetField( tif, TIFFTAG_PREDICTOR, predictor );
    TIFFSetField( tif, TIFFTAG_PHOTOMETRIC, photometric );
    TIFFSetField( tif, TIFFTAG_FILLORDER, fillorder );
    TIFFSetField( tif, TIFFTAG_DOCUMENTNAME, inf );
    TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, "converted PNM file" );
    TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel );
    TIFFSetField( tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip );
    /* TIFFSetField( tif, TIFFTAG_STRIPBYTECOUNTS, rows / rowsperstrip ); */
    TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );

    if ( chv == (colorhist_vector) 0 )
	cht = (colorhash_table) 0;
    else
	{
	/* Make TIFF colormap. */
	for ( i = 0; i < colors; ++i )
	    {
	    red[i] = (long) PPM_GETR( chv[i].color ) * 65535L / maxval;
	    grn[i] = (long) PPM_GETG( chv[i].color ) * 65535L / maxval;
	    blu[i] = (long) PPM_GETB( chv[i].color ) * 65535L / maxval;
	    }
	TIFFSetField( tif, TIFFTAG_COLORMAP, red, grn, blu );

	/* Convert color vector to color hash table, for fast lookup. */
	cht = ppm_colorhisttocolorhash( chv, colors );
	ppm_freecolorhist( chv );
	}

    /* Now write the TIFF data. */
    for ( row = 0; row < rows; ++row )
	{
	if ( PNM_FORMAT_TYPE(format) == PPM_TYPE && ! grayscale )
	    {
	    if ( cht == (colorhash_table) 0 )
		{
		for ( col = 0, xP = xels[row], tP = buf;
		      col < cols; ++col, ++xP )
		    {
		    register unsigned char s;

		    s = PPM_GETR( *xP );
		    if ( maxval != 255 )
			s = (long) s * 255 / maxval;
		    *tP++ = s;
		    s = PPM_GETG( *xP );
		    if ( maxval != 255 )
			s = (long) s * 255 / maxval;
		    *tP++ = s;
		    s = PPM_GETB( *xP );
		    if ( maxval != 255 )
			s = (long) s * 255 / maxval;
		    *tP++ = s;
		    }
		}
	    else
		{
		for ( col = 0, xP = xels[row], tP = buf;
		      col < cols; ++col, ++xP )
		    {
		    register int s;

		    s = ppm_lookupcolor( cht, xP );
		    if ( s == -1 )
			pm_error(
			    "color not found?!?  row=%d col=%d  r=%d g=%d b=%d",
			    row, col, PPM_GETR( *xP ), PPM_GETG( *xP ),
			    PPM_GETB( *xP ) );
		    *tP++ = (unsigned char) s;
		    }
		}
	    }
	else
	    {
	    register xelval bigger_maxval;
	    register int bitshift;
	    register unsigned char byte;
	    register xelval s;

	    bigger_maxval = pm_bitstomaxval( bitspersample );
	    bitshift = 8 - bitspersample;
	    byte = 0;
	    for ( col = 0, xP = xels[row], tP = buf; col < cols; ++col, ++xP )
		{
		s = PNM_GET1( *xP );
		if ( maxval != bigger_maxval )
		    s = (long) s * bigger_maxval / maxval;
		byte |= s << bitshift;
		bitshift -= bitspersample;
		if ( bitshift < 0 )
		    {
		    *tP++ = byte;
		    bitshift = 8 - bitspersample;
		    byte = 0;
		    }
		}
	    if ( bitshift != 8 - bitspersample )
		*tP++ = byte;
	    }

	if ( TIFFWriteScanline( tif, buf, row, 0 ) < 0 )
	    pm_error( "failed a scanline write on row %d", row );
	}
    TIFFFlushData( tif );
    TIFFClose( tif );

    exit( 0 );
    }
