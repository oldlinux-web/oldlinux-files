/*                              _ b w r i t e                              */

/* This set of routines replaces the original __flsbuf() function. The
 * FILE structure vectors the flush call to the appropriate routine.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

/* Flush all streams
 *
 * Flushes all output buffers. This ensures that all streams
 * are written out and closed. It's main purpose is to ensure
 * that all streams are flushed on exit. exit() knows about
 * the existence of __ioflush.
 */
void __ioflush F0()

{
  register FILE *fp;			/* pointer to current descriptor */

  for (fp = __iop; fp != NULL; fp = fp->__next) {
    (void) FFLUSH(fp);
    (void) close(fileno(fp));
  }
}

/* Flush a write buffer and write a character
 *
 * __bwr is the main function that flushes the output buffer
 * then writes a character.
 */
static int __bwr F2(register int, c, register FILE *, fp)

{
  if (GETFLAG(fp, (_IOWRITE | _IOERR)) == _IOWRITE) {
    for (;;) {
      if (UNUSEDINWRITEBUFFER(fp) > 0) {
	if (((FPUTC(c, fp) != '\n'   || ! TESTFLAG(fp, _IOLBF)) &&
	     ! TESTFLAG(fp, _IONBF)) || ! FFLUSH(fp))
	  return UCHAR(c);
	break;
      }
      if (FFLUSH(fp))
	break;
    }
  }
  return EOF;
}

/* Flush a write buffer
 *
 * __bwrflush is a function that flushes the output buffer.
 */
static int __bwrflush F1(register FILE *, fp)

{
  register __stdiosize_t length;		/* bytes to write */

  if (! TESTFLAG(fp, _IOERR)
#ifndef		OPEN3
      && (! TESTFLAG(fp, _IOAPPEND) || lseek(fileno(fp), 0L, SEEK_END) != -1L)
#endif
     ) {
    length = BYTESINWRITEBUFFER(fp);
    INITWRITEBUFFER(fp);
    if (__iowrite(fileno(fp), (char *) fp->__base, length) == length)
      return 0;
  }
  SETFLAG(fp, _IOERR);
  return EOF;
}

/* Check and initialise a writeonly stream
 *
 * __bwronly is intended for write only streams. It will check
 * that the stream has a buffer. The flush function will then
 * be directed to __bwr.
 */
int __bwronly F2(int, c, register FILE *, fp)

{
  __stdiobuf_t *p;			/* pointer to end */
  int streamtype;			/* type of stream */

  p = fp->__wend;

  __Zwrapup = __ioflush;

  if ((streamtype = GETFLAG(fp, _IOSTDX)) == _IOSTDOUT)
    __Zout = fp;
  else if (streamtype == _IOSTDERR)
    __Zerr = fp;

  if (TESTFLAG(fp, _IOINSERT)) {
    CLEARFLAG(fp, _IOINSERT);
    FINSERT(fp);
    if (__Zout == fp && isatty(fileno(fp))) {
#if	_IOFBF != 0
      CLEARFLAG(fp, _IOFBF);
#endif
      SETFLAG(fp, _IOLBF);
    }
  }

  INITWRITEBUFFER(fp);
  SETFLSBUF(fp, __bwr);
  SETFLUSH(fp, __bwrflush);

  return p ? 0 : __bwr(c, fp);
}
