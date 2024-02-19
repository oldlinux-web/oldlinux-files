/* pbmreduce.c - read a portable bitmap and reduce it N times
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

#include "pbm.h"

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    register bit** bitslice;
    register bit* newbitrow;
    register bit* nbP;
    int argn, n, rows, cols, format, newrows, newcols;
    int row, col, limitcol, subrow, subcol, count, direction;
    char* usage = "[-floyd|-fs | -threshold] [-value <val>] N [pbmfile]";
    int halftone;
#define QT_FS 1
#define QT_THRESH 2
#define SCALE 1024
#define HALFSCALE 512
    long threshval, sum;
    long* thiserr;
    long* nexterr;
    long* temperr;

    pbm_init( &argc, argv );

    argn = 1;
    halftone = QT_FS;
    threshval = HALFSCALE;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-fs", 2 ) ||
	     pm_keymatch( argv[argn], "-floyd", 2 ) )
	    halftone = QT_FS;
	else if ( pm_keymatch( argv[argn], "-threshold", 2 ) )
	    halftone = QT_THRESH;
	else if ( pm_keymatch( argv[argn], "-value", 2 ) )
	    {
	    float f;

	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &f ) != 1 ||
		 f < 0.0 || f > 1.0 )
		pm_usage( usage );
	    threshval = f * SCALE;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn == argc )
	pm_usage( usage );
    if ( sscanf( argv[argn], "%d", &n ) != 1 )
	pm_usage( usage );
    if ( n < 2 )
	pm_error( "N must be greater than 1" );
    ++argn;

    if ( argn == argc )
	ifp = stdin;
    else
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}

    if ( argn != argc )
	pm_usage( usage );

    pbm_readpbminit( ifp, &cols, &rows, &format );
    bitslice = pbm_allocarray( cols, n );

    newrows = rows / n;
    newcols = cols / n;
    pbm_writepbminit( stdout, newcols, newrows, 0 );
    newbitrow = pbm_allocrow( newcols );

    if ( halftone == QT_FS )
	{
	/* Initialize Floyd-Steinberg. */
	thiserr = (long*) malloc( ( newcols + 2 ) * sizeof(long) );
	nexterr = (long*) malloc( ( newcols + 2 ) * sizeof(long) );
	if ( thiserr == 0 || nexterr == 0 )
	    pm_error( "out of memory" );

	srandom( (int) ( time( 0 ) ^ getpid( ) ) );
	for ( col = 0; col < newcols + 2; ++col )
	    thiserr[col] = ( random( ) % SCALE - HALFSCALE ) / 4;
	    /* (random errors in [-SCALE/8 .. SCALE/8]) */
	}
    direction = 1;

    for ( row = 0; row < newrows; ++row )
	{
	for ( subrow = 0; subrow < n; ++subrow )
	    pbm_readpbmrow( ifp, bitslice[subrow], cols, format );

	if ( halftone == QT_FS )
	    for ( col = 0; col < newcols + 2; ++col )
		nexterr[col] = 0;
	if ( direction )
	    {
	    col = 0;
	    limitcol = newcols;
	    nbP = newbitrow;
	    }
	else
	    {
	    col = newcols - 1;
	    limitcol = -1;
	    nbP = &(newbitrow[col]);
	    }

	do
	    {
	    sum = 0;
	    count = 0;
	    for ( subrow = 0; subrow < n; ++subrow )
		for ( subcol = 0; subcol < n; ++subcol )
		    if ( row * n + subrow < rows && col * n + subcol < cols )
			{
			count += 1;
			if ( bitslice[subrow][col * n + subcol] == PBM_WHITE )
			    sum += 1;
			}
	    sum = ( sum * SCALE ) / count;

	    if ( halftone == QT_FS )
		sum += thiserr[col + 1];

	    if ( sum >= threshval )
		{
		*nbP = PBM_WHITE;
		if ( halftone == QT_FS )
		    sum = sum - threshval - HALFSCALE;
		}
	    else
		*nbP = PBM_BLACK;

	    if ( halftone == QT_FS )
		{
		if ( direction )
		    {
		    thiserr[col + 2] += ( sum * 7 ) / 16;
		    nexterr[col    ] += ( sum * 3 ) / 16;
		    nexterr[col + 1] += ( sum * 5 ) / 16;
		    nexterr[col + 2] += ( sum     ) / 16;
		    }
		else
		    {
		    thiserr[col    ] += ( sum * 7 ) / 16;
		    nexterr[col + 2] += ( sum * 3 ) / 16;
		    nexterr[col + 1] += ( sum * 5 ) / 16;
		    nexterr[col    ] += ( sum     ) / 16;
		    }
		}
	    if ( direction )
		{
		++col;
		++nbP;
		}
	    else
		{
		--col;
		--nbP;
		}
	    }
	while ( col != limitcol );

	pbm_writepbmrow( stdout, newbitrow, newcols, 0 );

	if ( halftone == QT_FS )
	    {
	    temperr = thiserr;
	    thiserr = nexterr;
	    nexterr = temperr;
	    direction = ! direction;
	    }
	}

    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
    }
