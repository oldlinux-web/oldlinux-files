/* pgmtofs.c - convert portable graymap to Usenix FaceSaver(tm) format
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

#include "pgm.h"

static void putinit ARGS(( int cols, int rows, int bps ));
static void putitem ARGS(( void ));
static void putgray ARGS(( gray g ));
static void putrest ARGS(( void ));

void
main( argc, argv )
int argc;
char* argv[];
    {
    FILE* ifp;
    gray** grays;
    register gray* gP;
    int argn, rows, cols, bps, padright, row, col;
    gray maxval, nmaxval;
    char* usage = "[pgmfile]";

    pgm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	{
	ifp = stdin;
	}

    if ( argn != argc )
	pm_usage( usage );

    grays = pgm_readpgm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );

    /* Figure out bps. */
    bps = pm_maxvaltobits( (int) maxval );
    if ( bps > 2 && bps < 4 )
	bps = 4;
    else if ( bps > 4 && bps < 8 )
	bps = 8;
    else if ( bps > 8 )
	pm_error(
	    "maxval of %d is too large for FaceSaver(tm)", maxval );
    nmaxval = pm_bitstomaxval( bps );
    
    /* Compute padding to round cols * bps up to the nearest multiple of 8. */
    padright = ( ( cols * bps + 7 ) / 8 ) * 8 - cols * bps;

    putinit( cols, rows, bps );
    for ( row = rows - 1; row >= 0; --row )
	{
        for ( col = 0, gP = grays[row]; col < cols; ++col, ++gP )
	    {
	    if ( maxval != nmaxval )
		*gP = (int) *gP * nmaxval / maxval;
	    putgray( *gP );
	    }
	for ( col = 0; col < padright; ++col )
	    putgray( 0 );
        }

    putrest( );

    exit( 0 );
    }


static int bitspersample, item, bitsperitem, bitshift, itemsperline, items;

static void
putinit( cols, rows, bps )
int cols, rows, bps;
    {
    printf( "FirstName: \n" );
    printf( "LastName: \n" );
    printf( "E-mail: \n" );
    printf( "Telephone: \n" );
    printf( "Company: \n" );
    printf( "Address1: \n" );
    printf( "Address2: \n" );
    printf( "CityStateZip: \n" );
    printf( "Date: \n" );
    printf( "PicData: %d %d %d\n", cols, rows, bps );
    printf( "Image: %d %d %d\n", cols, rows, bps );
    printf( "\n" );

    bitspersample = bps;
    itemsperline = items = 0;
    item = 0;
    bitsperitem = 0;
    bitshift = 8 - bitspersample;
    }

static void
putitem( )
    {
    char* hexits = "0123456789abcdef";

    if ( itemsperline == 30 )
	{
	putchar( '\n' );
	itemsperline = 0;
	}
    putchar( hexits[item >> 4] );
    putchar( hexits[item & 15] );
    ++itemsperline;
    ++items;
    item = 0;
    bitsperitem = 0;
    bitshift = 8 - bitspersample;
    }

#if __STDC__
static void
putgray( gray g )
#else /*__STDC__*/
static void
putgray( g )
gray g;
#endif /*__STDC__*/
    {
    if ( bitsperitem == 8 )
	putitem( );
    item += g << bitshift;
    bitsperitem += bitspersample;
    bitshift -= bitspersample;
    }

static void
putrest( )
    {
    if ( bitsperitem > 0 )
	putitem( );
    printf( "\n" );
    }
