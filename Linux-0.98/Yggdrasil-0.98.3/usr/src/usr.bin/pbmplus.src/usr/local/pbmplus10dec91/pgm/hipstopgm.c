/* hipstopgm.c - read a HIPS file and produce a portable graymap
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

struct HIPS_Header {
    char* orig_name;	/* An indication of the originator of this sequence. */
    char* seq_name;	/* The sequence name. */
    int num_frame;	/* The number of frames in this sequence. */
    char* orig_date;	/* The date the sequence was originated. */
    int rows;		/* The number of rows in each image, the height. */
    int cols;		/* The number of columns in each image, the width. */
    int bits_per_pixel;	/* The number of significant bits per pixel. */
    int bit_packing;	/* Nonzero if the bits were packed such as to
			   eliminate any unused bits resulting from a
			   bits_per_pixel value which was not an even
			   multiple of eight. */
    int pixel_format;	/* An indication of the format of each pixel. */
    char* seq_history;	/* A description of the sequence of transformations
			   leading up to the current image. */
    char* seq_desc;	/* A free form description of the contents of the
			   sequence. */
    };
#define HIPS_PFBYTE 0
#define HIPS_PFSHORT 1
#define HIPS_PFINT 2
#define HIPS_PFFLOAT 3
#define HIPS_PFCOMPLEX 4

static void read_hips_header ARGS(( FILE* fd, struct HIPS_Header* hP ));
static void read_line ARGS(( FILE* fd, char* buf, int size ));

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    int argn, row;
    register int col;
    int maxval;
    int rows, cols;
    struct HIPS_Header h;

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	argn++;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( "[hipsfile]" );

    read_hips_header( ifp, &h );

    cols = h.cols;
    rows = h.rows * h.num_frame;

    switch ( h.pixel_format )
	{
	case HIPS_PFBYTE:
	if ( h.bits_per_pixel != 8 )
	    pm_error(
		"can't handle unusual bits_per_pixel %d", h.bits_per_pixel );
	if ( h.bit_packing != 0 )
	    pm_error( "can't handle bit_packing" );
	maxval = 255;
	break;

	default:
	pm_error( "unknown pixel format %d", h.pixel_format );
	}
    if ( maxval > PGM_MAXMAXVAL )
	pm_error(
	  "bits_per_pixel is too large - try reconfiguring with PGM_BIGGRAYS" );

    pgm_writepgminit( stdout, cols, rows, (gray) maxval, 0 );
    grayrow = pgm_allocrow( cols );
    for ( row = 0; row < rows; row++)
	{
	for ( col = 0, gP = grayrow; col < cols; col++, gP++ )
	    {
	    int ich;

	    switch ( h.pixel_format )
		{
		case HIPS_PFBYTE:
		ich = getc( ifp );
		if ( ich == EOF )
		    pm_error( "EOF / read error" );
		*gP = (gray) ich;
		break;

		default:
		pm_error( "can't happen" );
		}
	    }
	pgm_writepgmrow( stdout, grayrow, cols, (gray) maxval, 0 );
	}
    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static void
read_hips_header( fd, hP )
FILE* fd;
struct HIPS_Header* hP;
    {
    char buf[5000];

    /* Read and toss orig_name. */
    read_line( fd, buf, 5000 );

    /* Read and toss seq_name. */
    read_line( fd, buf, 5000 );

    /* Read num_frame. */
    read_line( fd, buf, 5000 );
    hP->num_frame = atoi( buf );

    /* Read and toss orig_date. */
    read_line( fd, buf, 5000 );

    /* Read rows. */
    read_line( fd, buf, 5000 );
    hP->rows = atoi( buf );

    /* Read cols. */
    read_line( fd, buf, 5000 );
    hP->cols = atoi( buf );

    /* Read bits_per_pixel. */
    read_line( fd, buf, 5000 );
    hP->bits_per_pixel = atoi( buf );

    /* Read bit_packing. */
    read_line( fd, buf, 5000 );
    hP->bit_packing = atoi( buf );

    /* Read pixel_format. */
    read_line( fd, buf, 5000 );
    hP->pixel_format = atoi( buf );

    /* Now read and toss lines until we get one with just a period. */
    do
	{
	read_line( fd, buf, 5000 );
	}
    while ( strcmp( buf, ".\n" ) != 0 );
    }

static void
read_line( fd, buf, size )
FILE* fd;
char* buf;
int size;
    {
    if ( fgets( buf, size, fd ) == NULL )
	pm_error( "error reading header" );
    }
