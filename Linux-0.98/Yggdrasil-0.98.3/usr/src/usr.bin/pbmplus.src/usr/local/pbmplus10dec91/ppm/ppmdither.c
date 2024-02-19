/* ppmdither.c - Ordered dithering of a color ppm file to a specified number
**               of primary shades.
**
** Copyright (C) 1991 by Christos Zoulas.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

#define NC 256			/* Total number of colors		*/
#define NS 256			/* Max number of shades in primary	*/

typedef unsigned char ubyte;

static int dith_nr  =   5;	/* number of red shades			*/
static int dith_ng  =   9;	/* number of green shades		*/
static int dith_nb  =   5;	/* number of blue shades		*/
static int dith_nc  = 225;	/* total number of colors 5 x 9 x 5	*/
static int dith_dim =   4;	/* dimension of the dither matrix	*/
static int dith_dm2 =  16;	/* dim square				*/
static int **dith_mat; 		/* the dithering matrix			*/

/* COLOR():
 *	returns the index in the color table for the
 *      r, g, b values specified.
 */
#define COLOR(r,g,b) (((r) * dith_ng + (g)) * dith_nb + (b))

/* LEVELS():
 *	Returns the total number of levels after dithering.
 */
#define LEVELS(s)     (dith_dm2 * ((s) - 1) + 1)

/* DITHER():
 *	Returns the dithered color for a single primary.
 *      p = the input pixel
 *      d = entry in the dither matrix
 *      s = the number of levels of the primary
 *
 */
#define DITHER(p,d,s) ((ubyte) ((LEVELS(s) * (p) + (d)) / (dith_dm2 * NS)))


/* dith_value():
 *	Return the value of a dither matrix of size x size at x, y 
 *	[graphics gems, p. 714]
 */
static int
dith_value(y, x, size)
int y, x, size;
{
    register int d;

    /*
     * Think of d as the density. At every iteration, d is shifted
     * left one and a new bit is put in the low bit based on x and y.
     * If x is odd and y is even, or visa versa, then a bit is shifted in.
     * This generates the checkerboard pattern seen in dithering.
     * This quantity is shifted again and the low bit of y is added in.
     * This whole thing interleaves a checkerboard pattern and y's bits
     * which is what you want.
     */
    for (d = 0; size-- > 0; x >>= 1, y >>= 1)
	d = (d << 2) | (((x & 1) ^ (y & 1)) << 1) | (y & 1);
    return(d);
} /* end dith_value */


/* dith_matrix():
 *	Form the dithering matrix for the dimension specified
 *	(Scaled by NS)
 */
static void
dith_matrix(dim)
int dim;
{
    int x, y, *dat;

    dith_dim = (1 << dim);
    dith_dm2 = dith_dim * dith_dim;

    dith_mat = (int **) malloc((dith_dim * sizeof(int *)) + /* pointers */
			       (dith_dm2 * sizeof(int)));   /* data */

    if (dith_mat == NULL) 
	pm_error("out of memory");

    dat =  (int *) &dith_mat[dith_dim];
    for (y = 0; y < dith_dim; y++)
	dith_mat[y] = &dat[y * dith_dim];

    for (y = 0; y < dith_dim; y++) {
	for (x = 0; x < dith_dim; x++) {
	     dith_mat[y][x] = NS * dith_value(y, x, dim);
#ifdef DEBUG
	     (void) fprintf(stderr, "%4d ", dith_mat[y][x]);
#endif
	}
#ifdef DEBUG
	(void) fprintf(stderr, "\n");
#endif
    }
} /* end dith_matrix */

    
/* dith_setup():
 *	Setup the dithering parameters, lookup table and dithering matrix
 */
void
dith_setup(dim, nr, ng, nb, ptab)
int dim, nr, ng, nb;
pixel *ptab;
{
    register int r, g, b, i;

    dith_nr  = nr;
    dith_ng  = ng;
    dith_nb  = nb;
    dith_nc  = nr * ng * nb;

    if (dith_nc > NC)
	pm_error("too many shades %d, max %d", dith_nc, NC);
    if (dith_nr < 2) 
	pm_error("too few shades for red, minimum of 2");
    if (dith_ng < 2) 
	pm_error("too few shades for green, minimum of 2");
    if (dith_nb < 2) 
	pm_error("too few shades for blue, minimum of 2");
    
    for (r = 0; r < dith_nr; r++) 
	for (g = 0; g < dith_ng; g++) 
	    for (b = 0; b < dith_nb; b++) {
		i = COLOR(r,g,b);
		PPM_ASSIGN(ptab[COLOR(r,g,b)], 
		           (r * (NC-1) / (dith_nr - 1)),
		           (g * (NC-1) / (dith_ng - 1)),
		           (b * (NC-1) / (dith_nb - 1)));
	    }
    
    dith_matrix(dim);
} /* end dith_setup */


/* dith_color():
 *  Return the closest color index for the one we ask
 */
int
dith_color(r, g, b)
float r, g, b;
{
    int rr, gg, bb;

    rr = r * (dith_nr - 1);
    gg = g * (dith_ng - 1);
    bb = b * (dith_nb - 1);
    return((int) COLOR(rr, gg, bb));
} /* end dith_color */


/* dith_dither():
 *  Dither height scanlines at a time
 */
void
dith_dither(w, h, t, i, o)
int w, h;
register pixel *t;
register pixel *i;
register pixel *o;
{
    int y, dm = (dith_dim - 1);
    register int x, d;
    register int *m;

    for (y = 0; y < h; y++)
	for (m = dith_mat[y & dm], x = w; --x >= 0;i++) {
	    d = m[x & dm];
	    *o++ = t[COLOR(DITHER(PPM_GETR(*i), d, dith_nr), 
		           DITHER(PPM_GETG(*i), d, dith_ng), 
		           DITHER(PPM_GETB(*i), d, dith_nb))];
	}
} /* end dith_dither */


void
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel ptab[256];
    pixel **ipixels, **opixels;
    int cols, rows;
    pixval maxval;
    int argn;
    char* usage = 
	"[-dim <num>] [-red <num>] [-green <num>] [-blue <num>] [pbmfile]";

    ppm_init( &argc, argv );

    argn = 1;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-dim", 1) &&  argn + 1 < argc ) {
	    argn++;
	    if (sscanf(argv[argn], "%d", &dith_dim) != 1)
		pm_usage( usage );
	}
	else if ( pm_keymatch( argv[argn], "-red", 1 ) && argn + 1 < argc ) {
	    argn++;
	    if (sscanf(argv[argn], "%d", &dith_nr) != 1)
		pm_usage( usage );
	}
	else if ( pm_keymatch( argv[argn], "-green", 1 ) && argn + 1 < argc ) {
	    argn++;
	    if (sscanf(argv[argn], "%d", &dith_ng) != 1)
		pm_usage( usage );
	}
	else if ( pm_keymatch( argv[argn], "-blue", 1 ) && argn + 1 < argc ) {
	    argn++;
	    if (sscanf(argv[argn], "%d", &dith_nb) != 1)
		pm_usage( usage );
	}
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    ipixels = ppm_readppm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );
    opixels = ppm_allocarray(cols, rows);
    maxval = 255;
    dith_setup(dith_dim, dith_nr, dith_ng, dith_nb, ptab);
    dith_dither(cols, rows, ptab, &ipixels[0][0], &opixels[0][0]);
    ppm_writeppm(stdout, opixels, cols, rows, maxval, 0);
    pm_close(stdout);
    exit(0);
}
