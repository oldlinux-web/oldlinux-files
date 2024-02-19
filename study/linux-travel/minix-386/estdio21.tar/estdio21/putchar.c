/*                              p u t c h a r                              */

#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int putchar F1(int, ch)

{
  return stdout->__wptr >= stdout->__wend
         ? (*stdout->__flsbuf)(ch, stdout)
	 : UCHAR(*stdout->__wptr++ = ch);
}
