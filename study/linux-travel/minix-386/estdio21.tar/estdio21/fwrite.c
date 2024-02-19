/*                               f w r i t e                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

__stdiosize_t fwrite F4(void *,        ptr,
			__stdiosize_t, size,
			__stdiosize_t, nitems,
			FILE *,        fp)

{
  __stdiosize_t wrote;			/* bytes written in write call */
  __stdiobuf_t *p;			/* buffer pointer */
  __stdiobuf_t *q;			/* output buffer pointer */
  __stdiobuf_t *pbuf;			/* line buffered flush pointer */
  __stdiosize_t itemsleft;		/* whole or partial items left */
  register __stdiosize_t leftover;	/* bytes left over this iteration */
  __stdiosize_t nextover;		/* bytes left over next iteration */
  register __stdiosize_t writesize;	/* size of write */
  __stdiosize_t writeitem;		/* whole items written */
  __stdiosize_t oversize;		/* writesize less leftover */
  __stdiosize_t writebytes;		/* total bytes to write (small) */
  __stdiosize_t bytesleft;		/* bytes left to write (small) */
  __stdiosize_t byteswrote;		/* bytes written (small) */
  __stdiosize_t bytescan;		/* bytes to scan (small) */

/* Items left to write */
  if (CHECKWRITE(fp) || (itemsleft = nitems) == 0 || size == 0)
    return 0;

/* Fix void * casting problems */
  p = (__stdiobuf_t *) ptr;

/* Small amounts go via putc */
  if (! TESTFLAG(fp, _IONBF) &&
      (UNUSEDINWRITEBUFFER(fp)+BUFFERSIZE(fp))/size > itemsleft) {
    pbuf       = GETWRITEPTR(fp);
    writebytes = bytesleft = itemsleft*size;
    do {
      if ((writesize = UNUSEDINWRITEBUFFER(fp)) > bytesleft)
	writesize = bytesleft;

      if (writesize == 0) {
	if (FFLUSH(fp))
	  break;
      }
      else {
	bytesleft -= writesize;
	q = GETWRITEPTR(fp);
	if (writesize > FWRITETHRESHOLD) {
	  MEMCPY(q, p, (size_t) writesize);
	  p += writesize;
	  q += writesize;
	}
	else {
	  do
	    *q++ = *p++;
	  while (--writesize);
	}
	SETWRITEPTR(fp, q);
      }
    } while (bytesleft != 0);

    byteswrote = writebytes - bytesleft;

    if (TESTFLAG(fp, _IOLBF)) {
      bytescan = byteswrote;
      if (pbuf + bytescan != GETWRITEPTR(fp)) {
	pbuf     = fp->__base;
	bytescan = BYTESINWRITEBUFFER(fp);
      }
      if (MEMCHR(pbuf, '\n', (size_t) bytescan) != NULL)
        (void) FFLUSH(fp);
    }

    return byteswrote/size;
  }

/* Large amounts get fed directly to the file */
  if (BYTESINWRITEBUFFER(fp) != 0 && FFLUSH(fp))
    return 0;

/* The rest of the data goes to the file directly */
  for (nextover=writeitem=0; (itemsleft -= writeitem) != 0; p += writesize) {
    leftover  = nextover;
    writesize = ~((__stdiosize_t) 0);
    writeitem = 0;

/* Very large objects will require several large writes */
    if (writesize < leftover)
      nextover = leftover - writesize;

/* Account for end of last object and start of new ones */
    else {
      if (leftover != 0)
	writeitem = 1;
      oversize = writesize - leftover;
      if ((writeitem += oversize/size) < itemsleft)
	nextover = size - oversize%size;
      else {
	writeitem = itemsleft;
	writesize = leftover + itemsleft*size;
	if (leftover != 0)
	  writesize -= size;
      }
    }

    wrote = __iowrite(fileno(fp), (char *) p, writesize);

    if (wrote != writesize) {
      if (wrote >= leftover) {
	itemsleft -= (wrote-leftover)/size;
	if (leftover != 0)
	  itemsleft--;
      }
      break;
    }
  }
  return nitems-itemsleft;
}
