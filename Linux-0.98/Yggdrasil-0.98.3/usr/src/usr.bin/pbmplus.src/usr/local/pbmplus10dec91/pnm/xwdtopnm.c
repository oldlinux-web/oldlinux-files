/* xwdtopnm.c - read an X11 or X10 window dump file and write a portable anymap
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
#include "x10wd.h"
#include "x11wd.h"

static void getinit ARGS(( FILE* file, int* colsP, int* rowsP, int* padrightP, long* maxvalP, long* visualclassP, int* formatP, xel** colorsP ));
static unsigned long getpixnum ARGS(( FILE* file ));
static short bs_short ARGS(( short s ));
static long bs_long ARGS(( long l ));

static unsigned long red_mask, green_mask, blue_mask;
static int bits_per_item, bits_used, bit_shift, bits_per_pixel;
static char buf[4];
static char* byteP;
static short* shortP;
static long* longP;
static unsigned long pixel_mask;
static int byte_swap, byte_order, bit_order;

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel* xelrow;
    register xel* xP;
    xel* colors;
    int rows, cols, format, padright, row;
    register int col;
    long maxval, visualclass;

    pnm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[xwdfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows, &padright, &maxval, &visualclass, &format, &colors );

    pnm_writepnminit( stdout, cols, rows, (xelval) maxval, format, 0 );
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
	switch ( visualclass )
	    {
	    case StaticGray:
            case GrayScale:
	    case StaticColor:
	    case PseudoColor:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		*xP = colors[getpixnum( ifp )];
	    break;

	    case TrueColor:
            case DirectColor:
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		register unsigned long ul;

		ul = getpixnum( ifp );
		switch ( bits_per_pixel )
		    {
		    case 16:
		    PPM_ASSIGN( *xP,
			( ( ul & red_mask ) >> 0 ),
			( ( ul & green_mask ) >> 5 ),
			( ( ul & blue_mask ) >> 10 ) );
		    break;

		    case 24:
		    case 32:
		    PPM_ASSIGN( *xP, ( ( ul & 0xff0000 ) >> 16 ),
			( ( ul & 0xff00 ) >> 8 ),
			( ul & 0xff ) );
		    break;

		    default:
		    pm_error( "True/Direct only supports 16, 24, and 32 bits" );
		    }
		}
	    break;

	    default:
	    pm_error( "unknown visual class" );
	    }
        for ( col = 0; col < padright; ++col )
	    (void) getpixnum( ifp );
	pnm_writepnmrow( stdout, xelrow, cols, (xelval) maxval, format, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static void
getinit( file, colsP, rowsP, padrightP, maxvalP, visualclassP, formatP, colorsP )
    FILE* file;
    int* colsP;
    int* rowsP;
    int* padrightP;
    long* maxvalP;
    long* visualclassP;
    int* formatP;
    xel** colorsP;
    {
    int grayscale;
    /* Assume X11 headers are larger than X10 ones. */
    unsigned char header[sizeof(X11WDFileHeader)];
    X10WDFileHeader* h10P;
    X11WDFileHeader* h11P;

    byte_swap = 0;
    if ( PNM_MAXMAXVAL < 65535L )
	*maxvalP = PNM_MAXMAXVAL;
    else
	*maxvalP = 65535L;
    h10P = (X10WDFileHeader*) header;
    h11P = (X11WDFileHeader*) header;
    if ( sizeof(*h10P) > sizeof(*h11P) )
	pm_error(
"ARGH!  On this machine, X10 headers are larger than X11 headers!\n    You will have to re-write xwdtopnm." );

    /* Read an X10 header. */
    if ( fread( &header[0], sizeof(*h10P), 1, file ) != 1 )
	pm_error( "couldn't read XWD file header" );

    if ( h10P->file_version == X10WD_FILE_VERSION ||
	 bs_long( h10P->file_version ) == X10WD_FILE_VERSION )
	{
	int i;
	X10Color* x10colors;

	if ( h10P->file_version != X10WD_FILE_VERSION )
	    {
	    byte_swap = 1;
	    h10P->header_size = bs_long( h10P->header_size );
	    h10P->file_version = bs_long( h10P->file_version );
	    h10P->display_type = bs_long( h10P->display_type );
	    h10P->display_planes = bs_long( h10P->display_planes );
	    h10P->pixmap_format = bs_long( h10P->pixmap_format );
	    h10P->pixmap_width = bs_long( h10P->pixmap_width );
	    h10P->pixmap_height = bs_long( h10P->pixmap_height );
	    h10P->window_width = bs_short( h10P->window_width );
	    h10P->window_height = bs_short( h10P->window_height );
	    h10P->window_x = bs_short( h10P->window_x );
	    h10P->window_y = bs_short( h10P->window_y );
	    h10P->window_bdrwidth = bs_short( h10P->window_bdrwidth );
	    h10P->window_ncolors = bs_short( h10P->window_ncolors );
	    }
	for ( i = 0; i < h10P->header_size - sizeof(*h10P); ++i )
	    if ( getc( file ) == EOF )
		pm_error( "couldn't read rest of X10 XWD file header" );

	/* Check whether we can handle this dump. */
	if ( h10P->window_ncolors > 256 )
	    pm_error( "can't handle X10 window_ncolors > %d", 256 );
	if ( h10P->pixmap_format != ZFormat && h10P->display_planes != 1 )
	    pm_error(
		"can't handle X10 pixmap_format %d with planes != 1",
		h10P->pixmap_format );

	grayscale = 1;
	if ( h10P->window_ncolors != 0 )
	    {
	    /* Read X10 colormap. */
	    x10colors = (X10Color*) malloc(
		h10P->window_ncolors * sizeof(X10Color) );
	    if ( x10colors == 0 )
		pm_error( "out of memory" );
	    for ( i = 0; i < h10P->window_ncolors; ++i )
		{
		if ( fread( &x10colors[i], sizeof(X10Color), 1, file ) != 1 )
		    pm_error( "couldn't read X10 XWD colormap" );
		if ( byte_swap )
		    {
		    x10colors[i].red = bs_short( x10colors[i].red );
		    x10colors[i].green = bs_short( x10colors[i].green );
		    x10colors[i].blue = bs_short( x10colors[i].blue );
		    }
		if ( *maxvalP != 65535L )
		    {
		    x10colors[i].red =
			(long) x10colors[i].red * (long) *maxvalP / 65535L;
		    x10colors[i].green =
			(long) x10colors[i].green * (long) *maxvalP / 65535L;
		    x10colors[i].blue =
			(long) x10colors[i].blue * (long) *maxvalP / 65535L;
		    }
		if ( x10colors[i].red != x10colors[i].green ||
		     x10colors[i].green != x10colors[i].blue )
		    grayscale = 0;
		}
	    }

	if ( h10P->display_planes == 1 )
	    {
	    *formatP = PBM_TYPE;
	    *visualclassP = StaticGray;
	    *maxvalP = 1;
	    *colorsP = pnm_allocrow( 2 );
	    PNM_ASSIGN1( (*colorsP)[0], 0 );
	    PNM_ASSIGN1( (*colorsP)[1], *maxvalP );
	    *padrightP =
		( ( h10P->pixmap_width + 15 ) / 16 ) * 16 - h10P->pixmap_width;
	    bits_per_item = 16;
	    bits_per_pixel = 1;
	    }
	else if ( h10P->window_ncolors == 0 )
	    { /* Must be grayscale. */
	    *formatP = PGM_TYPE;
	    *visualclassP = StaticGray;
	    *maxvalP = ( 1 << h10P->display_planes ) - 1;
	    *colorsP = pnm_allocrow( *maxvalP + 1 );
	    for ( i = 0; i <= *maxvalP; ++i )
		PNM_ASSIGN1( (*colorsP)[i], i );
	    *padrightP =
		( ( h10P->pixmap_width + 15 ) / 16 ) * 16 - h10P->pixmap_width;
	    bits_per_item = 16;
	    bits_per_pixel = 1;
	    }
	else
	    {
	    *colorsP = pnm_allocrow( h10P->window_ncolors );
	    *visualclassP = PseudoColor;
            if ( grayscale )
                {
                *formatP = PGM_TYPE;
		for ( i = 0; i < h10P->window_ncolors; ++i )
		    PNM_ASSIGN1( (*colorsP)[i], x10colors[i].red );
                }
            else
                {
                *formatP = PPM_TYPE;
		for ( i = 0; i < h10P->window_ncolors; ++i )
		    PPM_ASSIGN(
			(*colorsP)[i], x10colors[i].red, x10colors[i].green,
			x10colors[i].blue);
                }

	    *padrightP = h10P->pixmap_width & 1;
	    bits_per_item = 8;
	    bits_per_pixel = 8;
	    }
	bits_used = bits_per_item;
	*colsP = h10P->pixmap_width;
	*rowsP = h10P->pixmap_height;
	if ( bits_per_pixel == sizeof(pixel_mask) * 8 )
	    pixel_mask = -1;
	else
	    pixel_mask = ( 1 << bits_per_pixel ) - 1;
	byte_order = MSBFirst;
	bit_order = LSBFirst;
	}
    else if ( h11P->file_version == X11WD_FILE_VERSION ||
	     bs_long( h11P->file_version ) == X11WD_FILE_VERSION )
	{
	int i;
	X11XColor* x11colors;

	if ( fread( &header[sizeof(*h10P)], sizeof(*h11P) - sizeof(*h10P), 1, file ) != 1 )
	    pm_error( "couldn't read X11 XWD file header" );
	if ( h11P->file_version != X11WD_FILE_VERSION )
	    {
	    byte_swap = 1;
	    h11P->header_size = bs_long( h11P->header_size );
	    h11P->file_version = bs_long( h11P->file_version );
	    h11P->pixmap_format = bs_long( h11P->pixmap_format );
	    h11P->pixmap_depth = bs_long( h11P->pixmap_depth );
	    h11P->pixmap_width = bs_long( h11P->pixmap_width );
	    h11P->pixmap_height = bs_long( h11P->pixmap_height );
	    h11P->xoffset = bs_long( h11P->xoffset );
	    h11P->byte_order = bs_long( h11P->byte_order );
	    h11P->bitmap_unit = bs_long( h11P->bitmap_unit );
	    h11P->bitmap_bit_order = bs_long( h11P->bitmap_bit_order );
	    h11P->bitmap_pad = bs_long( h11P->bitmap_pad );
	    h11P->bits_per_pixel = bs_long( h11P->bits_per_pixel );
	    h11P->bytes_per_line = bs_long( h11P->bytes_per_line );
	    h11P->visual_class = bs_long( h11P->visual_class );
	    h11P->red_mask = bs_long( h11P->red_mask );
	    h11P->green_mask = bs_long( h11P->green_mask );
	    h11P->blue_mask = bs_long( h11P->blue_mask );
	    h11P->bits_per_rgb = bs_long( h11P->bits_per_rgb );
	    h11P->colormap_entries = bs_long( h11P->colormap_entries );
	    h11P->ncolors = bs_long( h11P->ncolors );
	    h11P->window_width = bs_long( h11P->window_width );
	    h11P->window_height = bs_long( h11P->window_height );
	    h11P->window_x = bs_long( h11P->window_x );
	    h11P->window_y = bs_long( h11P->window_y );
	    h11P->window_bdrwidth = bs_long( h11P->window_bdrwidth );
	    }
	for ( i = 0; i < h11P->header_size - sizeof(*h11P); ++i )
	    if ( getc( file ) == EOF )
		pm_error( "couldn't read rest of X11 XWD file header" );

	/* Check whether we can handle this dump. */
	if ( h11P->pixmap_depth > 24 )
	    pm_error( "can't handle X11 pixmap_depth > 24" );
	if ( h11P->bits_per_rgb > 24 )
	    pm_error( "can't handle X11 bits_per_rgb > 24" );
	if ( h11P->pixmap_format != ZPixmap && h11P->pixmap_depth != 1 )
	    pm_error(
		"can't handle X11 pixmap_format %d with depth != 1",
		h11P->pixmap_format );
	if ( h11P->bitmap_unit != 8 && h11P->bitmap_unit != 16 &&
	     h11P->bitmap_unit != 32 )
	    pm_error(
		"X11 bitmap_unit (%d) is non-standard - can't handle",
		h11P->bitmap_unit );

	grayscale = 1;
	if ( h11P->ncolors > 0 )
	    {
	    /* Read X11 colormap. */
	    x11colors = (X11XColor*) malloc(
		h11P->ncolors * sizeof(X11XColor) );
	    if ( x11colors == 0 )
		pm_error( "out of memory" );
	    if ( fread( x11colors, sizeof(X11XColor), h11P->ncolors, file ) !=
			h11P->ncolors )
		pm_error( "couldn't read X11 XWD colormap" );
	    for ( i = 0; i < h11P->ncolors; ++i )
		{
		if ( byte_swap )
		    {
		    x11colors[i].red = bs_short( x11colors[i].red );
		    x11colors[i].green = bs_short( x11colors[i].green );
		    x11colors[i].blue = bs_short( x11colors[i].blue );
		    }
		if ( *maxvalP != 65535L )
		    {
		    x11colors[i].red =
			(long) x11colors[i].red * (long) *maxvalP / 65535L;
		    x11colors[i].green =
			(long) x11colors[i].green * (long) *maxvalP / 65535L;
		    x11colors[i].blue =
			(long) x11colors[i].blue * (long) *maxvalP / 65535L;
		    }
		if ( x11colors[i].red != x11colors[i].green ||
		     x11colors[i].green != x11colors[i].blue )
		    grayscale = 0;
		}
	    }

	*visualclassP = h11P->visual_class;
	if ( *visualclassP == TrueColor || *visualclassP == DirectColor )
	    {
	    *formatP = PPM_TYPE;
	    if ( h11P->bits_per_pixel == 16 )
	        *maxvalP = 31;
            else
		*maxvalP = 255;
	    }
	else if ( *visualclassP == StaticGray && h11P->bits_per_pixel == 1 )
	    {
	    *formatP = PBM_TYPE;
	    *maxvalP = 1;
	    *colorsP = pnm_allocrow( 2 );
	    PNM_ASSIGN1( (*colorsP)[0], *maxvalP );
	    PNM_ASSIGN1( (*colorsP)[1], 0 );
	    }
	else if ( *visualclassP == StaticGray )
	    {
	    *formatP = PGM_TYPE;
	    *maxvalP = ( 1 << h11P->bits_per_pixel ) - 1;
	    *colorsP = pnm_allocrow( *maxvalP + 1 );
	    for ( i = 0; i <= *maxvalP; ++i )
		PNM_ASSIGN1( (*colorsP)[i], i );
	    }
	else
	    {
	    *colorsP = pnm_allocrow( h11P->ncolors );
            if ( grayscale )
                {
                *formatP = PGM_TYPE;
		for ( i = 0; i < h11P->ncolors; ++i )
		    PNM_ASSIGN1( (*colorsP)[i], x11colors[i].red );
                }
            else
                {
                *formatP = PPM_TYPE;
		for ( i = 0; i < h11P->ncolors; ++i )
		    PPM_ASSIGN(
			(*colorsP)[i], x11colors[i].red, x11colors[i].green,
			x11colors[i].blue);
                }
	    }

	*colsP = h11P->pixmap_width;
	*rowsP = h11P->pixmap_height;
	*padrightP =
	    h11P->bytes_per_line * 8 / h11P->bits_per_pixel -
	    h11P->pixmap_width;
	bits_per_item = h11P->bitmap_unit;
	bits_used = bits_per_item;
	bits_per_pixel = h11P->bits_per_pixel;
	byte_order = h11P->byte_order;
	bit_order = h11P->bitmap_bit_order;
	if ( bits_per_pixel == sizeof(pixel_mask) * 8 )
	    pixel_mask = -1;
	else
	    pixel_mask = ( 1 << bits_per_pixel ) - 1;
        red_mask = h11P->red_mask;
        green_mask = h11P->green_mask;
        blue_mask = h11P->blue_mask;
	}
    else
	pm_error( "unknown XWD file version: %d", h11P->file_version );

    byteP = (char*) buf;
    shortP = (short*) buf;
    longP = (long*) buf;
    }

static unsigned long
getpixnum( file )
    FILE* file;
    {
    int n;

    if ( bits_used == bits_per_item )
	{
	switch ( bits_per_item )
	    {
	    case 8:
	    *byteP = getc( file );
	    break;

	    case 16:
	    if ( byte_order == MSBFirst )
		{
		if ( pm_readbigshort( file, shortP ) == -1 )
		    pm_error( "error reading image" );
		}
	    else
		{
		if ( pm_readlittleshort( file, shortP ) == -1 )
		    pm_error( "error reading image" );
		}
	    break;

	    case 32:
	    if ( byte_order == MSBFirst )
		{
		if ( pm_readbiglong( file, longP ) == -1 )
		    pm_error( "error reading image" );
		}
	    else
		{
		if ( pm_readlittlelong( file, longP ) == -1 )
		    pm_error( "error reading image" );
		}
	    break;

	    default:
	    pm_error( "can't happen" );
	    }
	bits_used = 0;

	if ( bit_order == MSBFirst )
	    bit_shift = bits_per_item - bits_per_pixel;
	else
	    bit_shift = 0;
	}

    switch ( bits_per_item )
	{
	case 8:
	n = ( *byteP >> bit_shift) & pixel_mask;
	break;

	case 16:
	n = ( *shortP >> bit_shift) & pixel_mask;
	break;

	case 32:
	n = ( *longP >> bit_shift) & pixel_mask;
	break;

	default:
	pm_error( "can't happen" );
	}

    if ( bit_order == MSBFirst )
	bit_shift -= bits_per_pixel;
    else
	bit_shift += bits_per_pixel;
    bits_used += bits_per_pixel;

    return n;
    }

/* Byte-swapping junk. */

union cheat {
    long l;
    short s;
    unsigned char c[4];
    };

#if __STDC__
static short
bs_short( short s )
#else /*__STDC__*/
static short
bs_short( s )
    short s;
#endif /*__STDC__*/
    {
    union cheat u;
    unsigned char t;

    u.s = s;
    t = u.c[0];
    u.c[0] = u.c[1];
    u.c[1] = t;
    return u.s;
    }

#if __STDC__
static long
bs_long( long l )
#else /*__STDC__*/
static long
bs_long( l )
    long l;
#endif /*__STDC__*/
    {
    union cheat u;
    unsigned char t;

    u.l = l;
    t = u.c[0];
    u.c[0] = u.c[3];
    u.c[3] = t;
    t = u.c[1];
    u.c[1] = u.c[2];
    u.c[2] = t;
    return u.l;
    }
