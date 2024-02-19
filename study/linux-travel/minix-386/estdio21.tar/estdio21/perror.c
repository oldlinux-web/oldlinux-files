/*                               p e r r o r                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

void perror F1(register CONST char *, s)

{
#define MAXDIGS	11			/* maximum digits in unknown */
  static char unknown[] = {'U','n','k','n','o','w','n',' ',
		           'e','r','r','o','r',' ',
		           '0','0','0','0','0','0','0','0','0','0','0',0};
  int errnum;				/* error number */
  char *errdesc;			/* error description */

  errnum = errno;

  if (__Zerr != NULL)
    (void) FFLUSH(__Zerr);

  if (s != NULL && *s != 0) {
    (void) __iowrite(STDERR_FILENO, (char *) s, strlen(s));
    (void) __iowrite(STDERR_FILENO, ": ", 2);
  }
  if (errnum > 0 && errnum < sys_nerr)
    errdesc = sys_errlist[errnum];
  else {
    errdesc = &unknown[sizeof(unknown)-MAXDIGS-1];
    if (errnum < 0) {
      *errdesc++ = '-';
      errnum = -errnum;
    }
    *__utoa(errdesc, (unsigned) errnum, 1) = 0;
    errdesc = &unknown[0];
  }
  (void) __iowrite(STDERR_FILENO, errdesc, strlen(errdesc));
  (void) __iowrite(STDERR_FILENO, "\n", 1);
}
