/* pbmtogem.c - read a portable bitmap and produce a GEM .img file
**
** Author: David Beckemeyer (bdt!david)
**
** Much of the code for this program was taken from other
** pbmto* programs.  I just modified the code to produce
** a .img header and generate .img "Bit Strings".
**
** Thanks to Diomidis D. Spinellis for the .img header format.
**
** Copyright (C) 1988 by David Beckemeyer (bdt!david) and Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "pbm.h"

/*
 * File header structure
 */
struct header {
	short           version;/* Image file version */
	unsigned short  hlen;	/* Header length in bytes */
	unsigned short  planes;	/* Number of planes */
	unsigned short  patlen;	/* Pattern definition length (bytes) */
	unsigned short  pxlen;	/* Pixel height (microns) */
	unsigned short  pxht;	/* Pixel height (microns) */
	unsigned short  linewid;/* Scan line width (pixels) */
	unsigned short  nitems;	/* Number of scan line items */
};

#define MAXCOL 60
static unsigned short outrow[MAXCOL];

static void putinit ARGS(( struct header* hdr ));
static void putbit ARGS(( bit b ));
static void putrest ARGS(( void ));
static void putitem ARGS(( void ));
static void putrow ARGS(( void ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    bit* bitrow;
    register bit* bP;
    int rows, cols, format, row, col, pad;
    struct header hd;

    pbm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pbm_readpbminit( ifp, &cols, &rows, &format );
    
    if (cols > MAXCOL * 16)
	cols = MAXCOL * 16;

    bitrow = pbm_allocrow( cols );
    
    hd.version = 1;	/* Image file version */
    hd.hlen = 16;	/* Header length in bytes */
    hd.planes = 1;	/* Number of planes */
    hd.patlen = 2;	/* Pattern definition length (bytes) */
    hd.pxlen = 372;	/* Pixel height (microns) */
    hd.pxht = 372;	/* Pixel height (microns) */
    hd.linewid = ((cols + 15) / 16) * 16;	/* Scan line width (pixels) */
    hd.nitems = rows;	/* Number of scan line items */

    pad = hd.linewid - cols;

    putinit( &hd );
    for ( row = 0; row < rows; ++row )
	{
	pbm_readpbmrow( ifp, bitrow, cols, format );
        for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
	    putbit( *bP );
        for ( col = 0; col < pad; ++col )
	    putbit( 0 );
        }

    pm_close( ifp );

    putrest( );

    exit( 0 );
    }

static short item, outcol, outmax;
static short bitsperitem, bitshift;

static void
putinit( hdr )
    struct header* hdr;
    {
    fwrite( hdr, 32, 1, stdout );
    item = 0;
    bitsperitem = 0;
    bitshift = 15;
    outcol = 0;
    outmax = hdr->linewid / 16;
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
    if ( bitsperitem == 16 )
	putitem( );
    ++bitsperitem;
    if ( b == PBM_BLACK )
	item += 1 << bitshift;
    --bitshift;
    }

static void
putrest( )
    {
    if ( bitsperitem > 0 )
	putitem( );
    if ( outcol > 0 )
	putrow( );
    }

static void
putitem( )
    {
    outrow[outcol++] = item;
    if (outcol >= outmax)
	putrow( );
    item = 0;
    bitsperitem = 0;
    bitshift = 15;
    }

static void
putrow( )
    {
    (void) putc(0x80, stdout);		/* a Bit string */
    (void) putc(outcol*2, stdout);	/* count */
    fwrite( outrow, outcol*2, 1, stdout );
    outcol = 0;
    }		
