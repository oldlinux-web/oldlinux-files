/* The <math.h> header contains prototypes for mathematical functions
 * for Linux.
 */

#ifndef _MATH_H
#define _MATH_H

#include <traditional.h>

#ifdef __cplusplus
extern "C" {
#endif

extern double acos _ARGS ((double __x));
extern double acosh _ARGS ((double __x));
extern double asin _ARGS ((double __x));
extern double asinh _ARGS ((double __x));
extern double atan _ARGS ((double __x));
extern double atan2 _ARGS ((double __y, double __x));
extern double atanh _ARGS ((double __x));
extern double ceil _ARGS ((double __x));
extern double cos _ARGS ((double __x));
extern double cosh _ARGS ((double __x));
extern double drem _ARGS ((double __x, double __y));
extern double exp _ARGS ((double __x));
extern double fabs _ARGS ((double __x));
extern double floor _ARGS ((double __x));
extern double fmod _ARGS ((double __x, double __y));
extern double hypot _ARGS ((double __x, double __y));
extern double frexp _ARGS ((double __x, int *__exp));
extern double ldexp _ARGS ((double __x, int __exp));
extern double log _ARGS ((double __x));
extern double log10 _ARGS ((double __x));
extern double modf _ARGS ((double __x, double *__iptr));
extern double pow _ARGS ((double __x, double __y));
extern double pow2 _ARGS ((double __x));
extern double pow10 _ARGS ((double __x));
extern double sin _ARGS ((double __x));
extern double sinh _ARGS ((double __x));
extern double sqrt _ARGS ((double __x));
extern double tan _ARGS ((double __x));
extern double tanh _ARGS ((double __x));

extern double erf  _ARGS ((double __x));
extern double erfc  _ARGS ((double __x));
extern double j0  _ARGS ((double __x));
extern double j1  _ARGS ((double __x));
extern double jn  _ARGS ((int __n, double __x));
extern double lgamma  _ARGS ((double __x));
extern double y0  _ARGS ((double __x));
extern double y1  _ARGS ((double __x));
extern double yn  _ARGS ((int __n, double __x));

#ifdef __cplusplus
}
#endif

#ifndef __LIBRARY__
extern int signgam;
#endif

#include <float.h>
#include <values.h>

#ifndef HUGE
#define HUGE		DBL_MAX
#endif

#ifndef HUGE_VAL
#define HUGE_VAL	DBL_MAX
#endif

#ifndef M_E
#define M_E         2.7182818284590452354
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154
#endif
#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962
#endif
#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440
#endif

#ifndef PI                      /* as in stroustrup */
#define PI  M_PI
#endif
#ifndef PI2
#define PI2  M_PI_2
#endif

#endif /* _MATH_H */
