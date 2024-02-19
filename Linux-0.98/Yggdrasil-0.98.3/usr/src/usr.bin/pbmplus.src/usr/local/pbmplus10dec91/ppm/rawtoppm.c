/* rawtoppm.c - convert raw RGB bytes into a portable pixmap
**
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

static void dorowskip ARGS(( FILE* ifp, int rowskip ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel* pixrow;
    register pixel* pP;
    int argn, headerskip, rowskip, rows, cols, row, i;
    register int col;
    int order;
#define ORD_RGB 1
#define ORD_RBG 2
#define ORD_GRB 3
#define ORD_GBR 4
#define ORD_BRG 5
#define ORD_BGR 6
int interleave;
#define INT_PIX 1
#define INT_ROW 2
    int val1, val2, val3;
    gray* grow1;
    gray* grow2;
    gray* grow3;
    register gray* g1P;
    register gray* g2P;
    register gray* g3P;
    char* usage = "[-headerskip N] [-rowskip N] [-rgb|-rbg|-grb|-gbr|-brg|-bgr] [-interpixel|-interrow] <width> <height> [rawfile]";
    double atof();

    ppm_init( &argc, argv );

    argn = 1;
    headerskip = 0;
    rowskip = 0;
    order = ORD_RGB;
    interleave = INT_PIX;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-headerskip", 2 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    headerskip = atoi( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-rowskip", 3 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    rowskip = atoi( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-rgb", 3 ) )
	    order = ORD_RGB;
	else if ( pm_keymatch( argv[argn], "-rbg", 3 ) )
	    order = ORD_RBG;
	else if ( pm_keymatch( argv[argn], "-grb", 3 ) )
	    order = ORD_GRB;
	else if ( pm_keymatch( argv[argn], "-gbr", 3 ) )
	    order = ORD_GBR;
	else if ( pm_keymatch( argv[argn], "-brg", 3 ) )
	    order = ORD_BRG;
	else if ( pm_keymatch( argv[argn], "-bgr", 3 ) )
	    order = ORD_BGR;
	else if ( pm_keymatch( argv[argn], "-interpixel", 7 ) )
	    interleave = INT_PIX;
	else if ( pm_keymatch( argv[argn], "-interrow", 7 ) )
	    interleave = INT_ROW;
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn + 2 > argc )
	pm_usage( usage );

    cols = atoi( argv[argn++] );
    rows = atoi( argv[argn++] );
    if ( cols <= 0 || rows <= 0 )
	pm_usage( usage );

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    ppm_writeppminit( stdout, cols, rows, (pixval) 255, 0 );
    pixrow = ppm_allocrow( cols );

    if ( interleave == INT_ROW )
	{
	grow1 = pgm_allocrow( cols );
	grow2 = pgm_allocrow( cols );
	grow3 = pgm_allocrow( cols );
	}

    for ( i = 0; i < headerskip; ++i )
	{
	val1 = getc( ifp );
	if ( val1 == EOF )
	    pm_error( "EOF / read error" );
	}

    for ( row = 0; row < rows; ++row)
	{
	switch ( interleave )
	    {
	    case INT_PIX:
	    for ( col = 0, pP = pixrow; col < cols; ++col, ++pP )
		{
		val1 = getc( ifp );
		if ( val1 == EOF )
		    pm_error( "EOF / read error" );
		val2 = getc( ifp );
		if ( val2 == EOF )
		    pm_error( "EOF / read error" );
		val3 = getc( ifp );
		if ( val3 == EOF )
		    pm_error( "EOF / read error" );
		switch ( order )
		    {
		    case ORD_RGB:
		    PPM_ASSIGN( *pP, val1, val2, val3 );
		    break;
		    case ORD_RBG:
		    PPM_ASSIGN( *pP, val1, val3, val2 );
		    break;
		    case ORD_GRB:
		    PPM_ASSIGN( *pP, val2, val1, val3 );
		    break;
		    case ORD_GBR:
		    PPM_ASSIGN( *pP, val3, val1, val2 );
		    break;
		    case ORD_BRG:
		    PPM_ASSIGN( *pP, val2, val3, val1 );
		    break;
		    case ORD_BGR:
		    PPM_ASSIGN( *pP, val3, val2, val1 );
		    break;
		    }
		}
	    dorowskip( ifp, rowskip );
	    break;

	    case INT_ROW:
	    for ( col = 0, g1P = grow1; col < cols; ++col, ++g1P )
		{
		val1 = getc( ifp );
		if ( val1 == EOF )
		    pm_error( "EOF / read error" );
		*g1P = val1;
		}
	    dorowskip( ifp, rowskip );
	    for ( col = 0, g2P = grow2; col < cols; ++col, ++g2P )
		{
		val2 = getc( ifp );
		if ( val2 == EOF )
		    pm_error( "EOF / read error" );
		*g2P = val2;
		}
	    dorowskip( ifp, rowskip );
	    for ( col = 0, g3P = grow3; col < cols; ++col, ++g3P )
		{
		val3 = getc( ifp );
		if ( val3 == EOF )
		    pm_error( "EOF / read error" );
		*g3P = val3;
		}
	    dorowskip( ifp, rowskip );
	    for ( col = 0, pP = pixrow, g1P = grow1, g2P = grow2, g3P = grow3;
		  col < cols; ++col, ++pP, ++g1P, ++g2P, ++g3P )
		{
		switch ( order )
		    {
		    case ORD_RGB:
		    PPM_ASSIGN( *pP, *g1P, *g2P, *g3P );
		    break;
		    case ORD_RBG:
		    PPM_ASSIGN( *pP, *g1P, *g3P, *g2P );
		    break;
		    case ORD_GRB:
		    PPM_ASSIGN( *pP, *g2P, *g1P, *g3P );
		    break;
		    case ORD_GBR:
		    PPM_ASSIGN( *pP, *g3P, *g1P, *g2P );
		    break;
		    case ORD_BRG:
		    PPM_ASSIGN( *pP, *g2P, *g3P, *g1P );
		    break;
		    case ORD_BGR:
		    PPM_ASSIGN( *pP, *g3P, *g2P, *g1P );
		    break;
		    }
		}
	    break;
	    }
	ppm_writeppmrow( stdout, pixrow, cols, (pixval) 255, 0 );
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }

static void
dorowskip( ifp, rowskip )
    FILE* ifp;
    int rowskip;
    {
    int i, val;

    for ( i = 0; i < rowskip; ++i )
	{
	val = getc( ifp );
	if ( val == EOF )
	    pm_error( "EOF / read error" );
	}
    }
