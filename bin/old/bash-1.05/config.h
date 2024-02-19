/* config.h -- Configuration file for bash. */

#ifndef _CONFIG_
#define _CONFIG_

#ifndef VOID
#ifdef NO_VOID
#define VOID char
#else
#define VOID void
#endif
#endif

#ifdef __GNUC__
#define alloca __builtin_alloca
#endif

#if defined (Sun386i) || defined (sparc)
#ifndef __GNUC__
#include <alloca.h>
#endif
#endif

#if defined (hpux) || defined (UNIXPC) || defined (Xenix)
#define SYSV
#endif

/* Define NO_DUP2 if your OS doesn't have a dup2 () call. */
#ifdef hpux
#define NO_DUP2
#endif

/* Define JOB_CONTROL if your operating system supports
   BSD-like job control. */
#define JOB_CONTROL

/* Note that System V machines don't support job control. */
#if defined (SYSV)
#undef JOB_CONTROL
#endif /* SYSV */

/* Define ALIAS if you want the alias features. */
#define ALIAS

/* Define PUSHD_AND_POPD if you want those commands to be compiled in.
   (Also the `dirs' commands.) */
#define PUSHD_AND_POPD

/* Define READLINE to get the nifty/glitzy editing features.
   This is on by default.  You can turn it off interactively
   with the -nolineediting flag. */
#define READLINE

/* The default value of the PATH variable. */
#define DEFAULT_PATH_VALUE \
	":/usr/gnu/bin:/usr/local/bin:/usr/ucb:/bin:/etc:/usr/etc"

/* Define V9_ECHO if you want to give the echo builtin backslash-escape
   interpretation, in the style of the Bell Labs 9th Edition version of
   echo. */
#define V9_ECHO

/* Defined CONTINUE_AFTER_KILL_ERROR if you want the kill command to
   continue processing arguments after one of them fails. */
/* #define CONTINUE_AFTER_KILL_ERROR */

/* Define NO_WAIT_H if your system doesn't seem to have sys/wait.h.
   This is true for HPUX, ALTOS and XENIX. */
#if defined (hpux) || defined (ALTOS) || defined (Xenix)
#define NO_WAIT_H
#endif

/* Here are some machines that I know are little endian (lsb comes first). */
#if defined (i386) || defined (ATT3B) || defined (ATT386)
#define LITTLE_ENDIAN
#endif

/* Define BREAK_COMPLAINS if you want the incompatible, but useful
   error messages about `break' and `continue' out of context. */
#define BREAK_COMPLAINS

#endif  /* _CONFIG_ */
