/*
 *   dpicheck of dvips.c.  Checks the dots per inch to make sure
 *   it exists.  The fonts which exist are DPI at the appropriate
 *   magnifications.
 */
#include "structures.h"
/*
 *   This macro multiplies an integer by a float, and returns
 *   the *rounded* value.  It is important that it do this
 *   correctly.
 */
#define scale(a,b) (i=a*b*2.0,(i<a*b*2.0?(i+1)/2:i/2))
/*
 *   The MARGIN is the amount of error the rounding can incur.
 *   We set this to 5; at 300 dpi, this is only 1.7%; at 2540 it
 *   is .2%.  Give at least three digits!
 */
#define MARGIN 5
static integer existsizes[11] ;
static int checkdpi ;
halfword
dpicheck(dpi)
halfword dpi ;
{
   integer i ;

   if (! checkdpi) {
      existsizes[0] = 0 ;
      existsizes[1] = DPI ;
      existsizes[2] = scale(DPI, 1.095445115) ;
      existsizes[3] = scale(DPI, 1.2) ;
      existsizes[4] = scale(DPI, 1.44) ;
      existsizes[5] = scale(DPI, 1.728) ;
      existsizes[6] = scale(DPI, 2.0736) ;
      existsizes[7] = scale(DPI, 2.48832) ;
      existsizes[8] = scale(DPI, 2.985984) ;
      existsizes[9] = scale(DPI, 3.5831808) ;
      existsizes[10] = 999999999 ;
      checkdpi = 1 ;
   }
   for (i=0; existsizes[i] < dpi; i++) ;
   if (existsizes[i]-dpi < MARGIN)
      return(existsizes[i]) ;
   else if (dpi-existsizes[i-1] < MARGIN)
      return(existsizes[i-1]) ;
   else
      return(dpi) ;
}
