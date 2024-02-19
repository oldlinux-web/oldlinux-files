/* float.h */
#ifndef _FLOAT_H___
#define _FLOAT_H___
/* Produced by enquire version 4.3, CWI, Amsterdam */

   /* Radix of exponent representation */
#define FLT_RADIX 2
   /* Number of base-FLT_RADIX digits in the significand of a float */
#define FLT_MANT_DIG 24
   /* Number of decimal digits of precision in a float */
#define FLT_DIG 6
   /* Addition rounds to 0: zero, 1: nearest, 2: +inf, 3: -inf, -1: unknown */
#define FLT_ROUNDS 1
   /* Difference between 1.0 and the minimum float greater than 1.0 */
#define FLT_EPSILON 1.19209290e-07F
   /* Minimum int x such that FLT_RADIX**(x-1) is a normalised float */
#define FLT_MIN_EXP (-125)
   /* Minimum normalised float */
#define FLT_MIN 1.17549435e-38F
   /* Minimum int x such that 10**x is a normalised float */
#define FLT_MIN_10_EXP (-37)
   /* Maximum int x such that FLT_RADIX**(x-1) is a representable float */
#define FLT_MAX_EXP 128
   /* Maximum float */
#define FLT_MAX 3.40282347e+38F
   /* Maximum int x such that 10**x is a representable float */
#define FLT_MAX_10_EXP 38

   /* Number of base-FLT_RADIX digits in the significand of a double */
#define DBL_MANT_DIG 53
   /* Number of decimal digits of precision in a double */
#define DBL_DIG 15
   /* Difference between 1.0 and the minimum double greater than 1.0 */
#define DBL_EPSILON 2.2204460492503131e-16
   /* Minimum int x such that FLT_RADIX**(x-1) is a normalised double */
#define DBL_MIN_EXP (-1021)
   /* Minimum normalised double */
#define DBL_MIN 2.2250738585072014e-308
   /* Minimum int x such that 10**x is a normalised double */
#define DBL_MIN_10_EXP (-307)
   /* Maximum int x such that FLT_RADIX**(x-1) is a representable double */
#define DBL_MAX_EXP 1024
   /* Maximum double */
#define DBL_MAX 1.7976931348623157e+308
   /* Maximum int x such that 10**x is a representable double */
#define DBL_MAX_10_EXP 308

   /* Number of base-FLT_RADIX digits in the significand of a long double */
#define LDBL_MANT_DIG 53
   /* Number of decimal digits of precision in a long double */
#define LDBL_DIG 15
   /* Difference between 1.0 and the minimum long double greater than 1.0 */
#define LDBL_EPSILON 2.2204460492503131e-16L
   /* Minimum int x such that FLT_RADIX**(x-1) is a normalised long double */
#define LDBL_MIN_EXP (-1021)
   /* Minimum normalised long double */
#define LDBL_MIN 2.2250738585072014e-308L
   /* Minimum int x such that 10**x is a normalised long double */
#define LDBL_MIN_10_EXP (-307)
   /* Maximum int x such that FLT_RADIX**(x-1) is a representable long double */
#define LDBL_MAX_EXP 1024
   /* Maximum long double */
#define LDBL_MAX 1.7976931348623157e+308L
   /* Maximum int x such that 10**x is a representable long double */
#define LDBL_MAX_10_EXP 308

#endif /*  _FLOAT_H___ */
