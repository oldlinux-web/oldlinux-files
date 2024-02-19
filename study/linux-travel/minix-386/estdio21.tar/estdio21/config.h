/*			c o n f i g
 *
 * Describe system configuration
 *
 * This file describes the environment under which stdio
 * is to be compiled.
 *
 * The macros that are defined in this file are:
 *
 * Code control:
 *
 * DENORMAL
 *	Provide support for denormalised numbers in _f_cvt().
 * ERRLIST
 *	Use an internal form of sys_errlist for perror().
 * FLOAT
 *	Use float.h.
 * HIDDEN
 *	Hide standard names.
 * LARGEPOWERS
 *	Large power of ten vector found in file lpowers.h.
 * LMR
 *	LDBL_MAX / FLT_RADIX found in lmr.h.
 * MEMORY
 *	Use memcpy(), memset() and memchr(). If this is not defined,
 *	users must provide definitions for MEMCPY, MEMSET and MEMCHR.
 * MYMEMCPY
 *	Use stdio code for memcpy.
 * MYMEMCHR
 *	Use stdio code for memchr.
 * MYMEMSET
 *	Use stdio code for memset.
 * NOFLOAT
 *	No floating point support.
 * OPEN3
 *	Use open(2) for three argument opens.
 * RENAME
 *	Use rename(2).
 * STDARG
 *	Use stdarg.h instead or varargs.h.
 * LIMITS
 *	Use limits.h.
 * SMALLPOWERS
 *	Small power of ten vector found in file spowers.h.
 * STRING
 *	Use string.h.
 * SYSTYPES
 *	Use sys/types.h.
 * TIME
 *	Use time.h.
 * TRUNCATE
 *	Floating point to integer conversion truncates.
 * UNSIGNEDCHAR
 *	Use (int) ((unsigned char) (x)) for unsigned casts instead
 *	of (x) & ((1 << CHAR_BIT) - 1).
 * UNISTD
 *	Use unistd.h.
 * VOIDSIGNAL
 *	Signal returns void rather than int.
 *
 * Stdio performance control:
 *
 * FREADTHRESHOLD
 * FWRITETHRESHOLD
 *	Threshold beyond which fwrite or fread will use memcpy() to
 *	do the transfer instead of PUTC().
 * MEMSET(s,v,n)
 *	Set a piece of memory to the specified value.
 * MEMCPY(d,s,n)
 *	Copy a piece of memory of length n from s to d. No return
 *	value expected.
 * MEMCHR(s,c,n)
 *	Look in a piece of memory s of length n for character c. Return
 *	a pointer to the c if found, otherwise null.
 * TOLOWER(c)
 *	Convert uppercase to lowercase.
 */

/****************************************************************
 *			System Configuration			*
 ***************************************************************/

#ifndef		SOURCE

# define FWRITETHRESHOLD	10	/* fwrite memcpy call threshold */
# define FREADTHRESHOLD		10	/* fread memcpy call threshold */

/****************************************************************
 *			Site Configuration			*
 ****************************************************************/
# include "site.h"
/*efine DENORMAL*/			/* support denormalised numbers */
/*efine ERRLIST*/			/* use internal sys_errlist */
/*efine FLOAT*/				/* use float.h */
/*efine HIDDEN*/			/* hide standard names */
/*efine LARGEPOWERS*/			/* use lpowers.h */
/*efine MEMORY*/			/* use mem*() */
/*efine MYMEMCPY*/			/* use my memcpy */
/*efine MYMEMCHR*/			/* use my memchr */
/*efine MYMEMSET*/			/* use my memset */
/*efine OPEN3*/				/* use 3 argument opens */
/*efine RENAME*/			/* use rename(2) */
/*efine STDARG*/			/* use stdarg.h */
/*efine LIMITS*/			/* use limits.h */
/*efine SMALLPOWERS*/			/* use spowers.h */
/*efine STRING*/			/* use string.h */
/*efine SYSTYPES*/			/* use sys/types.h */
/*efine TIME*/				/* use time.h */
/*efine TRUNCATE*/			/* double to int truncates */
/*efine UNISTD*/			/* use unistd.h */
/*efine UNSIGNEDCHAR*/			/* use (unsigned char) cast */
/*efine VOIDSIGNAL*/			/* use void signals */

/*efine MEMCPY(a,b,c)*/			/* definition of memcpy */
/*efine MEMCHR(a,b,c)*/			/* definition of memchr */
/*efine MEMSET(a,b,c,d)*/		/* definition of memset */
/*efine TOLOWER tolower*/		/* use tolower */

/****************************************************************
 *			System Support Code			*
 ****************************************************************/
# ifdef		__STDC__
#   define P(x)	x
# else
#   define P(x) ()
# endif

# ifdef		MYMEMCPY
    char *memcpy P((char *, char *, size_t));
#   define MEMCPY(d,s,n) memcpy((d),(s),(n))
# endif
# ifdef		MYMEMCHR
    char *memchr P((char *, int, size_t));
#   define MEMCHR(s,c,n) memchr((s),(c),(n))
# endif
# ifdef		MYMEMSET
    void memset P((char *, int, size_t));
#   define MEMSET(d,c,n) memset((d),(c),(n))
# endif

# undef P
#endif

#ifdef		SOURCE
# ifdef		MYMEMCPY
char *memcpy F3(char *, d, char *, s, size_t, n)

{
  char *p = d;

  if (n != 0)
    do
      *d++ = *s++;
    while (--n);

  return p;
}
# endif

# ifdef		MYMEMCHR
char *memchr F3(char *, s, int, c, size_t, n)

{
  if (n != 0)
    for (; *s++ != c && --n; )
      ;
  return n != 0 ? s-1 : 0;
}
# endif

# ifdef		MYMEMSET
void memset F3(char *, d, int, c, size_t, n)

{
  if (n != 0)
    for (; *d++ = c, --n; )
      ;
}
# endif

#endif
