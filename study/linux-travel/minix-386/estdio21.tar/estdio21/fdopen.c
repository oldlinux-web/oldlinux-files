/*                               f d o p e n                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *fdopen F2(register int, fd, CONST char *, mode)

{
  short flags;				/* flag settings */
  static int initfp;			/* initialiser flag */

  if (initfp == 0) {
    initfp = 1;
    __Zout = stdout;
    __Zerr = stderr;
    if (TESTFLAG(stdin, _IOINSERT)) {
      CLEARFLAG(stdin, _IOINSERT);
      FINSERT(stdin);
    }
    if (TESTFLAG(stdout, _IOINSERT)) {
      CLEARFLAG(stdout, _IOINSERT);
      FINSERT(stdout);
      if (isatty(fileno(stdout))) {
#if	_IOFBF != 0
	CLEARFLAG(stdout, _IOFBF);
#endif
	SETFLAG(stdout, _IOLBF);
      }
    }
    if (TESTFLAG(stderr, _IOINSERT)) {
      CLEARFLAG(stderr, _IOINSERT);
      FINSERT(stderr);
    }
  }

  return fd != __fopen((char *) NULL, mode, fd, &flags)
	 ? (FILE *) (NULL)
         : __file((FILE *) NULL, fd, flags);
}
