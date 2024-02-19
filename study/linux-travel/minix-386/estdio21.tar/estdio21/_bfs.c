/*                                 _ b f s                                 */

/* These are dummy functions that are used to return default success
 * or failure values.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*ARGSUSED*/

int __bffls F2(int, ch, FILE *, fp)

{
  return EOF;
}

/*ARGSUSED*/

int __bffil F1(FILE *, fp)

{
  return EOF;
}

/*ARGSUSED*/

int __btfls F1(FILE *, fp)

{
  return 0;
}
