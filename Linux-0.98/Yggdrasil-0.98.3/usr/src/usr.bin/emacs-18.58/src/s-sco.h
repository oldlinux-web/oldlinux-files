#include "s-usg5-3.h"

/* Use termios instead of termio, to turn off C-z.  */

#define HAVE_TCATTR

/* It's possible for Emcs to suspend itself.  */

#undef NOMULTIPLEJOBS

/* The setsid system call exists.  */

#define HAVE_SETSID

/* The rename system call exists.  */

#define HAVE_RENAME

/* Include ptem.h, not sioctl.h.  */

#define NO_SIOCTL_H
#define NEED_PTEM_H

/* Inhibit macro definition of `signal' in m-intel386.h.  */

#define DONT_DEFINE_SIGNAL

/* We cannot get alloca from -lPW because various other symbols
   in that library conflict with symbols in GCC.  */

#ifdef	__GNUC__
#define alloca(x) __builtin_alloca (x)
#else
#define C_ALLOCA
#undef HAVE_ALLOCA
#define STACK_DIRECTION	-1
#endif /* __GNUC__ */

/* TIOCWINSZ doesn't work on ptys.  */
#define BROKEN_TIOCGWINSZ
