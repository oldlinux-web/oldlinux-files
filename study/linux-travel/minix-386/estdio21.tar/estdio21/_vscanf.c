/*                              _ v s c a n f                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

int __vscanf F2(CONST char *, fmt, VA_LIST, args)

{
  return __vfscanf(stdin, fmt, args);
}
