/* lispmtopgm.c - read a file written by the tv:write-bit-array-file function
** of TI Explorer and Symbolics Lisp Machines, and write a PGM.
**
** Written by Jamie Zawinski based on code (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
**   When one writes a multi-plane bitmap with tv:write-bit-array-file, it is
**   usually a color image; but a color map is not written in the file, so we
**   treat this as a graymap instead.  Since the pgm reader can also read pbms,
**   this doesn't matter if you're using only single plane images.
*/

#include <stdio.h>
#include "pgm.h"

#define LISPM_MAGIC  "This is a BitMap file"

static void getinit ARGS(( FILE* file, short* colsP, short* rowsP, short* depthP, short* padrightP ));
static int depth_to_word_size ARGS(( int depth ));
static unsigned int getval ARGS(( FILE* file ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    short rows, cols, padright, row, col;
    short depth;
    int maxval;

    pgm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[lispmfile]" );

    if ( argc == 2 )
        ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    getinit( ifp, &cols, &rows, &depth, &padright );
    maxval = 1 << depth;

    if ( maxval > PGM_MAXMAXVAL )
	pm_error( "depth is too large - try reconfiguring with PGM_BIGGRAYS" );

    pgm_writepgminit( stdout, cols, rows, (gray) maxval, 0 );
    grayrow = pgm_allocrow( ( cols + 7 ) / 8 * 8 );

    for ( row = 0; row < rows; ++row )
	{
        for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
	    *gP = getval( ifp );
	pgm_writepgmrow( stdout, grayrow, cols, (gray) maxval, 0 );
	}
    pm_close( ifp );
    pm_close( stdout );
    exit( 0 );
    }

static long item, bitmask;
static unsigned int bitsperitem, maxbitsperitem, bitshift;

static void
getinit( file, colsP, rowsP, depthP, padrightP )
    FILE* file;
    short* colsP;
    short* rowsP;
    short* padrightP;
    short* depthP;
    {
    short cols_32;
    char magic[sizeof(LISPM_MAGIC)];
    int i;

    for ( i = 0; i < sizeof(magic)-1; ++i )
        magic[i] = getc( file );
    magic[i]='\0';
    if (0 != strcmp(LISPM_MAGIC, magic))
        pm_error( "bad id string in Lispm file" );
    
    if ( pm_readlittleshort( file, colsP ) == -1 )
        pm_error( "EOF / read error" );
    if ( pm_readlittleshort( file, rowsP ) == -1 )
        pm_error( "EOF / read error" );
    if ( pm_readlittleshort( file, &cols_32 ) == -1 )
        pm_error( "EOF / read error" );
    *depthP = getc( file );
    
    if ( *depthP == 0 )
	*depthP = 1;	/* very old file */
    
    *padrightP = ( ( *colsP + 31 ) / 32 ) * 32 - *colsP;
    
    if ( *colsP != (cols_32 - *padrightP) ) {
/*    pm_message( "inconsistent input: Width and Width(mod32) fields don't agree" );  */
/*    *padrightP = cols_32 - *colsP;   */ /*    hmmmm....   */
      /* This is a dilemma.  Usually the output is rounded up to mod32, but not always.
       * For the Lispm code to not round up, the array size must be the same size as the
       * portion being written - that is, the array itself must be an odd size, not just
       * the selected portion.  Since arrays that are odd sizes can't be handed to bitblt,
       * such arrays are probably not image data - so punt on it for now.
       *
       * Also, the lispm code for saving bitmaps has a bug, in that if you are writing a
       * bitmap which is not mod32 across, the file may be up to 7 bits too short!  They
       * round down instead of up.
       *
       * The code in 'pgmtolispm.c' always rounds up to mod32, which is totally reasonable.
       */
      }
    bitsperitem = 0;
    maxbitsperitem = depth_to_word_size( *depthP );
    bitmask = ( 1 << maxbitsperitem ) - 1;		/* for depth=3, mask=00000111 */

    for ( i = 0; i < 9; ++i )
	getc( file );	/* discard bytes reserved for future use */
    }

static int
depth_to_word_size (depth)	/* Lispm architecture specific - if a bitmap is written    */
  int depth;			/* out with a depth of 5, it really has a depth of 8, and  */
{				/* is stored that way in the file.			   */
    if (depth==0 || depth==1)	return ( 1);
    else if (depth ==  2)	return ( 2);
    else if (depth <=  4)	return ( 4);
    else if (depth <=  8)	return ( 8);
    else if (depth <= 16)	return (16);
    else if (depth <= 32)	return (32);
    else
      pm_error( "depth was %d, which is not in the range 1-32.", depth );
}

static unsigned int
getval( file )
    FILE* file;
    {
    unsigned int b;

    if ( bitsperitem == 0 )
	{
	if ( pm_readlittlelong( file, &item ) == -1 )
	    pm_error( "EOF / read error" );
	bitsperitem = 32;
	bitshift = 0;
	item = ~item;
	}
    b = ( ( item >> bitshift ) & bitmask );
    bitsperitem = bitsperitem - maxbitsperitem;
    bitshift = bitshift + maxbitsperitem;
    return b;
    }
