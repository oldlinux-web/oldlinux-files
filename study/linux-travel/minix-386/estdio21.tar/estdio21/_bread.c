/*                               _ b r e a d                               */

/* This set of routines replaces the original __filbuf() function. The
 * FILE structure vectors the fill call to the appropriate routine.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

/* Fill read buffer
 *
 * __brd is the main function that fills the output buffer
 * then reads a character.
 */
static int __brd F1(register FILE *, fp)

{
  register int bytes;			/* bytes read */

  bytes = 0;

  if (GETFLAG(fp, (_IOREAD | _IOERR | _IOEOF)) == _IOREAD) {

/* Flush related output stream */
    if (GETFLAG(fp, _IOSTDX) == _IOSTDIN && __Zout != NULL &&
	(ALLFLAGS(fp) & ALLFLAGS(__Zout) & _IOLBF) != 0)
      (void) FFLUSH(__Zout);

/* Read data into the buffer */
    bytes = __ioread(fileno(fp), (char *) fp->__base,
		     TESTFLAG(fp, _IONBF) ? 1 : fp->__bufsiz);

    if (bytes <= 0)  {
      SETFLAG(fp, bytes ? (_IOERR|_IOEOF) : _IOEOF);
      bytes = 0;
    }
    else if (TESTFLAG(fp, _IOLBF) &&
	     __Zrlbf != (__stdiosize_t (*) P((FILE *, __stdiosize_t))) NULL)
      bytes = (*__Zrlbf)(fp, bytes);

    INITREADBUFFER(fp, bytes);
  }

  return bytes ? FGETC(fp) : EOF;
}

/* Flush a read buffer
 *
 * __brdflush is a function that flushes the input buffer.
 */
static int __brdflush F1(register FILE *, fp)

{
  if (! TESTFLAG(fp, _IOERR)) {
    if (! TESTFLAG(fp, _IOEOF) &&
        lseek(fileno(fp), (long) (-BYTESINREADBUFFER(fp)), SEEK_CUR) == -1L)
      SETFLAG(fp, _IOERR);
    INITREADBUFFER(fp, 0);
    return 0;
  }
  return EOF;
}

/* Check and initialise a readonly stream
 *
 * __brdonly is intended for read only streams. It will check
 * that the stream has a buffer. The fill function will then
 * be directed to __brd.
 */
int __brdonly F1(register FILE *, fp)

{
  __stdiobuf_t *p;			/* pointer to end */

  p = fp->__rend;

  if (TESTFLAG(fp, _IOINSERT)) {
    CLEARFLAG(fp, _IOINSERT);
    FINSERT(fp);
  }

  INITREADBUFFER(fp, 0);
  SETFILBUF(fp, __brd);
  SETFLUSH(fp, __brdflush);

  return p ? 0 : __brd(fp);
}
