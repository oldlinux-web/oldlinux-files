/*				_ f i l e
 *
 * Allocate a stream descriptor
 *
 * Allocate and initialise a FILE structure. If the pointer passed
 * to the function is NULL, a FILE will be allocated, otherwise
 * the one specified will be used. The function will return
 * a pointer to the FILE structure, or NULL if it fails.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

#if	_IOREAD != 1 || _IOWRITE != 2
	_IOREAD == 1 and _IOWRITE == 2 assumed
#endif

static int (*filtable[]) P((FILE *)) =
  {__brdupdate, __brdonly, __bffil};

static int (*flstable[]) P((int, FILE *)) =
  {__bwrupdate, __bffls, __bwronly};

FILE *__file F3(register FILE *, fp, int, fd, register short, flags)

{

/* Retain flags and insert if necessary */
  if (fp != NULL) {
    if (TESTFLAG(fp, _IOINSERT)) {
      CLEARFLAG(fp, _IOINSERT);
      FINSERT(fp);
    }
    flags |= GETFLAG(fp, _IORETAIN);
  }

/* Allocate or find a file structure */
  else {
    for (fp = __iop; fp != NULL && TESTFLAG(fp, ~_IORETAIN); fp = fp->__next) {
      if (fileno(fp) == fd) {
/* Multiple fdopen() on a file descriptor is debatable */
/* Error return here breaks too many broken programs!  */
/*	return NULL; */
	__freebuf(fp);
	break;
      }
    }

/* Retain flags on existing descriptors */
    if (fp != NULL)
      flags |= GETFLAG(fp, _IORETAIN);

/* No existing descriptor */
    else {
      if ((fp = (FILE *) malloc(sizeof(*fp))) == NULL)
	return NULL;

      FINSERT(fp);
    }
  }

/* Stream descriptor needs to be initialised */
  fp->__rend   = NULL;
  fp->__rptr   = NULL;
  fp->__wend   = NULL;
  fp->__wptr   = NULL;
  fp->__base   = NULL;
  fp->__flag   = flags;
  fp->__file   = fd;
  fp->__bufsiz = __allocbuf(fp);

  flags       &= _IOREAD | _IOWRITE;
  fp->__filbuf = filtable[flags];
  fp->__flsbuf = flstable[flags];
  fp->__flush  = __btfls;

  return fp;
}
