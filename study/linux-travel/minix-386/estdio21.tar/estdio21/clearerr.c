/*                             c l e a r e r r                             */
 
#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

void clearerr F1(FILE *, fp)

{
  CLEARFLAG(fp, (_IOEOF | _IOERR));
}
