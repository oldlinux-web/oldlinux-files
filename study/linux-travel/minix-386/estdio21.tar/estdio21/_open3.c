/*                               _ o p e n 3                               */

/* For those ancient systems without three argument opens, we fake it.
 * The following code works for:
 *
 *		r  O_RDONLY
 *		w  O_WRONLY | O_CREAT | O_TRUNC
 *		a  O_WRONLY | O_CREAT | O_APPEND
 *		r+ O_RDWR
 *		w+ O_RDWR   | O_CREAT | O_TRUNC
 *		a+ O_RDWR   | O_CREAT | O_APPEND
 *	other modes may not work
 *
 * Code based on open3.c from Bruce Evan's stdio. It is necessary to
 * set the umask to zero so that creat works implies that open works.
 * The O_APPEND mode is ignored since two argument opens don't support
 * it at all.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*VARARGS2*/
/*ARGSUSED*/

#ifndef		OPEN3

#undef open

int __open3 F2V(CONST char *, path, int, mode,

{
  VA_LIST, arg,				/* argument vector */
  register int fd;			/* file descriptor */
  register int fmode;			/* file mode */
  int mask;				/* saved umask */
  int perms;				/* permissions */

  VA_START(arg, mode);)

/* Take fast local copy of mode */
  fmode = mode;

/* r, a, r+ and a+ modes */
  if ((fmode & O_TRUNC) == 0) {
    if ((fd = open(path, fmode & (O_WRONLY|O_RDONLY|O_RDWR))) >= 0 ||
	(fmode & O_CREAT) == 0                                     ||
	errno != ENOENT)
      goto Done;

/* Fall through on a and a+ modes => couldn't open because ENOENT */
  }

/* Extract file permissions */
  perms = VA_ARG(arg, int);

/* w and a modes */
  if ((fmode & (O_WRONLY|O_RDONLY|O_RDWR)) != O_RDWR)
      fd = creat(path, perms);

/* w+ and a+ modes */
  else {
      mask = umask(0);
      if ((fd = creat(path, perms)) >= 0) {
	  (void) close(fd);
	  fd = open(path, O_RDWR);
	  (void) chmod(path, ~mask & perms);
      }
      (void) umask(mask);
  }

Done:
  VA_END(arg);
  return fd;
}
#endif
