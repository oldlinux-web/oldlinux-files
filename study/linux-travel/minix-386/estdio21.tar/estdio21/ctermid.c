/*                              c t e r m i d                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

char *ctermid F1(register char *, s)

{
  static char ttyname[L_ctermid] = "/dev/tty";

  return s != NULL ? strcpy(s, ttyname) : ttyname;
}
