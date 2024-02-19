/*                               r e w i n d                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

void rewind F1(FILE *, fp)

{
  (void) fseek(fp, 0L, SEEK_SET);
  clearerr(fp);
}
