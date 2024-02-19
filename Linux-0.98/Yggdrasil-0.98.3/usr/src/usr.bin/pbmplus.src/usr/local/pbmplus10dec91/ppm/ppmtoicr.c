/* ppmtoicr.c - convert a portable pixmap to NCSA ICR protocol
**
** Copyright (C) 1990 by Kanthan Pillay (svpillay@Princeton.EDU)
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

#define MAXCOLORS 256
#define CLUTCOLORS 768

static int colorstobpp ARGS(( int colors ));
static int GetPixel ARGS(( int x, int y ));
static int rleit ARGS(( char* buf, char* bufto, int len ));

static pixel** pixels;
static colorhash_table cht;
static char* testimage;

void
main(argc, argv)
int argc;
char* argv[];
{
	FILE* ifp;
	int argn, rows, cols, colors, i, j, BitsPerPixel, newxsize;
	pixval maxval;
	colorhist_vector chv;
	char rgb[CLUTCOLORS];
	char* windowname;
	char* thischar;
	char* thisline;
	char* space;
	register unsigned char c;
	register char* p;
	int display, expand;
	int rleflag, winflag;
	char* usage = "[-windowname windowname] [-expand expand] [-display display] [-rle] [ppmfile]";

	ppm_init( &argc, argv );

	argn = 1;
	windowname = "untitled";
	winflag = 0;
	expand = 1;
	display = 0;
	rleflag = 0;

	while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	    {
	    if ( pm_keymatch(argv[argn],"-windowname",2) && argn + 1 < argc )
		{
		++argn;
		windowname = argv[argn];
		winflag = 1;
		}
	    else if ( pm_keymatch(argv[argn],"-expand",2) && argn + 1 < argc )
		{
		++argn;
		if ( sscanf( argv[argn], "%d",&expand ) != 1 )
		    pm_usage( usage );
		}
	    else if ( pm_keymatch(argv[argn],"-display",2) && argn + 1 < argc )
		{
		++argn;
		if ( sscanf( argv[argn], "%d",&display ) != 1 )
		    pm_usage( usage );
		}
	    else if ( pm_keymatch(argv[argn],"-rle",2) )
		rleflag = 1;
	    else if ( pm_keymatch(argv[argn],"-norle",2) )
		rleflag = 0;
	    else
		pm_usage( usage );
	    }

	if ( argn < argc )
	    {
	    ifp = pm_openr( argv[argn] );
	    if ( ! winflag )
		windowname = argv[argn];
	    ++argn;
	    }
	else
	    ifp = stdin;

	if ( argn != argc )
	    pm_usage( usage );

	pixels = ppm_readppm( ifp, &cols, &rows, &maxval );

	pm_close( ifp );

	for (i = 0; i < CLUTCOLORS; i++)
	    rgb[i] = 0;

	/* Figure out the colormap. */
	pm_message("computing colormap..." );
	chv = ppm_computecolorhist(pixels, cols, rows, MAXCOLORS, &colors);
	if (chv == (colorhist_vector) 0)
	pm_error( "too many colors - try doing a 'ppmquant %d'", MAXCOLORS );
	pm_message("%d colors found", colors );

	/* Turn the ppm colormap into an ICR colormap. */
	if (maxval > 255)
	pm_message(
		"maxval is not 255 - automatically rescaling colors" );
	for (i = 0; i < colors; i++)
	{
	j = (3 * i);
	if (maxval == 255)
		{
		rgb[j] = PPM_GETR(chv[i].color) ;
		j++;
		rgb[j] = PPM_GETG(chv[i].color) ;
		j++;
		rgb[j] = PPM_GETB(chv[i].color) ;
		}
	else
		{
		rgb[j] = (int) PPM_GETR(chv[i].color) * 255 / maxval;
		j++;
		rgb[j] = (int) PPM_GETG(chv[i].color) * 255 / maxval;
		j++;
		rgb[j] = (int) PPM_GETB(chv[i].color) * 255 / maxval;
		}
	}
	BitsPerPixel = colorstobpp(colors);

	/* And make a hash table for fast lookup. */
	cht = ppm_colorhisttocolorhash(chv, colors);
	ppm_freecolorhist(chv);


	/************** Create a new window using ICR protocol *********/
	/* Format is "ESC^W;left;top;width;height;display;windowname"  */

	pm_message("creating window %s ...", windowname );
	(void)printf("\033^W;%d;%d;%d;%d;%d;%s^",0,0,cols*expand,rows*expand,display,windowname);
	fflush(stdout);


	/****************** Download the colormap.  ********************/
	pm_message("downloading colormap for %s ...", windowname );

	(void)printf("\033^M;%d;%d;%d;%s^",0,MAXCOLORS,CLUTCOLORS,windowname);
	thischar = rgb;
	for (j=0; j<CLUTCOLORS; j++) {
	c = *thischar++;
		if (c > 31 && c < 123 ) {	 /* printable ASCII */
		putchar(c);
		}
		else {
		putchar((c>>6)+123);	 /* non-printable, so encode it */
		putchar((c & 0x3f) + 32);
		}
	}
	fflush(stdout);

	/**************** send out picture *************************/
	/* Protocol's RLE scheme is quicker but buggy              */

	if (rleflag) {	
		pm_message("sending run-length encoded picture data ..." );
		testimage = (char*) malloc(rows*cols);
		p = testimage;
		for (i=0; i<rows; i++)
			for (j=0; j<cols; j++) 
			*p++ = GetPixel(j,i);
		space = (char*) malloc(rows*3);
		thisline = testimage;
		for (i = 0; i < rows; i++) {
			newxsize = rleit(thisline,space,cols);
			thisline += cols;	/* increment to next line */
		(void)printf("\033^R;%d;%d;%d;%d;%s^",0,i*expand,expand,newxsize,windowname);
		thischar = space;
		for (j=0; j< newxsize; j++) {
			c= *thischar++;  /*get byte to send */
			if (c>31 && c <123) {
				putchar(c);
				}
			else {
				putchar((c>>6) + 123);
				putchar((c & 0x3f) + 32);
				}
			}
			fflush(stdout);
		}
		free(space);
		exit(0);
		}

	/* Otherwise, send out uncompressed pixel data via the slow method */

		else {
		pm_message("sending picture data ..." );
		for (i = 0; i < rows; i++) {
			(void)printf("\033^P;%d;%d;%d;%d;%s^",0,i*expand,expand,cols,windowname);
			for (j = 0; j < cols; j++) {
				c  = GetPixel(j,i);
				if (c > 31 && c < 123) {
						putchar(c);
						}
				else		{
						putchar((c>>6)+123);
						putchar((c & 0x3f) + 32);
						}
				}
			}
		fflush(stdout);
		exit(0);
		}
	}

static int
colorstobpp(colors)
int colors;
	{
	int bpp;

	if (colors <= 2)
	bpp = 1;
	else if (colors <= 4)
	bpp = 2;
	else if (colors <= 8)
	bpp = 3;
	else if (colors <= 16)
	bpp = 4;
	else if (colors <= 32)
	bpp = 5;
	else if (colors <= 64)
	bpp = 6;
	else if (colors <= 128)
	bpp = 7;
	else if (colors <= 256)
	bpp = 8;
	else
	pm_error("can't happen" );
	return bpp;
	}

static int
GetPixel(x, y)
int x, y;
	{
	int color;

	color = ppm_lookupcolor(cht, &pixels[y][x]);
	return color;
	}


/* rleit   compress with run length encoding as per NCSA's documentation */

static int
rleit(buf,bufto,len)
	char* buf;
	char* bufto;
	int len;
	{
	register char* p;
	register char* q;
	register char* cfoll;
	register char* clead;
	char* begp;
	int i;

	p = buf;
	cfoll = bufto;
	clead = cfoll + 1;

	begp = p;
	while (len > 0 ) {		/* encode until gone */
		
		q = p + 1;
		i = len-1;
	while (*p == *q && i+120 > len && i) {
		q++;
		i--;
	}

	if (q > p +2) {			/* three in a row */
		if (p > begp) {
			*cfoll = p - begp;
			cfoll = clead;
		}
		*cfoll++ = 128 | (q-p);		/*len of seq*/
		*cfoll++ = *p;			/* char of seq */
		len -= q-p;		/* subtract len of seq */
		p = q;
		clead = cfoll+1;
		begp = p;
	}
	else {
		*clead++ = *p++;	/* copy one char */
		len--;
		if (p>begp + 120) {
			*cfoll = p - begp;
			cfoll = clead++;
			begp = p;
		}
	}
	}

/* fillin last bytecount */

	if (p>begp)
		*cfoll = (p - begp);
	else
		clead--;

	return((int) (clead-bufto));	/*how many stored as encoded */
}
