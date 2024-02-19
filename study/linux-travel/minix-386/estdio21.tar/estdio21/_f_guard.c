/*                             _ f _ g u a r d                             */

/* This code implements a floating point number with added guard bits.
 * Each number is stored with only half the mantissa of the number used.
 * The least significant bits are combined with the guard bits and are
 * stored in the guard.
 */

#include "stdiolib.h"
#include <math.h>

/*LINTLIBRARY*/

/* Generate guard
 *
 * The mantissa of a->number is split into two. The least significant
 * bits are added to a->guard. The most significant bits remain in a->number.
 */
void __gguard F1(longguard *, a)

{
#if	FLT_RADIX == 2
#define FLTSHIFT	1
#endif
#if	FLT_RADIX == 4
#define FLTSHIFT	2
#endif
#if	FLT_RADIX == 8
#define FLTSHIFT	3
#endif
#if	FLT_RADIX == 16
#define FLTSHIFT	4
#endif
#ifdef	FLTSHIFT
  double m;				/* normalised mantissa */
  double g;				/* guard part of mantissa */
  int exp;				/* exponent */

  m    = LFREXP(a->number, &exp);
  exp -= LDBL_MANT_DIG/2*FLTSHIFT;
  m    = LLDEXP(m, LDBL_MANT_DIG/2*FLTSHIFT);
  g    = LMODF(m, &m);
  a->number = LLDEXP(m, exp);
  a->guard += LLDEXP(g, exp);
#else
  << Extra code required to support guard generation >>
#endif
}

/* Multiply and return unguarded
 *
 * Perform (a * b) and return the result in a longdouble.
 */
longdouble __gmul F2(register longguard *, a, register longguard *, b)

{
  longdouble r;				/* result */

  r  = a->guard  * b->guard;
  r += a->number * b->guard;
  r += a->guard  * b->number;
  r += a->number * b->number;

  return LLDEXP(r, a->exponent+b->exponent);
}

/* Multiply and return guarded
 *
 * Perform (a * b) and return the guarded result in r.
 */
void __ggmul F3(longguard *, r, longguard *, a, longguard *, b)

{
  r->guard    = a->guard  * b->guard
	      + a->number * b->guard
	      + a->guard  * b->number;
  r->number   = a->number * b->number;
  r->exponent = a->exponent + b->exponent;
}

/* Convert guarded to unguarded
 *
 * Combine g->number and g->guard to return a longdouble.
 */
longdouble __gnumber F1(register longguard *, g)

{
  longdouble r;				/* result */
  
  r = g->number + g->guard;
  return g->exponent ? LLDEXP(r, g->exponent) : r;
}

/* Normalise out exponent in guard
 *
 * Scale g->number and g->guard so that g->exponent is zero.
 */
void __gnormal F1(register longguard *, g)

{
  if (g->exponent) {
    g->number   = LLDEXP(g->number, g->exponent);
    g->guard    = LLDEXP(g->guard,  g->exponent);
    g->exponent = 0;
  }
}
