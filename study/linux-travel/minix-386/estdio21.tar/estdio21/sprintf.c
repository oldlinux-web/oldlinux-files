/*                              s p r i n t f                              */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS2*/
/*ARGSUSED*/

int sprintf F2V(char *, buf, CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = vsprintf(buf, fmt, arg);
  VA_END(arg);

  return v;
}
