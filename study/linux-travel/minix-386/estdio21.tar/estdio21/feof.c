/*                                 f e o f                                 */
 
#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int feof F1(FILE *, fp)

{
  return TESTFLAG(fp, _IOEOF);
}
