/*                                 g e t s                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

char *gets F1(register char *, buf)

{
  register char *p;			/* end of input buffer */

  if ((p = __fgetlx(buf, ~((__stdiosize_t) 0), stdin)) != NULL) {
    if (p != buf && *--p == '\n')
      *p = 0;
    p = buf;
  }
  return p;
}
