/*

      Convert a portable pixmap to an AutoCAD slide or DXB file

    Author:
	    John Walker
	    Autodesk SA
	    Avenue des Champs-Montants 14b
	    CH-2074 MARIN
	    Switzerland
	    Usenet: kelvin@Autodesk.com
	    Fax:    038/33 88 15
	    Voice:  038/33 76 33

    Permission	to  use, copy, modify, and distribute this software and
    its documentation  for  any  purpose  and  without	fee  is  hereby
    granted,  without any conditions or restrictions.  This software is
    provided "as is" without express or implied warranty.

*/

#include <stdio.h>
#include "ppm.h"
#include "ppmcmap.h"

#define TRUE	 1
#define FALSE	 0

#define EOS     '\0'

#define MAXHIST 	32767	      /* Colour histogram maximum size */

static pixel **pixels;		      /* Input pixel map */
static colorhash_table cht;	      /* Colour hash table */
static int curcol = -1; 	      /* Current slide output colour */
static int polymode = FALSE;	      /* Output filled polygons ? */
static int dxbmode = FALSE;	      /* Output .dxb format ? */
static int bgcol = -1;		      /* Screen background colour */
static double aspect = 1.0;	      /* Pixel aspect ratio correction */
static int gamut = 256; 	      /* Output colour gamut */

#include "autocad.h"                  /* AutoCAD standard colour assignments */

/*  OUTRUN  --	Output a run of pixels. */

static void outrun(colour, ysize, y, xstart, xend)
  int colour, ysize, y, xstart, xend;
{
    if (colour == 0) {
	return; 		      /* Let screen background handle this */
    }

    if (curcol != colour) {
	if (dxbmode) {
	    putchar(136);
	    (void) pm_writelittleshort(stdout, colour);
	} else {
	    (void) pm_writelittleshort(stdout, 0xFF00 | colour);
	}
	curcol = colour;
    }
    if (polymode) {
	int v, yb = (ysize - y) + 1, yspan = 1;

        /* Since  we're emitting filled polygons,  let's scan downward
	   in the pixmap and see if we can extend the run on this line
	   vertically  as  well.   If  so, emit a polygon that handles
	   both the horizontal and vertical run and clear  the	pixels
	   in the subsequent lines to the background colour. */

	for (v = y + 1; v <= ysize; v++) {
	    int j, mismatch = FALSE;

	    for (j = xstart; j <= xend; j++) {
		if (PPM_GETR(pixels[y][j]) != PPM_GETR(pixels[v][j])) {
		    mismatch = TRUE;
		    break;
		}
	    }
	    if (mismatch) {
		break;
	    }
	    for (j = xstart; j <= xend; j++) {
		PPM_ASSIGN(pixels[v][j], 0, 0, 0);
	    }
	}
	yspan = v - y;

	if (dxbmode) {
	    putchar(11);	      /* Solid */
	    (void) pm_writelittleshort(
		stdout, (int) (xstart * aspect + 0.4999));
	    (void) pm_writelittleshort(stdout, yb);

	    (void) pm_writelittleshort(
		stdout, (int) ((xend + 1) * aspect + 0.4999));
	    (void) pm_writelittleshort(stdout, yb);

	    (void) pm_writelittleshort(
		stdout, (int) (xstart * aspect + 0.4999));
	    (void) pm_writelittleshort(stdout, yb - yspan);

	    (void) pm_writelittleshort(
		stdout, (int) ((xend + 1) * aspect + 0.4999));
	    (void) pm_writelittleshort(stdout, yb - yspan);
	} else {
	    (void) pm_writelittleshort(stdout, 0xFD00); /* Solid fill header */
	    (void) pm_writelittleshort(stdout, 4);      /* Vertices to follow */
	    (void) pm_writelittleshort(stdout, -2);     /* Fill type */

	    (void) pm_writelittleshort(stdout, 0xFD00); /* Solid fill vertex */
	    (void) pm_writelittleshort(stdout, xstart);
	    (void) pm_writelittleshort(stdout, yb);

	    (void) pm_writelittleshort(stdout, 0xFD00); /* Solid fill vertex */
	    (void) pm_writelittleshort(stdout, xend + 1);
	    (void) pm_writelittleshort(stdout, yb);

	    (void) pm_writelittleshort(stdout, 0xFD00);	/* Solid fill vertex */
	    (void) pm_writelittleshort(stdout, xend + 1);
	    (void) pm_writelittleshort(stdout, yb - yspan);

	    (void) pm_writelittleshort(stdout, 0xFD00); /* Solid fill vertex */
	    (void) pm_writelittleshort(stdout, xstart);
	    (void) pm_writelittleshort(stdout, yb - yspan);

	    (void) pm_writelittleshort(stdout, 0xFD00);	/* Solid fill trailer */
	    (void) pm_writelittleshort(stdout, 4); /* Vertices that precede */
	    (void) pm_writelittleshort(stdout, -2);     /* Fill type */
	}
    } else {
	(void) pm_writelittleshort(stdout, xstart);     /* Vector:  From X */
	(void) pm_writelittleshort(stdout, ysize - y);  /*          From Y */
	(void) pm_writelittleshort(stdout, xend);       /*          To   X */
	(void) pm_writelittleshort(stdout, ysize - y);  /*          To   Y */
    }
}

/*  SLIDEOUT  --  Write an AutoCAD slide.  */

static void slideout(xdots, ydots, ncolours, red, green, blue)
  int xdots, ydots, ncolours;
  unsigned char *red, *green, *blue;
{
    static char sldhead[18] = "AutoCAD Slide\r\n\32";
    static char dxbhead[20] = "AutoCAD DXB 1.0\r\n\32";
    unsigned char *acadmap;
    int i, xsize, ysize;

    /* If the user has specified a non-black screen background colour,
       swap the screen background colour into colour  index  zero  and
       move  black into the slot previously occupied by the background
       colour. */

    if (bgcol > 0) {
	acadcol[0][0] = acadcol[bgcol][0];
	acadcol[0][1] = acadcol[bgcol][1];
	acadcol[0][2] = acadcol[bgcol][2];
	acadcol[bgcol][0] = acadcol[bgcol][1] = acadcol[bgcol][2] = 0;
    }

    acadmap = (unsigned char *) pm_allocrow(ncolours, sizeof(unsigned char));
    xsize = polymode ? xdots : (xdots - 1);
    ysize = polymode ? ydots : (ydots - 1);
    if (dxbmode) {
	fwrite(dxbhead, 19, 1, stdout); /* DXB file header */
	putchar(135);		      /* Number mode */
	(void) pm_writelittleshort(stdout, 0);	      /* ...short integers */
    } else {
	fwrite(sldhead, 17, 1, stdout); /* Slide file header */
	putchar(86);		      /* Number format indicator */
	putchar(2);		      /* File level number */
	(void) pm_writelittleshort(stdout, xsize); /* Max X co-ordinate value */
	(void) pm_writelittleshort(stdout, ysize); /* Max Y co-ordinate value */
				      /* Aspect ratio indicator */
	(void) pm_writelittlelong(
	    stdout, (long) ((((double) xsize) / ysize) * aspect * 1E7));
	(void) pm_writelittleshort(stdout, 2);	      /* Polygon fill type */
	(void) pm_writelittleshort(stdout, 0x1234);   /* Byte order indicator */
    }

    /* Now  build  a  mapping  from  the  image's computed colour map
       indices to the closest representation  of  each	colour	within
       AutoCAD's colour repertoire. */

    for (i = 0; i < ncolours; i++) {
	int best, j;
	long dist = 3 * 256 * 256;

	for (j = 0; j < gamut; j++) {
	    long dr = red[i] - acadcol[j][0],
		 dg = green[i] - acadcol[j][1],
		 db = blue[i] - acadcol[j][2];
	    long tdist = dr * dr + dg * dg + db * db;

	    if (tdist < dist) {
		dist = tdist;
		best = j;
	    }
	}
	acadmap[i] = best;
    }

    /* Swoop  over the entire map and replace each  RGB pixel with its
       closest	AutoCAD  colour  representation.   We  do  this  in  a
       separate  pass  since it avoids repetitive mapping of pixels as
       we examine them for compression. */

    for (i = 0; i < ydots; i++) {
	int x;

	for (x = 0; x < xdots; x++) {
	    PPM_ASSIGN(pixels[i][x],
		acadmap[ppm_lookupcolor(cht, &pixels[i][x])], 0 ,0);
	}
    }

    /* Output a run-length encoded expression of the pixmap as AutoCAD
       vectors. */

    for (i = 0; i < ydots; i++) {
	int x, rx, rpix = -1, nrun = 0;

	for (x = 0; x < xdots; x++) {
	    int pix = PPM_GETR(pixels[i][x]);

	    if (pix != rpix) {
		if (nrun > 0) {
		    if (rpix > 0) {
			outrun(rpix, ydots - 1, i, rx, x - 1);
		    }
		}
		rpix = pix;
		rx = x;
		nrun = 1;
	    }
	}
	if ((nrun > 0) && (rpix > 0)) {
	    outrun(rpix, ydots - 1, i, rx, xdots - 1);
	}
    }
    if (dxbmode) {
	putchar(0);		      /* DXB end sentinel */
    } else {
	(void) pm_writelittleshort(stdout, 0xFC00); /* End of file marker */
    }
    pm_freerow((char *) acadmap);
}

/*  Main program.  */

int main(argc, argv)
  int argc;
  char* argv[];
{
    FILE *ifp;
    int argn, rows, cols, ncolours, i;
    int aspectspec = FALSE;
    pixval maxval;
    colorhist_vector chv;
    unsigned char *Red, *Green, *Blue;
    char *usage =
        "[-poly] [-dxb] [-white] [-background <col>]\n\
                  [-aspect <f>] [-8] [ppmfile]";

    ppm_init(&argc, argv);

    argn = 1;
    while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != EOS) {
        if (pm_keymatch(argv[argn], "-dxb", 2)) {
	    dxbmode = polymode = TRUE;
        } else if (pm_keymatch(argv[argn], "-poly", 2)) {
	    polymode = TRUE;
        } else if (pm_keymatch(argv[argn], "-white", 2)) {
	    if (bgcol >= 0) {
                pm_error("already specified a background colour");
	    }
	    bgcol = 7;
        } else if (pm_keymatch(argv[argn], "-background", 2)) {
	    if (bgcol >= 0) {
                pm_error("already specified a background colour");
	    }
	    argn++;
            if ((argn == argc) || (sscanf(argv[argn], "%d", &bgcol) != 1))
		pm_usage(usage);
	    if (bgcol < 0) {
                pm_error("background colour must be >= 0 and <= 255");
	    }
        } else if (pm_keymatch(argv[argn], "-aspect", 2)) {
	    if (aspectspec) {
                pm_error("already specified an aspect ratio");
	    }
	    argn++;
            if ((argn == argc) || (sscanf(argv[argn], "%lf", &aspect) != 1))
		pm_usage(usage);
	    if (aspect <= 0.0) {
                pm_error("aspect ratio must be greater than 0");
	    }
	    aspectspec = TRUE;
        } else if (pm_keymatch(argv[argn], "-8", 2)) {
	    gamut = 8;
	} else {
	    pm_usage(usage);
	}
	argn++;
    }

    if (argn < argc) {
	ifp = pm_openr(argv[argn]);
	argn++;
    } else {
	ifp = stdin;
    }

    if (argn != argc) {
	pm_usage(usage);
    }

    pixels = ppm_readppm(ifp, &cols, &rows, &maxval);

    pm_close(ifp);

    /* Figure out the colormap.  Logic for squeezing depth to limit the
       number of colours in the image was swiped from ppmquant.c. */

    while (TRUE) {
	int row, col;
	pixval newmaxval;
	pixel *pP;

        pm_message("computing colourmap...");
	chv = ppm_computecolorhist(pixels, cols, rows, MAXHIST, &ncolours);
	if (chv != (colorhist_vector) 0)
	    break;
	newmaxval = maxval / 2;
	pm_message(
        "scaling colours from maxval=%d to maxval=%d to improve clustering...",
		   maxval, newmaxval );
	for (row = 0; row < rows; ++row) {
	    for (col = 0, pP = pixels[row]; col < cols; ++col, ++pP) {
		PPM_DEPTH(*pP, *pP, maxval, newmaxval);
	    }
	}
	maxval = newmaxval;
    }
    pm_message("%d colours found", ncolours);

    /* Scale the colour map derived for the PPM file into one compatible
       with AutoCAD's convention of 8 bit intensities. */

    if (maxval != 255) {
        pm_message("maxval is not 255 - automatically rescaling colours");
    }
    Red = (unsigned char *) pm_allocrow(ncolours, sizeof(unsigned char));
    Green = (unsigned char *) pm_allocrow(ncolours, sizeof(unsigned char));
    Blue = (unsigned char *) pm_allocrow(ncolours, sizeof(unsigned char));

    for (i = 0; i < ncolours; ++i) {
	if ( maxval == 255 ) {
	    Red[i] = PPM_GETR(chv[i].color);
	    Green[i] = PPM_GETG(chv[i].color);
	    Blue[i] = PPM_GETB( chv[i].color );
	} else {
	    Red[i] = ((int) PPM_GETR(chv[i].color) * 255) / maxval;
	    Green[i] = ((int) PPM_GETG(chv[i].color) * 255) / maxval;
	    Blue[i] = ((int) PPM_GETB(chv[i].color) * 255) / maxval;
	}
    }

    /* And make a hash table for fast lookup. */

    cht = ppm_colorhisttocolorhash(chv, ncolours);
    ppm_freecolorhist(chv);

    slideout(cols, rows, ncolours, Red, Green, Blue);
    pm_freerow((char *) Red);
    pm_freerow((char *) Green);
    pm_freerow((char *) Blue);
    exit(0);
}
