/*                             v f p r i n t f                             */

#include "stdiolib.h"

/*LINTLIBRARY*/

int vfprintf F3(FILE *, fp, CONST char *, fmt, VA_LIST, args)

{
  return __vfprintf(fp, fmt, args);
}
