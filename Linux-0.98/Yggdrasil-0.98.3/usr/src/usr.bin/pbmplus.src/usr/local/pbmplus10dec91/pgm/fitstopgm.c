/* fitstopgm.c - read a FITS file and produce a portable graymap
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

#include "pgm.h"

struct FITS_Header {
    int simple;		/* basic format or not */
    int bitpix;		/* number of bits per pixel */
    int naxis;		/* number of axes */
    int naxis1;		/* number of points on axis 1 */
    int naxis2;		/* number of points on axis 2 */
    int naxis3;		/* number of points on axis 3 */
    double datamin;	/* min # */
    double datamax;	/* max # */
    double bzer;	/* ??? */
    double bscale;	/* ??? */
    };

static void read_fits_header ARGS(( FILE* fp, struct FITS_Header* hP ));
static void read_card ARGS(( FILE* fp, char* buf ));

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    int argn, imagenum, image, row;
    register int col;
    gray maxval;
    double fmaxval, scale;
    int rows, cols, images;
    struct FITS_Header h;
    char* usage = "[-image N] [FITSfile]";

    pgm_init( &argc, argv );

    argn = 1;
    imagenum = 1;

    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-image", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &imagenum ) != 1 )
		pm_usage( usage );
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    read_fits_header( ifp, &h );

    if ( ! h.simple )
	pm_error( "FITS file is not in simple format, can't read" );
    switch ( h.bitpix )
	{
	case 8:
	fmaxval = 255.0;
	break;

	case 16:
	fmaxval = 65535.0;
	break;

	case 32:
	fmaxval = 4294967295.0;
	break;

	default:
	pm_error( "unusual bits per pixel (%d), can't read", h.bitpix );
	}
    if ( h.naxis != 2 && h.naxis != 3 )
	pm_error( "FITS file has %d axes, can't read", h.naxis );
    cols = h.naxis1;
    rows = h.naxis2;
    if ( h.naxis == 2 )
	images = 1;
    else
	images = h.naxis3;
    if ( imagenum > images )
	pm_error( "only %d image%s in this file",
		  images, images > 1 ? "s" : "" );
    maxval = min( fmaxval, PGM_MAXMAXVAL );
    if ( h.datamin != 0.0 || h.datamax != 1.0 )
	scale = maxval / ( h.datamax - h.datamin );
    else
	{
	/* FITS images are not required to contain DATAMIN and DATAMAX cards
	** In this case, it would be necessary to read through the image
	** twice to properly scale.  Take this shortcut instead, remembering
	** that all FITS integers are signed values.  This scheme works
	** on most images because most astronomical data reduction packages
	** scale the images when writing so as to make maximal use of the
	** dynamic range of the output format. */
	h.bscale = 1.;
	h.bzer = ( fmaxval + 1.0 ) * 0.5;
	scale = maxval / fmaxval;
	}

    pgm_writepgminit( stdout, cols, rows, maxval, 0 );
    grayrow = pgm_allocrow( cols );
    for ( image = 1; image <= imagenum; ++image )
	{
	if ( image != imagenum )
	    pm_message( "skipping image %d of %d", image, images );
	else if ( images > 1 )
	    pm_message( "reading image %d of %d", image, images );
	for ( row = 0; row < rows; ++row)
	    {
	    for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
		{
		int ich;
		double val;

		switch ( h.bitpix )
		    {
		    case 8:
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val = ich;
		    break;

		    case 16:
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val = ich << 8;
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val += ich;
		    break;

		    case 32:
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val = ich << 24;
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val += ich << 16;
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val += ich << 8;
		    ich = getc( ifp );
		    if ( ich == EOF )
			pm_error( "EOF / read error" );
		    val += ich;
		    break;

		    default:
		    pm_error( "can't happen" );
		    }
		*gP = (gray) ( scale * ( val * h.bscale + h.bzer - h.datamin) );
		}
	    if ( image == imagenum )
		pgm_writepgmrow( stdout, grayrow, cols, maxval, 0 );
	    }
	}
    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static void
read_fits_header( fp, hP )
FILE* fp;
struct FITS_Header* hP;
    {
    char buf[80];
    int seen_end;
    int i;
    char c;

    seen_end = 0;
    hP->simple = 0;
    hP->bzer = 0.0;
    hP->bscale = 1.0;
    hP->datamin = 0.0;
    hP->datamax = 1.0;

    while ( ! seen_end )
	for ( i = 0; i < 36; ++i )
	    {
	    read_card( fp, buf );

	    if ( sscanf( buf, "SIMPLE = %c", &c ) == 1 )
		{
		if ( c == 'T' || c == 't' )
		    hP->simple = 1;
		}
	    else if ( sscanf( buf, "BITPIX = %d", &(hP->bitpix) ) == 1 );
	    else if ( sscanf( buf, "NAXIS = %d", &(hP->naxis) ) == 1 );
	    else if ( sscanf( buf, "NAXIS1 = %d", &(hP->naxis1) ) == 1 );
	    else if ( sscanf( buf, "NAXIS2 = %d", &(hP->naxis2) ) == 1 );
	    else if ( sscanf( buf, "NAXIS3 = %d", &(hP->naxis3) ) == 1 );
	    else if ( sscanf( buf, "DATAMIN = %lf", &(hP->datamin) ) == 1 );
	    else if ( sscanf( buf, "DATAMAX = %lf", &(hP->datamax) ) == 1 );
	    else if ( sscanf( buf, "BZERO = %lf", &(hP->bzer) ) == 1 );
	    else if ( sscanf( buf, "BSCALE = %lf", &(hP->bscale) ) == 1 );
	    else if ( strncmp( buf, "END ", 4 ) == 0 ) seen_end = 1;
	    }
    }

static void
read_card( fp, buf )
FILE* fp;
char* buf;
    {
    if ( fread( buf, 1, 80, fp ) == 0 )
	pm_error( "error reading header" );
    }
