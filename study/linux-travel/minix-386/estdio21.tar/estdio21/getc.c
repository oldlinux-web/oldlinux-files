/*                                 g e t c                                 */

#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int getc F1(register FILE *, fp)

{
  return fp->__rptr >= fp->__rend
	 ? (*fp->__filbuf)(fp)
	 : UCHAR(*fp->__rptr++);
}
