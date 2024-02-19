/* pbmtoatk.c - convert portable bitmap to Andrew Toolkit raster object
**
** Copyright (C) 1991 by Bill Janssen.
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

#define DEFAULTSCALE (1<<16)
#define RASTERVERSION 2
#define TRUE		1
#define FALSE		0

void
main( argc, argv )
    int argc;
    char *argv[];
    {
    FILE *ifd;
    bit *bitrow;
    register bit *bP;
    int rows, cols, format, padright, row;
    register int col;
    char name[100], *cp;
    static char hexchar[] = "0123456789abcdef";
    unsigned char curbyte, newbyte;
    int curcount, gather, line;

    pbm_init ( &argc, argv );

    if ( argc > 2 )
	pm_usage( "[pbmfile]" );

    if ( argc == 2 )
	{
	ifd = pm_openr( argv[1] );
	strcpy( name, argv[1] );
	if ( strcmp( name, "-" ) == 0 )
	    strcpy( name, "noname" );

	if ( ( cp = index( name, '.' ) ) != 0 )
	    *cp = '\0';
	}
    else
	{
	ifd = stdin;
	strcpy( name, "noname" );
	}

    pbm_readpbminit( ifd, &cols, &rows, &format );
    bitrow = pbm_allocrow( cols );

    /* Compute padding to round cols up to the nearest multiple of 16. */
    padright = ( ( cols + 15 ) / 16 ) * 16 - cols;

    printf ("\\begindata{raster,%d}\n", 1);
    printf ("%ld %ld %ld %ld ", RASTERVERSION, 
	     0, DEFAULTSCALE, DEFAULTSCALE);
    printf ("%ld %ld %ld %ld\n",
	     0, 0, cols, rows);	/* subraster */
    printf ("bits %ld %ld %ld\n", 1, cols, rows);

    for ( row = 0; row < rows; row++ )
	{
	  pbm_readpbmrow( ifd, bitrow, cols, format );
	  bP = bitrow;
	  gather = 0;
	  newbyte = 0;
	  curbyte = 0;
	  curcount = 0;
	  col = 0;
	  while (col < cols)
	    {
	      if (gather > 7)
		{
		  process_atk_byte (&curcount, &curbyte, stdout, newbyte, FALSE);
		  gather = 0;
		  newbyte = 0;
		}
	      newbyte = (newbyte << 1) | (*bP++);
	      gather += 1;
	      col += 1;
	    }

	  if (gather > 0)
	    {
	      newbyte = (newbyte << (8 - gather));
	      process_atk_byte (&curcount, &curbyte, stdout, newbyte, TRUE);
	    }
        }

    pm_close( ifd );
    
    printf ("\\enddata{raster, %d}\n", 1);

    exit( 0 );
  }

write_atk_bytes (file, curbyte, curcount)
     FILE *file;
     unsigned char curbyte;
     unsigned int curcount;
{
    /* codes for data stream */
#define WHITEZERO	'f'
#define WHITETWENTY	'z'
#define BLACKZERO	'F'
#define BLACKTWENTY	'Z'
#define OTHERZERO	0x1F

#define	WHITEBYTE	0x00
#define	BLACKBYTE	0xFF

    /* WriteRow table for conversion of a byte value to two character
hex representation */

    static unsigned char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

  switch (curbyte) {
  case WHITEBYTE:
    while (curcount > 20) 
      fputc(WHITETWENTY, file),
      curcount -= 20;
    fputc(WHITEZERO + curcount, file);
    break;
  case BLACKBYTE:
    while (curcount > 20) 
      fputc(BLACKTWENTY, file),
      curcount -= 20;
    fputc(BLACKZERO + curcount, file);
    break;
  default:
    while (curcount > 16)
      fputc(OTHERZERO+16, file),
      fputc(hex[curbyte / 16], file),
      fputc(hex[curbyte & 15], file),
      curcount -= 16;
    if (curcount > 1)
      fputc(OTHERZERO+curcount, file);
    else ;  /* the byte written will represent a single instance */
    fputc(hex[curbyte / 16], file);
    fputc(hex[curbyte & 15], file);
  }
}

process_atk_byte (pcurcount, pcurbyte, file, newbyte, eolflag)
int *pcurcount;
unsigned char *pcurbyte;
FILE *file;
unsigned char newbyte;
int eolflag;
{
    int curcount = *pcurcount;
    unsigned char curbyte = *pcurbyte;

    if (curcount < 1)
    {
	*pcurbyte = curbyte = newbyte;
	*pcurcount = curcount = 1;
    }
    else if (newbyte == curbyte)
    {
	*pcurcount = (curcount += 1);
    }

    if (curcount > 0 && newbyte != curbyte)
      {
	write_atk_bytes (file, curbyte, curcount);
	*pcurcount = 1;
	*pcurbyte = newbyte;
      }

    if (eolflag)
      {
	write_atk_bytes (file, *pcurbyte, *pcurcount);
	fprintf (file, " |\n");
	*pcurcount = 0;
	*pcurbyte = 0;
      }
}
