/*                                 g e t w                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

int getw F1(FILE *, fp)

{
  int w;				/* word read */

  if (fread((void *) &w,
	    (__stdiosize_t) sizeof(int),
	    (__stdiosize_t) 1,
	    fp) != 1)
    w = EOF;

  return w;
}
