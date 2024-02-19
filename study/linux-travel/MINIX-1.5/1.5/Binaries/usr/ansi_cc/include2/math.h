/* The <math.h> header contains prototypes for mathematical functions. */

#ifndef _MATH_H
#define _MATH_H

#define HUGE_VAL	9.9e+999	/* though it will generate a warning */

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif
	
_PROTOTYPE( double acos,  (double _x)					);
_PROTOTYPE( double asin,  (double _x)					);
_PROTOTYPE( double atan,  (double _x)					);
_PROTOTYPE( double atan2, (double _y, double _x)			);
_PROTOTYPE( double ceil,  (double _x)					);
_PROTOTYPE( double cos,   (double _x)					);
_PROTOTYPE( double cosh,  (double _x)					);
_PROTOTYPE( double exp,   (double _x)					);
_PROTOTYPE( double fabs,  (double _x)					);
_PROTOTYPE( double floor, (double _x)					);
_PROTOTYPE( double fmod,  (double _x, double _y)			);
_PROTOTYPE( double frexp, (double _x, int *_exp)			);
_PROTOTYPE( double ldexp, (double _x, int _exp)				);
_PROTOTYPE( double log,   (double _x)					);
_PROTOTYPE( double log10, (double _x)					);
_PROTOTYPE( double modf,  (double _x, double *_iptr)			);
_PROTOTYPE( double pow,   (double _x, double _y)			);
_PROTOTYPE( double sin,   (double _x)					);
_PROTOTYPE( double sinh,  (double _x)					);
_PROTOTYPE( double sqrt,  (double _x)					);
_PROTOTYPE( double tan,   (double _x)					);
_PROTOTYPE( double tanh,  (double _x)					);

#endif /* _MATH_H */
