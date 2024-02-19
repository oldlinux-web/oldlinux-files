/*                               f c l o s e                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

int fclose F1(register FILE *, fp)

{
  register FILE **sp;			/* stream chain */
  FILE *p;				/* stream chain */

/* Stream descriptor yet to be inserted */
  if (TESTFLAG(fp, _IOINSERT)) {
    CLEARFLAG(fp, _IOINSERT);
    FINSERT(fp);
  }

/* Find the file descriptor */
  for (sp = &__iop; (p = *sp) != NULL && p != fp; sp = &p->__next)
    ;

/* Shutdown */
  if (p == NULL || (fileno(fp) >= 0 && (FFLUSH(fp) || close(fileno(fp)))))
    return EOF;

/* Free allocated buffer */
  __freebuf(fp);

/* Unlink from list or indicate that it's unused */
  if (TESTFLAG(fp, _IOSTDX))
    fp->__flag &= _IORETAIN;
  else {
    *sp = fp->__next;
    free((void *) fp);
  }

  return 0;
}
