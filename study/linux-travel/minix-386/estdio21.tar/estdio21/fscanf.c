/*                               f s c a n f                               */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS2*/
/*ARGSUSED*/

int fscanf F2V(FILE *, fp, CONST char *, fmt,

{
  register VA_LIST, arg,		/* argument vector */
  register int v;			/* return value */

  VA_START(arg, fmt);)
  v = __vfscanf(fp, fmt, arg);
  VA_END(arg);
  return v;
}
