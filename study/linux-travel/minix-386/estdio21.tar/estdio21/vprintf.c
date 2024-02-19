/*                              v p r i n t f                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

int vprintf F2(CONST char *, fmt, VA_LIST, args)

{
  return __vfprintf(stdout, fmt, args);
}
