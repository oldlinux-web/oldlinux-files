#include <soft.h>

#ifndef SOFT_387

double
modf(double x, double *iptr)
{
  double tmp;
  volatile short cw, cwtmp;

  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );
  cwtmp = cw | 0xc00;
  __asm__ volatile ("fldcw %0" : : "m" (cwtmp));
  __asm__ volatile ("frndint" : "=t" (tmp) : "0" (x));
  __asm__ volatile ("fldcw %0" : : "m" (cw));
  *iptr = tmp;

  return (x - tmp);
}

#else

#define shiftleft(dp,n)	{	/* n = 0 to 32 */ \
	dp->mant1 = ((dp->mant1 << (n)) + (dp->mant2 >> (32-(n)))) \
		& 0x0FFFFF; dp->mant2 <<= (n); dp->exp -= (n); }


/*  Returns fractional part of d, stores integer part in *integ
 */
double modf(double d, double *integ)
{
    struct bitdouble *dp = (struct bitdouble *)&d;
    struct bitdouble *ip = (struct bitdouble *)integ;
    int e = dp->exp - BIAS;

    if (e < 0) {		/* no integer part */
	*integ = 0;
	return d;
    }

    /* compute integer: clear fractional part where necessary 
     */
    *integ = d;
    if (e <= 20) {
	ip->mant1 &= (-1L << (20-e));		/* split in mant1... */
	ip->mant2 = 0;
    }
    else 
      if (e <= 52) 
	ip->mant2 &= (-1L << (52-e));		/* split in mant2 */
      else return 0;				/* no fractional part */

    /* compute fractional part: shift left over integer part
     */
    if (e)
      if (e <= 32)
	shiftleft(dp,e)
      else {
	dp->mant1 = (dp->mant2 << (e-32)) & 0x0FFFFF;
	dp->mant2 = 0;
	dp->exp -= e;
      }

    /* adjust fractional part shifting left... 
     */
    if (dp->mant1==0 && dp->mant2==0)	/* fraction is zero */
	return 0;

    while (!(dp->mant1 & 0x080000)) 	/* stack to the left */
	shiftleft(dp,1);

    shiftleft(dp,1);			/* lose 'invisible bit' */
    return d;
}
#endif
