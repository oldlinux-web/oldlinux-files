#include <soft.h>

/*  Stores binary exponent of d in e, and returns whole fraction of d
 *   (with binary exponent of 0) (special case for d=0)
 */
double frexp(double d, int *e)
{
    struct bitdouble *dp = (struct bitdouble *)&d;
 
    if (dp->exp == 0)	/* value is zero, return exponent of 0 */
	*e = 0;
    else {
	*e = dp->exp - BIAS + 1;
	dp->exp = BIAS - 1;
    }
    return d;
}
