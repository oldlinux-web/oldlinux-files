/*                               _ x u t o a                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

#if '0'+1 != '1' || '1'+1 != '2' || '2'+1 != '3' || '3'+1 != '4' || \
    '4'+1 != '5' || '5'+1 != '6' || '6'+1 != '7' || '7'+1 != '8' || \
    '8'+1 != '9'
  << Violation of collating sequence assumption >>
#endif

char *__utoa F3(char *, p, register unsigned int, n, int, ndigits)

{
  register int quotient;		/* divided by 10 */

  quotient = 0;
  if (--ndigits > 0 || n >= 10)
    p = __utoa(p, quotient = (n / 10), ndigits);

  *p++ = (n - (((quotient << 2) + quotient) << 1)) + '0';

  return p;
}
