/*                                f p u t c                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fputc F2(int, ch, register FILE *, fp)

{
  return putc(ch, fp);
}

