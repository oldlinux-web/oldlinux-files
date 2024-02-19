/*                                 p u t w                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

int putw F2(int, w, FILE *, fp)

{
  if (fwrite((void *) &w,
	     (__stdiosize_t) sizeof(int),
	     (__stdiosize_t) 1,
	     fp) != 1)
    w = EOF;
    
  return w;
}
