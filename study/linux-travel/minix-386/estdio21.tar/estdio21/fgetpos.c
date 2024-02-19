/*                              f g e t p o s                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fgetpos F2(FILE *, fp, fpos_t *, pp)

{
  register long pos;			/* reported position */
  
  return (pos = ftell(fp)) == -1L ? EOF : (*pp = pos, 0);
}
