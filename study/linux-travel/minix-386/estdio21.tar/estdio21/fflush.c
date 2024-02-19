/*                               f f l u s h                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fflush F1(FILE *, fp)

{
  return FFLUSH(fp);
}
