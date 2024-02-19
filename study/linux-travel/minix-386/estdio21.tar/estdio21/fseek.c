/*                                f s e e k                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fseek F3(register FILE *, fp, long, offset, int, ptr)

{
  if (FFLUSH(fp) || lseek(fileno(fp), (off_t) offset, ptr) == ((off_t) -1))
    return EOF;

  CLEARFLAG(fp, _IOEOF);

  if (TESTFLAG(fp, _IORW)) {
    SETFLSBUF(fp, __bwrupdate);
    SETFLUSH(fp, __btfls);
    SETFILBUF(fp, __brdupdate);
    CLEARFLAG(fp, (_IOREAD | _IOWRITE));
    CHECKNEXTREAD(fp);
    CHECKNEXTWRITE(fp);
  }

  return 0;
}
