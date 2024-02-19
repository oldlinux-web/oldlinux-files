/*                               r e m o v e                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

#define MACRO_SHADOW
#include "hidden.h"

int remove F1(CONST char *, name)

{
  return unlink(name) != 0 ? EOF : 0;
}
