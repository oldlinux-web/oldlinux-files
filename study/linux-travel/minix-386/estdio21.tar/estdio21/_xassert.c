/*                             _ x a s s e r t                             */

/* This code aborts the stdio package with an assertion failure
 * message. This is required since the standard assertion macro
 * may use stdio to print the error message.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

int __xassert F1(char *, file)

{
  static char *failure = "Assertion failed in stdio module ";

  (void) __iowrite(STDERR_FILENO, failure, strlen(failure));
  (void) __iowrite(STDERR_FILENO, file, strlen(file));
  (void) __iowrite(STDERR_FILENO, "\n", 1);
  abort();
  return 0;
}
