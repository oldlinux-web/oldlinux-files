#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

jmp_buf errjmp;

void exception(sig)

int sig;

{
  longjmp(errjmp, 1);
}

int main(argc, argv)

int argc;
char *argv[];

{
  int dbl_dig;
  int dbl_min_10_exp;
  int dbl_max_10_exp;
  int dbl_mant_dig;
  int flt_radix;
  int dbl_max_exp;
  int dbl_min_exp;
  double dbl_epsilon;
  double dbl_max;
  double dbl_min;
  int strcmp();
  double fullmantissa, e, m, f, g;
  int i, d, r;

  if (argc > 2)
    exit(1);

/* Decide on the radix. The way this is done is to generate e, the
 * smallest power of two for which 1 + e == 1. Once, e is discovered
 * determine the least m > e. The radix is then m - e.
 */
  for (e = 1.0; 1 + e != e; e *= 2.0)
    ;
  for (m = 1.0; e + m == e; m += 1.0)
    ;
  g = e + m;
  flt_radix = g - e;

/* Determine the smallest positive number for which 1 + x != 1.
 * This is done by dividing a single bigit by the radix until
 * 1 + x = 1.
 */
  for (e = 1.0; ; ) {
    f = e / flt_radix;
    if (1 + f == 1)
      break;
    e = f;
  }
  dbl_epsilon = e;

/* Determine the number of digits in the mantissa. This is done
 * by filling the mantissa with one digits. When the mantissa
 * overflows, the least significant digits will be lost. When
 * an attempt is made to undo the operation, the result will
 * not equal the original.
 */
  for (d = 0, m = 0.0, f = 1.0; ; f *= flt_radix) {
    g = m + f;
    if (g - f != m)
      break;
    m = g;
    d++;
  }
  dbl_mant_dig = d;

/* Generate a mantissa full of flt_radix-1 digits. This will be
 * used later for other calculations.
 */
  for (fullmantissa = 0, f = flt_radix-1, d = dbl_mant_dig; d > 0; d--) {
    fullmantissa += f;
    f *= flt_radix;
  }
  fullmantissa /= f/(flt_radix-1);

/* Determine the largest exponent. Assume that when the representation
 * saturates, either SIGFPE or result is infinity will occur.
 */
  signal(SIGFPE, exception);
  if (setjmp(errjmp) == 0) {
    for (d = 0, f = 1.0/flt_radix; ; d++) {
      f *= flt_radix;
      if (f == f / flt_radix)
	break;
    }
  }
  signal(SIGFPE, SIG_DFL);
  dbl_max_exp = d;

/* Determine the smallest exponent. Assume that when the representation
 * underflows, the result will be zero, or denormalised.
 */
  for (m = fullmantissa, d = 0; ; --d) {
    f = m / flt_radix;
    if (f == 0)
      break;
    if (f * flt_radix != m)
      break;
    m = f;
  }
  dbl_min_exp = d;

/* Determine the number of decimal digits held by the mantissa. This
 * can be done by introducing 9s until bits fall off.
 */
  for (d = 1, f = 9; ; d++) {
    g = f * 10 + 9;
    if (f != (g - 9) / 10)
      break;
    f = g;
  }
  dbl_dig = d;

/* Determine the largest number held by the representation. This
 * is done by multiplying a full mantissa by flt_radix^dbl_max_exp.
 */
  for (m = fullmantissa, d = dbl_max_exp; d > 0; m *= flt_radix, d--)
    ;
  dbl_max = m;

/* Determine the power of ten held by the floating point representation.
 */
  for (f = 1, d = 0; f <= dbl_max/10; d++)
    f *= 10;
  dbl_max_10_exp =  d;

/* Determine the smallest (normalised) number held by the representation.
 * Denormalisation can be caught be simultaneously dividing down a full
 * mantissa.
 */
  for (m = fullmantissa, e = 1.0/flt_radix; ; ) {
    f = e / flt_radix;
    if (f == 0)
      break;
    g = m / flt_radix;
    if (g * flt_radix != m)
      break;
    e = f;
    m = g;
  }
  dbl_min = e;

/* Determine the smallest (normalised) power of ten held by the representation.
 * This is done by repeatedly dividing down by 10.
 */
  for (d = 0, m = 1.0; ; d--) {
    f = m / 10;
    if (f == 0)
      break;
    if (f < dbl_min)
      break;
    m = f;
  }
  dbl_min_10_exp = d;

/* Print results */

  if (argc < 2 || strcmp(argv[1], "DBL_EPSILON") == 0)
    printf("#define DBL_EPSILON %.*lg\n", dbl_dig, dbl_epsilon);
  if (argc < 2 || strcmp(argv[1], "DBL_DIG") == 0)
    printf("#define DBL_DIG %d\n", dbl_dig);
  if (argc < 2 || strcmp(argv[1], "DBL_MANT_DIG") == 0)
    printf("#define DBL_MANT_DIG %d\n", dbl_mant_dig);
  if (argc < 2 || strcmp(argv[1], "DBL_MAX") == 0)
    printf("#define DBL_MAX %.*lg\n", dbl_dig, dbl_max);
  if (argc < 2 || strcmp(argv[1], "DBL_MAX_10_EXP") == 0)
    printf("#define DBL_MAX_10_EXP %d\n", dbl_max_10_exp);
  if (argc < 2 || strcmp(argv[1], "DBL_MAX_EXP") == 0)
    printf("#define DBL_MAX_EXP %d\n", dbl_max_exp);
  if (argc < 2 || strcmp(argv[1], "DBL_MIN") == 0)
    printf("#define DBL_MIN %.*lg\n", dbl_dig, dbl_min);
  if (argc < 2 || strcmp(argv[1], "DBL_MIN_10_EXP") == 0)
    printf("#define DBL_MIN_10_EXP %d\n", dbl_min_10_exp);
  if (argc < 2 || strcmp(argv[1], "DBL_MIN_EXP") == 0)
    printf("#define DBL_MIN_EXP %d\n", dbl_min_exp);

  if (argc < 2 || strcmp(argv[1], "LDBL_EPSILON") == 0)
    printf("#define LDBL_EPSILON %.*lg\n", dbl_dig, dbl_epsilon);
  if (argc < 2 || strcmp(argv[1], "LDBL_DIG") == 0)
    printf("#define LDBL_DIG %d\n", dbl_dig);
  if (argc < 2 || strcmp(argv[1], "LDBL_MANT_DIG") == 0)
    printf("#define LDBL_MANT_DIG %d\n", dbl_mant_dig);
  if (argc < 2 || strcmp(argv[1], "LDBL_MAX") == 0)
    printf("#define LDBL_MAX %.*lg\n", dbl_dig, dbl_max);
  if (argc < 2 || strcmp(argv[1], "LDBL_MAX_10_EXP") == 0)
    printf("#define LDBL_MAX_10_EXP %d\n", dbl_max_10_exp);
  if (argc < 2 || strcmp(argv[1], "LDBL_MAX_EXP") == 0)
    printf("#define LDBL_MAX_EXP %d\n", dbl_max_exp);
  if (argc < 2 || strcmp(argv[1], "LDBL_MIN") == 0)
    printf("#define LDBL_MIN %.*lg\n", dbl_dig, dbl_min);
  if (argc < 2 || strcmp(argv[1], "LDBL_MIN_10_EXP") == 0)
    printf("#define LDBL_MIN_10_EXP %d\n", dbl_min_10_exp);
  if (argc < 2 || strcmp(argv[1], "LDBL_MIN_EXP") == 0)
    printf("#define LDBL_MIN_EXP %d\n", dbl_min_exp);

  if (argc < 2 || strcmp(argv[1], "FLT_RADIX") == 0)
    printf("#define FLT_RADIX %d\n", flt_radix);

  return 0;
}
