/*                                 p u t c                                 */

#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int putc F2(int, ch, register FILE *, fp)

{
  return fp->__wptr >= fp->__wend
         ? (*fp->__flsbuf)(ch, fp)
	 : UCHAR(*fp->__wptr++ = ch);
}
