#include "../site.h"
#ifdef	FLOAT
# include <float.h>
# if	DBL_MANT_DIG < LDBL_MANT_DIG && \
	DBL_MAX_EXP  < LDBL_MAX_EXP  && \
	DBL_MIN_EXP  > LDBL_MIN_EXP
#   define LONGDOUBLE
# endif
#endif

#ifdef		LONGDOUBLE
  typedef long double longdouble;
#else
  typedef double longdouble;
# undef		LDBL_DIG
# define	LDBL_DIG	DBL_DIG
# undef		LDBL_EPSILON
# define	LDBL_EPSILON	DBL_EPSILON
# undef		LDBL_MANT_DIG
# define	LDBL_MANT_DIG	DBL_MANT_DIG
# undef		LDBL_MAX
# define	LDBL_MAX	DBL_MAX
# undef		LDBL_MAX_10_EXP
# define	LDBL_MAX_10_EXP	DBL_MAX_10_EXP
# undef		LDBL_MAX_EXP
# define	LDBL_MAX_EXP	DBL_MAX_EXP
# undef		LDBL_MIN
# define	LDBL_MIN	DBL_MIN
# undef		LDBL_MIN_10_EXP
# define	LDBL_MIN_10_EXP	DBL_MIN_10_EXP
# undef		LDBL_MIN_EXP
# define	LDBL_MIN_EXP	DBL_MIN_EXP
#endif

#include <stdio.h>

/* Dump the floating point number as long words */

void dump(n, m)

char *n;
longdouble m;

{
  char b[128];
  int *p;
  int i;
  int iwidth;
  int xwidth;

  iwidth = sizeof(int) < 4 ? 7 : 11;
  xwidth = sizeof(int) * 2;

  p = (int *) &m;
  printf("  ");
  for (i = sizeof(longdouble)/sizeof(int); i--; p++)
    printf("%*d,", iwidth, *p);
  printf("\n");

  p = (int *) &m;
  printf("/* ");
  for (i = sizeof(longdouble)/sizeof(int); i--; p++)
    printf("0x%*x, ", xwidth, *p);
  printf(" %s */\n", n);
}

int main(argc, argv)

int argc;
char **argv;

{
  longdouble f, m;
  longdouble fullmantissa;
  int d;

  printf("/* %d figit table generated by ylmr */\n\n", LDBL_MANT_DIG);

/* Generate a mantissa full of flt_radix-1 digits. This will be
 * used later for other calculations.
 */
  for (fullmantissa = 0, f = FLT_RADIX-1, d = DBL_MANT_DIG; d > 0; d--) {
    fullmantissa += f;
    f *= FLT_RADIX;
  }
  fullmantissa /= f/(FLT_RADIX-1);

/* Determine the largest number held by the representation. This
 * is done by multiplying a full mantissa by flt_radix^dbl_max_exp.
 */
  for (m = fullmantissa, d = DBL_MAX_EXP; d > 0; m *= FLT_RADIX, d--)
    ;

  dump("LDBL_MAX / FLT_RADIX", m/FLT_RADIX);

  return 0;
}
