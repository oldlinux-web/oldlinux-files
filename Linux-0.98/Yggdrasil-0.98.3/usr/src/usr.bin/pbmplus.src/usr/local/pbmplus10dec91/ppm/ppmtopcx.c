/* ppmtopcx.c - read a portable pixmap and produce a PCX file
**
** Copyright (C) 1990 by Michael Davidson.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <stdio.h>
#include "ppm.h"
#include "ppmcmap.h"

#define MAXCOLORS	256
#define	MAXPLANES	4

/*
 * Pointer to function returning an int
 */
typedef void (* vfunptr) ARGS(( int, int, unsigned char*, int, int ));

static void PCXEncode ARGS(( FILE* fp, int GWidth, int GHeight, int Colors, int Red[], int Green[], int Blue[], vfunptr GetPlanes ));
static void PutPlane ARGS(( FILE* fp, unsigned char* buf, int Size ));
static void ReadPlanes ARGS(( int y, int width, unsigned char* buf, int planes, int bits ));
static void Putword ARGS(( int w, FILE* fp ));
static void Putbyte ARGS(( int b, FILE* fp ));

static pixel** pixels;
static colorhash_table cht;

void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, rows, cols, colors, i;
    pixval maxval;
    pixel black_pixel;
    colorhist_vector chv;
    int Red[MAXCOLORS], Green[MAXCOLORS], Blue[MAXCOLORS];
    char* usage = "[ppmfile]";

    ppm_init( &argc, argv );

    argn = 1;

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );

    pm_close( ifp );

    /* Figure out the colormap. */
    pm_message( "computing colormap..." );
    chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors );
    if ( chv == (colorhist_vector) 0 )
	pm_error(
	    "too many colors - try doing a 'ppmquant %d'", MAXCOLORS );
    pm_message( "%d colors found", colors );

    /* Force black to slot 0 if possible. */
    PPM_ASSIGN(black_pixel, 0, 0, 0 );
    ppm_addtocolorhist(chv, &colors, MAXCOLORS, &black_pixel, 0, 0 );

    /* Now turn the ppm colormap into the appropriate PCX colormap. */
    if ( maxval > 255 )
	pm_message(
	    "maxval is not 255 - automatically rescaling colors" );
    for ( i = 0; i < colors; ++i )
	{
	if ( maxval == 255 )
	    {
	    Red[i] = PPM_GETR( chv[i].color );
	    Green[i] = PPM_GETG( chv[i].color );
	    Blue[i] = PPM_GETB( chv[i].color );
	    }
	else
	    {
	    Red[i] = (int) PPM_GETR( chv[i].color ) * 255 / maxval;
	    Green[i] = (int) PPM_GETG( chv[i].color ) * 255 / maxval;
	    Blue[i] = (int) PPM_GETB( chv[i].color ) * 255 / maxval;
	    }
	}

    /* And make a hash table for fast lookup. */
    cht = ppm_colorhisttocolorhash( chv, colors );
    ppm_freecolorhist( chv );

    /* All set, let's do it. */
    PCXEncode( stdout, cols, rows, colors, Red, Green, Blue, ReadPlanes );

    exit( 0 );
    }

/*****************************************************************************
 *
 * PCXENCODE.C    - PCX Image compression interface
 *
 * PCXEncode( FName, GHeight, GWidth, Colors, Red, Green, Blue, GetPlanes )
 *
 *****************************************************************************/

#define TRUE 1
#define FALSE 0

/* public */

static void
PCXEncode(fp, GWidth, GHeight, Colors, Red, Green, Blue, GetPlanes )
FILE* fp;
int GWidth, GHeight;
int Colors;
int Red[], Green[], Blue[];
vfunptr GetPlanes;
{
	int		BytesPerLine;
	int		Planes;
	int		BitsPerPixel;
	unsigned char	*buf;
	int		i;
	int		n;
	int		y;

	/*
	 * select number of planes and number of bits
	 * per pixel according to number of colors
	 */
	/*
	 * 16 colors or less are handled as 1 bit per pixel
	 * with 1, 2, 3 or 4 color planes.
	 * more than 16 colors are handled as 8 bits per pixel
	 * with 1 plane
	 */
	if (Colors > 16)
	{
		BitsPerPixel	= 8;
		Planes		= 1;
	}
	else
	{
		BitsPerPixel	= 1;
		if (Colors > 8)
			Planes = 4;
		else if (Colors > 4)
			Planes = 3;
		else if (Colors > 2)
			Planes = 2;
		else
			Planes = 1;
	}

        /*
         * Write the PCX header
         */
	Putbyte( 0x0a, fp);		/* .PCX magic number		*/
	Putbyte( 0x05, fp);		/* PC Paintbrush version	*/
	Putbyte( 0x01, fp);		/* .PCX run length encoding	*/
	Putbyte( BitsPerPixel, fp);	/* bits per pixel		*/

        Putword( 0, fp );		/* x1	- image left		*/
        Putword( 0, fp );		/* y1	- image top		*/
	Putword( GWidth-1, fp );	/* x2	- image right		*/
	Putword( GHeight-1, fp );	/* y2	- image bottom		*/

	Putword( GWidth, fp );		/* horizontal resolution	*/
	Putword( GHeight, fp );		/* vertical resolution		*/

        /*
         * Write out the Color Map for images with 16 colors or less
         */
	n = (Colors <= 16) ? Colors : 16;
        for (i = 0; i < n; ++i)
	{
                Putbyte( Red[i], fp );
                Putbyte( Green[i], fp );
                Putbyte( Blue[i], fp );
        }
        for (; i < 16; ++i)
	{
                Putbyte( 255, fp );
                Putbyte( 255, fp );
                Putbyte( 255, fp );
        }

	Putbyte( 0, fp);		/* reserved byte		*/

	Putbyte( Planes, fp);		/* number of color planes	*/

	BytesPerLine	= ((GWidth * BitsPerPixel) + 7) / 8;
	Putword( BytesPerLine, fp );	/* number of bytes per scanline	*/

	Putword( 1, fp);		/* pallette info		*/

	for (i = 0; i < 58; ++i)	/* fill to end of header	*/
		Putbyte( 0, fp );

	buf	= (unsigned char *)malloc( MAXPLANES * BytesPerLine );

	for (y = 0; y < GHeight; ++y)
	{
		(*GetPlanes)(y, GWidth, buf, Planes, BitsPerPixel);

		for (i = 0; i < Planes; ++i)
			PutPlane(fp, buf + (i * BytesPerLine), BytesPerLine);
	}

	/*
	 * color map for > 16 colors is at end of file
	 */
	if (Colors > 16)
	{
		Putbyte( 0x0c, fp);		/* magic for 256 colors */
        	for (i = 0; i < Colors; ++i)
		{
                	Putbyte( Red[i], fp );
                	Putbyte( Green[i], fp );
                	Putbyte( Blue[i], fp );
        	}
	        for (; i < MAXCOLORS; ++i)
		{
                	Putbyte( 255, fp );
                	Putbyte( 255, fp );
                	Putbyte( 255, fp );
        	}
	}

        fclose( fp );
}

static void
PutPlane(fp, buf, Size)
FILE		*fp;
unsigned char	*buf;
int		Size;
{
	unsigned char	*end;
	int		c;
	int		previous;
	int		count;

	end	= buf + Size;

	previous = *buf++;
	count	 = 1;

	while (buf < end)
	{
		c = *buf++;
		if (c == previous && count < 63)
		{
			++count;
			continue;
		}

		if (count > 1 || (previous & 0xc0) == 0xc0)
		{
			count |= 0xc0;
			Putbyte ( count , fp );
		}
		Putbyte(previous, fp);
		previous = c;
		count	= 1;
	}

	if (count > 1 || (previous & 0xc0) == 0xc0)
	{
		count |= 0xc0;
		Putbyte ( count , fp );
	}
	Putbyte(previous, fp);
}

static unsigned long PixMap[8][16] =
{
	0x00000000L,	0x00000080L,	0x00008000L,	0x00008080L,
	0x00800000L,	0x00800080L,	0x00808000L,	0x00808080L,
	0x80000000L,	0x80000080L,	0x80008000L,	0x80008080L,
	0x80800000L,	0x80800080L,	0x80808000L,	0x80808080L,
};

static void
ReadPlanes(y, width, buf, planes, bits)
int		y;
int		width;
unsigned char	*buf;
int		planes;
int		bits;
{
	static int	first_time = 1;
	unsigned char	*plane0, *plane1, *plane2, *plane3;
	int		i, j, x;

	/*
	 * 256 color, 1 plane, 8 bits per pixel
	 */
	if (planes == 1 && bits == 8)
	{
		for (x = 0; x < width; ++x)
			buf[x] = ppm_lookupcolor( cht, &pixels[y][x] );
		return;
	}

	/*
	 * must be 16 colors or less, 4 planes or less, 1 bit per pixel
	 */
	if (first_time)
	{
		for (i = 1; i < 8; ++i)
			for (j = 0; j < 16; ++j)
				PixMap[i][j] = PixMap[0][j] >> i;
		first_time = 0;
	}

	i = (width + 7) / 8;

	plane0	= buf;
	plane1	= plane0 + i;
	plane2	= plane1 + i;
	plane3	= plane2 + i;

	i	= 0;
	x	= 0;

	while ( x < width )
	{
		register unsigned long	t;

		t	 = PixMap[0][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[1][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[2][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[3][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[4][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[5][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[6][ppm_lookupcolor( cht, &pixels[y][x++] )];
		t	|= PixMap[7][ppm_lookupcolor( cht, &pixels[y][x++] )];

		plane0[i] = t;
		plane1[i] = t >> 8;
		plane2[i] = t >> 16;
		plane3[i++] = t >> 24;
	}
}

/*
 * Write out a word to the PCX file
 */
static void
Putword( w, fp )
int w;
FILE *fp;
{
        fputc( w & 0xff, fp );
        fputc( (w / 256) & 0xff, fp );
}

/*
 * Write out a byte to the PCX file
 */
static void
Putbyte( b, fp )
int b;
FILE *fp;
{
        fputc( b & 0xff, fp );
}

/* The End */
