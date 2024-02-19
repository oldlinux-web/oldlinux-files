/*                             _ f _ p o w 1 0                             */

/* Floating point powers of ten.
 *
 * __gpow10 computes 10**y where y is an integer. It returns the
 * result as a guarded longdouble.
 *
 * __pow10 is the same as __gpow10 but returns an unguarded longdouble.
 *
 * __mpow10 is the smallest power available. __Mpow10 is the largest
 * power available.
 *
 * __fpow10 is a pointer into the small power table for floating point
 * versions of __ipow10. At least __Mipow10 values are available.
 */

#include "stdiolib.h"
#include <math.h>

/*LINTLIBRARY*/

/* Floating point power table indices
 *
 * These indices yield important offsets into the power tables:
 *
 *	N_SMALLPOWERS	number of longdoubles in the small power table
 *	N_LARGEPOWERS	number of longdoubles in the large power table
 *	B_SMALLPOWERS	longdouble index to locate 1e0
 *	B_LARGEPOWERS	longdouble index to locate largest negative power
 *	M_SMALLPOWERS	largest power in small power table plus one
 *	M_LARGEPOWERS	largest power in large power table
 */
#define N_SMALLPOWERS	(sizeof(smallpowers) / sizeof(longdouble))
#define B_SMALLPOWERS	((N_SMALLPOWERS - 1) / 3 * 2)
#define M_SMALLPOWERS	((N_SMALLPOWERS - 1) / 3 + 1)
#define N_LARGEPOWERS	(sizeof(largepowers) / sizeof(longdouble))
#define B_LARGEPOWERS	(N_LARGEPOWERS / 2 - 2)
#define M_LARGEPOWERS	(N_LARGEPOWERS / 4 * M_SMALLPOWERS)

#ifdef	POWERS
  static int smallpowers[] = {
#define SMALL
#include "powers.h"
  };
  static int largepowers[] = {
#define LARGE
#include "powers.h"
  };
#else
  static longdouble smallpowers[] = {
    1e-15, 0e0,   1e-14, 0e0,   1e-13, 0e0,   1e-12, 0e0,
    1e-11, 0e0,   1e-10, 0e0,   1e-9,  0e0,   1e-8,  0e0,
    1e-7,  0e0,   1e-6,  0e0,   1e-5,  0e0,   1e-4,  0e0,
    1e-3,  0e0,   1e-2,  0e0,   1e-1,  0e0,
    1e0,   1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,
    1e8,   1e9,   1e10,  1e11,  1e12,  1e13,  1e14,  1e15,
  };
  static longdouble largepowers[] = {
#if LDBL_MAX_10_EXP >= 320
    << Large power table needs to be extended >>
#endif
                 1e-304, 0e0, 1e-288, 0e0, 1e-272, 0e0,
    1e-256, 0e0, 1e-240, 0e0, 1e-224, 0e0, 1e-208, 0e0,
    1e-192, 0e0, 1e-176, 0e0, 1e-160, 0e0, 1e-144, 0e0,
    1e-128, 0e0, 1e-112, 0e0, 1e-96,  0e0, 1e-80,  0e0,
    1e-64,  0e0, 1e-48,  0e0, 1e-32,  0e0, 1e-16,  0e0,
    1e016,  0e0, 1e032,  0e0, 1e048,  0e0, 1e064,  0e0,
    1e080,  0e0, 1e096,  0e0, 1e112,  0e0, 1e128,  0e0,
    1e144,  0e0, 1e160,  0e0, 1e176,  0e0, 1e192,  0e0,
    1e208,  0e0, 1e224,  0e0, 1e240,  0e0, 1e256,  0e0,
    1e272,  0e0, 1e288,  0e0, 1e304,  0e0,
  };
#endif

int __mpow10 = 1 - M_LARGEPOWERS - M_SMALLPOWERS;
int __Mpow10 = LDBL_MAX_10_EXP;

/* Small floating point powers of ten
 *
 *  Ensure that at least __Mipow10 values are available.
 */

longdouble *__fpow10 = &((longdouble *) smallpowers)[B_SMALLPOWERS];

static longdouble __xpow10 F2(int, exp, longguard *, r)

{
  unsigned int aexp;			/* absolute value of exponent */
  int seg;				/* large power index */
  int off;				/* small power index */
  longdouble *p;			/* array pointer */
  longguard n;				/* guarded multiplicand */
  longguard m;				/* guarded multiplier */

  aexp = exp < 0 ? -exp : exp;

  if (aexp < M_SMALLPOWERS) {
    if (exp < 0)
      exp *= 2;
    p = &((longdouble *) smallpowers)[exp + B_SMALLPOWERS];
    if (r == NULL)
      return exp < 0 ? p[0] + p[1] : p[0];
    else {
      r->number   = p[0];
      r->guard    = exp < 0 ? p[1] : 0.0;
      r->exponent = 0;
      if (exp > HALFMANTDIGITS)
	__gguard(r);
    }
  }

  else if (exp >= __mpow10 && exp <= __Mpow10) {

    seg = aexp / M_SMALLPOWERS;
    off = aexp - (seg * M_SMALLPOWERS);

    if (exp >= 0)
      m.exponent = POSEXPONENT;
    else {
      seg  = 1 - seg;
      off  = -off;
      off *= 2;
      m.exponent  = NEGEXPONENT;
    }
    p = &((longdouble *) smallpowers)[off + B_SMALLPOWERS];
    n.number   = p[0];
    n.guard    = off < 0 ? p[1] : 0.0;
    n.exponent = 0;
    __gguard(&n);

    p = &((longdouble *) largepowers)[2*seg + B_LARGEPOWERS];
    m.number = p[0];
    m.guard  = p[1];

    if (r == NULL)
      return __gmul(&n, &m);
    else {
      __ggmul(r, &n, &m);
      __gguard(r);
    }
  }

  else if (r == NULL)
    return HUGE_VAL;
  else {
    r->number   = HUGE_VAL;
    r->guard    = 0.0;
    r->exponent = 0;
  }
  return 0.0;
}

void __gpow10 F2(int, exp, longguard *, r)

{
  (void) __xpow10(exp, r);
}

longdouble __pow10 F1(int, exp)

{
  return __xpow10(exp, (longguard *) NULL);
}
