/*                              g e t c h a r                              */
 
#include "stdiolib.h"

#define MACRO_SHADOW
#include "hidden.h"

/*LINTLIBRARY*/

int getchar F0()

{
  return stdin->__rptr >= stdin->__rend
	 ? (*stdin->__filbuf)(stdin)
	 : UCHAR(*stdin->__rptr++);
}
