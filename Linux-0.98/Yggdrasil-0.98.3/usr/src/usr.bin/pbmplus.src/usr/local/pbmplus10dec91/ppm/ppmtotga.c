/* ppmtotga.c - read a portable pixmap and produce a TrueVision Targa file
**
** Copyright (C) 1989, 1991 by Mark Shand and Jef Poskanzer
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"
#include "ppmcmap.h"
#include "tga.h"

/* Max number of colors allowed for colormapped output. */
#define MAXCOLORS 256

/* Forward routines. */
static void writetga ARGS(( struct ImageHeader* tgaP, char* id ));
static void put_map_entry ARGS(( pixel* valueP, int size, pixval maxval ));
static void compute_runlengths ARGS(( int cols, pixel* pixelrow, int* runlength ));
static void put_pixel ARGS(( pixel* pP, int imgtype, pixval maxval, colorhash_table cht ));
static void put_mono ARGS(( pixel* pP, pixval maxval ));
static void put_map ARGS(( pixel* pP, colorhash_table cht ));
static void put_rgb ARGS(( pixel* pP, pixval maxval ));

/* Routines. */

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel** pixels;
    register pixel* pP;
    pixel p;
    int argn, rle_flag, rows, cols, ncolors, row, col, i, format, realrow;
    pixval maxval;
    colorhist_vector chv;
    colorhash_table cht;
    char out_name[100];
    char* cp;
    int* runlength;
    char* usage = "[-name <tganame>] [-mono|-cmap|-rgb] [-norle] [ppmfile]";
    struct ImageHeader tgaHeader;

    ppm_init( &argc, argv );
    out_name[0] = '\0';

    /* Check for command line options. */
    argn = 1;
    tgaHeader.ImgType = TGA_Null;
    rle_flag = 1;
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
        {
        if ( pm_keymatch( argv[argn], "-name", 2 ) )
            {
            ++argn;
            if ( argn == argc )
        	pm_usage( usage );
            (void) strcpy( out_name, argv[argn] );
            }
        else if ( pm_keymatch( argv[argn], "-cmap", 2 ) )
            tgaHeader.ImgType = TGA_Map;
        else if ( pm_keymatch( argv[argn], "-mono", 2 ) )
            tgaHeader.ImgType = TGA_Mono;
        else if ( pm_keymatch( argv[argn], "-rgb", 2 ) )
            tgaHeader.ImgType = TGA_RGB;
        else if ( pm_keymatch( argv[argn], "-norle", 2 ) )
            rle_flag = 0;
        else
            pm_usage( usage );
        ++argn;
        }

    if ( argn != argc )
        {
        /* Open the input file. */
        ifp = pm_openr( argv[argn] );

        /* If output filename not specified, use input filename as default. */
        if ( out_name[0] == '\0' )
            {
            (void) strcpy( out_name, argv[argn] );
            cp = index( out_name, '.' );
            if ( cp != 0 )
        	*cp = '\0';	/* remove extension */
            if ( strcmp( out_name, "-" ) == 0 )
        	(void) strcpy( out_name, "noname" );
            }

        ++argn;
        }
    else
        {
        /* No input file specified. */
        ifp = stdin;
        if ( out_name[0] == '\0' )
            (void) strcpy( out_name, "noname" );
        }

    if ( argn != argc )
        pm_usage( usage );

    /* Read in the ppm file. */
    ppm_readppminit( ifp, &cols, &rows, &maxval, &format);
    pixels = ppm_allocarray( cols, rows );
    for ( row = 0; row < rows; ++row )
	ppm_readppmrow( ifp, pixels[row], cols, maxval, format );
    pm_close( ifp );

    /* Figure out the colormap. */
    switch ( PPM_FORMAT_TYPE( format ) )
	{
        case PPM_TYPE:
	if ( tgaHeader.ImgType == TGA_Mono )
	    pm_error( "input is not a graymap, filter through ppmtopgm first" );
	if ( tgaHeader.ImgType == TGA_Null || tgaHeader.ImgType == TGA_Map )
	    {
	    pm_message( "computing colormap..." );
	    chv = ppm_computecolorhist(
		pixels, cols, rows, MAXCOLORS, &ncolors );
	    if ( chv == (colorhist_vector) 0 )
		{
		if ( tgaHeader.ImgType == TGA_Map )
		    pm_error(
			"too many colors - try doing a 'ppmquant %d'",
			MAXCOLORS );
		else
		    tgaHeader.ImgType = TGA_RGB;
		}
	    else
		{
		pm_message( "%d colors found", ncolors );
		if ( tgaHeader.ImgType == TGA_Null )
		    tgaHeader.ImgType = TGA_Map;
		}
	    }
	break;

        case PGM_TYPE:
        case PBM_TYPE:
	if ( tgaHeader.ImgType == TGA_Null )
	    tgaHeader.ImgType = TGA_Mono;
	else if ( tgaHeader.ImgType == TGA_Map )
	    {
	    pm_message( "computing colormap..." );
	    chv = ppm_computecolorhist(
		pixels, cols, rows, MAXCOLORS, &ncolors );
	    if ( chv == (colorhist_vector) 0 )
		pm_error( "can't happen" );
	    pm_message( "%d colors found", ncolors );
	    }
	break;

        default:
	pm_error( "can't happen" );
	}

    if ( rle_flag )
	{
	switch ( tgaHeader.ImgType )
	    {
	    case TGA_Mono:
	    tgaHeader.ImgType = TGA_RLEMono;
	    break;
	    case TGA_Map:
	    tgaHeader.ImgType = TGA_RLEMap;
	    break;
	    case TGA_RGB:
	    tgaHeader.ImgType = TGA_RLERGB;
	    break;
	    default:
	    pm_error( "can't happen" );
	    }
	runlength = (int*) pm_allocrow( cols, sizeof(int) );
	}
    
    tgaHeader.IDLength = 0;
    tgaHeader.Index_lo = 0;
    tgaHeader.Index_hi = 0;
    if ( tgaHeader.ImgType == TGA_Map || tgaHeader.ImgType == TGA_RLEMap )
	{
        /* Make a hash table for fast color lookup. */
        cht = ppm_colorhisttocolorhash( chv, ncolors );

        tgaHeader.CoMapType = 1;
        tgaHeader.Length_lo = ncolors % 256;
        tgaHeader.Length_hi = ncolors / 256;
        tgaHeader.CoSize = 24;
	}
    else
	{
        tgaHeader.CoMapType = 0;
        tgaHeader.Length_lo = 0;
        tgaHeader.Length_hi = 0;
        tgaHeader.CoSize = 0;
	}
    if ( tgaHeader.ImgType == TGA_RGB || tgaHeader.ImgType == TGA_RLERGB )
	tgaHeader.PixelSize = 24;
    else
	tgaHeader.PixelSize = 8;
    tgaHeader.X_org_lo = tgaHeader.X_org_hi = 0;
    tgaHeader.Y_org_lo = tgaHeader.Y_org_hi = 0;
    tgaHeader.Width_lo = cols % 256;
    tgaHeader.Width_hi = cols / 256;
    tgaHeader.Height_lo = rows % 256;
    tgaHeader.Height_hi = rows / 256;
    tgaHeader.AttBits = 0;
    tgaHeader.Rsrvd = 0;
    tgaHeader.IntrLve = 0;
    tgaHeader.OrgBit = 0;

    /* Write out the Targa header. */
    writetga( &tgaHeader, (char*) 0 );

    if ( tgaHeader.ImgType == TGA_Map || tgaHeader.ImgType == TGA_RLEMap )
	{
        /* Write out the Targa colormap. */
        for ( i = 0; i < ncolors; ++i )
            put_map_entry( &chv[i].color, tgaHeader.CoSize, maxval );
	}

    /* Write out the pixels */
    for ( row = 0; row < rows; ++row )
	{
	realrow = row;
	if ( tgaHeader.OrgBit == 0 )
	    realrow = rows - realrow - 1;
	if ( rle_flag )
	    {
	    compute_runlengths( cols, pixels[realrow], runlength );
	    for ( col = 0; col < cols; )
		{
		if ( runlength[col] > 0 )
		    {
		    putchar( 0x80 + runlength[col] - 1 );
		    put_pixel(
			&(pixels[realrow][col]),
			tgaHeader.ImgType, maxval, cht );
		    col += runlength[col];
		    }
		else if ( runlength[col] < 0 )
		    {
		    putchar( -runlength[col] - 1 );
		    for ( i = 0; i < -runlength[col]; ++i )
			put_pixel(
			    &(pixels[realrow][col + i]),
			    tgaHeader.ImgType, maxval, cht );
		    col += -runlength[col];
		    }
		else
		    pm_error( "can't happen" );
		}
	    }
	else
	    {
	    for ( col = 0, pP = pixels[realrow]; col < cols; ++col, ++pP )
		put_pixel( pP, tgaHeader.ImgType, maxval, cht );
	    }
	}

    exit( 0 );
    }

static void
writetga( tgaP, id )
    struct ImageHeader* tgaP;
    char* id;
    {
    unsigned char flags;

    putchar( tgaP->IDLength );
    putchar( tgaP->CoMapType );
    putchar( tgaP->ImgType );
    putchar( tgaP->Index_lo );
    putchar( tgaP->Index_hi );
    putchar( tgaP->Length_lo );
    putchar( tgaP->Length_hi );
    putchar( tgaP->CoSize );
    putchar( tgaP->X_org_lo );
    putchar( tgaP->X_org_hi );
    putchar( tgaP->Y_org_lo );
    putchar( tgaP->Y_org_hi );
    putchar( tgaP->Width_lo );
    putchar( tgaP->Width_hi );
    putchar( tgaP->Height_lo );
    putchar( tgaP->Height_hi );
    putchar( tgaP->PixelSize );
    flags = ( tgaP->AttBits & 0xf ) | ( ( tgaP->Rsrvd & 0x1 ) << 4 ) |
	    ( ( tgaP->OrgBit & 0x1 ) << 5 ) | ( ( tgaP->OrgBit & 0x3 ) << 6 );
    putchar( flags );
    if ( tgaP->IDLength )
        fwrite( id, 1, (int) tgaP->IDLength, stdout );
    }
    
#if __STDC__
static void
put_map_entry( pixel* valueP, int size, pixval maxval )
#else /*__STDC__*/
static void
put_map_entry( valueP, size, maxval )
    pixel* valueP;
    int size;
    pixval maxval;
#endif /*__STDC__*/
    {
    int j;
    pixel p;
    
    switch ( size )
	{
	case 8:				/* Grey scale. */
	put_mono( valueP, maxval );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch for byte order. */
	PPM_DEPTH( p, *valueP, maxval, 31 );
	j = (int) PPM_GETB( p ) | ( (int) PPM_GETG( p ) << 5 ) |
	    ( (int) PPM_GETR( p ) << 10 );
	putchar( j % 256 );
	putchar( j / 256 );
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	put_rgb( valueP, maxval );
	break;

	default:
	pm_error( "unknown colormap pixel size (#2) - %d", size );
	}
    }

static void
compute_runlengths( cols, pixelrow, runlength )
    int cols;
    pixel* pixelrow;
    int* runlength;
    {
    int col, start;

    /* Initialize all run lengths to 0.  (This is just an error check.) */
    for ( col = 0; col < cols; ++col )
	runlength[col] = 0;
    
    /* Find runs of identical pixels. */
    for ( col = 0; col < cols; )
	{
	start = col;
	do {
	    ++col;
	    }
	while ( col < cols &&
		col - start < 128 &&
		PPM_EQUAL( pixelrow[col], pixelrow[start] ) );
	runlength[start] = col - start;
	}
    
    /* Now look for runs of length-1 runs, and turn them into negative runs. */
    for ( col = 0; col < cols; )
	{
	if ( runlength[col] == 1 )
	    {
	    start = col;
	    while ( col < cols &&
		    col - start < 128 &&
		    runlength[col] == 1 )
		{
		runlength[col] = 0;
		++col;
		}
	    runlength[start] = - ( col - start );
	    }
	else
	    col += runlength[col];
	}
    }

#if __STDC__
static void
put_pixel( pixel* pP, int imgtype, pixval maxval, colorhash_table cht )
#else /*__STDC__*/
static void
put_pixel( pP, imgtype, maxval, cht )
    pixel* pP;
    int imgtype;
    pixval maxval;
    colorhash_table cht;
#endif /*__STDC__*/
    {
    switch ( imgtype )
	{
	case TGA_Mono:
	case TGA_RLEMono:
	put_mono( pP, maxval );
	break;
	case TGA_Map:
	case TGA_RLEMap:
	put_map( pP, cht );
	break;
	case TGA_RGB:
	case TGA_RLERGB:
	put_rgb( pP, maxval );
	break;
	default:
	pm_error( "can't happen" );
	}
    }

#if __STDC__
static void
put_mono( pixel* pP, pixval maxval )
#else /*__STDC__*/
static void
put_mono( pP, maxval )
    pixel* pP;
    pixval maxval;
#endif /*__STDC__*/
    {
    PPM_DEPTH( *pP, *pP, maxval, (pixval) 255 );
    putchar( PPM_GETR( *pP ) );
    }

static void
put_map( pP, cht )
    pixel* pP;
    colorhash_table cht;
    {
    putchar( ppm_lookupcolor( cht, pP ) );
    }

#if __STDC__
static void
put_rgb( pixel* pP, pixval maxval )
#else /*__STDC__*/
static void
put_rgb( pP, maxval )
    pixel* pP;
    pixval maxval;
#endif /*__STDC__*/
    {
    PPM_DEPTH( *pP, *pP, maxval, (pixval) 255 );
    putchar( PPM_GETB( *pP ) );
    putchar( PPM_GETG( *pP ) );
    putchar( PPM_GETR( *pP ) );
    }
