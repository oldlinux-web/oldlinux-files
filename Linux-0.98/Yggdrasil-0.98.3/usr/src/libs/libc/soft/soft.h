#ifndef __SOFT_H
#define __SOFT_H

#ifndef __LIBRARY__
#define __LIBRARY__
#endif

#include <math.h>
#include <errno.h>
#include <endian.h>

extern int finite(double);
extern double infnan(int);

extern double pio2;
extern double *double_inf, *double_minf, *double_NaN;
extern double *double_huge, *double_tiny;
extern float *float_huge, *float_tiny;

#ifdef LITTLE_ENDIAN
struct bitdouble {	/* internal format of an IEEE double */
	unsigned mant2;
	unsigned mant1 : 20;
	unsigned exp : 11;
	unsigned sign : 1;	/* 0=pos, 1=neg */
};			/* value = 2^(exp-BIAS) * 0b1.|mant|mant2 */

struct bitfloat {	/* format of a float (single-precision IEEE) */
	unsigned mant : 23;
	unsigned exp : 8;
	unsigned sign : 1;
};
#else
struct bitdouble {	/* internal format of an IEEE double */
	unsigned sign : 1;	/* 0=pos, 1=neg */
	unsigned exp : 11;
	unsigned mant1 : 20;
	unsigned mant2;
};			/* value = 2^(exp-BIAS) * 0b1.|mant|mant2 */

struct bitfloat {	/* format of a float (single-precision IEEE) */
	unsigned sign : 1;
	unsigned exp : 8;
	unsigned mant : 23;
};
#endif

/* note: GNU CC floating-point constants are all treated as 'float's, 
 * so loss of precision will result accordingly when using constants;
 * an eventual atof() should handle full double values, with which
 * HUGEDOUBLE may be used. -meg
 */

#define INFINITE	(*double_inf)
#define NEG_INFINITE	(*double_minf)
#define NOT_A_NUMBER	(*double_NaN)
#define DHUGE		(*double_huge)
#define DTINY		(*double_tiny)
#define FHUGE		(*float_huge)
#define FTINY		(*float_tiny)

#define BIAS		0x3ff	/* added to exp of bitdouble */

#endif
