/*
 * pcxtoppm.c - Converts from a PC Paintbrush PCX file to a PPM file.
 *
 * Copyright (c) 1990 by Michael Davidson
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */

#include	<stdio.h>
#include	"ppm.h"

#define	PCX_MAGIC	0x0a		/* PCX magic number		*/
#define	PCX_HDR_SIZE	128		/* size of PCX header		*/
#define	PCX_256_COLORS	0x0c		/* magic number for 256 colors	*/

#define	MAXCOLORS   	256
#define	MAXPLANES	4
#define	PCX_MAXVAL	255

static void read_pcx_image ARGS(( FILE *fp, unsigned char *buf, int BytesPerLine, int Planes, int Height ));
static void pcx_planes_to_pixels ARGS(( unsigned char *pixels, unsigned char *bitplanes, int bytesperline, int planes, int bitsperpixel ));
static void pcx_unpack_pixels ARGS(( unsigned char *pixels, unsigned char *bitplanes, int bytesperline, int planes, int bitsperpixel ));
static int GetByte ARGS(( FILE *fp ));
static int GetWord ARGS(( FILE *fp ));

void
main(argc, argv)
    int		argc;
    char	*argv[];
{
    register int	i;
    FILE		*ifp;
    char		*ifname;
    int			Version;
    int			Xmin, Ymin, Xmax, Ymax;
    int			Width, Height;
    register int	x, y;
    int			Planes;
    int			BitsPerPixel;
    int			BytesPerLine;
    unsigned char	Red[MAXCOLORS], Green[MAXCOLORS], Blue[MAXCOLORS];
    unsigned char	*pcximage;
    unsigned char	*pcxplanes;
    unsigned char	*pcxpixels;
    pixel		**pixels;

    ppm_init( &argc, argv );

    switch (argc)
    {
	 case 1:
	     ifname	= "standard input";
	     ifp	= stdin;
	     break;
	 case 2:
	     ifname	= argv[1];
	     ifp	= pm_openr(ifname);
	     break;
	 default:
	     pm_usage("[pcxfile]");
	     break;
    }

    /*
     * read the PCX header
     */
    if (GetByte(ifp) != PCX_MAGIC)
	 pm_error("%s is not a PCX file", ifname );

    Version	= GetByte(ifp);	 /* get version #			*/

    if (GetByte(ifp) != 1)	 /* check for PCX run length encoding	*/
	 pm_error("%s has unknown encoding scheme", ifname );

    BitsPerPixel= GetByte(ifp);
    Xmin	= GetWord(ifp);
    Ymin	= GetWord(ifp);
    Xmax	= GetWord(ifp);
    Ymax	= GetWord(ifp);

    Width	= (Xmax - Xmin) + 1;
    Height	= (Ymax - Ymin) + 1;

    (void) GetWord(ifp);		/* ignore horizontal resolution	*/
    (void) GetWord(ifp);		/* ignore vertical resolution	*/

    /*
     * get the 16-color color map
     */
    for (i = 0; i < 16; i++)
    {
	 Red[i]	   = GetByte(ifp);
	 Green[i]  = GetByte(ifp);
	 Blue[i]   = GetByte(ifp);
    }

    (void) GetByte(ifp);		/* skip reserved byte	 */
    Planes	= GetByte(ifp);		/* # of color planes	 */
    BytesPerLine= GetWord(ifp);		/* # of bytes per line	 */
    (void) GetWord(ifp);		/* ignore palette info	 */

    /*
     * check that we can handle this image format
     */
    switch (BitsPerPixel)
    {
	case 1:
	    if (Planes > 4)
		pm_error("can't handle image with more than 4 planes");
	    break;

	case 2:
	case 4:
	case 8:
	    if (Planes == 1)
		break;
	default:
	    pm_error("can't handle %d bits per pixel image with %d planes",
			BitsPerPixel,Planes);
    }


    /*
     * read the pcx format image
     */
    fseek(ifp, (long)PCX_HDR_SIZE, 0);
    pcximage	= (unsigned char *)pm_allocrow(BytesPerLine * Planes, Height);
    read_pcx_image(ifp, pcximage, BytesPerLine, Planes, Height);

    /*
     * 256 color images have their color map at the end of the file
     * preceeded by a magic byte
     */
    if (BitsPerPixel == 8)
    {
	if (GetByte(ifp) != PCX_256_COLORS)
	    pm_error("bad color map signature" );

	for (i = 0; i < MAXCOLORS; i++)
	{
	    Red[i]	= GetByte(ifp);
	    Green[i]	= GetByte(ifp);
	    Blue[i]	= GetByte(ifp);
	}
    }

    pixels	= ppm_allocarray(Width, Height);
    pcxpixels	= (unsigned char *)pm_allocrow(Width+7, 1);

    /*
     * convert the image
     */
    for (y = 0; y < Height; y++)
    {
	 pcxplanes = pcximage + (y * BytesPerLine * Planes);

	 if (Planes == 1)
	 {
	     pcx_unpack_pixels(pcxpixels, pcxplanes,
	 	 BytesPerLine, Planes, BitsPerPixel);
	 }
	 else
	 {
	     pcx_planes_to_pixels(pcxpixels, pcxplanes,
	 	 BytesPerLine, Planes, BitsPerPixel);
	 }

	 for (x = 0; x < Width; x++)
	 {
	     i = pcxpixels[x];
	     PPM_ASSIGN(pixels[y][x], Red[i], Green[i], Blue[i]);
	 }
    }

    pm_close(ifp);

    ppm_writeppm(stdout, pixels, Width, Height, (pixval) 255, 0 );

    pm_close(stdout);

    exit(0);
}

static void
read_pcx_image(fp, buf, BytesPerLine, Planes, Height)
    FILE	*fp;
    unsigned char   *buf;
    int	 	BytesPerLine;
    int	 	Planes;
    int	 	Height;
{
    int		c;
    int		nbytes;
    int		count;

    nbytes	= BytesPerLine * Planes * Height;

    while (nbytes > 0)
    {
	 c    = GetByte(fp);
	 if ((c & 0xc0) != 0xc0)
	 {
	     *buf++    = c;
	     --nbytes;
	     continue;
	 }

	 count    = c & 0x3f;
	 c    = GetByte(fp);
	 if (count > nbytes)
	     pm_error("repeat count spans end of image, count = %d, nbytes = %d", count, nbytes);

	 nbytes    -= count;
	 while (--count >= 0)
	     *buf++ = c;
    }
}

/*
 * convert multi-plane format into 1 pixel per byte
 */
static void
pcx_planes_to_pixels(pixels, bitplanes, bytesperline, planes, bitsperpixel)
unsigned char	*pixels;
unsigned char	*bitplanes;
int		bytesperline;
int	 	planes;
int	 	bitsperpixel;
{
    int	 i, j;
    int	 npixels;
    unsigned char    *p;

    if (planes > 4)
	 pm_error("can't handle more than 4 planes" );
    if (bitsperpixel != 1)
	 pm_error("can't handle more than 1 bit per pixel" );

    /*
     * clear the pixel buffer
     */
    npixels = (bytesperline * 8) / bitsperpixel;
    p    = pixels;
    while (--npixels >= 0)
	 *p++ = 0;

    /*
     * do the format conversion
     */
    for (i = 0; i < planes; i++)
    {
	 int pixbit, bits, mask;

	 p    = pixels;
	 pixbit    = (1 << i);
	 for (j = 0; j < bytesperline; j++)
	 {
	     bits = *bitplanes++;
	     for (mask = 0x80; mask != 0; mask >>= 1, p++)
	 	 if (bits & mask)
	 	     *p |= pixbit;
	 }
     }
}

/*
 * convert packed pixel format into 1 pixel per byte
 */
static void
pcx_unpack_pixels(pixels, bitplanes, bytesperline, planes, bitsperpixel)
unsigned char   *pixels;
unsigned char   *bitplanes;
int		bytesperline;
int	 	planes;
int	 	bitsperpixel;
{
    register int	bits;

    if (planes != 1)
	 pm_error("can't handle packed pixels with more than 1 plane" );
    if (bitsperpixel == 8)
    {
	while (--bytesperline >= 0)
	    *pixels++ = *bitplanes++;
    }
    else if (bitsperpixel == 4)
    {
	while (--bytesperline >= 0)
	{
	    bits	= *bitplanes++;
	    *pixels++	= (bits >> 4) & 0x0f;
	    *pixels++	= (bits     ) & 0x0f;
	}
    }
    else if (bitsperpixel == 2)
    {
	while (--bytesperline >= 0)
	{
	    bits	= *bitplanes++;
	    *pixels++	= (bits >> 6) & 0x03;
	    *pixels++	= (bits >> 4) & 0x03;
	    *pixels++	= (bits >> 2) & 0x03;
	    *pixels++	= (bits     ) & 0x03;
	}
    }
    else if (bitsperpixel == 1)
    {
	while (--bytesperline >= 0)
	{
	    bits	= *bitplanes++;
	    *pixels++	= ((bits & 0x80) != 0);
	    *pixels++	= ((bits & 0x40) != 0);
	    *pixels++	= ((bits & 0x20) != 0);
	    *pixels++	= ((bits & 0x10) != 0);
	    *pixels++	= ((bits & 0x08) != 0);
	    *pixels++	= ((bits & 0x04) != 0);
	    *pixels++	= ((bits & 0x02) != 0);
	    *pixels++	= ((bits & 0x01) != 0);
	}
    }
}

static int
GetByte(fp)
FILE    *fp;
{
    int    c;

    if ((c = getc(fp)) == EOF)
	 pm_error("unexpected end of file" );

    return c;
}

static int
GetWord(fp)
FILE    *fp;
{
    int    c;

    c  = GetByte(fp);
    c |= (GetByte(fp) << 8);
    return c;
}
