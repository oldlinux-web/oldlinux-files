/*                                f t e l l                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

long ftell F1(register FILE *, fp)

{
  off_t pos;				/* current location */

  if ((pos = lseek(fileno(fp), (off_t) 0, SEEK_CUR)) == ((off_t) -1) ||
      TESTFLAG(fp, _IONBF))
    return pos;

  if (TESTFLAG(fp, _IOWRITE))
    return ISCHECKWRITE(fp) ? pos : pos + BYTESINWRITEBUFFER(fp);
  else
    return pos - BYTESINREADBUFFER(fp);
}
