/*                               s e t b u f                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

void setbuf F2(FILE *, fp, char *, buf)

{
  if (buf != NULL)
    (void) setvbuf(fp, buf, _IOFBF, BUFSIZ);
  else
    (void) setvbuf(fp, (char *) NULL, _IONBF, 0);
}
