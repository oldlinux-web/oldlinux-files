/* rasttopnm.c - read a Sun rasterfile and produce a portable anymap
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
#include "rast.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    struct rasterfile header;
    colormap_t pr_colormap;
    int grayscale;
    struct pixrect* pr;
    xel* xelrow;
    register xel* xP;
    int argn, rows, cols, format, depth, i, row, mask;
    register int col;
    xelval maxval;
    xel zero, one;
    int linesize;
    unsigned char* data;
    unsigned char* byteP;

    pnm_init( &argc, argv );

    argn = 1;

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( "[rastfile]" );

    /* Read in the rasterfile.  First the header. */
    if ( pr_load_header( ifp, &header ) != 0 )
	pm_error( "unable to read in rasterfile header" );

    cols = header.ras_width;
    rows = header.ras_height;
    depth = header.ras_depth;

    if ( cols <= 0 )
	pm_error( "invalid cols: %d", cols );
    if ( rows <= 0 )
	pm_error( "invalid rows: %d", rows );

    /* If there is a color map, read it. */
    grayscale = 1;
    if ( header.ras_maplength != 0 )
	{
	if ( pr_load_colormap( ifp, &header, &pr_colormap ) != 0 )
	    pm_error( "unable to skip colormap data" );
	for ( i = 0; i < header.ras_maplength / 3; ++i )
	    if ( pr_colormap.map[0][i] != pr_colormap.map[1][i] ||
		 pr_colormap.map[1][i] != pr_colormap.map[2][i] )
		{
		grayscale = 0;
		break;
		}
	}

    /* Check the depth and color map. */
    switch ( depth )
	{
	case 1:
	if ( header.ras_maptype == RMT_NONE && header.ras_maplength == 0 )
	    {
	    maxval = pnm_pbmmaxval;
	    format = PBM_TYPE;
	    PNM_ASSIGN1( zero, maxval );
	    PNM_ASSIGN1( one, 0 );
	    }
	else if ( header.ras_maptype == RMT_EQUAL_RGB &&
		  header.ras_maplength == 6 )
	    {
	    if ( grayscale )
		{
		maxval = 255;
		format = PGM_TYPE;
		PNM_ASSIGN1( zero, pr_colormap.map[0][0] );
		PNM_ASSIGN1( one, pr_colormap.map[0][1] );
		}
	    else
		{
		maxval = 255;
		format = PPM_TYPE;
		PPM_ASSIGN(
		    zero, pr_colormap.map[0][0], pr_colormap.map[1][0],
		    pr_colormap.map[2][0] );
		PPM_ASSIGN(
		    one, pr_colormap.map[0][1], pr_colormap.map[1][1],
		    pr_colormap.map[2][1] );
		}
	    }
	else
	    pm_error(
      "this depth-1 rasterfile has a non-standard colormap - type %d length %d",
		header.ras_maptype, header.ras_maplength );
	break;

	case 8:
	if ( grayscale )
	    {
	    maxval = 255;
	    format = PGM_TYPE;
	    }
	else if ( header.ras_maptype == RMT_EQUAL_RGB )
	    {
	    maxval = 255;
	    format = PPM_TYPE;
	    }
	else
	    pm_error(
      "this depth-8 rasterfile has a non-standard colormap - type %d length %d",
		header.ras_maptype, header.ras_maplength );
	break;

	case 24:
	case 32:
	if ( header.ras_maptype == RMT_NONE && header.ras_maplength == 0 )
	    ;
	else if ( header.ras_maptype == RMT_RAW || header.ras_maplength == 768 )
	    ;
	else
	    pm_error(
     "this depth-%d rasterfile has a non-standard colormap - type %d length %d",
		depth, header.ras_maptype, header.ras_maplength );
	maxval = 255;
	format = PPM_TYPE;
	break;

	default:
	pm_error(
	    "invalid depth: %d.  Can only handle depth 1, 8, 24, or 32.",
	    depth );
	}

    /* Now load the data.  The pixrect returned is a memory pixrect. */
    if ( ( pr = pr_load_image( ifp, &header, NULL ) ) == NULL )
	pm_error(
	    "unable to read in the image from the rasterfile" );

    linesize = ( (struct mpr_data*) pr->pr_data )->md_linebytes;
    data = ( (struct mpr_data*) pr->pr_data )->md_image;

    pm_close( ifp );

    /* Now write out the anymap. */
    pnm_writepnminit( stdout, cols, rows, maxval, format, 0 );
    xelrow = pnm_allocrow( cols );
    switch ( PNM_FORMAT_TYPE(format) )
        {
        case PBM_TYPE:
        pm_message( "writing PBM file" );
        break;

        case PGM_TYPE:
        pm_message( "writing PGM file" );
        break;

        case PPM_TYPE:
        pm_message( "writing PPM file" );
        break;

        default:
        pm_error( "shouldn't happen" );
        }

    for ( row = 0; row < rows; ++row )
	{
	byteP = data;
	switch ( depth )
	    {
	    case 1:
	    mask = 0x80;
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		if ( mask == 0 )
		    {
		    ++byteP;
		    mask = 0x80;
		    }
		*xP = ( *byteP & mask ) ? one : zero;
		mask = mask >> 1;
		}
	    break;

	    case 8:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		if ( header.ras_maplength == 0 )
		    PNM_ASSIGN1( *xP, *byteP );
		else if ( grayscale )
		    PNM_ASSIGN1( *xP, pr_colormap.map[0][*byteP] );
		else
		    PPM_ASSIGN(
			*xP, pr_colormap.map[0][*byteP],
			pr_colormap.map[1][*byteP],
			pr_colormap.map[2][*byteP] );
		++byteP;
		}
	    break;

	    case 24:
	    case 32:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		register xelval r, g, b;

		if ( depth == 32 )
		    ++byteP;
		if ( header.ras_type == RT_FORMAT_RGB )
		    {
		    r = *byteP++;
		    g = *byteP++;
		    b = *byteP++;
		    }
		else
		    {
		    b = *byteP++;
		    g = *byteP++;
		    r = *byteP++;
		    }
		if ( header.ras_maplength == 0 )
		    PPM_ASSIGN( *xP, r, g, b );
		else
		    PPM_ASSIGN(
			*xP, pr_colormap.map[0][r], pr_colormap.map[1][g],
			pr_colormap.map[2][b] );
		}
	    break;

	    default:
	    pm_error( "can't happen" );
	    }
	data += linesize;
	pnm_writepnmrow( stdout, xelrow, cols, maxval, format, 0 );
	}

    pm_close( stdout );

    exit( 0 );
    }
