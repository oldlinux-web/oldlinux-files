/*                              t m p f i l e                              */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *tmpfile F0()

{
  char *name;				/* name of file */
  register int i;			/* retry count */
  register FILE *fp;			/* temporary file */

  for (i = TMP_MAX;
       i-- && (fp = fopen(name = tmpnam((char *) 0), "w+")) == NULL;
      )
    ;
  if (fp != NULL)
    remove(name);
  return fp;
}
