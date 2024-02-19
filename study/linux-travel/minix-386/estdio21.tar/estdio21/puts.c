/*                                 p u t s                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

int puts F1(CONST char *, s)

{
  char lbuf;				/* line buffered */
  int err;				/* error return */
  
  lbuf = 0;
  if (TESTFLAG(stdout, _IOLBF)) {
    lbuf = 1;
    CLEARFLAG(stdout, _IOLBF);
#if	_IOFBF != 0
    SETFLAG(stdout, _IOFBF);
#endif
    SETWRITEFULLBUFFERING(stdout);
  }

  err = fputs(s, stdout);

  if (lbuf) {
#if	_IOFBF != 0
    CLEARFLAG(stdout, _IOFBF);
#endif
    SETFLAG(stdout, _IOLBF);
    SETWRITELINEBUFFERING(stdout);
  }
  
  return err != 0 || NPUTC('\n', stdout) == EOF ? EOF : 0;
}
