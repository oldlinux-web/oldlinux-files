/*
** ppmtopict.c - read a portable pixmap and produce a Macintosh PICT2 file.
**
** Copyright (C) 1990 by Ken Yap <ken@cs.rochester.edu>.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"
#include "ppmcmap.h"

#define HEADER_SIZE		512

#define	RUN_THRESH		3
#define	MAX_RUN			128		/* 0xff = 2, 0xfe = 3, etc */
#define	MAX_COUNT		128		/* 0x00 = 1, 0x01 = 2, etc */

/* Opcodes */
#define PICT_NOP		0x00
#define PICT_clipRgn		0x01
#define PICT_bkPat		0x02
#define PICT_txFont		0x03
#define PICT_txFace		0x04
#define PICT_txMode		0x05
#define PICT_spExtra		0x06
#define PICT_pnSize		0x07
#define PICT_pnMode		0x08
#define PICT_pnPat		0x09
#define PICT_thePat		0x0A
#define PICT_ovSize		0x0B
#define PICT_origin		0x0C
#define PICT_txSize		0x0D
#define PICT_fgColor		0x0E
#define PICT_bkColor		0x0F
#define PICT_txRatio		0x10
#define PICT_picVersion		0x11
#define	PICT_blPixPat		0x12
#define	PICT_pnPixPat		0x13
#define	PICT_fillPixPat		0x14
#define	PICT_pnLocHFrac		0x15
#define	PICT_chExtra		0x16
#define	PICT_rgbFgCol		0x1A
#define	PICT_rgbBkCol		0x1B
#define	PICT_hiliteMode		0x1C
#define	PICT_hiliteColor	0x1D
#define	PICT_defHilite		0x1E
#define	PICT_opColor		0x1F
#define PICT_line		0x20
#define PICT_line_from		0x21
#define PICT_short_line		0x22
#define PICT_short_line_from	0x23
#define PICT_long_text		0x28
#define PICT_DH_text		0x29
#define PICT_DV_text		0x2A
#define PICT_DHDV_text		0x2B
#define PICT_frameRect		0x30
#define PICT_paintRect		0x31
#define PICT_eraseRect		0x32
#define PICT_invertRect		0x33
#define PICT_fillRect		0x34
#define PICT_frameSameRect	0x38
#define PICT_paintSameRect	0x39
#define PICT_eraseSameRect	0x3A
#define PICT_invertSameRect	0x3B
#define PICT_fillSameRect	0x3C
#define PICT_frameRRect		0x40
#define PICT_paintRRect		0x41
#define PICT_eraseRRect		0x42
#define PICT_invertRRect	0x43
#define PICT_fillRRect		0x44
#define PICT_frameSameRRect	0x48
#define PICT_paintSameRRect	0x49
#define PICT_eraseSameRRect	0x4A
#define PICT_invertSameRRect	0x4B
#define PICT_fillSameRRect	0x4C
#define PICT_frameOval		0x50
#define PICT_paintOval		0x51
#define PICT_eraseOval		0x52
#define PICT_invertOval		0x53
#define PICT_fillOval		0x54
#define PICT_frameSameOval	0x58
#define PICT_paintSameOval	0x59
#define PICT_eraseSameOval	0x5A
#define PICT_invertSameOval	0x5B
#define PICT_fillSameOval	0x5C
#define PICT_frameArc		0x60
#define PICT_paintArc		0x61
#define PICT_eraseArc		0x62
#define PICT_invertArc		0x63
#define PICT_fillArc		0x64
#define PICT_frameSameArc	0x68
#define PICT_paintSameArc	0x69
#define PICT_eraseSameArc	0x6A
#define PICT_invertSameArc	0x6B
#define PICT_fillSameArc	0x6C
#define PICT_framePoly		0x70
#define PICT_paintPoly		0x71
#define PICT_erasePoly		0x72
#define PICT_invertPoly		0x73
#define PICT_fillPoly		0x74
#define PICT_frameSamePoly	0x78
#define PICT_paintSamePoly	0x79
#define PICT_eraseSamePoly	0x7A
#define PICT_invertSamePoly	0x7B
#define PICT_fillSamePoly	0x7C
#define PICT_frameRgn		0x80
#define PICT_paintRgn		0x81
#define PICT_eraseRgn		0x82
#define PICT_invertRgn		0x83
#define PICT_fillRgn		0x84
#define PICT_frameSameRgn	0x88
#define PICT_paintSameRgn	0x89
#define PICT_eraseSameRgn	0x8A
#define PICT_invertSameRgn	0x8B
#define PICT_fillSameRgn	0x8C
#define PICT_BitsRect		0x90
#define PICT_BitsRgn		0x91
#define PICT_PackBitsRect	0x98
#define PICT_PackBitsRgn	0x99
#define PICT_shortComment	0xA0
#define PICT_longComment	0xA1
#define PICT_EndOfPicture	0xFF
#define	PICT_headerOp		0x0C00

static void putFill ARGS(( FILE *fd, int n ));
static void putShort ARGS(( FILE *fd, int i ));
static void putLong ARGS(( FILE *fd, long i ));
static void putFixed ARGS(( FILE *fd, int in, int frac ));
static void putRect ARGS(( FILE *fd, int x1, int x2, int y1, int y2 ));
static int putRow ARGS(( FILE *fd, int row, int cols, pixel *rowpixels, char *packed ));

#define MAXCOLORS 256
static colorhash_table cht;

void
main(argc, argv)
int argc;
char *argv[];
{
	FILE *ifp;
	int argn, rows, cols, colors, i, row, oc;
	register pixel **pixels;
	char *packed;
	pixval maxval;
	long lmaxval, rval, gval, bval;
	colorhist_vector chv;

	ppm_init( &argc, argv );

	argn = 1;
	if (argn < argc)
	{
		ifp = pm_openr(argv[1]);
		argn++;
	}
	else
		ifp = stdin;
	if (argn != argc)
		pm_usage("[ppmfile]");

	pixels = ppm_readppm(ifp, &cols, &rows, &maxval);
	if (cols < 8)
		pm_error("ppm input too narrow, must be >= 8 pixels wide" );
	lmaxval = (long)maxval;
	pm_close(ifp);

	/* Figure out the colormap. */
	pm_message("computing colormap..." );
	chv = ppm_computecolorhist(pixels, cols, rows, MAXCOLORS, &colors);
	if (chv == (colorhist_vector) 0)
		pm_error("too many colors - try doing a 'ppmquant %d'", MAXCOLORS);
	pm_message("%d colors found", colors );

	/* Make a hash table for fast color lookup. */
	cht = ppm_colorhisttocolorhash(chv, colors);

	/* write the header */
	putFill(stdout, HEADER_SIZE);

	/* write picSize and picFrame */
	putShort(stdout, 0);
	putRect(stdout, 0, 0, rows, cols);

	/* write version op and version */
	putShort(stdout, PICT_picVersion);
	putShort(stdout, 0x02FF);
	putShort(stdout, PICT_headerOp);
	putLong(stdout, -1L);
	putFixed(stdout, 0, 0);
	putFixed(stdout, 0, 0);
	putFixed(stdout, cols, 0);
	putFixed(stdout, rows, 0);
	putFill(stdout, 4);

	/* seems to be needed by many PICT2 programs */
	putShort(stdout, PICT_clipRgn);
	putShort(stdout, 10);
	putRect(stdout, 0, 0, rows, cols);

	/* write picture */
	putShort(stdout, PICT_PackBitsRect);
	putShort(stdout, cols | 0x8000);
	putRect(stdout, 0, 0, rows, cols);
	putShort(stdout, 0);	/* pmVersion */
	putShort(stdout, 0);	/* packType */
	putLong(stdout, 0L);	/* packSize */
	putFixed(stdout, 72, 0);	/* hRes */
	putFixed(stdout, 72, 0);	/* vRes */
	putShort(stdout, 0);	/* pixelType */
	putShort(stdout, 8);	/* pixelSize */
	putShort(stdout, 1);	/* cmpCount */
	putShort(stdout, 8);	/* cmpSize */
	putLong(stdout, 0L);	/* planeBytes */
	putLong(stdout, 0L);	/* pmTable */
	putLong(stdout, 0L);	/* pmReserved */
	putLong(stdout, 0L);	/* ctSeed */
	putShort(stdout, 0);	/* ctFlags */
	putShort(stdout, colors-1);	/* ctSize */

	/* Write out the colormap. */
	for (i = 0; i < colors; i++)
	{
		putShort(stdout, i);
		rval = PPM_GETR(chv[i].color);
		gval = PPM_GETG(chv[i].color);
		bval = PPM_GETB(chv[i].color);
		if (lmaxval != 65535L)
		{
			rval = rval * 65535L / lmaxval;
			gval = gval * 65535L / lmaxval;
			bval = bval * 65535L / lmaxval;
		}
		putShort(stdout, (short)rval);
		putShort(stdout, (short)gval);
		putShort(stdout, (short)bval);
	}

	putRect(stdout, 0, 0, rows, cols);	/* srcRect */
	putRect(stdout, 0, 0, rows, cols);	/* dstRect */
	putShort(stdout, 0);			/* mode */

	/* Finally, write out the data. */
	packed = (char*) malloc((unsigned)(cols+cols/MAX_COUNT+1));
	oc = 0;
	for (row = 0; row < rows; row++)
		oc += putRow(stdout, row, cols, pixels[row], packed);

	/* if we wrote an odd number of pixdata bytes, pad */
	if (oc & 1)
		(void) putc(0, stdout);
	putShort(stdout, PICT_EndOfPicture);

	lmaxval = ftell(stdout) - HEADER_SIZE;
	if (fseek(stdout, (long)HEADER_SIZE, 0) >= 0)
		putShort(stdout, (short)(lmaxval & 0xffff));

	exit(0);
}

static void
putFill(fd, n)
FILE *fd;
int n;
{
	register int i;

	for (i = 0; i < n; i++)
		(void) putc(0, fd);
}

static void
putShort(fd, i)
FILE *fd;
int i;
{
	(void) putc((i >> 8) & 0xff, fd);
	(void) putc(i & 0xff, fd);
}

#if __STDC__
static void
putLong( FILE *fd, long i )
#else /*__STDC__*/
static void
putLong(fd, i)
FILE *fd;
long i;
#endif /*__STDC__*/
{
	(void) putc((int)((i >> 24) & 0xff), fd);
	(void) putc(((int)(i >> 16) & 0xff), fd);
	(void) putc(((int)(i >> 8) & 0xff), fd);
	(void) putc((int)(i & 0xff), fd);
}

static void
putFixed(fd, in, frac)
FILE *fd;
int in, frac;
{
	putShort(fd, in);
	putShort(fd, frac);
}

static void
putRect(fd, x1, x2, y1, y2)
FILE *fd;
int x1, x2, y1, y2;
{
	putShort(fd, x1);
	putShort(fd, x2);
	putShort(fd, y1);
	putShort(fd, y2);
}

#define	RUNLENGTH
#ifdef	RUNLENGTH

#define		runtochar(c)	(257-(c))
#define		counttochar(c)	((c)-1)

static int
putRow(fd, row, cols, rowpixels, packed)
FILE *fd;
int row, cols;
pixel *rowpixels;
char *packed;
{
	register int i;
	int packcols, count, run, rep, oc;
	register pixel *pP;
	pixel lastp;
	register char *p;

	run = count = 0;
	for (cols--, i = cols, pP = rowpixels + cols, p = packed, lastp = *pP;
		i >= 0; i--, lastp = *pP, pP--)
	{
		if (PPM_EQUAL(lastp, *pP))
			run++;
		else if (run < RUN_THRESH)
		{
			while (run > 0)
			{
				*p++ = ppm_lookupcolor(cht, &lastp);
				run--;
				count++;
				if (count == MAX_COUNT)
				{
					*p++ = counttochar(MAX_COUNT);
					count -= MAX_COUNT;
				}
			}
			run = 1;
		}
		else
		{
			if (count > 0)
				*p++ = counttochar(count);
			count = 0;
			while (run > 0)
			{
				rep = run > MAX_RUN ? MAX_RUN : run;
				*p++ = ppm_lookupcolor(cht, &lastp);
				*p++ = runtochar(rep);
				run -= rep;
			}
			run = 1;
		}
	}
	if (run < RUN_THRESH)
	{
		while (run > 0)
		{
			*p++ = ppm_lookupcolor(cht, &lastp);
			run--;
			count++;
			if (count == MAX_COUNT)
			{
				*p++ = counttochar(MAX_COUNT);
				count -= MAX_COUNT;
			}
		}
	}
	else
	{
		if (count > 0)
			*p++ = counttochar(count);
		count = 0;
		while (run > 0)
		{
			rep = run > MAX_RUN ? MAX_RUN : run;
			*p++ = ppm_lookupcolor(cht, &lastp);
			*p++ = runtochar(rep);
				run -= rep;
		}
		run = 1;
	}
	if (count > 0)
		*p++ = counttochar(count);

	packcols = p - packed;		/* how many did we write? */
	if (cols > 250)
	{
		putShort(fd, packcols);
		oc = packcols + 2;
	}
	else
	{
		(void) putc(packcols, fd);
		oc = packcols + 1;
	}

	/* now write out the packed row */
	while(p != packed)
	{
		--p;
		(void) putc(*p, fd);
	}

	return (oc);
}

#else	/* RUNLENGTH */

/* real dumb putRow with no compression */
static int
putRow(fd, row, cols, rowpixels, packed)
FILE *fd;
int row, cols;
pixel *rowpixels;
char *packed;
{
	register int i, j, bc, oc;
	register pixel *pP;

#if notdef
	bzero(aux, cols); /* aux?? */
#endif /*notdef*/
	bc = cols + (cols + MAX_COUNT - 1) / MAX_COUNT;
	if (bc > 250)
	{
		putShort(fd, bc);
		oc = bc + 2;
	}
	else
	{
		(void) putc(bc, fd);
		oc = bc + 1;
	}
	for (i = 0, pP = rowpixels; i < cols;)
	{
		if (cols - i > MAX_COUNT)
		{
			(void) putc(MAX_COUNT - 1, fd);
			for (j = 0; j < MAX_COUNT; j++)
			{
				(void) putc(ppm_lookupcolor(cht, pP), fd);
				pP++;
			}
			i += MAX_COUNT;
		}
		else
		{
			(void) putc(cols - i - 1, fd);
			for (j = 0; j < cols - i; j++)
			{
				(void) putc(ppm_lookupcolor(cht, pP), fd);
				pP++;
			}
			i = cols;
		}
	}
	return (oc);
}
#endif	/* RUNLENGTH */
