/*                               f i l e n o                               */
 
#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int fileno F1(FILE *, fp)

{
  return fp->__file;
}
