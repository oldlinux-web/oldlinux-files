/* ppmtopj.c - convert a portable pixmap to an HP PainJetXL image
**
** Copyright (C) 1990 by Christos Zoulas (christos@ee.cornell.edu)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

/*
 * XXX: Only 8.5 x 11 paper
 */
#define WIDTH	  8.5
#define HEIGHT	  11.0
#define DPI	  180
#define XPIX	  ((int) ((DPI * WIDTH + 7) / 8) << 3)
#define YPIX	  ((int) ((DPI * HEIGHT + 7) / 8) << 3)

#define C_RESET 			"\033E"
#define C_RENDER 			"\033*t%dJ"
# define C_RENDER_NONE			0
# define C_RENDER_SNAP			1
# define C_RENDER_BW			2
# define C_RENDER_DITHER		3
# define C_RENDER_DIFFUSE		4
# define C_RENDER_MONODITHER		5
# define C_RENDER_MONODIFFUSE		6
# define C_RENDER_MONO_CL_DITHER	5
# define C_RENDER_MONO_CL_DIFFUSE	6
#define C_BACK_SCALE			"\033*t%dK"
# define C_BACK_SCALE_LIGHT		0
# define C_BACK_SCALE_DARK		1
#define C_GAMMA				"\033*t%dI"
#define C_IMAGE_WIDTH			"\033*r%dS"
#define C_IMAGE_HEIGHT			"\033*r%dT"
#define C_DATA_PLANES			"\033*r%dU"
#define C_TRANS_MODE			"\033*b%dM"
# define C_TRANS_MODE_STD		0
# define C_TRANS_MODE_RLE		1
# define C_TRANS_MODE_TIFF		2
#define C_SEND_PLANE			"\033*b%dV"
#define C_LAST_PLANE			"\033*b%dW"
#define C_BEGIN_RASTER			"\033*r%dA"
# define C_BEGIN_RASTER_MARGIN		0
# define C_BEGIN_RASTER_ACTIVE		1
# define C_BEGIN_RASTER_NOSCALE		0
# define C_BEGIN_RASTER_SCALE		2
#define C_END_RASTER			"\033*r%dC"
# define C_END_RASTER_UNUSED		0
#define C_RESOLUTION			"\033*t%dR"
# define C_RESOLUTION_90DPI		90
# define C_RESOLUTION_180DPI		180
#define C_MOVE_X			"\033*p+%dX"
#define C_MOVE_Y			"\033*p+%dY"

char *testimage;

static char *rmode[] = { "none", "snap", "bw", "dither", "diffuse", 
			 "monodither", "monodiffuse", "clusterdither", 
			 "monoclusterdither", NULL };

/*
 * Run-length encoding for the PaintJet. We have pairs of <instances>
 * <value>, where instances goes from 0 (meaning one instance) to 255
 * If we are unlucky we can double the size of the image.
 */
static int
compress_row(op, oe, cp)
unsigned char *op, *oe, *cp;
{
    unsigned char *ce = cp;
    while ( op < oe ) {	
	unsigned char px = *op++;
	unsigned char *pr = op;
	while ( op < oe && *op == px && op - pr < 255) op++;
	*ce++ = op - pr;
	*ce++ = px;
    }
    return ce - cp;
}

void main(argc, argv)
int argc;
char *argv[];
{
	pixel **pixels;
	FILE *ifp;
	int argn, rows, cols, colors, r, c, k, m, p;
	pixval maxval;
	int planes = 3;
	unsigned char *obuf, *op, *cbuf;
	int render_mode = C_RENDER_NONE;
	int back_scale = C_BACK_SCALE_DARK;
	int gamma = 0;
	int mode = C_TRANS_MODE_STD;
	int deciwidth = 0, deciheight = 0;
	int center = 0;
	int xoff = 0, yoff = 0;
	/*
	 * XXX: Someday we could make this command line options.
	 */
	int posscale = C_BEGIN_RASTER_MARGIN | C_BEGIN_RASTER_NOSCALE;
	int resolution = C_RESOLUTION_180DPI;

	char *usage = "[-center] [-xpos <pos>] [-ypos <pos>] [-gamma <val>] [-back <dark|lite>] [-rle] [-render <none|snap|bw|dither|diffuse|monodither|monodiffuse|clusterdither|monoclusterdither>] [ppmfile]";

	ppm_init( &argc, argv );

	argn = 1;
	while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	    {
	    if ( pm_keymatch(argv[argn],"-render",2) && argn + 1 < argc )
		{
		++argn;
		for (r = 0; rmode[r] != NULL; r++)
		     if (strcmp(rmode[r], argv[argn]) == 0)
			 break;
		if (rmode[r] != NULL)
		    render_mode = r;
		else
		    pm_usage(usage);
		}
	    else if ( pm_keymatch(argv[argn],"-back",2) && argn + 1 < argc )
		{
		++argn;
		if (strcmp(argv[argn], "dark") == 0)
		    back_scale = C_BACK_SCALE_DARK;
		else if (strcmp(argv[argn], "lite") == 0)
		    back_scale = C_BACK_SCALE_LIGHT;
		else
		    pm_usage(usage);
		}
	    else if ( pm_keymatch(argv[argn],"-gamma",2) && argn + 1 < argc )
		{
		++argn;
		if ( sscanf( argv[argn], "%d",&gamma ) != 1 )
		    pm_usage( usage );
		}
	    else if ( pm_keymatch(argv[argn],"-xpos",2) && argn + 1 < argc )
		{
		++argn;
		if ( sscanf( argv[argn], "%d",&xoff ) != 1 )
		    pm_usage( usage );
		}
	    else if ( pm_keymatch(argv[argn],"-ypos",2) && argn + 1 < argc )
		{
		++argn;
		if ( sscanf( argv[argn], "%d",&yoff ) != 1 )
		    pm_usage( usage );
		}
	    else if (pm_keymatch(argv[argn],"-rle",2))
		mode = C_TRANS_MODE_RLE;
	    else if (pm_keymatch(argv[argn],"-center",2))
		center = 1;
	    else
		pm_usage( usage );
	    ++argn;
	    }

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
	obuf = (unsigned char *) pm_allocrow(cols, sizeof(unsigned char));
	cbuf = (unsigned char *) pm_allocrow(cols * 2, sizeof(unsigned char));

        if (cols > XPIX || rows > YPIX)
	    pm_message("image too large for page");
        if (center) {
	    if (xoff || yoff)
		pm_error("cannot specify both center and position");
	    xoff = (XPIX - cols) / 2;
	    yoff = (YPIX - rows) / 2;
	}

	(void) printf(C_RESET);
	/*
	 * Set the resolution before begin raster otherwise it
	 * does not work.
	 */
	(void) printf(C_RESOLUTION, resolution);
	(void) printf(C_BEGIN_RASTER, posscale);
	if (xoff)
	    (void) printf(C_MOVE_X, xoff);
	if (yoff)
	    (void) printf(C_MOVE_Y, yoff);
	(void) printf(C_TRANS_MODE, mode);
	(void) printf(C_RENDER, render_mode);
	(void) printf(C_BACK_SCALE, back_scale);
	(void) printf(C_GAMMA,	 gamma);
	(void) printf(C_IMAGE_WIDTH, cols);
	(void) printf(C_IMAGE_HEIGHT, rows);
	(void) printf(C_DATA_PLANES, 3);

        for (r = 0; r < rows; r++)
	    /* for each primary */
	    for (p = 0; p < 3; p++) {
		switch (p) {
		case 0:
		    for (c = 0, op = &obuf[-1]; c < cols; c++) {
			if ((k = (c & 7)) == 0)
			    *++op = 0;
			if (PPM_GETR(pixels[r][c]) > maxval / 2)
			    *op |= 1 << (7 - k);
		    }
		    break;
		case 1:
		    for (c = 0, op = &obuf[-1]; c < cols; c++) {
			if ((k = (c & 7)) == 0)
			    *++op = 0;
			if (PPM_GETG(pixels[r][c]) > maxval / 2)
			    *op |= 1 << (7 - k);
		    }
		    break;
		case 2:
		    for (c = 0, op = &obuf[-1]; c < cols; c++) {
			if ((k = (c & 7)) == 0)
			    *++op = 0;
			if (PPM_GETB(pixels[r][c]) > maxval / 2)
			    *op |= 1 << (7 - k);
		    }
		    break;
		}
		++op;
		if (mode == C_TRANS_MODE_RLE) {
		    k = compress_row(obuf, op, cbuf);
		    op = cbuf;
		}
		else {
		    k = op - obuf;
		    op = obuf;
		}
		(void) printf(p == 2 ? C_LAST_PLANE : C_SEND_PLANE, k);
		(void) fwrite(op, 1, k, stdout);
	    }
	(void) printf(C_END_RASTER, C_END_RASTER_UNUSED);
	exit(0);
}
