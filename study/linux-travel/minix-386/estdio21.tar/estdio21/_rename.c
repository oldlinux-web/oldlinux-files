/*                              _ r e n a m e                              */

/* The file named in the from part is given the name in the to
 * part. Return EOF if something goes wrong, otherwise return 0.
 *
 * This code is attributed to Freeman Pascal.
 *
 * rename() will not rename across file systems or file types.
 * If an attempt is made to copy across file systems both files
 * will be left intact and an error will be returned. This
 * is Posix conformant.
 *
 * Problems:
 *
 * o During the unlink(to), link(from, to) phase, to momentarily
 *   points nowhere in direct contravention of the Posix standard.
 *
 * o Directories aren't handled properly:
 *
 *	. link() and unlink() won't work on directories unless
 *	  you're superuser
 *	. some systems don't have mkdir() and rmdir() and
 *	. even if they did, there would still be the small
 *	  window of time in which the to link points nowhere.
 *
 * Patchlevel 2.2
 *
 * Edit History:
 * 08-Oct-1989	Do nothing if files are the same.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

#ifndef		RENAME

int rename F2(CONST char *, from, CONST char *, to)

{
  struct stat s_to, s_from;
  signal_t s_int, s_hup, s_quit;
  register int ret;
  
  if (stat(from, &s_from) != 0)
    return -1;
  else {
    if (stat(to, &s_to) == 0) {
      if (s_to.st_dev  == s_from.st_dev) {
	errno = EXDEV;
	return -1;
      }
      else if (s_to.st_ino == s_from.st_ino)
	return 0;
    }

    s_int  = signal(SIGINT,  SIG_IGN);
    s_hup  = signal(SIGHUP,  SIG_IGN);
    s_quit = signal(SIGQUIT, SIG_IGN);  

    ret = unlink(to);
    if ((ret = link(from, to)) == 0) 
      ret = unlink(from);

    (void) signal(SIGINT,  s_int);
    (void) signal(SIGHUP,  s_hup);
    (void) signal(SIGQUIT, s_quit);
    return ret;
  }
}

#endif
