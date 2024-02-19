/*                               f e r r o r                               */
 
#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int ferror F1(FILE *, fp)

{
  return TESTFLAG(fp, _IOERR);
}
