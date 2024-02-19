/*                                f r e a d                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

__stdiosize_t fread F4(void *,        ptr,
		       __stdiosize_t, size,
		       __stdiosize_t, nitems,
		       FILE *,        fp)

{
  __stdiosize_t red;			/* bytes read in read call */
  __stdiosize_t reed;			/* amount to read */
  __stdiobuf_t *p;			/* buffer pointer */
  __stdiobuf_t *q;			/* pointer into getc buffer */
  __stdiosize_t itemsleft;		/* whole or partial items left */
  __stdiosize_t nextover;		/* bytes left over next iteration */
  register __stdiosize_t leftover;	/* bytes left over this iteration */
  __stdiosize_t readitem;		/* whole items to read */
  register __stdiosize_t readsize;	/* amount of data to read */
  __stdiosize_t oversize;		/* readsize less leftover */
  int ch;				/* single character read */

  if (CHECKREAD(fp) || (itemsleft = nitems) == 0 || size == 0)
    return 0;

  if (GETFLAG(fp, _IOSTDX) == _IOSTDIN && __Zout != NULL &&
      (ALLFLAGS(fp) & ALLFLAGS(__Zout) & _IOLBF) != 0)
    (void) FFLUSH(__Zout);

/* Fix void * casting problems */
  p = (__stdiobuf_t *) ptr;

/* Try to avoid specialisation for unbuffered case */
  ASSERT(sizeof(fp->__buf) == 1);

/* Absorb as much data from the input buffer as possible */
  readsize = BYTESINREADBUFFER(fp);
  if (BUFFERSIZE(fp)/size >= itemsleft) {
    reed = size * itemsleft;
    readitem = itemsleft;
    nextover = 0;
  }
  else {
    reed = readsize;
    readitem = readsize/size;
    nextover = (leftover = readsize%size) != 0 ? size - leftover : 0;
  }

  while (reed != 0) {
    if (readsize == 0) {
      if ((ch = getc(fp)) == EOF)
	return (readitem*size + nextover - reed)/size;
      *p++ = ch;
      reed--;
      readsize = BYTESINREADBUFFER(fp);
    }
    else {
      if (readsize > reed)
	readsize = reed;
      reed -= readsize;

      q = GETREADPTR(fp);
      if (readsize > FREADTHRESHOLD) {
	MEMCPY(p, q, (size_t) readsize);
	p += readsize;
	q += readsize;
      }
      else {
	do
	  *p++ = *q++;
	while (--readsize);
      }
      SETREADPTR(fp, q);
    }
  }

/* The rest of the data comes from the file directly */
  while ((itemsleft -= readitem) != 0) {
    leftover = nextover;
    readsize = UINT_MAX;
    readitem = 0;

/* Very large objects will require several large reads */
    if (readsize < leftover)
      nextover = leftover - readsize;

/* Account for end of last object and start of new ones */
    else {
      if (leftover != 0)
	readitem = 1;
      oversize = readsize - leftover;
      if ((readitem += oversize/size) < itemsleft)
	nextover = size - oversize%size;
      else {
	readitem = itemsleft;
	readsize = leftover + itemsleft*size;
	if (leftover != 0)
	  readsize -= size;
      }
    }

    for (reed = readsize;
         reed != 0 && (red = __ioread(fileno(fp), (char *) p, reed)) != 0;
	 p += red, reed -= red)
      ;

    if (reed != 0) {
      if ((readsize -= reed) >= leftover) {
	itemsleft -= (readsize-leftover)/size;
	if (leftover != 0)
	  itemsleft--;
      }
      break;
    }
  }

  return nitems-itemsleft;
}
