/*

	    Fractal forgery generator for the PPM toolkit

	Originally  designed  and  implemented	in December of 1989 by
	John Walker as a stand-alone program for the  Sun  and	MS-DOS
	under  Turbo C.  Adapted in September of 1991 for use with Jef
        Poskanzer's raster toolkit.

	References cited in the comments are:

	    Foley, J. D., and Van Dam, A., Fundamentals of Interactive
		Computer  Graphics,  Reading,  Massachusetts:  Addison
		Wesley, 1984.

	    Peitgen, H.-O., and Saupe, D. eds., The Science Of Fractal
		Images, New York: Springer Verlag, 1988.

	    Press, W. H., Flannery, B. P., Teukolsky, S. A., Vetterling,
		W. T., Numerical Recipes In C, New Rochelle: Cambridge
		University Press, 1988.

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

				PLUGWARE!

    If you like this kind of stuff, you may also enjoy "James  Gleick's
    Chaos--The  Software"  for  MS-DOS,  available for $59.95 from your
    local software store or directly from Autodesk, Inc., Attn: Science
    Series,  2320  Marinship Way, Sausalito, CA 94965, USA.  Telephone:
    (800) 688-2344 toll-free or, outside the  U.S. (415)  332-2344  Ext
    4886.   Fax: (415) 289-4718.  "Chaos--The Software" includes a more
    comprehensive   fractal    forgery	  generator    which	creates
    three-dimensional  landscapes  as  well as clouds and planets, plus
    five more modules which explore other aspects of Chaos.   The  user
    guide  of  more  than  200	pages includes an introduction by James
    Gleick and detailed explanations by Rudy Rucker of the  mathematics
    and algorithms used by each program.

*/

#include "ppm.h"
#include <math.h>

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif
#ifndef M_E
#define M_E	2.7182818284590452354
#endif

/* Definitions used to address real and imaginary parts in a two-dimensional
   array of complex numbers as stored by fourn(). */

#define Real(v, x, y)  v[1 + (((x) * meshsize) + (y)) * 2]
#define Imag(v, x, y)  v[2 + (((x) * meshsize) + (y)) * 2]

/* Co-ordinate indices within arrays. */

#define X     0
#define Y     1
#define Z     2

/* Definition for obtaining random numbers. */

#define nrand 4 		      /* Gauss() sample count */
#define Cast(low, high) ((low)+(((high)-(low)) * ((random() & 0x7FFF) / arand)))

/* Utility definition to get an  array's  element  count  (at  compile
   time).   For  example:  

       int  arr[] = {1,2,3,4,5};
       ... 
       printf("%d", ELEMENTS(arr));

   would print a five.  ELEMENTS("abc") can also be used to  tell  how
   many  bytes are in a string constant INCLUDING THE TRAILING NULL. */
   
#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

/*  Data types	*/

typedef int Boolean;
#define FALSE 0
#define TRUE 1

#define V     (void)

/*  Display parameters	*/

#define SCRSAT	 255		      /* Screen maximum intensity */

/*  Local variables  */

static double arand, gaussadd, gaussfac; /* Gaussian random parameters */
static double fracdim;		      /* Fractal dimension */
static double powscale; 	      /* Power law scaling exponent */
static int meshsize = 256;	      /* FFT mesh size */
static unsigned int rseed;	      /* Current random seed */
static Boolean seedspec = FALSE;      /* Did the user specify a seed ? */
static Boolean clouds = FALSE;	      /* Just generate clouds */
static Boolean stars = FALSE;	      /* Just generate stars */
static int screenxsize = 256;	      /* Screen X size */
static int screenysize = 256;	      /* Screen Y size */
static double inclangle, hourangle;   /* Star position relative to planet */
static Boolean inclspec = FALSE;      /* No inclination specified yet */
static Boolean hourspec = FALSE;      /* No hour specified yet */
static double icelevel; 	      /* Ice cap theshold */
static double glaciers; 	      /* Glacier level */
static int starfraction;	      /* Star fraction */
static int starcolour;		      /* Star colour saturation */

/*	FOURN  --  Multi-dimensional fast Fourier transform

	Called with arguments:

	   data       A  one-dimensional  array  of  floats  (NOTE!!!	NOT
		      DOUBLES!!), indexed from one (NOTE!!!   NOT  ZERO!!),
		      containing  pairs of numbers representing the complex
		      valued samples.  The Fourier transformed results	are
		      returned in the same array.

	   nn	      An  array specifying the edge size in each dimension.
		      THIS ARRAY IS INDEXED FROM  ONE,	AND  ALL  THE  EDGE
		      SIZES MUST BE POWERS OF TWO!!!

	   ndim       Number of dimensions of FFT to perform.  Set to 2 for
		      two dimensional FFT.

	   isign      If 1, a Fourier transform is done; if -1 the  inverse
		      transformation is performed.

        This  function  is essentially as given in Press et al., "Numerical
        Recipes In C", Section 12.11, pp.  467-470.
*/

static void fourn(data, nn, ndim, isign)
  float data[];
  int nn[], ndim, isign;
{
    register int i1, i2, i3;
    int i2rev, i3rev, ip1, ip2, ip3, ifp1, ifp2;
    int ibit, idim, k1, k2, n, nprev, nrem, ntot;
    float tempi, tempr;
    double theta, wi, wpi, wpr, wr, wtemp;

#define SWAP(a,b) tempr=(a); (a) = (b); (b) = tempr

    ntot = 1;
    for (idim = 1; idim <= ndim; idim++)
	ntot *= nn[idim];
    nprev = 1;
    for (idim = ndim; idim >= 1; idim--) {
	n = nn[idim];
	nrem = ntot / (n * nprev);
	ip1 = nprev << 1;
	ip2 = ip1 * n;
	ip3 = ip2 * nrem;
	i2rev = 1;
	for (i2 = 1; i2 <= ip2; i2 += ip1) {
	    if (i2 < i2rev) {
		for (i1 = i2; i1 <= i2 + ip1 - 2; i1 += 2) {
		    for (i3 = i1; i3 <= ip3; i3 += ip2) {
			i3rev = i2rev + i3 - i2;
			SWAP(data[i3], data[i3rev]);
			SWAP(data[i3 + 1], data[i3rev + 1]);
		    }
		}
	    }
	    ibit = ip2 >> 1;
	    while (ibit >= ip1 && i2rev > ibit) {
		i2rev -= ibit;
		ibit >>= 1;
	    }
	    i2rev += ibit;
	}
	ifp1 = ip1;
	while (ifp1 < ip2) {
	    ifp2 = ifp1 << 1;
	    theta = isign * (M_PI * 2) / (ifp2 / ip1);
	    wtemp = sin(0.5 * theta);
	    wpr = -2.0 * wtemp * wtemp;
	    wpi = sin(theta);
	    wr = 1.0;
	    wi = 0.0;
	    for (i3 = 1; i3 <= ifp1; i3 += ip1) {
		for (i1 = i3; i1 <= i3 + ip1 - 2; i1 += 2) {
		    for (i2 = i1; i2 <= ip3; i2 += ifp2) {
			k1 = i2;
			k2 = k1 + ifp1;
			tempr = wr * data[k2] - wi * data[k2 + 1];
			tempi = wr * data[k2 + 1] + wi * data[k2];
			data[k2] = data[k1] - tempr;
			data[k2 + 1] = data[k1 + 1] - tempi;
			data[k1] += tempr;
			data[k1 + 1] += tempi;
		    }
		}
		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	    }
	    ifp1 = ifp2;
	}
	nprev *= n;
    }
}
#undef SWAP

/*  INITGAUSS  --  Initialise random number generators.  As given in
		   Peitgen & Saupe, page 77. */

static void initgauss(seed)
  unsigned int seed;
{
    /* Range of random generator */
    arand = pow(2.0, 15.0) - 1.0;
    gaussadd = sqrt(3.0 * nrand);
    gaussfac = 2 * gaussadd / (nrand * arand);
    srandom(seed);
}

/*  GAUSS  --  Return a Gaussian random number.  As given in Peitgen
	       & Saupe, page 77. */

static double gauss()
{
    int i;
    double sum = 0.0;

    for (i = 1; i <= nrand; i++) {
	sum += (random() & 0x7FFF);
    }
    return gaussfac * sum - gaussadd;
}

/*  SPECTRALSYNTH  --  Spectrally  synthesised	fractal  motion in two
		       dimensions.  This algorithm is given under  the
		       name   SpectralSynthesisFM2D  on  page  108  of
		       Peitgen & Saupe. */

static void spectralsynth(x, n, h)
  float **x;
  unsigned int n;
  double h;
{
    unsigned bl;
    int i, j, i0, j0, nsize[3];
    double rad, phase, rcos, rsin;
    float *a;

    bl = ((((unsigned long) n) * n) + 1) * 2 * sizeof(float);
    a = (float *) calloc(bl, 1);
    if (a == (float *) 0) {
        pm_error("Cannot allocate %d x %d result array (%ld bytes).",
	   n, n, bl);
    }
    *x = a;

    for (i = 0; i <= n / 2; i++) {
	for (j = 0; j <= n / 2; j++) {
	    phase = 2 * M_PI * ((random() & 0x7FFF) / arand);
	    if (i != 0 || j != 0) {
		rad = pow((double) (i * i + j * j), -(h + 1) / 2) * gauss();
	    } else {
		rad = 0;
	    }
	    rcos = rad * cos(phase);
	    rsin = rad * sin(phase);
	    Real(a, i, j) = rcos;
	    Imag(a, i, j) = rsin;
	    i0 = (i == 0) ? 0 : n - i;
	    j0 = (j == 0) ? 0 : n - j;
	    Real(a, i0, j0) = rcos;
	    Imag(a, i0, j0) = - rsin;
	}
    }
    Imag(a, n / 2, 0) = 0;
    Imag(a, 0, n / 2) = 0;
    Imag(a, n / 2, n / 2) = 0;
    for (i = 1; i <= n / 2 - 1; i++) {
	for (j = 1; j <= n / 2 - 1; j++) {
	    phase = 2 * M_PI * ((random() & 0x7FFF) / arand);
	    rad = pow((double) (i * i + j * j), -(h + 1) / 2) * gauss();
	    rcos = rad * cos(phase);
	    rsin = rad * sin(phase);
	    Real(a, i, n - j) = rcos;
	    Imag(a, i, n - j) = rsin;
	    Real(a, n - i, j) = rcos;
	    Imag(a, n - i, j) = - rsin;
	}
    }

    nsize[0] = 0;
    nsize[1] = nsize[2] = n;	      /* Dimension of frequency domain array */
    fourn(a, nsize, 2, -1);	      /* Take inverse 2D Fourier transform */
}

/*  INITSEED  --  Generate initial random seed, if needed.  */

static void initseed()
{
    int i = time((long *) 0) ^ 0xF37C;
    V srandom(i);
    for (i = 0; i < 7; i++)
	V random();
    rseed = random();
}

/*  TEMPRGB  --  Calculate the relative R, G, and B components	for  a
		 black	body  emitting	light  at a given temperature.
		 The Planck radiation equation is solved directly  for
		 the R, G, and B wavelengths defined for the CIE  1931
		 Standard    Colorimetric    Observer.	  The	colour
		 temperature is specified in degrees Kelvin. */

static void temprgb(temp, r, g, b)
  double temp;
  double *r, *g, *b;
{
    double c1 = 3.7403e10,
	   c2 = 14384.0,
	   er, eg, eb, es;

/* Lambda is the wavelength in microns: 5500 angstroms is 0.55 microns. */

#define Planck(lambda)  ((c1 * pow((double) lambda, -5.0)) /  \
			 (pow(M_E, c2 / (lambda * temp)) - 1))

    er = Planck(0.7000);
    eg = Planck(0.5461);
    eb = Planck(0.4358);
#undef Planck

    es = 1.0 / max(er, max(eg, eb));

    *r = er * es;
    *g = eg * es;
    *b = eb * es;
}

/*  ETOILE  --	Set a pixel in the starry sky.	*/

static void etoile(pix)
  pixel *pix;
{
    if ((random() % 1000) < starfraction) {
#define StarQuality	0.5	      /* Brightness distribution exponent */
#define StarIntensity	8	      /* Brightness scale factor */
#define StarTintExp	0.5	      /* Tint distribution exponent */
	double v = StarIntensity * pow(1 / (1 - Cast(0, 0.9999)),
				       (double) StarQuality),
	       temp,
	       r, g, b;

	if (v > 255) {
	    v = 255;
	}

	/* We make a special case for star colour  of zero in order to
	   prevent  floating  point  roundoff  which  would  otherwise
	   result  in  more  than  256 star colours.  We can guarantee
	   that if you specify no star colour, you never get more than
	   256 shades in the image. */

	if (starcolour == 0) {
	    int vi = v;

	    PPM_ASSIGN(*pix, vi, vi, vi);
	} else {
	    temp = 5500 + starcolour *
			  pow(1 / (1 - Cast(0, 0.9999)), StarTintExp) *
			      ((random() & 7) ? -1 : 1);
	    /* Constrain temperature to a reasonable value: >= 2600K 
	       (S Cephei/R Andromedae), <= 28,000 (Spica). */
	    temp = max(2600, min(28000, temp));
	    temprgb(temp, &r, &g, &b);
	    PPM_ASSIGN(*pix, (int) (r * v + 0.499),
			     (int) (g * v + 0.499),
			     (int) (b * v + 0.499));
	}
    } else {
	PPM_ASSIGN(*pix, 0, 0, 0);
    }
}

/*  GENPLANET  --  Generate planet from elevation array.  */

static void genplanet(a, n)
  float *a;
  unsigned int n;
{
    int i, j;
    unsigned char *cp, *ap;
    double *u, *u1;
    unsigned int *bxf, *bxc;

#define RGBQuant    255
    pixel *pixels;		      /* Pixel vector */
    pixel *rpix;		      /* Current pixel pointer */

#define Atthick 1.03		      /* Atmosphere thickness as a percentage
                                         of planet's diameter */
    double athfac = sqrt(Atthick * Atthick - 1.0);
    double sunvec[3];

    Boolean flipped = FALSE;
    double shang, siang;

    ppm_writeppminit(stdout, screenxsize, screenysize,
		     (pixval) RGBQuant, FALSE);

    if (!stars) {
	u = (double *) malloc((unsigned int) (screenxsize * sizeof(double)));
	u1 = (double *) malloc((unsigned int) (screenxsize * sizeof(double)));
	bxf = (unsigned int *) malloc((unsigned int) screenxsize *
	      sizeof(unsigned int));
	bxc = (unsigned int *) malloc((unsigned int) screenxsize *
	      sizeof(unsigned int));

	if (u == (double *) 0 || u1 == (double *) 0 ||
	    bxf == (unsigned int *) 0 || bxc == (unsigned int *) 0) {
            pm_error("Cannot allocate %d element interpolation tables.",
		     screenxsize);
	}

	/* Compute incident light direction vector. */

	shang = hourspec ? hourangle : Cast(0, 2 * M_PI);
	siang = inclspec ? inclangle : Cast(-M_PI * 0.12, M_PI * 0.12);

	sunvec[X] = sin(shang) * cos(siang);
	sunvec[Y] = sin(siang);
	sunvec[Z] = cos(shang) * cos(siang);

	/* Allow only 25% of random pictures to be crescents */

	if (!hourspec && ((random() % 100) < 75)) {
	    flipped = sunvec[Z] < 0 ? TRUE : FALSE;
	    sunvec[Z] = abs(sunvec[Z]);
	}
        pm_message("%s: -seed %d -dimension %.2f -power %.2f -mesh %d",
            clouds ? "clouds" : "planet",
	    rseed, fracdim, powscale, meshsize);
	if (!clouds) {
	    pm_message(
               "        -inclination %.0f -hour %d -ice %.2f -glaciers %.2f",
	       (siang * (180.0 / M_PI)),
	       (int) (((shang * (12.0 / M_PI)) + 12 +
		  (flipped ? 12 : 0)) + 0.5) % 24, icelevel, glaciers);
            pm_message("        -stars %d -saturation %d.",
		starfraction, starcolour);
	}

	/* Prescale the grid points into intensities. */

	cp = (unsigned char *) malloc(n * n);
	if (cp == (unsigned char *) 0)
	    return;
	ap = cp;
	for (i = 0; i < n; i++) {
	    for (j = 0; j < n; j++) {
		*ap++ = (255.0 * (Real(a, i, j) + 1.0)) / 2.0;
	    }
	}

	/* Fill the screen from the computed  intensity  grid  by  mapping
	   screen  points onto the grid, then calculating each pixel value
	   by bilinear interpolation from  the	surrounding  grid  points.
	   (N.b. the pictures would undoubtedly look better when generated
	   with small grids if	a  more  well-behaved  interpolation  were
	   used.)

	   Before  we get started, precompute the line-level interpolation
           parameters and store them in an array so we don't  have  to  do
	   this every time around the inner loop. */

#define UPRJ(a,size) ((a)/((size)-1.0))

	for (j = 0; j < screenxsize; j++) {
	    double bx = (n - 1) * UPRJ(j, screenxsize);

	    bxf[j] = floor(bx);
	    bxc[j] = bxf[j] + 1;
	    u[j] = bx - bxf[j];
	    u1[j] = 1 - u[j];
	}
    } else {
        pm_message("night: -seed %d -stars %d -saturation %d.",
	    rseed, starfraction, starcolour);
    }

    pixels = ppm_allocrow(screenxsize);
    for (i = 0; i < screenysize; i++) {
	double t, t1, by, dy, dysq, sqomdysq, icet, svx, svy, svz,
	       azimuth;
	int byf, byc, lcos;

	if (!stars) {		      /* Skip all this setup if just stars */
	    by = (n - 1) * UPRJ(i, screenysize);
	    dy = 2 * (((screenysize / 2) - i) / ((double) screenysize));
	    dysq = dy * dy;
	    sqomdysq = sqrt(1.0 - dysq);
	    svx = sunvec[X];
	    svy = sunvec[Y] * dy;
	    svz = sunvec[Z] * sqomdysq;
	    byf = floor(by) * n;
	    byc = byf + n;
	    t = by - floor(by);
	    t1 = 1 - t;
	}

	if (clouds) {

	    /* Render the FFT output as clouds. */

	    for (j = 0; j < screenxsize; j++) {
		double r = t1 * u1[j] * cp[byf + bxf[j]] +
			   t  * u1[j] * cp[byc + bxf[j]] +
			   t  * u[j]  * cp[byc + bxc[j]] +
			   t1 * u[j]  * cp[byf + bxc[j]];
		pixval w = (r > 127.0) ? (RGBQuant * ((r - 127.0) / 128.0)) :
			   0;

		PPM_ASSIGN(*(pixels + j), w, w, RGBQuant);
	    }
	} else if (stars) {

	    /* Generate a starry sky.  Note  that no FFT is performed;
	       the output is  generated  directly  from  a  power  law
	       mapping	of  a  pseudorandom sequence into intensities. */

	    for (j = 0; j < screenxsize; j++) {
		etoile(pixels + j);
	    }
	} else {
	    for (j = 0; j < screenxsize; j++) {
		PPM_ASSIGN(*(pixels + j), 0, 0, 0);
	    }
	    azimuth = asin(((((double) i) / (screenysize - 1)) * 2) - 1);
	    icet = pow(abs(sin(azimuth)), 1.0 / icelevel) - 0.5;
	    lcos = (screenysize / 2) * abs(cos(azimuth));
	    rpix = pixels + (screenxsize / 2) - lcos;
	    for (j = (screenxsize / 2) - lcos;
		 j <= (screenxsize / 2) + lcos; j++) {
		double r = t1 * u1[j] * cp[byf + bxf[j]] +
			   t  * u1[j] * cp[byc + bxf[j]] +
			   t  * u[j]  * cp[byc + bxc[j]] +
			   t1 * u[j]  * cp[byf + bxc[j]],
		       ice;
		int ir, ig, ib;
		static unsigned char pgnd[][3] = {
		   {206, 205, 0}, {208, 207, 0}, {211, 208, 0},
		   {214, 208, 0}, {217, 208, 0}, {220, 208, 0},
		   {222, 207, 0}, {225, 205, 0}, {227, 204, 0},
		   {229, 202, 0}, {231, 199, 0}, {232, 197, 0},
		   {233, 194, 0}, {234, 191, 0}, {234, 188, 0},
		   {233, 185, 0}, {232, 183, 0}, {231, 180, 0},
		   {229, 178, 0}, {227, 176, 0}, {225, 174, 0},
		   {223, 172, 0}, {221, 170, 0}, {219, 168, 0},
		   {216, 166, 0}, {214, 164, 0}, {212, 162, 0},
		   {210, 161, 0}, {207, 159, 0}, {205, 157, 0},
		   {203, 156, 0}, {200, 154, 0}, {198, 152, 0},
		   {195, 151, 0}, {193, 149, 0}, {190, 148, 0},
		   {188, 147, 0}, {185, 145, 0}, {183, 144, 0},
		   {180, 143, 0}, {177, 141, 0}, {175, 140, 0},
		   {172, 139, 0}, {169, 138, 0}, {167, 137, 0},
		   {164, 136, 0}, {161, 135, 0}, {158, 134, 0},
		   {156, 133, 0}, {153, 132, 0}, {150, 132, 0},
		   {147, 131, 0}, {145, 130, 0}, {142, 130, 0},
		   {139, 129, 0}, {136, 128, 0}, {133, 128, 0},
		   {130, 127, 0}, {127, 127, 0}, {125, 127, 0},
		   {122, 127, 0}, {119, 127, 0}, {116, 127, 0},
		   {113, 127, 0}, {110, 128, 0}, {107, 128, 0},
		   {104, 128, 0}, {102, 127, 0}, { 99, 126, 0},
		   { 97, 124, 0}, { 95, 122, 0}, { 93, 120, 0},
		   { 92, 117, 0}, { 92, 114, 0}, { 92, 111, 0},
		   { 93, 108, 0}, { 94, 106, 0}, { 96, 104, 0},
		   { 98, 102, 0}, {100, 100, 0}, {103,	99, 0},
		   {106,  99, 0}, {109,  99, 0}, {111, 100, 0},
		   {114, 101, 0}, {117, 102, 0}, {120, 103, 0},
		   {123, 102, 0}, {125, 102, 0}, {128, 100, 0},
		   {130,  98, 0}, {132,  96, 0}, {133,	94, 0},
		   {134,  91, 0}, {134,  88, 0}, {134,	85, 0},
		   {133,  82, 0}, {131,  80, 0}, {129,	78, 0}
		};

		if (r >= 128) {
		    int ix = ((r - 128) * (ELEMENTS(pgnd) - 1)) / 127;

		    /* Land area.  Look up colour based on elevation from
		       precomputed colour map table. */

		    ir = pgnd[ix][0];
		    ig = pgnd[ix][1];
		    ib = pgnd[ix][2];
		} else {

		    /* Water.  Generate clouds above water based on
		       elevation.  */

		    ir = ig = r > 64 ? (r - 64) * 4 : 0;
		    ib = 255;
		}

		/* Generate polar ice caps. */

		ice = max(0.0, (icet + glaciers * max(-0.5, (r - 128) / 128.0)));
		if  (ice > 0.125) {
		    ir = ig = ib = 255;
		}

		/* Apply limb darkening by cosine rule. */

		{   double dx = 2 * (((screenxsize / 2) - j) /
				((double) screenysize)),
			   dxsq = dx * dx,
			   ds, di, inx;
		    double dsq, dsat;
		    di = svx * dx + svy + svz * sqrt(1.0 - dxsq);
#define 	    PlanetAmbient  0.05
		    if (di < 0)
			di = 0;
		    di = min(1.0, di);

		    ds = sqrt(dxsq + dysq);
		    ds = min(1.0, ds);

		    /* Calculate  atmospheric absorption  based on the
		       thickness of atmosphere traversed by  light  on
		       its way to the surface. */

#define 	    AtSatFac 1.0
		    dsq = ds * ds;
		    dsat = AtSatFac * ((sqrt(Atthick * Atthick - dsq) -
			    sqrt(1.0 * 1.0 - dsq)) / athfac);
#define 	    AtSat(x,y) x = ((x)*(1.0-dsat))+(y)*dsat
		    AtSat(ir, 127);
		    AtSat(ig, 127);
		    AtSat(ib, 255);

		    inx = PlanetAmbient + (1.0 - PlanetAmbient) * di;
		    ir *= inx;
		    ig *= inx;
		    ib *= inx;
		}

		PPM_ASSIGN(*rpix, ir, ig, ib);
		rpix++;
	    }

	    /* Left stars */

#define StarClose	2
	    for (j = 0; j < (screenxsize / 2) - (lcos + StarClose); j++) {
		etoile(pixels + j);
	    }

	    /* Right stars */

	    for (j = (screenxsize / 2) + (lcos + StarClose);
		 j < screenxsize; j++) {
		etoile(pixels + j);
	    }
	}
	ppm_writeppmrow(stdout, pixels, screenxsize, RGBQuant, FALSE);
    }
    pm_close(stdout);

    ppm_freerow(pixels);
    if (!stars) {
	free((char *) cp);
	free((char *) u);
	free((char *) u1);
	free((char *) bxf);
	free((char *) bxc);
    }
}

/*  PLANET  --	Make a planet.	*/

static Boolean planet()
{
    float *a = (float *) 0;
    int i, j;
    double rmin = 1e50, rmax = -1e50, rmean, rrange;

    if (!seedspec) {
	initseed();
    }
    initgauss(rseed);

    if (!stars) {

	spectralsynth(&a, meshsize, 3.0 - fracdim);
	if (a == (float *) 0) {
	    return FALSE;
	}

	/* Apply power law scaling if non-unity scale is requested. */

	if (powscale != 1.0) {
	    for (i = 0; i < meshsize; i++) {
		for (j = 0; j < meshsize; j++) {
		   double r = Real(a, i, j);

		    if (r > 0) {
			Real(a, i, j) = pow(r, powscale);
		    }
		}
	    }
	}

	/* Compute extrema for autoscaling. */

	for (i = 0; i < meshsize; i++) {
	    for (j = 0; j < meshsize; j++) {
		double r = Real(a, i, j);

		rmin = min(rmin, r);
		rmax = max(rmax, r);
	    }
	}
	rmean = (rmin + rmax) / 2;
	rrange = (rmax - rmin) / 2;
	for (i = 0; i < meshsize; i++) {
	    for (j = 0; j < meshsize; j++) {
		Real(a, i, j) = (Real(a, i, j) - rmean) / rrange;
	    }
	}
    }
    genplanet(a, meshsize);
    if (a != (float *) 0) {
	free((char *) a);
    }
    return TRUE;
}

/*  MAIN  --  Main program.  */

int main(argc, argv)
  int argc;
  char *argv[];
{
    int i;
    char *usage = "\n\
      [-width|-xsize <x>] [-height|-ysize <y>] [-mesh <n>]\n\
      [-clouds] [-dimension <f>] [-power <f>] [-seed <n>]\n\
      [-hour <f>] [-inclination|-tilt <f>] [-ice <f>] [-glaciers <f>]\n\
      [-night] [-stars <n>] [-saturation <n>]";
    Boolean dimspec = FALSE, meshspec = FALSE, powerspec = FALSE,
	    widspec = FALSE, hgtspec = FALSE, icespec = FALSE,
	    glacspec = FALSE, starspec = FALSE, starcspec = FALSE;

    ppm_init(&argc, argv);
    i = 1;
    while ((i < argc) && (argv[i][0] == '-') && (argv[i][1] != '\0')) {

        if (pm_keymatch(argv[i], "-clouds", 2)) {
	    clouds = TRUE;
        } else if (pm_keymatch(argv[i], "-night", 2)) {
	    stars = TRUE;
        } else if (pm_keymatch(argv[i], "-dimension", 2)) {
	    if (dimspec) {
                pm_error("already specified a dimension");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &fracdim)  != 1))
		pm_usage(usage);
	    if (fracdim <= 0.0) {
                pm_error("fractal dimension must be greater than 0");
	    }
	    dimspec = TRUE;
        } else if (pm_keymatch(argv[i], "-hour", 3)) {
	    if (hourspec) {
                pm_error("already specified an hour");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &hourangle) != 1))
		pm_usage(usage);
	     hourangle = (M_PI / 12.0) * (hourangle + 12.0);
	     hourspec = TRUE;
        } else if (pm_keymatch(argv[i], "-inclination", 3) ||
                   pm_keymatch(argv[i], "-tilt", 2)) {
	    if (inclspec) {
                pm_error("already specified an inclination/tilt");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &inclangle) != 1))
		pm_usage(usage);
	    inclangle = (M_PI / 180.0) * inclangle;
	    inclspec = TRUE;
        } else if (pm_keymatch(argv[i], "-mesh", 2)) {
	    unsigned int j;

	    if (meshspec) {
                pm_error("already specified a mesh size");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &meshsize) != 1))
		pm_usage(usage);

	    /* Force FFT mesh to the next larger power of 2. */

	    for (j = meshsize; (j & 1) == 0; j >>= 1) ;

	    if (j != 1) {
		for (j = 2; j < meshsize; j <<= 1) ;
		meshsize = j;
	    }
	    meshspec = TRUE;
        } else if (pm_keymatch(argv[i], "-power", 2)) {
	    if (powerspec) {
                pm_error("already specified a power factor");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &powscale) != 1))
		pm_usage(usage);
	    if (powscale <= 0.0) {
                pm_error("power factor must be greater than 0");
	    }
	    powerspec = TRUE;
        } else if (pm_keymatch(argv[i], "-ice", 3)) {
	    if (icespec) {
                pm_error("already specified ice cap level");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &icelevel) != 1))
		pm_usage(usage);
	    if (icelevel <= 0.0) {
                pm_error("ice cap level must be greater than 0");
	    }
	    icespec = TRUE;
        } else if (pm_keymatch(argv[i], "-glaciers", 2)) {
	    if (glacspec) {
                pm_error("already specified glacier level");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%lf", &glaciers) != 1))
		pm_usage(usage);
	    if (glaciers <= 0.0) {
                pm_error("glacier level must be greater than 0");
	    }
	    glacspec = TRUE;
        } else if (pm_keymatch(argv[i], "-stars", 3)) {
	    if (starspec) {
                pm_error("already specified a star fraction");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &starfraction) != 1))
		pm_usage(usage);
	    starspec = TRUE;
        } else if (pm_keymatch(argv[i], "-saturation", 3)) {
	    if (starcspec) {
                pm_error("already specified a star colour saturation");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &starcolour) != 1))
		pm_usage(usage);
	    starcspec = TRUE;
        } else if (pm_keymatch(argv[i], "-seed", 3)) {
	    if (seedspec) {
                pm_error("already specified a random seed");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &rseed) != 1))
		pm_usage(usage);
	    seedspec = TRUE;
        } else if (pm_keymatch(argv[i], "-xsize", 2) ||
                   pm_keymatch(argv[i], "-width", 2)) {
	    if (widspec) {
                pm_error("already specified a width/xsize");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &screenxsize) != 1))
		pm_usage(usage);
	    widspec = TRUE;
        } else if (pm_keymatch(argv[i], "-ysize", 2) ||
                   pm_keymatch(argv[i], "-height", 3)) {
	    if (hgtspec) {
                pm_error("already specified a height/ysize");
	    }
	    i++;
            if ((i == argc) || (sscanf(argv[i], "%d", &screenysize) != 1))
		pm_usage(usage);
	    hgtspec = TRUE;
	} else {
	    pm_usage(usage);
	}
	i++;
    }

    /* Set defaults when explicit specifications were not given.

       The  default  fractal  dimension  and  power  scale depend upon
       whether we're generating a planet or clouds. */

    if (!dimspec) {
	fracdim = clouds ? 2.15 : 2.4;
    }
    if (!powerspec) {
	powscale = clouds ? 0.75 : 1.2;
    }
    if (!icespec) {
	icelevel = 0.4;
    }
    if (!glacspec) {
	glaciers = 0.75;
    }
    if (!starspec) {
	starfraction = 100;
    }
    if (!starcspec) {
	starcolour = 125;
    }

    /* Force  screen to be at least  as wide as it is high.  Long,
       skinny screens  cause  crashes  because	picture  width	is
       calculated based on height.  */

    screenxsize = max(screenysize, screenxsize);
    screenxsize = (screenxsize + 1) & (~1);
    exit(planet() ? 0 : 1);
}
