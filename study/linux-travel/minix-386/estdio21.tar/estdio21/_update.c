/*                              _ u p d a t e                              */

/* This file contains the additional buffering functions required
 * to implement the update modes of access (r+, w+ and a+).
 *
 * The functions perform adjustments to the FILE descriptor before
 * passing control to the appropriate rdonly or wronly function.
 *
 * The functions are placed here so that they will not be linked
 * in to a basic stdio application unless they are absolutely
 * necessary (specifically if __fopen() or fseek() are used).
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

/* Check and initialise a read update stream
 *
 * __brdupdate is intended for read/write streams. It will check
 * that the stream has a buffer and will cope with direction
 * changes. The fill function will then be directed to __brd.
 */
int __brdupdate F1(register FILE *, fp)

{
  if (TESTFLAG(fp, (_IOWRITE | _IOEOF | _IOERR)))
    return EOF;

  SETFLAG(fp, _IOREAD);

  return __brdonly(fp);
}

/* Check and initialise a write update stream.
 *
 * __bwrupdate is intended for read/write streams. It will check
 * that the stream has a buffer and will cope with direction
 * changes. The flush function will then be directed to __bwr.
 */
int __bwrupdate F2(int, c, register FILE *, fp)

{
  if (GETFLAG(fp, (_IOREAD | _IOEOF)) == _IOREAD || TESTFLAG(fp, _IOERR))
    return EOF;

  SETFLAG(fp, _IOWRITE);
  CLEARFLAG(fp, _IOREAD);

  return __bwronly(c, fp);
}
