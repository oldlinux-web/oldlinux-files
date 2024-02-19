#include "soft.h"

/*  Adds 'e' to binary exponent of d (unless d=0), result is returned
 */
double ldexp(double d, int e)
{
    struct bitdouble *dp = (struct bitdouble *)&d;
 
    if (dp->exp) dp->exp += e;
    return d;
}
