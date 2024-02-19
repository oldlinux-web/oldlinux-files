/*

	  Convert an AutoCAD slide (.sld) file to PPM format

    An AutoCAD slide is a compressed sequence of  vectors  and	filled
    polygons.	The  ppmdraw  package  is  used  to scan convert these
    geometrical objects into a portable pixmap.

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

#include "ppm.h"
#include "ppmdraw.h"
#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

/*  Define a variable type accepting numbers -127 <= n <= 127.	But note
    that we still expect it to act UNSIGNED. */

#define smallint unsigned char	      /* Small integers */

#define TRUE	 1
#define FALSE	 0

#define EOS     '\0'

/* Screen point */

struct spoint {
	int x, y;
};

/* Screen polygon */

struct spolygon { 
	int npoints,		      /* Number of points in polygon */
	      fill;		      /* Fill type */
	struct spoint pt[11];	      /* Actual points */
};

/* Screen vector */

struct svector {
	struct spoint f;	      /* From point */
	struct spoint t;	      /* To point */
};


static int ixdots, iydots;	      /* Screen size in dots */
static FILE *slfile;		      /* Slide file descriptor */
static int blither = FALSE;	      /* Dump slide file information ? */
static int info = FALSE;	      /* Print header information */
static pixel **pixels;		      /* Pixel map */
static int pixcols, pixrows;	      /* Pixel map size */
#define pixmaxval 255		      /* Largest pixel value */
static double uscale = -1;	      /* Uniform scale factor */
static int sxsize = -1, sysize = -1;  /* Scale to X, Y size ? */

#include "autocad.h"                  /* AutoCAD standard colour assignments */

/*  Local variables  */

struct slhead {
    char slh[17];		      /* Primate-readable header */
    char sntype;		      /* Machine type (for number compat) */
    char slevel;		      /* Format type */
    short sxdots, sydots;	      /* Display X, Y dots */
    double sdsar;		      /* Display aspect ratio */
    short shwfill;		      /* Display hardware fill type */
    char spad;			      /* Pad to even byte length */
};

static int adjust = FALSE;	      /* Adjust to correct aspect ratio ? */
static struct slhead slfrof;	      /* Slide file header */
static long xfac, yfac; 	      /* Aspect ratio scale factors */

static int sdrawkcab;		      /* Slide drawing kinematic conversion of
					 ass-backwards data flag */

/*  EXTEND  --	Turn a smallint into an int with sign extension, whether
		or not that happens automatically.  */

static int extend(ch)
  smallint ch;
{
    return ((int) ((ch & 0x80) ? (ch | ~0xFF) : ch));
}

/*  SLI  --  Input word from slide file  */

static int sli()
{
    short wd;

    if (fread(&wd, sizeof wd, 1, slfile) != 1) {
        pm_error("error reading slide file");
    } else {
	if (sdrawkcab) {
	    wd = ((wd >> 8) & 0xFF) | (wd << 8);
	}
    }
    return wd;
}

/*  SLIB  --  Input byte from slide file  */

static int slib()
{
    smallint ch = 0;

    if (fread(&ch, sizeof ch, 1, slfile) != 1) {
        pm_error("error reading slide file");
    }
    return extend(ch);
}

/*  VSCALE -- scale screen coordinates for mismatched display.	*/

static void vscale(px, py)
  int *px, *py;
{
    *px = (((unsigned) *px) * xfac) >> 16;
    *py = (((unsigned) *py) * yfac) >> 16;
}

/*  SLIDER  --	Read slide file.  This is called with the name of the
		file to be read and function pointers to the routines
		which process vectors and polygon fill requests
		respectively.
*/

static void slider(slvec, slflood)
  void (*slvec)();
  void (*slflood)();
{
    int i, rescale;
    unsigned char ubfr[4];	      /* Utility character buffer */
    int lx, ly; 		      /* Last x and y point */
    int slx, sly;		      /* Last x and y scaled screen point */
    struct svector vec; 	      /* Screen vector */
    struct spolygon poly;	      /* Screen polygon */
    unsigned short cw;		      /* Control word */
    double dsar;		      /* Screen aspect ratio */
    long ldsar; 		      /* Scaled long DSAR */
    short rtest;		      /* Value to test byte reversal */
    short btest = 0x1234;	      /* Value to test byte-reversal */
    static struct slhead slhi =       /* Master slide header sample */
        {"AutoCAD Slide\r\n\32", 86,2, 0,0, 0.0, 0};
    int curcolour = 7;		      /* Current vector colour */
    pixel rgbcolour;		      /* Pixel used to clear pixmap */

    lx = ly = 32000;

    /* Process the header of the slide file.  */

    sdrawkcab = FALSE;		      /* Initially guess byte order is OK */
    fread(slfrof.slh, 17, 1, slfile);
    fread(&slfrof.sntype, sizeof(char), 1, slfile);
    fread(&slfrof.slevel, sizeof(char), 1, slfile);
    fread(&slfrof.sxdots, sizeof(short), 1, slfile);
    fread(&slfrof.sydots, sizeof(short), 1, slfile);
    fread(ubfr, 4, 1, slfile);
    fread(&slfrof.shwfill, sizeof(short), 1, slfile);
    fread(&rtest, sizeof rtest, 1, slfile);

    /* Verify that slide format is compatible with this program. */

    if (strcmp(slfrof.slh, slhi.slh) != 0) {
        pm_error("this is not an AutoCAD slide file.");
    }

    /* Verify that the number format and file level in the header  are
       compatible.  All slides written by versions of AutoCAD released
       since September of 1987 are compatible with this format.  */

    if ((slfrof.sntype != slhi.sntype) || (slfrof.slevel != slhi.slevel)) {
        pm_error("incompatible slide file format");
    }

    /* Build SDSAR value from long scaled version. */

    ldsar = 0L;
    for (i = 3; i >= 0; i--) {
	ldsar = (ldsar << 8) | ubfr[i];
    }
    slfrof.sdsar = ((double) ldsar) / 1E7;

    /* Examine the byte order test value.   If it's backwards, set the
       byte-reversal flag and correct all of the values we've read  in
       so far. */

    if (btest != rtest) {
	sdrawkcab = TRUE;
#define rshort(x) x = ((x >> 8) & 0xFF) | (x << 8)
	rshort(slfrof.sxdots);
	rshort(slfrof.sydots);
	rshort(slfrof.shwfill);
#undef rshort
    }

    /* Dump the header if we're blithering. */

    if (blither || info) {
        pm_message("Slide file type %d, level %d, hwfill type %d.",
	    slfrof.sntype, slfrof.slevel, slfrof.shwfill);
        pm_message("Original screen size %dx%d, aspect ratio %.3f.",
	    slfrof.sxdots + 1, slfrof.sydots + 1, slfrof.sdsar);
        pm_message("Byte order is %s.",
            sdrawkcab ? "being reversed" : "the same");
    }

    /* If the display aspect ratio indicates that the  pixels  on  the
       sending	screen	were  not  square,  adjust  the  size  of  the
       generated bitmap to correct the	aspect	ratio  to  square  the
       pixels.

       We  always  correct  the aspect ratio by adjusting the width of
       the image.  This guarantees that output from the SHADE command,
       which  is  essentially  scan-line  data written in vector form,
       will not be corrupted. */

    dsar = ((double) slfrof.sxdots) / slfrof.sydots;
    if (abs(slfrof.sdsar - dsar) > 0.0001) {
	if (adjust) {
	    ixdots = slfrof.sxdots * (slfrof.sdsar / dsar) + 0.5;
	    iydots = slfrof.sydots;
	    dsar = ((double) ixdots) / iydots;
	} else {
            pm_message("Warning - pixels on source screen were non-square.");
            pm_message("          Specifying -adjust will correct image width to compensate.");
	    ixdots = slfrof.sxdots;
	    iydots = slfrof.sydots;
	    dsar = slfrof.sdsar;
	}
    } else {
	/* Source pixels were square. */
	ixdots = slfrof.sxdots;
	iydots = slfrof.sydots;
	dsar = slfrof.sdsar;
	adjust = FALSE; 	      /* Mark no adjustment needed */
    }

    /* If there's a uniform scale factor specified, apply it. */

    if (uscale > 0) {
	ixdots = (ixdots * uscale) + 0.5;
	iydots = (iydots * uscale) + 0.5;
    }

    /* If the image is to be stretched	to  a  given  width,  set  the
       output  image  sizes accordingly.  If only a height or width is
       given, scale the other direction proportionally to preserve the
       aspect ratio. */

    if (sxsize > 0) {
	if (sysize > 0) {
	    iydots = sysize - 1;
	} else {
	    iydots = ((((long) iydots) * (sxsize - 1)) +
		      (iydots / 2)) / ixdots;
	}
	ixdots = sxsize - 1;
    } else if (sysize > 0) {
	if (sxsize > 0) {
	    ixdots = sxsize - 1;
	} else {
	    ixdots = ((((long) ixdots) * (sysize - 1)) +
		      (ixdots / 2)) / iydots;
	}
	iydots = sysize - 1;
    }

    if (adjust) {
	pm_message(
            "Resized from %dx%d to %dx%d to correct pixel aspect ratio.",
	    slfrof.sxdots + 1, slfrof.sydots + 1, ixdots + 1, iydots + 1);
    }

    /* Allocate image buffer and clear it to black. */

    pixels = ppm_allocarray(pixcols = ixdots + 1, pixrows = iydots + 1);
    PPM_ASSIGN(rgbcolour, 0, 0, 0);
    ppmd_filledrectangle(pixels, pixcols, pixrows, pixmaxval, 0, 0,
	 pixcols, pixrows, PPMD_NULLDRAWPROC,
	 (char *) &rgbcolour);

    if ((rescale = slfrof.sxdots != ixdots ||
	slfrof.sydots != iydots ||
	slfrof.sdsar != dsar) != 0) {

        /* Rescale all coords. so they'll look (more or less)
	   right on this display.  */

	xfac = (ixdots + 1) * 0x10000L;
	xfac /= (long) (slfrof.sxdots + 1);
	yfac = (iydots + 1) * 0x10000L;
	yfac /= (long) (slfrof.sydots + 1);
	if (dsar < slfrof.sdsar) {
	    yfac = yfac * dsar / slfrof.sdsar;
       } else {
	    xfac = xfac * slfrof.sdsar / dsar;
	}
    }

    poly.npoints = 0;		      /* No flood in progress. */

    while ((cw = sli()) != 0xFC00) {
	switch (cw & 0xFF00) {
	    case 0xFB00:	      /*  Short vector compressed  */
		vec.f.x = lx + extend(cw & 0xFF);
		vec.f.y = ly + slib();
		vec.t.x = lx + slib();
		vec.t.y = ly + slib();
		lx = vec.f.x;
		ly = vec.f.y;
		if (rescale) {
		    vscale(&vec.f.x, &vec.f.y);
		    vscale(&vec.t.x, &vec.t.y);
		}
		(*slvec)(&vec, curcolour);/* Draw vector on screen */
		slx = vec.f.x;	      /* Save scaled point */
		sly = vec.f.y;
		break;

	    case 0xFC00:	      /*  End of file  */
		break;

	    case 0xFD00:	      /*  Flood command  */
		vec.f.x = sli();
		vec.f.y = sli();
		if ((int) vec.f.y < 0) { /* start or end */
		    if (poly.npoints != 0) { /* end?  */
			if (poly.npoints > 2 && poly.npoints < 11) {
			    (*slflood)(&poly, curcolour);
			} else {
                            pm_error("Bad polygon vertex count (%d)",
			       poly.npoints);
			}
			poly.npoints = 0;
		    } else {
			poly.fill = -vec.f.y;  /* Start */
		    }
		} else {	      /* Polygon vertex */
		    if (poly.npoints < 10) {
			if (rescale) {
			    vscale(&vec.f.x, &vec.f.y);
			}
			poly.pt[poly.npoints].x = vec.f.x;
			poly.pt[poly.npoints].y = vec.f.y;
		    }
		    poly.npoints++;
		}
		break;

	    case 0xFE00:	      /*  Common endpoint compressed  */
		vec.f.x = lx + extend(cw & 0xFF);
		vec.f.y = ly + slib();
		lx = vec.f.x;
		ly = vec.f.y;
		vec.t.x = slx;
		vec.t.y = sly;
		if (rescale) {
		    vscale(&vec.f.x, &vec.f.y);
		}
		(*slvec)(&vec, curcolour);/* Draw vector */
		slx = vec.f.x;	      /* Save scaled point */
		sly = vec.f.y;
		break;

	    case 0xFF00:	      /*  Change colour  */
		curcolour = cw & 0xFF;
		break;

	    default:		      /*  Co-ordinates	*/
		lx = vec.f.x = cw;
		ly = vec.f.y = sli();
		vec.t.x = sli();
		vec.t.y = sli();
		if (rescale) {
		   vscale(&vec.f.x, &vec.f.y);
		   vscale(&vec.t.x, &vec.t.y);
		}
		(*slvec)(&vec, curcolour);
		slx = vec.f.x;	      /* Save scaled point */
		sly = vec.f.y;
		break;
	}
    }
}

/*  SLIDEFIND  --  Find  a  slide  in  a  library  or,	if  DIRONLY is
		   nonzero, print a directory listing of the  library.
		   If  UCASEN  is nonzero, the requested slide name is
		   converted to upper case. */

static void slidefind(sname, dironly, ucasen)
  char *sname;
  int dironly, ucasen;
{
    char uname[32];
    unsigned char libent[36];
    long pos;

    if (dironly) {
        pm_message("Slides in library:");
    } else {
	int i;
	char *ip = sname;

	for (i = 0; i < 31; i++) {
	    char ch = *ip++;
	    if (ch == EOS) {
		break;
	    }
	    if (ucasen && islower(ch)) {
		ch = toupper(ch);
	    }
	    uname[i] = ch;
	}
	uname[i] = EOS;
    }

    /* Read slide library header and verify. */

    if ((fread(libent, 32, 1, slfile) != 1) ||
        (strcmp(libent, "AutoCAD Slide Library 1.0\015\012\32") != 0)) {
        pm_error("not an AutoCAD slide library file.");
    }
    pos = 32;

    /* Search for a slide with the requested name. */

    while (TRUE) {
	if ((fread(libent, 36, 1, slfile) != 1) ||
	    (strlen(libent) == 0)) {
	    if (dironly) {
		return;
	    }
            pm_error("slide %s not in library.", sname);
	}
	pos += 36;
	if (dironly) {
            pm_message("  %s", libent);
	} else if (strcmp(libent, uname) == 0) {
	    long dpos = (((((libent[35] << 8) | libent[34]) << 8) |
			     libent[33]) << 8) | libent[32];
	    if ((slfile == stdin) || (fseek(slfile, dpos, 0) == -1)) {
		dpos -= pos;

		while (dpos-- > 0) {
		    (void) getc(slfile);
		}
	    }
	    break;
	}
    }
}

/*  DRAW  --  Draw a vector in the given AutoCAD colour.  */

static void draw(vec, colour)
  struct svector *vec;
  int colour;
{
    pixel rgbcolour;

    if (blither) {
        pm_message("Vector (%d, %d) - (%d, %d)  Colour %d",
		   vec->f.x, vec->f.y, vec->t.x, vec->t.y, colour);
    }
    assert(vec->f.x >= 0 && vec->f.x < pixcols);
    assert(vec->f.y >= 0 && vec->f.y < pixrows);
    assert(vec->t.x >= 0 && vec->t.x < pixcols);
    assert(vec->t.y >= 0 && vec->t.y < pixrows);
    PPM_ASSIGN(rgbcolour,
	       acadcol[colour][0], acadcol[colour][1], acadcol[colour][2]);
    ppmd_line(pixels, pixcols, pixrows, pixmaxval,
	      vec->f.x, iydots - vec->f.y, vec->t.x, iydots - vec->t.y,
	      PPMD_NULLDRAWPROC,
	      (char *) &rgbcolour);
}

/*  FLOOD  --  Draw a filled polygon.  */

static void flood(poly, colour)
  struct spolygon *poly;
  int colour;
{
    int i;
    char *handle = ppmd_fill_init();
    pixel rgbcolour;

    if (blither) {
        pm_message("Polygon: %d points, fill type %d, colour %d",
		   poly->npoints, poly->fill, colour);
	for (i = 0; i < poly->npoints; i++) {
           pm_message("   Point %d:  (%d, %d)", i + 1,
		      poly->pt[i].x, poly->pt[i].y);
	}
    }

    PPM_ASSIGN(rgbcolour,
	       acadcol[colour][0], acadcol[colour][1], acadcol[colour][2]);
    for (i = 0; i < poly->npoints; i++) {
	assert(poly->pt[i].x >= 0 && poly->pt[i].x < pixcols);
	assert(poly->pt[i].y >= 0 && poly->pt[i].y < pixrows);
	ppmd_line(pixels, pixcols, pixrows, pixmaxval,
		  poly->pt[i].x, iydots - poly->pt[i].y, 
		  poly->pt[(i + 1) % poly->npoints].x,
		  iydots - poly->pt[(i + 1) % poly->npoints].y,
		  ppmd_fill_drawproc, handle);
    }
    ppmd_fill(pixels, pixcols, pixrows, pixmaxval,
	      handle, PPMD_NULLDRAWPROC, (char *) &rgbcolour);
}

/*  Main program. */

int main(argc, argv)
  int argc;
  char *argv[];
{
    int argn;
    char *usage = "[-verbose] [-info] [-adjust] [-scale <s>]\n\
                 [-dir] [-lib|-Lib <name>]\n\
                 [-xsize|-width <x>] [-ysize|-height <y>] [sldfile]";
    int scalespec = FALSE, widspec = FALSE, hgtspec = FALSE, dironly = FALSE,
	ucasen;
    char *slobber = (char *) 0;	      /* Slide library item */

    ppm_init(&argc, argv);
    argn = 1;

    while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0') {
        if (pm_keymatch(argv[argn], "-verbose", 2)) {
	    blither = TRUE;
        } else if (pm_keymatch(argv[argn], "-adjust", 2)) {
	    adjust = TRUE;
        } else if (pm_keymatch(argv[argn], "-dir", 2)) {
	    dironly = TRUE;
        } else if (pm_keymatch(argv[argn], "-info", 2)) {
	    info = TRUE;
        } else if (pm_keymatch(argv[argn], "-lib", 2)) {
	    if (slobber != (char *) 0) {
                pm_error("already specified a library item");
	    }
            ucasen = argv[argn][1] != 'L';
	    argn++;
	    if (argn == argc) {
		pm_usage(usage);
	    }
	    slobber = argv[argn];
        } else if (pm_keymatch(argv[argn], "-scale", 2)) {
	    if (scalespec) {
                pm_error("already specified a scale factor");
	    }
	    argn++;
            if ((argn == argc) || (sscanf(argv[argn], "%lf", &uscale) != 1))
		pm_usage(usage);
	    if (uscale <= 0.0) {
                pm_error("scale factor must be greater than 0");
	    }
	    scalespec = TRUE;
        } else if (pm_keymatch(argv[argn], "-xsize", 2) ||
                   pm_keymatch(argv[argn], "-width", 2)) {
	    if (widspec) {
                pm_error("already specified a width/xsize");
	    }
	    argn++;
            if ((argn == argc) || (sscanf(argv[argn], "%d", &sxsize) != 1))
		pm_usage(usage);
	    widspec = TRUE;
        } else if (pm_keymatch(argv[argn], "-ysize", 2) ||
                   pm_keymatch(argv[argn], "-height", 2)) {
	    if (hgtspec) {
                pm_error("already specified a height/ysize");
	    }
	    argn++;
            if ((argn == argc) || (sscanf(argv[argn], "%d", &sysize) != 1))
		pm_usage(usage);
	    hgtspec = TRUE;
	} else {
	    pm_usage(usage);
	}
	argn++;
    }

    /* If a file name is specified, open it.  Otherwise read from
       standard input. */

    if (argn < argc) {
	slfile = pm_openr(argv[argn]);
	argn++;
    } else {
	slfile = stdin;
    }

    if (argn != argc) { 	      /* Extra bogus arguments ? */
	pm_usage(usage);
    }

    /* If we're extracting an item from a slide library, position the
       input stream to the start of the chosen slide. */

    if (dironly || (slobber != (char *) 0)) {
	slidefind(slobber, dironly, ucasen);
    }

    if (!dironly) {
	slider(draw, flood);
	ppm_writeppm(stdout, pixels, pixcols, pixrows, pixmaxval, FALSE);
    }
    pm_close(slfile);
    pm_close(stdout);
    exit(0);
}
