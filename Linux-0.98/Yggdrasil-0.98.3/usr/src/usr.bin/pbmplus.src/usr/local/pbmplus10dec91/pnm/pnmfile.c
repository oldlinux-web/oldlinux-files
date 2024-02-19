/* pnmfile.c - describe a portable anymap
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

#include "pnm.h"

static void describe_one ARGS(( char* name, FILE* file ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    int argn;
    FILE* ifp;

    pnm_init( &argc, argv );

    if ( argc == 1 )
	describe_one( "stdin", stdin );
    else
	{
	for ( argn = 1; argn < argc; ++argn )
	    {
	    ifp = pm_openr( argv[argn] );
	    describe_one( argv[argn], ifp );
	    pm_close( ifp );
	    }
	}

    exit( 0 );
    }

static void
describe_one( name, file )
    char* name;
    FILE* file;
    {
    xelval maxval;
    int rows, cols, format;

    pnm_readpnminit( file, &cols, &rows, &maxval, &format );

    printf( "%s:\t", name );
    switch ( format )
	{
	case PBM_FORMAT:
	printf( "PBM plain, %d by %d\n", cols, rows );
	break;

	case RPBM_FORMAT:
	printf( "PBM raw, %d by %d\n", cols, rows );
	break;

	case PGM_FORMAT:
	printf( "PGM plain, %d by %d  maxval %d\n", cols, rows, maxval );
	break;

	case RPGM_FORMAT:
	printf( "PGM raw, %d by %d  maxval %d\n", cols, rows, maxval );
	break;

	case PPM_FORMAT:
	printf( "PPM plain, %d by %d  maxval %d\n", cols, rows, maxval );
	break;

	case RPPM_FORMAT:
	printf( "PPM raw, %d by %d  maxval %d\n", cols, rows, maxval );
	break;
	}
    }
