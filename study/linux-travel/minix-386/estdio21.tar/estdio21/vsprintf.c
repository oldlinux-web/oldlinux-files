/*                             v s p r i n t f                             */

#include "stdiolib.h"

/*LINTLIBRARY*/

int vsprintf F3(char *, buf, CONST char *, fmt, VA_LIST, args)

{
  register int v;			/* return value */
  FILE f;				/* temporary file */

  f.__flag   = _IOWRITE | _IOSTRING;
  f.__base   = (__stdiobuf_t *) buf;
  f.__bufsiz = (__stdiobuf_t *) (~ (unsigned long) 0) - buf;

  f.__filbuf = __bffil;
  f.__flsbuf = __bffls;
  f.__flush  = __bffil;

  INITWRITEBUFFER(&f);

  v = __vfprintf(&f, fmt, args);
  (void) FPUTC(0, &f);

  return v;
}
