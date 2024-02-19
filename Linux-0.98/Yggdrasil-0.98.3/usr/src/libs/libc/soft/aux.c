#include <soft.h>

/*  I made these up - I don't know what are the actual bit-representations
 *	of infinites and NaN's    -meg
 */

#ifdef LITTLE_ENDIAN
static struct bitdouble bdouble_inf  = {0,0,0x7ff,0};
static struct bitdouble bdouble_minf = {0,0,0x7ff,1};
static struct bitdouble bdouble_huge = {0xffffffff,0xfffff,0x7ff,0};
static struct bitdouble bdouble_tiny = {0x00000000,0x00000,0x001,0};
static struct bitfloat  bfloat_huge  = {0x7fffff,0xff,0};
static struct bitfloat  bfloat_tiny  = {0x000000,0x01,0};
#else
static struct bitdouble bdouble_inf  = {0,0x7ff,0,0};
static struct bitdouble bdouble_minf = {1,0x7ff,0,0};
static struct bitdouble bdouble_NaN  = {0,0x001,0,1};
static struct bitdouble bdouble_huge = {0,0x7ff,0xfffff,0xffffffff};
static struct bitdouble bdouble_tiny = {0,0x001,0x00000,0x00000000};
static struct bitfloat  bfloat_huge  = {0,0xff,0x7fffff};
static struct bitfloat  bfloat_tiny  = {0,0x01,0x000000};
double *double_NaN	= (double*)&bdouble_NaN;
#endif

double *double_inf	= (double*)&bdouble_inf;
double *double_minf	= (double*)&bdouble_minf;
double *double_huge     = (double*)&bdouble_huge;
double *double_tiny     = (double*)&bdouble_tiny;
float  *float_huge      = (float*)&bfloat_huge;
float  *float_tiny      = (float*)&bfloat_tiny;

int finite(double arg)
{
    if (arg==*double_inf || arg==*double_minf)
	return 0;
    return 1;
}

double infnan(int iarg)
{
    /* should signal error handling; called by libm routines upon error */
    switch (iarg) {
	case EDOM:	return *double_NaN;
	case ERANGE:	return *double_inf;
	case -ERANGE:	return *double_minf;
	default:	return *double_NaN;
    }
}
