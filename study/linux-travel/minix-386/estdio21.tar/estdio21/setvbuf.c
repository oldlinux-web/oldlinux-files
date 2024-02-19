/*                              s e t v b u f                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

int setvbuf F4(register FILE *, fp,
               char *,          buf,
	       register int,    type,
	       __stdiosize_t,   size)

{
  char mybuf;				/* buffer allocated by stdio */

  if (GETREADPTR(fp) != NULL || GETWRITEPTR(fp) != NULL)
    return EOF;

  if (TESTFLAG(fp, _IOINSERT)) {
    CLEARFLAG(fp, _IOINSERT);
    FINSERT(fp);
  }

  if (type == _IOFBF || type == _IOLBF) {
    if (size == 0)
      return EOF;

    mybuf = 0;
    if (buf == NULL) {
      if ((buf = (char *) malloc(size)) == NULL)
        return EOF;
      mybuf = 1;
    }

    __freebuf(fp);
    fp->__base   = (__stdiobuf_t *) buf;
    fp->__bufsiz = size;

    if (mybuf)
      SETFLAG(fp, _IOMYBUF);

    if (type == _IOLBF)
      __Zrlbf = __rlbf;
  }
  else if (type == _IONBF) {
    __freebuf(fp);
    fp->__base   = &fp->__buf;
    fp->__bufsiz = sizeof(fp->__buf);
  }
  else
    return EOF;

  CLEARFLAG(fp, (_IOFBF | _IONBF | _IOLBF));
  SETFLAG(fp, type);
  return 0;
}
