/* pgmtolispm.c - read a pgm and write a file acceptable to the 
** tv:read-bit-array-file function of TI Explorer and Symbolics Lisp Machines.
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
**   treat this as a graymap instead.  To convert a color image to Lispm 
**   format, you must convert it to a pgm, and hand-edit a color map...  Ick.
*/

#include <stdio.h>
#include "pgm.h"

#define LISPM_MAGIC  "This is a BitMap file"

static void putinit ARGS(( int cols, int rows, int depth ));
static int depth_to_word_size ARGS(( int depth ));
static void putval ARGS(( gray b ));
static void putrest ARGS(( void ));
static void putitem ARGS(( void ));

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    gray *grayrow;
    register gray* gP;
    int rows, cols, depth, format, padright, row, col;
    gray maxval;

    pgm_init( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pgmfile]" );
    if ( argc == 2 )
	ifp = pm_openr( argv[1] );
    else
	ifp = stdin;

    pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
    grayrow = pgm_allocrow( cols );
    depth = pm_maxvaltobits( maxval );

    /* Compute padding to round cols up to the nearest multiple of 32. */
    padright = ( ( cols + 31 ) / 32 ) * 32 - cols;

    putinit( cols, rows, depth );
    for ( row = 0; row < rows; ++row )
	{
	pgm_readpgmrow( ifp, grayrow, cols, maxval, format );
        for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
	    putval( *gP );
	for ( col = 0; col < padright; ++col )
	    putval( 0 );
        }

    pm_close( ifp );

    putrest( );

    exit( 0 );
    }

static unsigned int item;
static unsigned int bitsperitem, maxbitsperitem, bitshift;

static void
putinit( cols, rows, depth )
    int cols, rows, depth;
    {
    int i;
    int cols32 = ( ( cols + 31 ) / 32 ) * 32;	/* Lispms are able to write bit files that are not mod32 wide, but we   */
						/* don't.  This should be ok, since bit arrays which are not mod32 wide */
    printf(LISPM_MAGIC);			/* are pretty useless on a lispm (can't hand them to bitblt).		*/
    pm_writelittleshort( stdout, cols );
    pm_writelittleshort( stdout, rows );
    pm_writelittleshort( stdout, cols32 );
    putchar(depth & 0xFF);

    for ( i = 0; i < 9; ++i )
	putchar( 0 );	/* pad bytes */

    item = 0;
    bitsperitem = 0;
    maxbitsperitem = depth_to_word_size( depth );
    bitshift = 0;
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
      pm_error( "depth was %d, which is not in the range 1-32", depth );
}

#if __STDC__
static void
putval( gray b )
#else /*__STDC__*/
static void
putval( b )
gray b;
#endif /*__STDC__*/
    {
    if ( bitsperitem == 32 )
	putitem( );
    item = item | ( b << bitshift );
    bitsperitem = bitsperitem + maxbitsperitem;
    bitshift = bitshift + maxbitsperitem;
    }

static void
putrest( )
    {
    if ( bitsperitem > 0 )
	putitem( );
    }

static void
putitem( )
    {
    pm_writelittlelong( stdout, ~item );
    item = 0;
    bitsperitem = 0;
    bitshift = 0;
    }
