/* Configuration for GCC for Intel i386 running Linux.
 *
 * Written by H.J. Lu (hlu@eecs.wsu.edu)
 *
 * Linux is a POSIX.1 compatible UNIX clone for i386, which uses GNU
 * stuffs as the native stuffs.
 */

#include "xm-i386.h"
#include "xm-svr3.h"

/* I don't know how to do this. I am using a cross-compiler to compile
 * Linux's compiler. I need host rtl.o .....
 */
#ifdef linux

#undef BSTRING
#define BSTRING
#undef bcmp
#undef bcopy
#undef bzero
#undef index
#undef rindex
#define HAVE_PUTENV

#include <stdlib.h>		/* for malloc definition */
#undef malloc(n)
#define malloc(n)	malloc ((n) ? (n) : 1)
#undef calloc(n,e)
#define calloc(n,e)	calloc (((n) ? (n) : 1), ((e) ? (e) : 1))

#endif
