/* pgmnorm.c - read a portable graymap and normalize the contrast
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

#include "pgm.h"

static int hist[PGM_MAXMAXVAL+1];

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray maxval;
    gray** grays;
    gray* grayrow;
    register gray* gP;
    int argn, rows, cols, format, row;
    register int col;
    int i, size, cutoff, count, val;
    float bpercent, wpercent;
    int bvalue, wvalue, range;
    int specbpercent, specbvalue, specwpercent, specwvalue;
    char* usage = "[-bpercent N | -bvalue N] [-wpercent N | -wvalue N] [pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;
    bpercent = 2.0;
    wpercent = 1.0;
    specbpercent = specbvalue = specwpercent = specwvalue = 0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-bpercent", 3 ) )
	    {
	    if ( specbvalue )
		pm_error( "only one of -bpercent and -bvalue may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &bpercent ) != 1 )
		pm_usage( usage );
	    if ( bpercent < 0.0  || bpercent > 100.0 )
		pm_error( "black percentage must between 0 and 100" );
	    specbpercent = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-bvalue", 3 ) )
	    {
	    if ( specbpercent )
		pm_error( "only one of -bpercent and -bvalue may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &bvalue ) != 1 )
		pm_usage( usage );
	    if ( bvalue < 0 )
		pm_error( "black value must be >= 0" );
	    specbvalue = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-wpercent", 3 ) )
	    {
	    if ( specwvalue )
		pm_error( "only one of -wpercent and -wvalue may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &wpercent ) != 1 )
		pm_usage( usage );
	    if ( wpercent < 0.0 || wpercent > 100.0 )
		pm_error( "white percentage must be between 0 and 100" );
	    specwpercent = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-wvalue", 3 ) )
	    {
	    if ( specwpercent )
		pm_error( "only one of -wpercent and -wvalue may be specified" );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &wvalue ) != 1 )
		pm_usage( usage );
	    if ( wvalue < 0 )
		pm_error( "white value must be >= 0" );
	    specwvalue = 1;
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

    if ( specbvalue && specwvalue )
	{
	/* Rescale so that bvalue maps to 0, wvalue maps to maxval. */
	pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
	grayrow = pgm_allocrow( cols );
	pgm_writepgminit( stdout, cols, rows, maxval, 0 );
	for ( i = 0; i <= bvalue; ++i )
	    hist[i] = 0;
	for ( i = wvalue; i <= maxval; ++i )
	    hist[i] = maxval;
	range = wvalue - bvalue;
	for ( i = bvalue+1, val = maxval; i < wvalue; ++i, val += maxval )
	    hist[i] = val / range;
	for ( row = 0; row < rows; ++row )
	    {
	    pgm_readpgmrow( ifp, grayrow, cols, maxval, format );
	    for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
		*gP = hist[*gP];
	    pgm_writepgmrow( stdout, grayrow, cols, maxval, 0 );
	    }
	pm_close( ifp );
	}
    else
	{
	grays = pgm_readpgm( ifp, &cols, &rows, &maxval );
	pm_close( ifp );

	/* Build histogram. */
	for ( i = 0; i <= maxval; ++i )
	    hist[i] = 0;
	for ( row = 0; row < rows; ++row )
	    for ( col = 0, gP = grays[row]; col < cols; ++col, ++gP )
		++hist[*gP];
	size = rows * cols;
	if ( ! specbvalue )
	    { /* Compute bvalue from bpercent. */
	    cutoff = size * bpercent / 100.0;
	    count = 0;
	    for ( bvalue = 0; bvalue <= maxval; ++bvalue )
		{
		count += hist[bvalue];
		if ( count > cutoff )
		break;
		}
	    }
	if ( ! specwvalue )
	    { /* Compute wvalue from wpercent. */
	    cutoff = size * wpercent / 100.0;
	    count = 0;
	    for ( wvalue = maxval; wvalue >= 0; wvalue-- )
		{
		count += hist[wvalue];
		if ( count > cutoff )
		    break;
		}
	    }

	/* Now rescale so that bvalue maps to 0, wvalue maps to maxval. */
	pm_message(
	    "remapping %d..%d to %d..%d", bvalue, wvalue, 0, maxval, 0 );
	pgm_writepgminit( stdout, cols, rows, maxval, 0 );
	for ( i = 0; i <= bvalue; ++i )
	    hist[i] = 0;
	for ( i = wvalue; i <= maxval; ++i )
	    hist[i] = maxval;
	range = wvalue - bvalue;
	for ( i = bvalue + 1, val = maxval; i < wvalue; ++i, val += maxval )
	    hist[i] = val / range;
	for ( row = 0; row < rows; ++row )
	    {
	    for ( col = 0, gP = grays[row]; col < cols; ++col, ++gP )
		*gP = hist[*gP];
	    pgm_writepgmrow( stdout, grays[row], cols, maxval, 0 );
	    }
	}

    pm_close( stdout );
    exit( 0 );
    }
