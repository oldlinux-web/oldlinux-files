/*                               p r i n t f                               */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS1*/
/*ARGSUSED*/

int printf F1V(CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = __vfprintf(stdout, fmt, arg);
  VA_END(arg);
  return v;
}
