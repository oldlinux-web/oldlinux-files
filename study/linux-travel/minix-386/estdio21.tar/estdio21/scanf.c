/*                                s c a n f                                */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS1*/
/*ARGSUSED*/

int scanf F1V(CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = __vfscanf(stdin, fmt, arg);
  VA_END(arg);
  return v;
}
