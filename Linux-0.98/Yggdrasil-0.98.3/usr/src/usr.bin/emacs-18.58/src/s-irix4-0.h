#include "s-irix3-3.h"

#define USG5_3

/* Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used. */
#define HAVE_ALLOCA
#undef C_ALLOCA
#define alloca __builtin_alloca

/* use K&R C */
#define C_SWITCH_MACHINE -cckr

#ifdef 0 /* This has been suggested, but does not currently work..  */
/* No need to use sprintf to get the tty name--we get that from _getpty.  */
#define PTY_TTY_NAME_SPRINTF
/* No need to get the pty name at all.  */
#define PTY_NAME_SPRINTF
/* We need only try once to open a pty.  */
#define PTY_ITERATION
/* Here is how to do it.  */
/* ??? People say it is necessary to prevent SIGCHLD signals within _getpty.
   The right way is probably to block them.  Someone should try it.  */
#define PTY_OPEN						\
{								\
  char *name = _getpty (&fd, O_RDWR | O_NDELAY, 0600, 0);	\
  if (name == 0)						\
    return -1;							\
  if (fd < 0)							\
    return -1;							\
  if (fstat (fd, &stb) < 0)					\
    return -1;							\
  strcpy (pty_name, name);					\
}
#endif /* 0 */
