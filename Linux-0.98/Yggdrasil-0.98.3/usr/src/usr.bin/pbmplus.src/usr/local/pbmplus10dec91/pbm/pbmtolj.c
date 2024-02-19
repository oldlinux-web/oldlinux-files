/* pbmtolj.c - read a portable bitmap and produce a LaserJet bitmap file
**	
**	based on pbmtops.c
**
**	Michael Haberler HP Vienna mah@hpuviea.uucp
**				   mcvax!tuvie!mah
**	misfeatures: 
**		no positioning
**
**      Bug fix Dec 12, 1988 :
**              lines in putbit() reshuffled 
**              now runs OK on HP-UX 6.0 with X10R4 and HP Laserjet II
**      Bo Thide', Swedish Institute of Space Physics, Uppsala <bt@irfu.se>
**
** Copyright (C) 1988 by Jef Poskanzer and Michael Haberler.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

static int dpi = 75;

static void putinit ARGS(( void ));
static void putbit ARGS(( bit b ));
static void putrest ARGS(( void ));
static void putitem ARGS(( void ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int argn, rows, cols, format, rucols, padright, row;
    register int nzcol, col;
    char* usage = "[-resolution N] [pbmfile]\n\tresolution = [75|100|150|300] (dpi)";

    pbm_init( &argc, argv );

    argn = 1;

    /* Check for flags. */
    if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-resolution", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &dpi ) != 1 )
		pm_usage( usage );
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pbm_readpbminit( ifp, &cols, &rows, &format );
    bitrow = pbm_allocrow( cols );

    putinit( );
    for ( row = 0; row < rows; ++row )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );

	/* Find rightmost black pixel. */
	for ( nzcol = cols - 1; nzcol >= 0 && bitrow[nzcol] == PBM_WHITE; --nzcol )
	    continue;

	/* Round up to the nearest multiple of 8. */
	rucols = ( nzcol + 8 ) / 8;
	rucols = rucols * 8;
	padright = rucols - (nzcol + 1);

	/* Transfer raster graphics */
 	printf("\033*b%dW",rucols/8);
        for ( col = 0, bP = bitrow; col <= nzcol; ++col, ++bP )
	    putbit( *bP );
	for ( col = 0; col < padright; ++col )
	    putbit( 0 );
        }

    pm_close( ifp );

    putrest( );

    exit( 0 );
    }

static int item, bitsperitem, bitshift, itemsperline, firstitem;

static void
putinit( )
    {
    /* Printer reset. */
    printf("\033E");

    /* Ensure top margin is zero */
    printf("\033&l0E");

    /* Set raster graphics resolution */
    printf("\033*t%dR",dpi);

    /* Start raster graphics, relative adressing */
    printf("\033*r1A");

    itemsperline = 0;
    bitsperitem = 1;
    item = 0;
    bitshift = 7;
    firstitem = 1;
    }

#if __STDC__
static void
putbit( bit b )
#else /*__STDC__*/
static void
putbit( b )
bit b;
#endif /*__STDC__*/
    {
    if ( b == PBM_BLACK )
	item += 1 << bitshift;
    bitshift--;
    if ( bitsperitem == 8 ) {
	putitem( );
        bitshift = 7;
    }
    bitsperitem++;
    }

static void
putrest( )
    {
    if ( bitsperitem > 1 )
	putitem( );

    /* end raster graphics */
    printf( "\033*rB" );

    /* Printer reset. */
    printf("\033E");
    }

static void
putitem( )
    {
    putchar( item );
    bitsperitem = 0;
    item = 0;
    }
