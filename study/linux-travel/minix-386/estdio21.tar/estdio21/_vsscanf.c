/*                             _ v s s c a n f                             */

#include "stdiolib.h"

/*LINTLIBRARY*/

int __vsscanf F3(CONST char *, buf, CONST char *, fmt, VA_LIST, args)

{
  FILE f;				/* temporary file */

  f.__flag   = _IOREAD | _IOSTRING;
  f.__base   = (__stdiobuf_t *) buf;
  f.__bufsiz = strlen(buf);

  f.__filbuf = __bffil;
  f.__flsbuf = __bffls;
  f.__flush  = __bffil;

  INITREADBUFFER(&f, f.__bufsiz);

  return __vfscanf(&f, fmt, args);
}
