/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.10 GENERAL UTILITIES	<stdlib.h>
 */

#ifndef	_STDLIB_H

#define	_STDLIB_H	1
#include <features.h>

/* Get size_t, wchar_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_wchar_t
#define	__need_NULL
#include <stddef.h>

#define	__need_Emath
#include <errno.h>

/* Get HUGE_VAL (returned by strtod on overflow) from <float.h>.  */
#define	__need_HUGE_VAL
#include <float.h>

/* C++ wants it here. */
#ifdef __cplusplus
#include <getopt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Returned by `div'.  */
typedef struct
  {
    int quot;	/* Quotient.  */
    int rem;	/* Remainder.  */
  } div_t;

/* Returned by `ldiv'.  */
typedef struct
  {
    long quot;	/* Quotient.  */
    long rem;	/* Remainder.  */
  } ldiv_t;


/* The largest number rand will return (same as INT_MAX).  */
#ifndef RAND_MAX
#define	RAND_MAX	2147483647
#endif


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */


/* Maximum length of a multibyte character in the current locale.
   This is just one until the fancy locale support is finished.  */
#define	MB_CUR_MAX	1


/* Convert a string to a floating-point number.  */
extern double atof(const char *__nptr);
/* Convert a string to an integer.  */
extern int atoi(const char *__nptr);
/* Convert a string to a long integer.  */
extern long int atol(const char *__nptr);

/* Convert a string to a floating-point number.  */
extern double strtod(const char *__nptr, char **__endptr);
/* Convert a string to a long integer.  */
extern long int strtol(const char *__nptr, char **__endptr,
			       int __base);
/* Convert a string to an unsigned long integer.  */
extern unsigned long int strtoul(const char *__nptr,
					 char **__endptr, int __base);

#ifdef	__OPTIMIZE__
#define	atof(nptr)	strtod((nptr), (char **) NULL)
#define	atoi(nptr)	((int) atol(nptr))
#define	atol(nptr)	strtol((nptr), (char **) NULL, 10)
#endif	/* Optimizing.  */


/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand(void);
/* Seed the random number generator with the given number.  */
extern void srand(unsigned int __seed);

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and RAND_MAX inclusive.  */
extern long int __random(void);
/* Seed the random number generator with the given number.  */
extern void __srandom(unsigned int __seed);

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern void * __initstate(unsigned int __seed, void * __statebuf,
			       size_t __statelen);
/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern void * __setstate(void * __statebuf);

#ifdef	__USE_BSD
extern long int random(void);
extern void srandom(unsigned int __seed);
extern void * initstate(unsigned int __seed, void * __statebuf,
			     size_t __statelen);
extern void * setstate(void * __statebuf);

#ifdef	__OPTIMIZE__
#define	random()		__random()
#define	srandom(seed)		__srandom(seed)
#define	initstate(s, b, n)	__initstate((s), (b), (n))
#define	setstate(state)		__setstate(state)
#endif	/* Optimizing.  */
#endif	/* Use BSD.  */

#ifdef	__OPTIMIZE__
#define	rand()		((int) __random())
#define	srand(seed)	__srandom(seed)
#endif	/* Optimizing.  */


/* Allocate SIZE bytes of memory.  */
extern void * malloc(size_t __size);
/* Re-allocate the previously allocated block
   in void *, making the new block SIZE bytes long.  */
extern void * realloc(void * __ptr, size_t __size);
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern void * calloc(size_t __nmemb, size_t __size);
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free(void * __ptr);	

#ifdef	__USE_MISC
/* Free a block.  An alias for `free'.	(Sun Unices).  */
extern void cfree(void * __ptr);

#ifdef	__OPTIMIZE__
#define	cfree(ptr)	free(ptr)
#endif	/* Optimizing.  */
#endif	/* Use misc.  */

/* fix malloc (0) only if you include stdlib.h */
#ifndef NO_FIX_MALLOC
static __inline__ void* __gnu_calloc (size_t __nmemb, size_t __n)
{
  return calloc (__nmemb ? __nmemb : 1, __n ? __n : 1);
}

static __inline__ void* __gnu_malloc (size_t __n)
{
  return malloc (__n ? __n : 1);
}

#define calloc(nmemb,n)	__gnu_calloc((nmemb),(n))
#define malloc(n)	__gnu_malloc((n))
#endif

#if defined(__USE_GNU) || defined(__USE_BSD) || defined(__USE_MISC)
#include <alloca.h>
#endif	/* Use GNU, BSD, or misc.  */

#ifdef	__USE_BSD
/* Allocate SIZE bytes on a page boundary.  The storage cannot be freed.  */
extern void * valloc(size_t __size);
#endif


#ifndef	__NORETURN
#ifdef	__GNUC__
/* The `volatile' keyword tells GCC that a function never returns.  */
#define	__NORETURN	__volatile
#else	/* Not GCC.  */
#define	__NORETURN
#endif	/* GCC.  */
#endif	/* __NORETURN not defined.  */

/* Abort execution and generate a core-dump.  */
extern __NORETURN void abort(void);


/* Register a function to be called when `exit' is called.  */
extern int atexit(void (*__func)(void));

#ifdef	__USE_MISC
/* Register a function to be called with the status
   given to `exit' and the given argument.  */
extern int on_exit(void (*__func)(int __status, void * __arg),
			   void * __arg);
#endif

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered
   perform stdio cleanup, and terminate program execution with STATUS.  */
extern __NORETURN void exit(int __status);


/* Return the value of envariable NAME, or NULL if it doesn't exist.  */
extern char *getenv(const char *__name);

#ifdef	__USE_SVID
/* The SVID says this is in <stdio.h>, but this seems a better place.	*/
/* Put STRING, which is of the form "NAME=VALUE", in the environment.
   If there is no `=', remove NAME from the environment.  */
extern int putenv(const char *__string);
#endif

/* Execute the given line as a shell command.  */
extern int system(const char *__command);


/* Shorthand for type of comparison functions.  */
typedef int (*__compar_fn_t) (const void *, const void *);

#ifdef	__GNUC__
#define	comparison_fn_t	__compar_fn_t
#endif

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern void * bsearch(const void * __key, const void * __base,
			   size_t __nmemb, size_t __size,
			   __compar_fn_t __compar);

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort(void * __base, size_t __nmemb, size_t __size,
			  __compar_fn_t __compar);


#ifndef	__CONSTVALUE
#ifdef	__GNUC__
/* The `const' keyword tells GCC that a function's return value is
   based solely on its arguments, and there are no side-effects.  */
#define	__CONSTVALUE	__const
#else
#define	__CONSTVALUE
#endif	/* GCC.  */
#endif	/* __CONSTVALUE not defined.  */

/* Return the absolute value of X.  */
extern __CONSTVALUE int abs(int __x);
extern __CONSTVALUE long int labs(long int __x);

#if	defined(__GNUC__) && defined(__OPTIMIZE__)
#ifndef __cplusplus
/* It is defined in libg++.a as builtin. */
#define	abs(x)	__builtin_abs(x)
#endif
#define	labs(x)	__builtin_labs(x)
#endif	/* GCC and optimizing.  */


/* Return the `div_t' or `ldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern __CONSTVALUE div_t div(int __numer, int __denom);
extern __CONSTVALUE ldiv_t ldiv(long int __numer, long int __denom);


/* Return the length of the multibyte character
   in S, which is no longer than N.  */
extern int mblen(const char *__s, size_t __n);
/* Return the length of the given multibyte character,
   putting its `wchar_t' representation in *PWC.  */
extern int mbtowc(wchar_t *__pwc, const char *__s, size_t __n);
/* Put the multibyte character represented
   by WCHAR in S, returning its length.  */
extern int wctomb(char *__s, wchar_t __wchar);

#ifdef	__OPTIMIZE__
#define	mblen(s, n)	mbtowc((wchar_t *) NULL, (s), (n))
#endif	/* Optimizing.  */


/* Convert a multibyte string to a wide char string.  */
extern size_t mbstowcs(wchar_t *__pwcs, const char *__s, size_t __n);
/* Convert a wide char string to multibyte string.  */
extern size_t wcstombs(char *__s, const wchar_t *__pwcs, size_t __n);


/* I added the followings to Linux. H.J. */
#if defined(__USE_MISC)

extern char **environ;

extern void*	memalign(size_t __alignment, size_t __size);
extern void*	valloc(size_t __size);

extern char*	ecvt(double __value, size_t __ndigit, int *__decpt,
			int *__sign);
extern char*	fcvt(double __value, size_t __ndigit, int *__decpt,
			int *__sign);
extern char*	gcvt(double __value, size_t __ndigit, char *__buf);

#ifndef __cplusplus
/* It is defined in libg++.a as builtin. */
extern char*	dtoa(double __d, int __mode, int __ndigits,
			int *__decpt, int *__sign, char **__rve);
#endif

extern double	drand48(void);
extern double	erand48(unsigned short int __xsubi[3]);
extern long int	lrand48(void);
extern long int	nrand48(unsigned short int __xsubi[3]);
extern long int	mrand48(void);
extern long int	jrand48(unsigned short int __xsubi[3]);
extern void	srand48(long int __seedval);
extern unsigned short int
			*seed48(unsigned short int __seed16v[3]);
extern void	lcong48(unsigned short int __param[7]);

extern int	setenv(const char *__name, const char *__value,
			int __overwrite);
extern void	unsetenv(const char *__name);

#endif

#ifdef __cplusplus
}
#endif

#endif	/* stdlib.h  */
