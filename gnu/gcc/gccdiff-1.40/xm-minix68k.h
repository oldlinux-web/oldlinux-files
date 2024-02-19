/* The following Macros control the compilation
 *
 *	CROSS_MINIX	defined when making cross compiler for MINIX
 *	NATIVE_MINIX	defined when making compiler for MINIX
 */

/* we use an extra file, as we don't do links */

/* generic m68k config file */

#include "xm-m68k.h"

/* exc that we don't want the funny alloca... */

#ifndef CROSS_MINIX
#undef alloca
#endif

/* #include "stddef.h" */


/* for new-version (format 2) symsegs, as defined by the symseg.h
   taken from gdb */
typedef long CORE_ADDR;

/* Oops.  Looks like this won't work anyhow; new GDB read syms 
   incrementally, so we can't use it.  Use GDB 2.6; old symseg fmt...
#define FORMAT_2_SYMSEG
...*/
