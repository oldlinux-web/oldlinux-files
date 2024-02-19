/* pbmtogo.c - read a portable bitmap and produce a GraphOn terminal raster file
**	
**	Rev 1.1 was based on pbmtolj.c
**
**	Bo Thide', Swedish Institute of Space Physics, bt@irfu.se
**				   
**
** $Log:	pbmtogo.c,v $
 * Revision 1.5  89/11/25  00:24:12  00:24:12  root (Bo Thide)
 * Bug found: The byte after 64 repeated bytes sometimes lost. Fixed.
 * 
 * Revision 1.4  89/11/24  14:56:04  14:56:04  root (Bo Thide)
 * Fixed the command line parsing since pbmtogo now always uses 2D
 * compression.  Added a few comments to the source.
 * 
 * Revision 1.3  89/11/24  13:43:43  13:43:43  root (Bo Thide)
 * Added support for > 63 repeated bytes and > 62 repeated lines in
 * the 2D compression scheme.
 * 
 * Revision 1.2  89/11/15  01:04:47  01:04:47  root (Bo Thide)
 * First version that works reasonably well with GraphOn 2D runlength
 * encoding/compression.
 * 
 * Revision 1.1  89/11/02  23:25:25  23:25:25  root (Bo Thide)
 * Initial revision
 * 
**
** Copyright (C) 1988, 1989 by Jef Poskanzer, Michael Haberler, and Bo Thide'.
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

#define TRUE 1
#define BUFSIZE 132	/* GraphOn has 132 byte/1056 bit wide raster lines */
#define REPEAT_CURRENT_LINE_MASK	0x00 
#define SKIP_AND_PLOT_MASK		0x40 
#define REPEAT_PLOT_MASK		0x80 
#define PLOT_ARBITRARY_DATA_MASK	0xc0 
#define MAX_REPEAT 64

static unsigned char *scanlineptr;		/* Pointer to current scan line byte */

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
    int argn, rows, cols, format, rucols, padright, row, col;
    int nbyte, bytesperrow, ecount, ucount, nout, i, linerepeat;
    int	olditem;
    unsigned char oldscanline[BUFSIZE];
    unsigned char newscanline[BUFSIZE];
    unsigned char diff[BUFSIZE];
    unsigned char buffer[BUFSIZE];
    unsigned char outbuffer[2*(BUFSIZE+1)];	/* Worst case.  Should malloc */
    char* usage = "[-c] [pbmfile]";

    pbm_init( &argc, argv );

    argn = 2;

    /* Check for flags. */
    if (argc > argn + 1)
	pm_usage(usage);

    if (argc == argn)
	ifp = pm_openr( argv[argn-1] );
    else
	ifp = stdin;

    pbm_readpbminit(ifp, &cols, &rows, &format);
    bitrow = pbm_allocrow(cols);

    /* Round cols up to the nearest multiple of 8. */
    rucols = ( cols + 7 ) / 8;
    bytesperrow = rucols;			/* GraphOn uses bytes */
    rucols = rucols * 8;
    padright = rucols - cols;

    for (i = 0; i < BUFSIZE; ++i )
	buffer[i] = 0;
    putinit();

    /* Start donwloading screen raster */
    printf("\033P0;1;0;4;1;%d;%d;1!R1/", rows, rucols);

    linerepeat = 63;			/*  63 means "Start new picture" */
    for (row = 0; row < rows; row++)
    {
      /* Store scan line data in the new scan line vector */
      scanlineptr = newscanline;
      pbm_readpbmrow(ifp, bitrow, cols, format);
      /* Transfer raster graphics */
      for (col = 0, bP = bitrow; col < cols; col++, bP++)
	putbit(*bP);
      for (col = 0; col < padright; col++)
	putbit(0);
      
      /* XOR data from the new scan line with data from the old scan line */
      for (i = 0; i < bytesperrow; i++)
	diff[i] = oldscanline[i]^newscanline[i];

      /*
      ** If the difference map is different from the current internal buffer,
      ** encode the difference and put it in the output buffer. 
      ** Else, increase the counter for the current buffer by one.
      */

      if ((bcmp(buffer, diff, bytesperrow) != 0) || (row == 0))
      {
	/*
	** Since the data in the buffer has changed, send the scan line
	** repeat count to cause the old line(s) to be plotted on the screen,
	** copy the new data into the internal buffer, and reset the counters.
	*/
	putchar(linerepeat);
	for (i = 0; i < bytesperrow; ++i)
	    buffer[i] = diff[i];
	nbyte = 0;			/* Internal buffer byte counter */
	nout = 0;			/* Output buffer byte counter */

	/* Run length encode the new internal buffer (= difference map) */
	while (TRUE)
	{
	  ucount = 0;			/* Unique items counter */
	  do				/* Find unique patterns */
	  {
	    olditem = buffer[nbyte++];
	    ucount++;
	  } while ((olditem != buffer[nbyte])
	    && (ucount < min(bytesperrow, MAX_REPEAT)));

	  if ((ucount != MAX_REPEAT) && (nbyte != bytesperrow))
	  {
	    /* Back up to the last truly unique pattern */
	    ucount--;
	    nbyte--;
	  }

	  if (ucount > 0)		/* Output the unique patterns */
	  {
	    outbuffer[nout++] = (ucount-1) | PLOT_ARBITRARY_DATA_MASK;
	    for (i = nbyte-ucount; i < nbyte; i++)
	      outbuffer[nout++] = buffer[i];
	  }

	  /*
	  ** If we already are at the end of the current scan line, skip the
	  ** rest of the encoding and start with a new scan line.
	  */
	  if (nbyte >= bytesperrow)
	    goto nextrow;

	  ecount = 0;			/* Equal items counter */
	  do				/* Find equal patterns */
	  {
	    olditem = buffer[nbyte++];
	    ecount++;
	  } while ((olditem == buffer[nbyte])
	    && (ecount < min(bytesperrow, MAX_REPEAT)));

	  if (ecount > 1)			/* More than 1 equal pattern */
	  {
	    if (olditem == '\0')		/* White patterns */
	    {
	      if (nbyte >= bytesperrow-1)	/* No more valid data ahead */
	      {
		outbuffer[nout++] = (ecount-2) | SKIP_AND_PLOT_MASK;
		outbuffer[nout++] = buffer[nbyte-1];
	      } 
	      else				/* More valid data ahead */
	      {
		outbuffer[nout++] = (ecount-1) | SKIP_AND_PLOT_MASK;
		outbuffer[nout++] = buffer[nbyte++];
	      } 
	    }
	    else				/* Non-white patterns */
	    {
	      outbuffer[nout++] = (ecount-1) | REPEAT_PLOT_MASK;
	      outbuffer[nout++] = olditem;
	    } /* if (olditem == '\0') */
	  } /* if (ecount > 1) */
	  else
	    nbyte--;				/* No equal items found */

	  if (nbyte >= bytesperrow)
	    goto nextrow;
	} /* while (TRUE) */

nextrow: printf("%d/", nout+1);		/* Total bytes to transfer = nout+1 */
        fflush(stdout);

	/* Output the plot data */
	write(1, outbuffer, nout);

	/* Reset the counters */
	linerepeat = 0;
    }
    else
    {
      linerepeat++;
      if (linerepeat == 62)		/* 62 lines max, then restart */
      {
	putchar(linerepeat);
	printf("%d/", nout+1);
	fflush(stdout);
	write(1, outbuffer, nout);
	linerepeat = 0;
      }
    }

    /* Now we are ready for a new scan line */
    for (i = 0; i < bytesperrow; ++i)
	oldscanline[i] = newscanline[i];
  }
  putchar(linerepeat);			/* For the last line(s) to be plotted */
  pm_close(ifp);
  putrest();
  exit(0);
}

static int item, bitsperitem, bitshift;

static void
putinit()
{
  /* Enter graphics window */
  printf("\0331");

  /* Erase graphics window */
  printf("\033\014");

  /* Set graphics window in raster mode */
  printf("\033r");

  /* Select standard Tek coding **/
  printf("\033[=11l");

  bitsperitem = 1;
  item = 0;
  bitshift = 7;
}

#if __STDC__
static void
putbit(bit b)
#else /*__STDC__*/
static void
putbit(b)
bit b;
#endif /*__STDC__*/
{
  if (b == PBM_BLACK)
    item += 1 << bitshift;
  bitshift--;
  if (bitsperitem == 8)
  {
    putitem();
    bitshift = 7;
  }
  bitsperitem++;
}

static void
putrest()
{
  if (bitsperitem > 1)
      putitem();

  /* end raster downloading */
  printf("\033\134");

  /* Exit raster mode */
  printf("\033t");

  /* Exit graphics window
  printf("\0332"); */
}

static void
putitem()
  {
  *scanlineptr++ = item;
  bitsperitem = 0;
  item = 0;
  }
