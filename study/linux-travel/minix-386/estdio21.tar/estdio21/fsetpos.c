/*                              f s e t p o s                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fsetpos F2(FILE *, fp, fpos_t *, pp)

{
  return fseek(fp, *pp, SEEK_SET) != 0 ? EOF : 0;
}
