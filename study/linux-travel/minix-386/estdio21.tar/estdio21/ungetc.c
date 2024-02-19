/*                               u n g e t c                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

int ungetc F2(int, ch, register FILE *, fp)

{
  if (ch == EOF  || TESTFLAG(fp, _IOREAD) == 0 ||
      fp->__base == NULL || (fp->__rptr == fp->__base && ! TESTFLAG(fp, _IONBF)))
    return EOF;

  if (TESTFLAG(fp, _IONBF) != 0)
    fp->__rptr = fp->__rend = fp->__base + 1;

  return UCHAR(*--fp->__rptr = ch);
}
