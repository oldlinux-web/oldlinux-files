/*                               s s c a n f                               */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS2*/
/*ARGSUSED*/

int sscanf F2V(CONST char *, buf, CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = __vsscanf(buf, fmt, arg);
  VA_END(arg);

  return v;
}
