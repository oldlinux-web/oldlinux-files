/* ppmpat.c - make a pixmap
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

#include "ppm.h"
#include "ppmdraw.h"
#include <math.h>
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif /*M_PI*/

static pixel random_color ARGS(( pixval maxval ));
static pixel random_bright_color ARGS(( pixval maxval ));
static pixel random_dark_color ARGS(( pixval maxval ));
static pixel average_two_colors ARGS(( pixel p1, pixel p2 ));
static void average_drawproc ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata ));
static void gingham2 ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void gingham3 ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void madras ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void tartan ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void poles ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void sq_measurecircle_drawproc ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata ));
static void sq_rainbowcircle_drawproc ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata ));
static void squig ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));
static void sq_assign_colors ARGS(( int circlecount, pixval maxval, pixel* colors ));
static pixel random_camo_color ARGS(( pixval maxval ));
static pixel random_anticamo_color ARGS(( pixval maxval ));
static float rnduni ARGS(( void ));
static void camo ARGS(( pixel** pixels, int cols, int rows, pixval maxval, int antiflag ));
static void test ARGS(( pixel** pixels, int cols, int rows, pixval maxval ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    pixel** pixels;
    int argn, pattern, cols, rows;
#define PAT_NONE 0
#define PAT_GINGHAM2 1
#define PAT_GINGHAM3 2
#define PAT_MADRAS 3
#define PAT_TARTAN 4
#define PAT_POLES 5
#define PAT_SQUIG 6
#define PAT_CAMO 7
#define PAT_ANTICAMO 8
#define PAT_TEST 9
    char* usage = "-gingham|-g2|-gingham3|-g3|-madras|-tartan|-poles|-squig|-camo|-anticamo <width> <height>";

    ppm_init( &argc, argv );

    argn = 1;
    pattern = PAT_NONE;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-gingham2", 9 ) ||
	     pm_keymatch( argv[argn], "-g2", 3 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_GINGHAM2;
	    }
	else if ( pm_keymatch( argv[argn], "-gingham3", 9 ) ||
	     pm_keymatch( argv[argn], "-g3", 3 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_GINGHAM3;
	    }
	else if ( pm_keymatch( argv[argn], "-madras", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_MADRAS;
	    }
	else if ( pm_keymatch( argv[argn], "-tartan", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_TARTAN;
	    }
	else if ( pm_keymatch( argv[argn], "-poles", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_POLES;
	    }
	else if ( pm_keymatch( argv[argn], "-squig", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_SQUIG;
	    }
	else if ( pm_keymatch( argv[argn], "-camo", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_CAMO;
	    }
	else if ( pm_keymatch( argv[argn], "-anticamo", 2 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_ANTICAMO;
	    }
	else if ( pm_keymatch( argv[argn], "-test", 3 ) )
	    {
	    if ( pattern != PAT_NONE )
		pm_error( "only one base pattern may be specified" );
	    pattern = PAT_TEST;
	    }
	else
	    pm_usage( usage );
	++argn;
	}
    if ( pattern == PAT_NONE )
	pm_error( "a base pattern must be specified" );

    if ( argn == argc )
	pm_usage( usage);
    if ( sscanf( argv[argn], "%d", &cols ) != 1 )
	pm_usage( usage );
    ++argn;
    if ( argn == argc )
	pm_usage( usage);
    if ( sscanf( argv[argn], "%d", &rows ) != 1 )
	pm_usage( usage );
    ++argn;

    if ( argn != argc )
	pm_usage( usage);

    srandom( (int) ( time( 0 ) ^ getpid( ) ) );
    pixels = ppm_allocarray( cols, rows );

    switch ( pattern )
	{
	case PAT_GINGHAM2:
	gingham2( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_GINGHAM3:
	gingham3( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_MADRAS:
	madras( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_TARTAN:
	tartan( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_POLES:
	poles( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_SQUIG:
	squig( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	case PAT_CAMO:
	camo( pixels, cols, rows, PPM_MAXMAXVAL, 0 );
	break;

	case PAT_ANTICAMO:
	camo( pixels, cols, rows, PPM_MAXMAXVAL, 1 );
	break;

	case PAT_TEST:
	test( pixels, cols, rows, PPM_MAXMAXVAL );
	break;

	default:
	pm_error( "can't happen!" );
	}

    /* All done, write it out. */
    ppm_writeppm( stdout, pixels, cols, rows, PPM_MAXMAXVAL, 0 );
    pm_close( stdout );

    exit( 0 );
    }

#if __STDC__
static pixel
random_color( pixval maxval )
#else /*__STDC__*/
static pixel
random_color( maxval )
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel p;

    PPM_ASSIGN(
	p, random() % ( maxval + 1 ), random() % ( maxval + 1 ),
	random() % ( maxval + 1 ) );

    return p;
    }

#define DARK_THRESH 0.25

#if __STDC__
static pixel
random_bright_color( pixval maxval )
#else /*__STDC__*/
static pixel
random_bright_color( maxval )
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel p;

    do
	{
	p = random_color( maxval );
	}
    while ( PPM_LUMIN( p ) <= maxval * DARK_THRESH );

    return p;
    }

#if __STDC__
static pixel
random_dark_color( pixval maxval )
#else /*__STDC__*/
static pixel
random_dark_color( maxval )
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel p;

    do
	{
	p = random_color( maxval );
	}
    while ( PPM_LUMIN( p ) > maxval * DARK_THRESH );

    return p;
    }

static pixel
average_two_colors( p1, p2 )
pixel p1, p2;
    {
    pixel p;

    PPM_ASSIGN(
	p, ( (int) PPM_GETR(p1) + (int) PPM_GETR(p2) ) / 2,
	( (int) PPM_GETG(p1) + (int) PPM_GETG(p2) ) / 2,
	( (int) PPM_GETB(p1) + (int) PPM_GETB(p2) ) / 2 );

    return p;
    }

#if __STDC__
static void
average_drawproc( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata )
#else /*__STDC__*/
static void
average_drawproc( pixels, cols, rows, maxval, col, row, clientdata )
    pixel** pixels;
    int cols, rows, col, row;
    pixval maxval;
    char* clientdata;
#endif /*__STDC__*/
    {
    if ( col >= 0 && col < cols && row >= 0 && row < rows )
	pixels[row][col] =
	    average_two_colors( pixels[row][col], *( (pixel*) clientdata ) );
    }

/* Gingham stuff. */

#if __STDC__
static void
gingham2( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
gingham2( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    int colso2, rowso2;
    pixel backcolor, forecolor;

    colso2 = cols / 2;
    rowso2 = rows / 2;
    backcolor = random_dark_color( maxval );
    forecolor = random_bright_color( maxval );

    /* Warp. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, colso2, rows, PPMD_NULLDRAWPROC,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, colso2, 0, cols - colso2, rows,
	PPMD_NULLDRAWPROC, (char*) &forecolor );

    /* Woof. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rowso2, average_drawproc,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rowso2, cols, rows - rowso2,
	average_drawproc, (char*) &forecolor );
    }

#if __STDC__
static void
gingham3( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
gingham3( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    int colso4, rowso4;
    pixel backcolor, fore1color, fore2color;

    colso4 = cols / 4;
    rowso4 = rows / 4;
    backcolor = random_dark_color( maxval );
    fore1color = random_bright_color( maxval );
    fore2color = random_bright_color( maxval );

    /* Warp. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, colso4, rows, PPMD_NULLDRAWPROC,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, colso4, 0, colso4, rows, PPMD_NULLDRAWPROC,
	(char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 2 * colso4, 0, colso4, rows,
	PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 3 * colso4, 0, cols - colso4, rows,
	PPMD_NULLDRAWPROC, (char*) &fore1color );

    /* Woof. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rowso4, average_drawproc,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rowso4, cols, rowso4, average_drawproc,
	(char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 2 * rowso4, cols, rowso4,
	average_drawproc, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 3 * rowso4, cols, rows - rowso4,
	average_drawproc, (char*) &fore1color );
    }

#if __STDC__
static void
madras( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
madras( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    int cols2, rows2, cols3, rows3, cols12, rows12, cols6a, rows6a, cols6b,
	rows6b;
    pixel backcolor, fore1color, fore2color;

    cols2 = cols * 2 / 44;
    rows2 = rows * 2 / 44;
    cols3 = cols * 3 / 44;
    rows3 = rows * 3 / 44;
    cols12 = cols - 10 * cols2 - 4 * cols3;
    rows12 = rows - 10 * rows2 - 4 * rows3;
    cols6a = cols12 / 2;
    rows6a = rows12 / 2;
    cols6b = cols12 - cols6a;
    rows6b = rows12 - rows6a;
    backcolor = random_dark_color( maxval );
    fore1color = random_bright_color( maxval );
    fore2color = random_bright_color( maxval );

    /* Warp. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols2, rows, PPMD_NULLDRAWPROC,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols2, 0, cols3, rows, PPMD_NULLDRAWPROC,
	(char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols2 + cols3, 0, cols2, rows,
	PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 2 * cols2 + cols3, 0, cols2, rows,
	PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 3 * cols2 + cols3, 0, cols2, rows,
	PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 4 * cols2 + cols3, 0, cols6a, rows,
	PPMD_NULLDRAWPROC, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 4 * cols2 + cols3 + cols6a, 0, cols2, rows,
	PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 5 * cols2 + cols3 + cols6a, 0, cols3, rows,
	PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 5 * cols2 + 2 * cols3 + cols6a, 0, cols2,
	rows, PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 6 * cols2 + 2 * cols3 + cols6a, 0, cols3,
	rows, PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 6 * cols2 + 3 * cols3 + cols6a, 0, cols2,
	rows, PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 7 * cols2 + 3 * cols3 + cols6a, 0, cols6b,
	rows, PPMD_NULLDRAWPROC, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 7 * cols2 + 3 * cols3 + cols6a + cols6b, 0,
	cols2, rows, PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 8 * cols2 + 3 * cols3 + cols6a + cols6b, 0,
	cols2, rows, PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 9 * cols2 + 3 * cols3 + cols6a + cols6b, 0,
	cols2, rows, PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 10 * cols2 + 3 * cols3 + cols6a + cols6b, 0,
	cols3, rows, PPMD_NULLDRAWPROC, (char*) &fore1color );

    /* Woof. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rows2, average_drawproc,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows2, cols, rows3, average_drawproc,
	(char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows2 + rows3, cols, rows2,
	average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 2 * rows2 + rows3, cols, rows2,
	average_drawproc, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 3 * rows2 + rows3, cols, rows2,
	average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 4 * rows2 + rows3, cols, rows6a,
	average_drawproc, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 4 * rows2 + rows3 + rows6a, cols, rows2,
	average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 5 * rows2 + rows3 + rows6a, cols, rows3,
	average_drawproc, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 5 * rows2 + 2 * rows3 + rows6a, cols,
	rows2, average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 6 * rows2 + 2 * rows3 + rows6a, cols,
	rows3, average_drawproc, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 6 * rows2 + 3 * rows3 + rows6a, cols,
	rows2, average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 7 * rows2 + 3 * rows3 + rows6a, cols,
	rows6b, average_drawproc, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 7 * rows2 + 3 * rows3 + rows6a + rows6b,
	cols, rows2, average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 8 * rows2 + 3 * rows3 + rows6a + rows6b,
	cols, rows2, average_drawproc, (char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 9 * rows2 + 3 * rows3 + rows6a + rows6b,
	cols, rows2, average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 10 * rows2 + 3 * rows3 + rows6a + rows6b,
	cols, rows3, average_drawproc, (char*) &fore2color );
    }

#if __STDC__
static void
tartan( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
tartan( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    int cols1, rows1, cols3, rows3, cols10, rows10, cols5a, rows5a, cols5b,
	rows5b;
    pixel backcolor, fore1color, fore2color;

    cols1 = cols / 22;
    rows1 = rows / 22;
    cols3 = cols * 3 / 22;
    rows3 = rows * 3 / 22;
    cols10 = cols - 3 * cols1 - 3 * cols3;
    rows10 = rows - 3 * rows1 - 3 * rows3;
    cols5a = cols10 / 2;
    rows5a = rows10 / 2;
    cols5b = cols10 - cols5a;
    rows5b = rows10 - rows5a;
    backcolor = random_dark_color( maxval );
    fore1color = random_bright_color( maxval );
    fore2color = random_bright_color( maxval );

    /* Warp. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols5a, rows, PPMD_NULLDRAWPROC,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols5a, 0, cols1, rows, PPMD_NULLDRAWPROC,
	(char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols5a + cols1, 0, cols5b, rows,
	PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols10 + cols1, 0, cols3, rows,
	PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols10 + cols1 + cols3, 0, cols1, rows,
	PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols10 + 2 * cols1 + cols3, 0, cols3, rows,
	PPMD_NULLDRAWPROC, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols10 + 2 * cols1 + 2 * cols3, 0, cols1,
	rows, PPMD_NULLDRAWPROC, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, cols10 + 3 * cols1 + 2 * cols3, 0, cols3,
	rows, PPMD_NULLDRAWPROC, (char*) &fore2color );

    /* Woof. */
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rows5a, average_drawproc,
	(char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows5a, cols, rows1, average_drawproc,
	(char*) &fore1color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows5a + rows1, cols, rows5b,
	average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows10 + rows1, cols, rows3,
	average_drawproc, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows10 + rows1 + rows3, cols, rows1,
	average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows10 + 2 * rows1 + rows3, cols, rows3,
	average_drawproc, (char*) &fore2color );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows10 + 2 * rows1 + 2 * rows3, cols,
	rows1, average_drawproc, (char*) &backcolor );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, rows10 + 3 * rows1 + 2 * rows3, cols,
	rows3, average_drawproc, (char*) &fore2color );
    }

/* Poles stuff. */

#define MAXPOLES 500

#if __STDC__
static void
poles( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
poles( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    int poles, i, xs[MAXPOLES], ys[MAXPOLES], col, row;
    pixel colors[MAXPOLES];

    poles = cols * rows / 30000;

    /* Place and color poles randomly. */
    for ( i = 0; i < poles; ++i )
	{
	xs[i] = random() % cols;
	ys[i] = random() % rows;
	colors[i] = random_bright_color( maxval );
	}

    /* Now interpolate points. */
    for ( row = 0; row < rows; ++row )
	for ( col = 0; col < cols; ++col )
	    {
	    register long dist1, dist2, newdist, r, g, b;
	    pixel color1, color2;

	    /* Find two closest poles. */
	    dist1 = dist2 = 2000000000;
	    for ( i = 0; i < poles; ++i )
		{
		newdist = ( col - xs[i] ) * ( col - xs[i] ) +
			  ( row - ys[i] ) * ( row - ys[i] );
		if ( newdist < dist1 )
		    {
		    dist1 = newdist;
		    color1 = colors[i];
		    }
		else if ( newdist < dist2 )
		    {
		    dist2 = newdist;
		    color2 = colors[i];
		    }
		}

	    /* And assign interpolated color. */
	    newdist = dist1 + dist2;
	    r = PPM_GETR(color1)*dist2/newdist + PPM_GETR(color2)*dist1/newdist;
	    g = PPM_GETG(color1)*dist2/newdist + PPM_GETG(color2)*dist1/newdist;
	    b = PPM_GETB(color1)*dist2/newdist + PPM_GETB(color2)*dist1/newdist;
	    PPM_ASSIGN( pixels[row][col], r, g, b );
	    }
    }

/* Squig stuff. */

#define SQUIGS 5
#define SQ_POINTS 7
#define SQ_MAXCIRCLE_POINTS 5000

static int sq_radius, sq_circlecount;
static pixel sq_colors[SQ_MAXCIRCLE_POINTS];
static int sq_xoffs[SQ_MAXCIRCLE_POINTS], sq_yoffs[SQ_MAXCIRCLE_POINTS];

#if __STDC__
static void
sq_measurecircle_drawproc( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata )
#else /*__STDC__*/
static void
sq_measurecircle_drawproc( pixels, cols, rows, maxval, col, row, clientdata )
    pixel** pixels;
    int cols, rows, col, row;
    pixval maxval;
    char* clientdata;
#endif /*__STDC__*/
    {
    sq_xoffs[sq_circlecount] = col;
    sq_yoffs[sq_circlecount] = row;
    ++sq_circlecount;
    }

#if __STDC__
static void
sq_rainbowcircle_drawproc( pixel** pixels, int cols, int rows, pixval maxval, int col, int row, char* clientdata )
#else /*__STDC__*/
static void
sq_rainbowcircle_drawproc( pixels, cols, rows, maxval, col, row, clientdata )
    pixel** pixels;
    int cols, rows, col, row;
    pixval maxval;
    char* clientdata;
#endif /*__STDC__*/
    {
    int i;

    for ( i = 0; i < sq_circlecount; ++i )
	ppmd_point_drawproc(
	    pixels, cols, rows, maxval, col + sq_xoffs[i], row + sq_yoffs[i],
	    (char*) &(sq_colors[i]) );
    }

#if __STDC__
static void
squig( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
squig( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel color;
    int i, j, xc[SQ_POINTS], yc[SQ_POINTS], x0, y0, x1, y1, x2, y2, x3, y3;

    /* Clear image to black. */
    PPM_ASSIGN( color, 0, 0, 0 );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rows, PPMD_NULLDRAWPROC,
	(char*) &color );

    /* Draw the squigs. */
    (void) ppmd_setlinetype( PPMD_LINETYPE_NODIAGS );
    (void) ppmd_setlineclip( 0 );
    for ( i = SQUIGS; i > 0; --i )
	{
	/* Measure circle. */
	sq_radius = ( cols + rows ) / 2 / ( 25 + i * 2 );
	sq_circlecount = 0;
	ppmd_circle(
	    pixels, cols, rows, maxval, 0, 0, sq_radius,
	    sq_measurecircle_drawproc, 0 );
	sq_assign_colors( sq_circlecount, maxval, sq_colors );

	/* Choose wrap-around point. */
	switch ( random() % 4 )
	    {
	    case 0:
	    x1 = random() % cols;
	    y1 = 0;
	    if ( x1 < cols / 2 )
		xc[0] = random() % ( x1 * 2 );
	    else
		xc[0] = cols - 1 - random() % ( ( cols - x1 ) * 2 );
	    yc[0] = random() % rows;
	    x2 = x1;
	    y2 = rows - 1;
	    xc[SQ_POINTS - 1] = 2 * x2 - xc[0];
	    yc[SQ_POINTS - 1] = y2 - yc[0];
	    x0 = xc[SQ_POINTS - 1];
	    y0 = yc[SQ_POINTS - 1] - rows;
	    x3 = xc[0];
	    y3 = yc[0] + rows;
	    break;

	    case 1:
	    x2 = random() % cols;
	    y2 = 0;
	    if ( x2 < cols / 2 )
		xc[SQ_POINTS - 1] = random() % ( x2 * 2 );
	    else
		xc[SQ_POINTS - 1] = cols - 1 - random() % ( ( cols - x2 ) * 2 );
	    yc[SQ_POINTS - 1] = random() % rows;
	    x1 = x2;
	    y1 = rows - 1;
	    xc[0] = 2 * x1 - xc[SQ_POINTS - 1];
	    yc[0] = y1 - yc[SQ_POINTS - 1];
	    x0 = xc[SQ_POINTS - 1];
	    y0 = yc[SQ_POINTS - 1] + rows;
	    x3 = xc[0];
	    y3 = yc[0] - rows;
	    break;

	    case 2:
	    x1 = 0;
	    y1 = random() % rows;
	    xc[0] = random() % cols;
	    if ( y1 < rows / 2 )
		yc[0] = random() % ( y1 * 2 );
	    else
		yc[0] = rows - 1 - random() % ( ( rows - y1 ) * 2 );
	    x2 = cols - 1;
	    y2 = y1;
	    xc[SQ_POINTS - 1] = x2 - xc[0];
	    yc[SQ_POINTS - 1] = 2 * y2 - yc[0];
	    x0 = xc[SQ_POINTS - 1] - cols;
	    y0 = yc[SQ_POINTS - 1];
	    x3 = xc[0] + cols;
	    y3 = yc[0];
	    break;

	    case 3:
	    x2 = 0;
	    y2 = random() % rows;
	    xc[SQ_POINTS - 1] = random() % cols;
	    if ( y2 < rows / 2 )
		yc[SQ_POINTS - 1] = random() % ( y2 * 2 );
	    else
		yc[SQ_POINTS - 1] = rows - 1 - random() % ( ( rows - y2 ) * 2 );
	    x1 = cols - 1;
	    y1 = y2;
	    xc[0] = x1 - xc[SQ_POINTS - 1];
	    yc[0] = 2 * y1 - yc[SQ_POINTS - 1];
	    x0 = xc[SQ_POINTS - 1] + cols;
	    y0 = yc[SQ_POINTS - 1];
	    x3 = xc[0] - cols;
	    y3 = yc[0];
	    break;
	    }

	for ( j = 1; j < SQ_POINTS - 1; ++j )
	    {
	    xc[j] = ( random() % ( cols - 2 * sq_radius ) ) + sq_radius;
	    yc[j] = ( random() % ( rows - 2 * sq_radius ) ) + sq_radius;
	    }

	ppmd_line(
	    pixels, cols, rows, maxval, x0, y0, x1, y1,
	    sq_rainbowcircle_drawproc, 0 );
	ppmd_polyspline(
	    pixels, cols, rows, maxval, x1, y1, SQ_POINTS, xc, yc, x2, y2,
	    sq_rainbowcircle_drawproc, 0 );
	ppmd_line(
	    pixels, cols, rows, maxval, x2, y2, x3, y3,
	    sq_rainbowcircle_drawproc, 0 );
	}
    }

#if __STDC__
static void
sq_assign_colors( int circlecount, pixval maxval, pixel* colors )
#else /*__STDC__*/
static void
sq_assign_colors( circlecount, maxval, colors )
    int circlecount;
    pixval maxval;
    pixel* colors;
#endif /*__STDC__*/
    {
    pixel rc1, rc2, rc3;
    float cco3;
    int i;

    rc1 = random_bright_color( maxval );
    rc2 = random_bright_color( maxval );
    rc3 = random_bright_color( maxval );
    cco3 = ( circlecount - 1 ) / 3.0;

    for ( i = 0; i < circlecount ; ++i )
	{
	if ( i < cco3 )
	    PPM_ASSIGN(
		colors[i],
		(float) PPM_GETR(rc1) +
		    ( (float) PPM_GETR(rc2) - (float) PPM_GETR(rc1) ) *
		    (float) i / cco3,
		(float) PPM_GETG(rc1) +
		    ( (float) PPM_GETG(rc2) - (float) PPM_GETG(rc1) ) *
		    (float) i / cco3,
		(float) PPM_GETB(rc1) +
		    ( (float) PPM_GETB(rc2) - (float) PPM_GETB(rc1) ) *
		    (float) i / cco3 );
	else if ( i < 2.0 * cco3 )
	    PPM_ASSIGN(
		colors[i],
		(float) PPM_GETR(rc2) +
		    ( (float) PPM_GETR(rc3) - (float) PPM_GETR(rc2) ) *
		    ( (float) i / cco3 - 1.0 ),
		(float) PPM_GETG(rc2) +
		    ( (float) PPM_GETG(rc3) - (float) PPM_GETG(rc2) ) *
		    ( (float) i / cco3 - 1.0 ),
		(float) PPM_GETB(rc2) +
		    ( (float) PPM_GETB(rc3) - (float) PPM_GETB(rc2) ) *
		    ( (float) i / cco3 - 1.0 ) );
	else
	    PPM_ASSIGN(
		colors[i],
		(float) PPM_GETR(rc3) +
		    ( (float) PPM_GETR(rc1) - (float) PPM_GETR(rc3) ) *
		    ( (float) i / cco3 - 2.0 ),
		(float) PPM_GETG(rc3) +
		    ( (float) PPM_GETG(rc1) - (float) PPM_GETG(rc3) ) *
		    ( (float) i / cco3 - 2.0 ),
		(float) PPM_GETB(rc3) +
		    ( (float) PPM_GETB(rc1) - (float) PPM_GETB(rc3) ) *
		    ( (float) i / cco3 - 2.0 ) );
	}
    }

/* Camouflage stuff. */

#if __STDC__
static pixel
random_camo_color( pixval maxval )
#else /*__STDC__*/
static pixel
random_camo_color( maxval )
    pixval maxval;
#endif /*__STDC__*/
    {
    int v1, v2, v3;
    pixel p;

    v1 = ( (int) maxval + 1 ) / 8;
    v2 = ( (int) maxval + 1 ) / 4;
    v3 = ( (int) maxval + 1 ) / 2;

    switch ( random() % 10 )
	{
	case 0:	case 1: case 2: /* light brown */
	PPM_ASSIGN(
	    p, random() % v3 + v3, random() % v3 + v2, random() % v3 + v2 );
	break;

	case 3:	case 4: case 5: /* dark green */
	PPM_ASSIGN( p, random() % v2, random() % v2 + 3 * v1, random() % v2 );
	break;

	case 6:	case 7: /* brown */
	PPM_ASSIGN( p, random() % v2 + v2, random() % v2, random() % v2 );
	break;

	case 8:	case 9: /* dark brown */
	PPM_ASSIGN( p, random() % v1 + v1, random() % v1, random() % v1 );
	break;
	}

    return p;
    }

#if __STDC__
static pixel
random_anticamo_color( pixval maxval )
#else /*__STDC__*/
static pixel
random_anticamo_color( maxval )
    pixval maxval;
#endif /*__STDC__*/
    {
    int v1, v2, v3;
    pixel p;

    v1 = ( (int) maxval + 1 ) / 4;
    v2 = ( (int) maxval + 1 ) / 2;
    v3 = 3 * v1;

    switch ( random() % 15 )
	{
	case 0: case 1:
	PPM_ASSIGN( p, random() % v1 + v3, random() % v2, random() % v2 );
	break;

	case 2: case 3:
	PPM_ASSIGN( p, random() % v2, random() % v1 + v3, random() % v2 );
	break;

	case 4: case 5:
	PPM_ASSIGN( p, random() % v2, random() % v2, random() % v1 + v3 );
	break;

	case 6: case 7: case 8:
	PPM_ASSIGN( p, random() % v2, random() % v1 + v3, random() % v1 + v3 );
	break;

	case 9: case 10: case 11:
	PPM_ASSIGN( p, random() % v1 + v3, random() % v2, random() % v1 + v3 );
	break;

	case 12: case 13: case 14:
	PPM_ASSIGN( p, random() % v1 + v3, random() % v1 + v3, random() % v2 );
	break;

	}

    return p;
    }

static float
rnduni( )
    {
    return random() % 32767 / 32767.0;
    }

#define BLOBRAD 50

#define MIN_POINTS 7
#define MAX_POINTS 13

#define MIN_ELLIPSE_FACTOR 0.5
#define MAX_ELLIPSE_FACTOR 2.0

#define MIN_POINT_FACTOR 0.5
#define MAX_POINT_FACTOR 2.0

#if __STDC__
static void
camo( pixel** pixels, int cols, int rows, pixval maxval, int antiflag )
#else /*__STDC__*/
static void
camo( pixels, cols, rows, maxval, antiflag )
    pixel** pixels;
    int cols, rows, antiflag;
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel color;
    int n, i, cx, cy;
    char* fh;

    /* Clear background. */
    if ( antiflag )
	color = random_anticamo_color( maxval );
    else
	color = random_camo_color( maxval );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rows, PPMD_NULLDRAWPROC,
	(char*) &color );

    n = ( rows * cols ) / ( BLOBRAD * BLOBRAD ) * 5;
    for ( i = 0; i < n; ++i )
	{
	int points, p, xs[MAX_POINTS], ys[MAX_POINTS], x0, y0;
	float a, b, c, theta, tang, tx, ty;

	cx = random() % cols;
	cy = random() % rows;

	points = random() % ( MAX_POINTS - MIN_POINTS + 1 ) + MIN_POINTS;
	a = rnduni() * ( MAX_ELLIPSE_FACTOR - MIN_ELLIPSE_FACTOR ) +
	    MIN_ELLIPSE_FACTOR;
	b = rnduni() * ( MAX_ELLIPSE_FACTOR - MIN_ELLIPSE_FACTOR ) +
	    MIN_ELLIPSE_FACTOR;
	theta = rnduni() * 2.0 * M_PI;
	for ( p = 0; p < points; ++p )
	    {
	    tx = a * sin( p * 2.0 * M_PI / points );
	    ty = b * cos( p * 2.0 * M_PI / points );
	    tang = atan2( ty, tx ) + theta;
	    c = rnduni() * ( MAX_POINT_FACTOR - MIN_POINT_FACTOR ) +
		MIN_POINT_FACTOR;
	    xs[p] = cx + BLOBRAD * c * sin( tang );
	    ys[p] = cy + BLOBRAD * c * cos( tang );
	    }
	x0 = ( xs[0] + xs[points - 1] ) / 2;
	y0 = ( ys[0] + ys[points - 1] ) / 2;

	fh = ppmd_fill_init();

	ppmd_polyspline(
	    pixels, cols, rows, maxval, x0, y0, points, xs, ys, x0, y0,
	    ppmd_fill_drawproc, fh );

	if ( antiflag )
	    color = random_anticamo_color( maxval );
	else
	    color = random_camo_color( maxval );
	ppmd_fill( pixels, cols, rows, maxval, fh, PPMD_NULLDRAWPROC, (char*) &color );
	}
    }

/* Test pattern.  Just a place to put ppmdraw exercises. */

#if __STDC__
static void
test( pixel** pixels, int cols, int rows, pixval maxval )
#else /*__STDC__*/
static void
test( pixels, cols, rows, maxval )
    pixel** pixels;
    int cols, rows;
    pixval maxval;
#endif /*__STDC__*/
    {
    pixel color;
    char* fh;

    /* Clear image to black. */
    PPM_ASSIGN( color, 0, 0, 0 );
    ppmd_filledrectangle(
	pixels, cols, rows, maxval, 0, 0, cols, rows, PPMD_NULLDRAWPROC,
	(char*) &color );

    fh = ppmd_fill_init();

    ppmd_line( pixels, cols, rows, maxval, cols/8, rows/8, cols/2, rows/4, ppmd_fill_drawproc, fh );
    ppmd_line( pixels, cols, rows, maxval, cols/2, rows/4, cols-cols/8, rows/8, ppmd_fill_drawproc, fh );
    ppmd_line( pixels, cols, rows, maxval, cols-cols/8, rows/8, cols/2, rows/2, ppmd_fill_drawproc, fh );
    ppmd_spline3( pixels, cols, rows, maxval, cols/2, rows/2, cols/2-cols/16, rows/2-rows/10, cols/2-cols/8, rows/2, ppmd_fill_drawproc, fh );
    ppmd_spline3( pixels, cols, rows, maxval, cols/2-cols/8, rows/2, cols/4+cols/16, rows/2+rows/10, cols/4, rows/2, ppmd_fill_drawproc, fh );
    ppmd_line( pixels, cols, rows, maxval, cols/4, rows/2, cols/8, rows/2, ppmd_fill_drawproc, fh );
    ppmd_line( pixels, cols, rows, maxval, cols/8, rows/2, cols/8, rows/8, ppmd_fill_drawproc, fh );

    PPM_ASSIGN( color, maxval, maxval, maxval );
    ppmd_fill( pixels, cols, rows, maxval, fh, PPMD_NULLDRAWPROC, (char*) &color );
    }
