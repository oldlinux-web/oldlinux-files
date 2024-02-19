/* pnmtoxwd.c - read a portable anymap and produce a color X11 window dump
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
#include "x11wd.h"

#include "ppmcmap.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel** xels;
    register xel* xP;
    char* dumpname;
    int argn, rows, cols, format, colors, i, row, col;
    int pseudodepth;
    int forcedirect, direct, grayscale;
    xelval maxval;
    long lmaxval, xmaxval;
    colorhist_vector chv;
    colorhash_table cht;
    X11WDFileHeader h11;
    X11XColor color;
    char* usage = "[-pseudodepth n] [-directcolor] [pnmfile]";

    pnm_init( &argc, argv );

    argn = 1;
    pseudodepth = 8;
    forcedirect = 0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-pseudodepth", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &pseudodepth ) != 1 )
		pm_usage( usage );
	    if ( pseudodepth < 1 || pseudodepth > 16 )
		pm_usage( usage );
	    }
	else if ( pm_keymatch( argv[argn], "-directcolor", 2 ) )
	    forcedirect = 1;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	dumpname = argv[1];
	ifp = pm_openr( argv[1] );
	++argn;
	}
    else
	{
	dumpname = "stdin";
	ifp = stdin;
	}
    
    if ( argn != argc )
	pm_usage( usage );

    xels = pnm_readpnm( ifp, &cols, &rows, &maxval, &format );
    lmaxval = (long) maxval;
    xmaxval = ( 1 << pseudodepth ) - 1;
    pm_close( ifp );
    
    if ( forcedirect )
	direct = 1;
    else
	{
	/* Figure out the colormap. */
	switch ( PNM_FORMAT_TYPE(format) )
	    {
	    case PPM_TYPE:
	    direct = 0;
	    pm_message( "computing colormap..." );
	    chv = ppm_computecolorhist( xels, cols, rows, xmaxval+1, &colors );
	    if ( chv == (colorhist_vector) 0 )
		{
		pm_message(
		 "Too many colors - proceeding to write a 24-bit DirectColor" );
		pm_message(
	      "dump file.  If you want PseudoColor, try doing a 'ppmquant %d'.",
		    xmaxval );
		direct = 1;
		}
	    else
		{
		pm_message( "%d colors found", colors );
		grayscale = 0;
		direct = 0;
		/* Make a hash table for fast color lookup. */
		cht = ppm_colorhisttocolorhash( chv, colors );
		}
	    break;

	    default:
	    direct = 0;
	    grayscale = 1;
	    break;
	    }

	}

    /* Set up the header. */
    h11.header_size = sizeof(h11) + strlen( dumpname ) + 1;
    h11.file_version = X11WD_FILE_VERSION;
    h11.pixmap_format = ZPixmap;
    h11.pixmap_width = cols;
    h11.pixmap_height = rows;
    h11.xoffset = 0;
    h11.byte_order = MSBFirst;
    h11.bitmap_bit_order = MSBFirst;
    h11.window_width = cols;
    h11.window_height = rows;
    h11.window_x = 0;
    h11.window_y = 0;
    h11.window_bdrwidth = 0;

    if ( direct )
	{
	h11.pixmap_depth = 24;
	h11.bitmap_unit = 32;
	h11.bitmap_pad = 32;
	h11.bits_per_pixel = 32;
	h11.visual_class = DirectColor;
	h11.colormap_entries = 256;
	h11.ncolors = 0;
	h11.red_mask = 0xff0000;
	h11.green_mask = 0xff00;
	h11.blue_mask = 0xff;
	h11.bytes_per_line = cols * 4;
	}
    else if ( grayscale )
	{
	if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
	    {
	    h11.pixmap_depth = 1;
	    h11.bits_per_pixel = 1;
	    colors = 2;
	    h11.colormap_entries = colors;
	    h11.ncolors = colors;
	    h11.bytes_per_line = ( cols + 7 ) / 8;
	    }
	else
	    {
	    h11.pixmap_depth = pseudodepth;
	    h11.bits_per_pixel = pseudodepth;
	    colors = xmaxval + 1;
	    h11.colormap_entries = colors;
	    h11.ncolors = colors;
	    h11.bytes_per_line = cols;
	    }
	h11.bitmap_unit = 8;
	h11.bitmap_pad = 8;
	h11.visual_class = StaticGray;
	h11.red_mask = 0;
	h11.green_mask = 0;
	h11.blue_mask = 0;
	}
    else
	{
	h11.pixmap_depth = pseudodepth;
	h11.bits_per_pixel = pseudodepth;
	h11.visual_class = PseudoColor;
	h11.colormap_entries = xmaxval + 1;
	h11.ncolors = colors;
	h11.red_mask = 0;
	h11.green_mask = 0;
	h11.blue_mask = 0;
	h11.bytes_per_line = cols;
	h11.bitmap_unit = 8;
	h11.bitmap_pad = 8;
	}
    h11.bits_per_rgb = h11.pixmap_depth;

    /* Write out the header in big-endian order. */
    pm_writebiglong( stdout, h11.header_size );
    pm_writebiglong( stdout, h11.file_version );
    pm_writebiglong( stdout, h11.pixmap_format );
    pm_writebiglong( stdout, h11.pixmap_depth );
    pm_writebiglong( stdout, h11.pixmap_width );
    pm_writebiglong( stdout, h11.pixmap_height );
    pm_writebiglong( stdout, h11.xoffset );
    pm_writebiglong( stdout, h11.byte_order );
    pm_writebiglong( stdout, h11.bitmap_unit );
    pm_writebiglong( stdout, h11.bitmap_bit_order );
    pm_writebiglong( stdout, h11.bitmap_pad );
    pm_writebiglong( stdout, h11.bits_per_pixel );
    pm_writebiglong( stdout, h11.bytes_per_line );
    pm_writebiglong( stdout, h11.visual_class );
    pm_writebiglong( stdout, h11.red_mask );
    pm_writebiglong( stdout, h11.green_mask );
    pm_writebiglong( stdout, h11.blue_mask );
    pm_writebiglong( stdout, h11.bits_per_rgb );
    pm_writebiglong( stdout, h11.colormap_entries );
    pm_writebiglong( stdout, h11.ncolors );
    pm_writebiglong( stdout, h11.window_width );
    pm_writebiglong( stdout, h11.window_height );
    pm_writebiglong( stdout, h11.window_x );
    pm_writebiglong( stdout, h11.window_y );
    pm_writebiglong( stdout, h11.window_bdrwidth );

    /* Write out the dump name. */
    fwrite( dumpname, 1, strlen( dumpname ) + 1, stdout );

    if ( ! direct )
	{
	/* Write out the colormap, big-endian order. */
	color.flags = 7;
	color.pad = 0;
	for ( i = 0; i < colors; ++i )
	    {
	    color.num = i;
	    if ( grayscale )
		{

		/* Stupid hack because xloadimage and xwud disagree on
		** how to interpret bitmaps. */
		if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
		    color.red = (long) ( colors-1-i ) * 65535L / ( colors - 1 );
		else
		    color.red = (long) i * 65535L / ( colors - 1 );

		color.green = color.red;
		color.blue = color.red;
		}
	    else
		{
		color.red = PPM_GETR( chv[i].color );
		color.green = PPM_GETG( chv[i].color );
		color.blue = PPM_GETB( chv[i].color );
		if ( lmaxval != 65535L )
		    {
		    color.red = (long) color.red * 65535L / lmaxval;
		    color.green = (long) color.green * 65535L / lmaxval;
		    color.blue = (long) color.blue * 65535L / lmaxval;
		    }
		}
	    pm_writebiglong( stdout, color.num );
	    pm_writebigshort( stdout, color.red );
	    pm_writebigshort( stdout, color.green );
	    pm_writebigshort( stdout, color.blue );
	    (void) putc( color.flags, stdout );
	    (void) putc( color.pad, stdout );
	    }
	}

    /* Finally, write out the data. */
    for ( row = 0; row < rows; ++row )
	if ( direct )
	    {
	    switch ( PNM_FORMAT_TYPE(format) )
		{
		case PPM_TYPE:
		for ( col = 0, xP = xels[row]; col < cols; ++col, ++xP )
		    {
		    unsigned long ul;

		    ul = ( ( PPM_GETR( *xP ) * xmaxval / lmaxval ) << 16 ) |
			 ( ( PPM_GETG( *xP ) * xmaxval / lmaxval ) << 8 ) |
			 ( PPM_GETB( *xP ) * xmaxval / lmaxval );
		    fwrite( &ul, sizeof(ul), 1, stdout );
		    }
		break;

		default:
		for ( col = 0, xP = xels[row]; col < cols; ++col, ++xP )
		    {
		    unsigned long ul;
		    register unsigned long val;

		    val = PNM_GET1( *xP );
		    ul = ( ( val * xmaxval / lmaxval ) << 16 ) |
			 ( ( val * xmaxval / lmaxval ) << 8 ) |
			 ( val * xmaxval / lmaxval );
		    fwrite( &ul, sizeof(ul), 1, stdout );
		    }
		break;
		}
	    }
	else if ( grayscale )
	    {
	    register xelval bigger_maxval;
	    register int bitshift;
	    unsigned char byte;
	    register xelval s;

	    bigger_maxval = pm_bitstomaxval( h11.bits_per_pixel );
	    bitshift = 8 - h11.bits_per_pixel;
	    byte = 0;
	    for ( col = 0, xP = xels[row]; col < cols; ++col, ++xP )
		{
		s = PNM_GET1( *xP );

		/* More stupid hack. */
		if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
		    s = 1 - s;

		if ( maxval != bigger_maxval )
		    s = (long) s * bigger_maxval / maxval;
		byte |= s << bitshift;
		bitshift -= h11.bits_per_pixel;
		if ( bitshift < 0 )
		    {
		    putchar( byte );
		    bitshift = 8 - h11.bits_per_pixel;
		    byte = 0;
		    }
		}
	    if ( bitshift < 8 - h11.bits_per_pixel )
		putchar( byte );
	    }
	else
	    {
	    for ( col = 0, xP = xels[row]; col < cols; ++col, ++xP )
		putchar( ppm_lookupcolor( cht, xP ) );
	    }

    exit( 0 );
    }
