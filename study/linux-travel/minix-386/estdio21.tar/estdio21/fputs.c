/*                                f p u t s                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

#define WRITEAHEAD	4		/* write-ahead factor */

int fputs F2(CONST char *, s, FILE *, fp)

{
  register __stdiobuf_t *wp;		/* output buffer pointer */
  register __stdiobuf_t *p;		/* buffer pointer */
  register __stdiobuf_t *we;		/* end of output buffer */
  __stdiosize_t len;			/* length of string */

  if (CHECKWRITE(fp))
    return 0;

/* Fix char casting problems */
  p = (__stdiobuf_t *) s;
  
/* Dump directly into unbuffered streams */
  if (TESTFLAG(fp, _IONBF)) {
    len = strlen(p);
    return __iowrite(fileno(fp), p, len) != len ? EOF : 0;
  }

  for (;;) {
    wp = GETWRITEPTR(fp);
    we = GETWRITELIMIT(fp);

/* A fairly empty buffer -- must match WRITEAHEAD */
    if (BUFFERSIZE(fp) >= WRITEAHEAD) {
      we -= WRITEAHEAD;
      while (wp < we) {
#if WRITEAHEAD != 4
  << Writeahead factor assumed to be 4 >>
#else
	if ((wp[0] = p[0]) == 0) goto Done0;
	if ((wp[1] = p[1]) == 0) goto Done1;
	if ((wp[2] = p[2]) == 0) goto Done2;
	if ((wp[3] = p[3]) == 0) goto Done3;
#endif
	wp += WRITEAHEAD;
	p  += WRITEAHEAD;
      }
      we += WRITEAHEAD;
    }

/* Near end of buffer */
    for (; wp < we; wp++) {
      if ((*wp = *p++) == 0)
	goto Done0;
    }
    SETWRITEPTR(fp, wp);
    (void) FFLUSH(fp);
  }
  
Done3: wp += 3; goto Done0;
Done2: wp += 2; goto Done0;
Done1: wp += 1;
Done0:

  SETWRITEPTR(fp, wp);
  if (TESTFLAG(fp, _IOLBF) && strchr(s, '\n') != NULL)
    (void) FFLUSH(fp);
  return ferror(fp) ? EOF : 0;
}
