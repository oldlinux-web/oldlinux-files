/* pgmtofits.c - read a portable pixmap and produce a FITS file
**
** Copyright (C) 1989 by Wilson H. Bent (whb@hoh-2.att.com).
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pgm.h"

static void putFitsHead ARGS(( int cols, int rows, gray maxval ));
static void write_card ARGS(( char* str ));

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    int argn, rows, cols, i, format;
    register int col, row;
    gray maxval;

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( "[pgmfile]" );

    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    if ( maxval > 255 )
	pm_error( "maxval (%d) is greater than maximum of 255", maxval );
    grayrow = pgm_allocrow( cols );

    putFitsHead( cols, rows, maxval );

    for ( row = 0; row < rows; ++row )
	{
	pgm_readpgmrow( ifp, grayrow, cols, maxval, format );
	for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
	    putchar( *gP );
	}

    for ( i = ( rows * cols ) % 2880; i < 2880; ++i )
	putchar( '\0' );

    pm_close( ifp );

    exit( 0 );
    }

#if __STDC__
static void
putFitsHead( int cols, int rows, gray maxval )
#else /*__STDC__*/
static void
putFitsHead( cols, rows, maxval )
int cols, rows;
gray maxval;
#endif /*__STDC__*/
    {
    int i = 0;
    char card[81];

    sprintf( card, "SIMPLE  =                    T                                                  " );
    write_card( card ); ++i;
    sprintf( card, "BITPIX  =                    8                                                  " );
    write_card( card ); ++i;
    sprintf( card, "NAXIS   =                    2                                                  " );
    write_card( card ); ++i;
    sprintf( card, "NAXIS1  =           %10d                                                  ", cols );
    write_card( card ); ++i;
    sprintf( card, "NAXIS2  =           %10d                                                  ", rows );
    write_card( card ); ++i;
    sprintf( card, "DATAMIN =           %10d                                                  ", 0 );
    write_card( card ); ++i;
    sprintf( card, "DATAMAX =           %10d                                                  ", (int) maxval );
    write_card( card ); ++i;
    sprintf( card, "BZERO   =                  0.0                                                  " );
    write_card( card ); ++i;
    sprintf( card, "BSCALE  =                  1.0                                                  " );
    write_card( card ); ++i;
    sprintf( card, "HISTORY Created by fitstopgm.                                                   " );
    write_card( card ); ++i;
    sprintf( card, "END                                                                             " );
    write_card( card ); ++i;
    sprintf( card, "                                                                                " );
    while ( i < 36 )
	{
	write_card( card ); ++i;
	}
    }

static void
write_card( str )
char* str;
    {
    fwrite( str, sizeof(char), 80, stdout );
    }
