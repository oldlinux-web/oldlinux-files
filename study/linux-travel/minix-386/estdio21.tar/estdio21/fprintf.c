/*                              f p r i n t f                              */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS2*/
/*ARGSUSED*/

int fprintf F2V(FILE *, fp, CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = __vfprintf(fp, fmt, arg);
  VA_END(arg);
  return v;
}
