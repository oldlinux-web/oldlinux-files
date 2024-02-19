/*                                _ r l b f                                */

/* This function provides line buffered read support. When reading
 * from a line buffered stream, it is necessary to discard any
 * readahead. This function is dynamically linked to __Zrlbf by
 * setvbuf to minimise code size when line buffered input is not
 * required.
 *
 * This function scans the input buffer and returns a shortened
 * buffer size. If no \n is found, the buffer size is returned.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

__stdiosize_t __rlbf F2(register FILE *, fp, register __stdiosize_t, bytes)

{
  __stdiobuf_t *nlp;			/* \n pointer */
  int offset;				/* offset back to one past \n */
  
  return ((nlp = (__stdiobuf_t *) MEMCHR(fp->__base, '\n', (size_t) bytes))
						      != NULL &&
          (offset = nlp - (fp->__base + bytes) + 1)   != 0    &&
          lseek(fileno(fp), (off_t) offset, SEEK_CUR) != -1L)
         ? bytes + offset : bytes;
}
