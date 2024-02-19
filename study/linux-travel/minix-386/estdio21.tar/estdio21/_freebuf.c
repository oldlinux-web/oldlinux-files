/*			_ f r e e b u f
 *
 * Deallocate a buffer to a stream
 *
 * Return the allocated buffer to the free pool or to the heap. Return
 * zero on success, non-zero on failure.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

void __freebuf F1(register FILE *,fp)

{
  int i;				/* pool scanner */

  if (TESTFLAG(fp, _IOMYBUF))
    free(fp->__base);

  else if (TESTFLAG(fp, _IOPOOLBUF)) {
    for (i = 0; i < POOLSIZE; i++) {
      if (__iob[i] == NULL) {
	__iob[i] = fp->__base;
	break;
      }
    }
    ASSERT(i < POOLSIZE);
  }

  CLEARFLAG(fp, (_IOMYBUF | _IOPOOLBUF));
}
