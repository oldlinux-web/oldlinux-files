/*                                f g e t s                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

char *fgets F3(register char *, buf, int, n, FILE *, fp)

{
  return n <= 0
         ? buf
	 : __fgetlx(buf, (__stdiosize_t) n, fp) != NULL ? buf : NULL;
}
