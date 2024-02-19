/*                                f o p e n                                */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *fopen F2(CONST char *, name, CONST char *, mode)

{
  register int fd;			/* opened file descriptor */
  short flags;				/* flag settings */

  return (fd = __fopen(name, mode, -1, &flags)) < 0
    ? (FILE *) (NULL)
    : __file((FILE *) NULL, fd, flags);
}
