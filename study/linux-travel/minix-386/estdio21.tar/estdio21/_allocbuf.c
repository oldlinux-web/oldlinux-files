/*                            _ a l l o c b u f                            */

/* Malloc a buffer for a stream. Unbuffered streams use the built
 * in buffers. Streams connected to terminals use small buffers.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

__stdiosize_t __allocbuf F1(register FILE *,fp)

{
  register int i;			/* pool scanner */

  if (! TESTFLAG(fp, _IONBF)) {
    fp->__bufsiz = BUFSIZ;

    if (isatty(fileno(fp))) {
      fp->__bufsiz = TTYBUFSIZ;
      if (TESTFLAG(fp, _IOWRITE))
	SETFLAG(fp, _IOLBF);
    }

#if	TTYBUFSIZ > BUFSIZ
  << TTYBUFSIZE overruns BUFSIZ for buffer pool >>
#endif

    for (i = 0; i < POOLSIZE; i++) {
      if (__iob[i] != NULL) {
	fp->__base = __iob[i];
	__iob[i]   = NULL;
	SETFLAG(fp, _IOPOOLBUF);
	goto Gotbuffer;
      }
    }

    if ((fp->__base = (__stdiobuf_t *) malloc(BUFSIZ)) != NULL)
      SETFLAG(fp, _IOMYBUF);
    else {
      CLEARFLAG(fp, (_IOFBF | _IOLBF));
      SETFLAG(fp, _IONBF);
    }
  }

  if (TESTFLAG(fp, _IONBF)) {
    fp->__base   = &fp->__buf;
    fp->__bufsiz = sizeof(fp->__buf);
  }

Gotbuffer:
  return fp->__bufsiz;
}
