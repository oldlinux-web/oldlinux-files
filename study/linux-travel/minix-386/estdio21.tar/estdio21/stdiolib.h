/*			s t d i o l i b
 *
 * (C) Copyright C E Chew
 *
 * Feel free to copy, use and distribute this software provided:
 *
 *	1. you do not pretend that you wrote it
 *	2. you leave this copyright notice intact.
 *
 * This include file is used by the stdio code. It provides some
 * useful macro definitions that make the code a bit easier to
 * write.
 *
 * Patchlevel 3.0
 */

#ifndef	STDIOLIB_H
#define STDIOLIB_H

#include "config.h"

/* Hide standard names */

#ifdef	HIDDEN
# include "hidden.h"
#endif

/* System types */

#ifdef		SYSTYPES
# include <sys/types.h>
#endif

/* Promotions for system call parameter passing */

#ifndef		MODE_T
# define MODE_T	int
#endif

#ifndef		UID_T
# define UID_T	int
#endif

/* System file control */

#include <fcntl.h>

/* Time types */

#ifdef		TIME
# include <time.h>
#endif

/* Posix standard things */

#ifdef		UNISTD
# include <unistd.h>
#endif

/* Limits */

#ifdef		LIMITS
# include <limits.h>
#endif

/* Floating point limits */

#ifndef		NOFLOAT
# ifdef		FLOAT
#   include <float.h>
#   if	DBL_MANT_DIG < LDBL_MANT_DIG && \
	DBL_MAX_EXP  < LDBL_MAX_EXP  && \
	DBL_MIN_EXP  > LDBL_MIN_EXP
#     define LONGDOUBLE
#   endif
# endif
#endif

/* Long doubles */

#ifndef		NOFLOAT
# ifdef		LONGDOUBLE
    typedef long double longdouble;
#   define LMODF  lmodf
#   define LFREXP lfrexp
#   define LLDEXP lldexp
# else
    typedef double longdouble;
#   define LMODF  modf
#   define LFREXP frexp
#   define LLDEXP ldexp
#   undef	LDBL_DIG
#   define	LDBL_DIG	DBL_DIG
#   undef	LDBL_EPSILON
#   define	LDBL_EPSILON	DBL_EPSILON
#   undef	LDBL_MANT_DIG
#   define	LDBL_MANT_DIG	DBL_MANT_DIG
#   undef	LDBL_MAX
#   define	LDBL_MAX	DBL_MAX
#   undef	LDBL_MAX_10_EXP
#   define	LDBL_MAX_10_EXP	DBL_MAX_10_EXP
#   undef	LDBL_MAX_EXP
#   define	LDBL_MAX_EXP	DBL_MAX_EXP
#   undef	LDBL_MIN
#   define	LDBL_MIN	DBL_MIN
#   undef	LDBL_MIN_10_EXP
#   define	LDBL_MIN_10_EXP	DBL_MIN_10_EXP
#   undef	LDBL_MIN_EXP
#   define	LDBL_MIN_EXP	DBL_MIN_EXP
# endif
#endif

/* Significant digits in mantissa
 *
 * Since 10^k = 5^k.2^k, the highest power of ten that will fit in N bits
 * is N.log_5 2, if the exponent base if binary.
 */

#ifndef	NOFLOAT
# if	FLT_RADIX == 2
#   define MANTDIGITS		(LDBL_MANT_DIG * 1 * 43 / 100)
#   define HALFMANTDIGITS	(LDBL_MANT_DIG / 2 * 1 * 43 / 100)
# else
# if	FLT_RADIX == 4
#   define MANTDIGITS		(LDBL_MANT_DIG * 2 * 43 / 100)
#   define HALFMANTDIGITS	(LDBL_MANT_DIG / 2 * 2 * 43 / 100)
# else
# if	FLT_RADIX == 8
#   define MANTDIGITS		(LDBL_MANT_DIG * 3 * 43 / 100)
#   define HALFMANTDIGITS	(LDBL_MANT_DIG / 2 * 3 * 43 / 100)
# else
# if	FLT_RADIX == 16
#   define MANTDIGITS		(LDBL_MANT_DIG * 4 * 43 / 100)
#   define HALFMANTDIGITS	(LDBL_MANT_DIG / 2 * 4 * 43 / 100)
# else
# if	FLT_RADIX == 10
#   define MANTDIGITS		(LDBL_MANT_DIG)
#   define HALFMANTDIGITS	(LDBL_MANT_DIG/2)
# else
  << FLT_RADIX not on mantissa significant digits list >>
# endif
# endif
# endif
# endif
# endif
#endif

/* Constants */

#ifdef		__STDC__
# define CONST	const
#else
# define CONST
#endif

/* Variable argument list hiding */

#ifdef		STDARG
# include <stdarg.h>
# define VA_START(n,l)	va_start(n,l)
# ifdef		__STDC__
#   define VA_ALIST	, ...
# else
#   define VA_ALIST
# endif
# define VA_LIST	va_list
# define VA_END(n)	va_end(n)
# define VA_ARG(n,t)	va_arg(n,t)
# define VA_DCL
#else
# include <varargs.h>
# define VA_START(n,l)	va_start(n)
# define VA_ALIST	va_alist
# define VA_LIST	va_list
# define VA_END(n)	va_end(n)
# define VA_ARG(n,t)	va_arg(n,t)
# define VA_DCL		va_dcl
#endif

/* Function prototype hiding */

#ifdef		__STDC__
# define	P(x)	x
# define	F8(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8) \
		  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)
# define	F7(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7) \
		  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)
# define	F6(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6) \
		  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)
# define	F5(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5) \
		  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)
# define	F4(t1,n1,t2,n2,t3,n3,t4,n4) \
		  (t1 n1, t2 n2, t3 n3, t4 n4)
# define	F3(t1,n1,t2,n2,t3,n3) \
		  (t1 n1, t2 n2, t3 n3)
# define	F2(t1,n1,t2,n2) \
		  (t1 n1, t2 n2)
# define	F1(t1,n1) \
		  (t1 n1)
# define	F0()	(void)
#else
# define	P(x)	()
# define	F8(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8) \
			(n1, n2, n3, n4, n5, n6, n7, n8) \
			t1 n1;t2 n2;t3 n3;t4 n4;t5 n5;t6 n6;t7 n7;t8 n8;
# define	F7(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7) \
			(n1, n2, n3, n4, n5, n6, n7) \
			t1 n1;t2 n2;t3 n3;t4 n4;t5 n5;t6 n6;t7 n7;
# define	F6(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6) \
			(n1, n2, n3, n4, n5, n6) \
			t1 n1;t2 n2;t3 n3;t4 n4;t5 n5;t6 n6;
# define	F5(t1,n1,t2,n2,t3,n3,t4,n4,t5,n5) \
			(n1, n2, n3, n4, n5) \
			t1 n1;t2 n2;t3 n3;t4 n4;t5 n5;
# define	F4(t1,n1,t2,n2,t3,n3,t4,n4) \
			(n1, n2, n3, n4) \
			t1 n1;t2 n2;t3 n3;t4 n4;
# define	F3(t1,n1,t2,n2,t3,n3) \
			(n1, n2, n3) \
			t1 n1;t2 n2;t3 n3;
# define	F2(t1,n1,t2,n2) \
			(n1, n2) \
			t1 n1;t2 n2;
# define	F1(t1,n1) \
			(n1) \
			t1 n1;
# define	F0()	()
#endif

/* Variadic function prototype hiding */

#ifdef STDARG
# ifdef __STDC__
#   define	F2V(t1,n1,t2,n2,t,a,l) \
		   (t1 n1, t2 n2 \
		    VA_ALIST) t a; l
#   define	F1V(t1,n1,t,a,l) \
		   (t1 n1 \
		    VA_ALIST) t a; l
# else
#   define	F2V(t1,n1,t2,n2,t,a,l) \
		   (n1,n2 VA_ALIST) \
		   t1 n1; t2 n2; \
		   t a; l
#   define	F1V(t1,n1,t,a,l) \
		   (n1 VA_ALIST) \
		   t1 n1; \
		   t a; l
# endif
#else
# define	F2V(t1,n1,t2,n2,t,a,l) \
			(VA_ALIST) VA_DCL \
			t a; t1 n1;t2 n2; l \
			n1 = VA_ARG(a,t1); \
			n2 = VA_ARG(a,t2);
# define	F1V(t1,n1,t,a,l) \
			(VA_ALIST) VA_DCL \
			t a; t1 n1; l \
			n1 = VA_ARG(a,t1);
#endif

/* Strings */

#ifdef		STRING
# include <string.h>
#endif

/* Memory */

#ifdef		MEMORY
# define MEMCPY(d,s,n)		((void) memcpy((d),(s),(n)))
# define MEMCHR(s,c,n)		(memchr((s),(c),(n)))
# define MEMSET(s,v,n)		((void) memset((s),(v),(n)))
# define MEMCCPY(d,s,c,n)	(memccpy((d),(s),(c),(n)))
#endif

/* Unsigned char casting */

#ifdef		UNSIGNEDCHAR
# define UCHAR(x)	((int) ((unsigned char) (x)))
#else
# define UCHAR(x)	((int) (x) & ((1 << CHAR_BIT) - 1))
#endif

/* Signal return type */

#include <signal.h>

#ifdef		VOIDSIGNAL
  typedef void (*signal_t) P((int));
#else
  typedef int (*signal_t) P((int));
#endif

/* System error list */

#ifdef		ERRLIST
/* Conditional referenced in _errlist.c */
#endif

/* Rest of includes */

#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>

typedef char size_t_check[((__stdiosize_t) -1) > 0 ? 1 : -1];

/* Assertion */

#ifndef	NDEBUG
#define	ASSERT(x)	(void) ((x) ? 0 : __xassert(__FILE__))
#else
#define	ASSERT(x)
#endif

/* Useful macros */

#define MAX(a,b)	((a)>(b)?(a):(b))

/* Print buffer size calculation */

#define MININTBITS	3		/* least bits required --- octal */
#define MAXINTPREFIX	3		/* overhead for prefix (+0x) */
#define MAXINTINSERT	0		/* overhead for insertions */
#define MAXINTSUFFIX	0		/* overhead for suffix */
#define MAXINTDIGITS	((sizeof(long)*CHAR_BIT+MININTBITS-1) \
			 /MININTBITS*MININTBITS)
#define PINTBUFFER	(MAXINTDIGITS+MAXINTPREFIX+MAXINTINSERT+MAXINTSUFFIX)

#ifndef	NOFLOAT
# define MAXFLTPREFIX	1		/* overhead for prefix (+) */
# define MAXFLTINSERT	1		/* overhead for insertions (.) */
# define MAXFLTSUFFIX	5		/* overhead for suffix (e-308) */
# define MAXFLTDIGITS	(LDBL_DIG+2)
# define PFLTBUFFER	(MAXFLTDIGITS+MAXFLTPREFIX+MAXFLTINSERT+MAXFLTSUFFIX)
#else
# define PFLTBUFFER	0
#endif

#define PWRITEAHEAD	4		/* write ahead in _vfprintf */

#define PBUFFERSIZE	MAX(MAX(PINTBUFFER, PFLTBUFFER),PWRITEAHEAD)

/* Formatted output conversion status bits */

#define F_LEFTJUSTIFY	0x0001		/* left justify */
#define F_SHOWSIGN	0x0002		/* display a sign */
#define F_BLANKPREFIX	0x0004		/* prefix with blank */
#define F_ALTERNATE	0x0008		/* alternate format */
#define F_SHORT		0x0010		/* argument is short */
#define F_LONG		0x0020		/* argument is long */
#define F_LONGDOUBLE	0x0040		/* argument is long double */
#define F_0X		0x0080		/* 0X instead of 0x */
#define F_FPFORMATF	0x0100		/* %f format */
#define F_FPFORMATE	0x0200		/* %e format */
#define F_FPFORMATG	0x0400		/* %g format */
#define F_FPCAPITAL	0x0800		/* %E or %G */
#define F_FPFORMAT	0x0f00		/* floating point converted */
#define F_PRECISION	0x1000		/* precision valid */

/* Formatted output vector element */

typedef struct {
  int att;				/* attributes */
  __stdiosize_t len;			/* length of vector */
  char *arg;				/* vector */
} FV;

#define FV_F_VECTOR	0x0000		/* length and pointer */
#define FV_F_PADDING	0x0001		/* padding only */
#define FV_F_STRING	0x0002		/* null terminated string */

#define FV_FMTPREFIX	0		/* global format prefix */
#define FV_FMTARGS	1

#define FV_FLT1SIGN	1		/* prefix */
#define FV_FLT1INT	2		/* integer digits */
#define FV_FLT1INTFILL	3		/* integer fill */
#define FV_FLT1DPFRAC	4		/* decimal point and fraction */
#define FV_FLT1FRACFILL	5		/* fractional fill */
#define FV_FLT1ARGS	6

#define FV_FLT2SIGN	1		/* prefix */
#define FV_FLT2INTDP	2		/* integer digits and decimal point */
#define FV_FLT2DPFILL	3		/* decimal point fill */
#define FV_FLT2FRAC	4		/* fractional digits */
#define FV_FLT2FRACFILL	5		/* fractional fill */
#define FV_FLT2ARGS	6

#define FV_FLT3SIGN	1		/* prefix */
#define FV_FLT3MANTISSA	2		/* mantissa */
#define FV_FLT3FRACFILL	3		/* fractional fill */
#define FV_FLT3SUFFIX	4		/* suffix */
#define FV_FLT3ARGS	5

#define FV_INTSIGN	1		/* sign or prefix */
#define FV_INTINT	2		/* integer digits */
#define FV_INTARGS	3

#define FV_ARGS		MAX(FV_FLT1ARGS, MAX(FV_FLT2ARGS, \
			MAX(FV_FLT3ARGS, FV_INTARGS)))

/* Internal buffer pool */

#define POOLSIZE	2		/* stdin and stdout only */

/* Stdio internal types */

typedef void (*atexit_t) P((void));	/* exit handlers */
#ifndef	NOFLOAT
typedef struct {			/* longdouble with guard */
  longdouble number;
  longdouble guard;
  int exponent;
} longguard;
#endif

/* Function prototypes */

int __bffil	P((FILE *));		/* fail */
int __bffls	P((int, FILE *));	/* fail */
int __btfls	P((FILE *));		/* succeed */

int __bwrupdate	P((int, FILE *));	/* write update */
int __bwronly	P((int, FILE *));	/* write only */

int __brdupdate	P((FILE *));		/* read update */
int __brdonly	P((FILE *));		/* read only */

__stdiosize_t __iowrite P((int, char *, __stdiosize_t)); /* repeated write */
__stdiosize_t __ioread	P((int, char *, __stdiosize_t)); /* repeated read */

void __ioflush	P((void));		/* flush output */
__stdiosize_t __allocbuf P((FILE *));	/* allocate buffer */
void __freebuf	P((FILE *));		/* deallocate buffer */
int __fopen	P((const char *, const char *, int, short *));
					/* fopen assist */
FILE *__file	P((FILE *, int, short));/* initialise FILE */

char *__fgetlx	P((char *, __stdiosize_t, FILE *));
					/* read line extended */
__stdiosize_t __rlbf P((FILE *, __stdiosize_t));
					/* adjust line buffered read */

int __cvt	P((__stdiosize_t *, FV *, char *, VA_LIST *, int, int));
					/* output */
int __tvc	P((FILE *, int, VA_LIST *, int, char *));
					/* input */

char *__utoa	P((char *, unsigned, int)); /* unsigned to string conversion */

int __xassert	P((char *));		/* assertion failed */

#ifndef	NOFLOAT
void __gpow10      P((int, longguard *));/* compute 10^x with guard */
longdouble __pow10 P((int));		 /* compute 10^x without guard */

longdouble __gmul    P((longguard *, longguard *));
void __ggmul         P((longguard *, longguard *, longguard *));
void __gguard        P((longguard *));
longdouble __gnumber P((longguard *));
void __gnormal       P((longguard *));
#endif

/* Extras not in ANSI but probably could be */

int __vscanf	P((const char *, VA_LIST));
int __vfscanf	P((FILE *, const char *, VA_LIST));
int __vsscanf	P((const char *, const char *, VA_LIST));

int __vfprintf	P((FILE *, const char *, VA_LIST));

/* Library globals */

extern int errno;			/* system error number */
extern int sys_nerr;			/* size of error table */
extern char *sys_errlist[];		/* error descriptions */

/* Stdio internal variables */

extern void (*__Zwrapup) P((void));	/* flush stdio linkage */
extern void (*__Zatexit) P((void));	/* exit handler linkage */
extern FILE *__Zout;			/* stdout linkage */
extern FILE *__Zerr;			/* stderr linkage */
extern __stdiosize_t (*__Zrlbf) P((FILE *, __stdiosize_t));
					/* rlbf() linkage */
extern FILE *__iop;			/* stream list */
extern __stdiobuf_t *__iob[POOLSIZE];	/* buffer pool */
extern char __zfill[];			/* zero fill */
extern long __ipow10[];			/* powers of 10 */
extern int __Mipow10;			/* exponent of the largest power */

#ifndef	NOFLOAT
extern longdouble *__fpow10;		/* floating point version */
extern char __xfptvc;			/* tvc linkage for library */
extern char __xfpcvt;			/* cvt linkage for library */
#endif

/* Library calls */

void *malloc	P((size_t));		/* memory allocator */
void free	P((void *));		/* free memory */

#ifndef	NOFLOAT
longdouble LMODF  P((longdouble, double *)); /* separate integer and fraction */
longdouble LFREXP P((longdouble, int *));    /* separate mantissa and exponent */
longdouble LLDEXP P((longdouble, int));      /* scale by a power of two */
#endif

/* System calls */

#ifndef		OPEN3
int open	P((const char *, int));	/* two argument open */
# define open	__open3			/* fake three argument open */
# undef O_CREAT
# undef O_APPEND
# undef O_TRUNC
# define O_CREAT	(~(O_RDONLY|O_WRONLY|O_RDWR) ^ \
			(~(O_RDONLY|O_WRONLY|O_RDWR) & \
                        (~(O_RDONLY|O_WRONLY|O_RDWR) - 1)))
# define O_APPEND	(~(O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) ^ \
			(~(O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) & \
                        (~(O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) - 1)))
# define O_TRUNC	(~(O_APPEND|O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) ^ \
			(~(O_APPEND|O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) & \
                        (~(O_APPEND|O_CREAT|O_RDONLY|O_WRONLY|O_RDWR) - 1)))
#endif

void		abort    P((void));		/* abort to system */
void		_exit    P((int));		/* exit to system */
int		isatty   P((int));		/* channel is tty */
int		close    P((int));		/* close channel */
int		creat    P((const char *, MODE_T)); /* create a file */
int		open     P((const char *, int, ...)); /* open a file */
int		dup2     P((int, int));		/* duplicate file descriptor */
off_t		lseek    P((int, off_t, int));	/* seek within file */
int		read     P((int, char *, unsigned int)); /* read channel */
int		write    P((int, char *, unsigned int)); /* write channel */
int		unlink   P((const char *));	/* unlink a file */
int		link     P((const char *, const char *)); /* link a file */
int		stat     P((const char *, struct stat *)); /* file status */
mode_t		umask    P((MODE_T));		/* permission mask */
int		chmod    P((const char *, MODE_T)); /* change permission */
pid_t		getpid   P((void));		/* get process id */
time_t		time     P((time_t *));		/* get time */
uid_t		geteuid  P((void));		/* get effective user id */
uid_t		getuid   P((void));		/* get user id */
struct passwd * getpwuid P((UID_T));		/* get passwd from uid */
signal_t	signal   P((int, signal_t));	/* set signals */

/* Size of tty buffers */

#define TTYBUFSIZ		81

/* Flag manipulation macros */

#define TESTFLAG(f,x)		(((f)->__flag & (x)) != 0)
#define SETFLAG(f,x)		((f)->__flag |= (x))
#define CLEARFLAG(f,x)		((f)->__flag &= ~(x))
#define GETFLAG(f,x)		((f)->__flag & (x))
#define ALLFLAGS(f)		((f)->__flag)
#define TOGGLEFLAG(f,x)		((f)->__flag ^= (x))
#define SAVEFLAG(f,p)		((p) = (int) ((f)->__flag))
#define RESTOREFLAG(f,p)	((f)->__flag = (short) (p))

/* Insert stream descriptor into __iop chain
 *
 * The __iop list element pointed to by p is inserted into the
 * __iop chain.
 */
#define FINSERT(p) ( (p)->__next=__iop, __iop=(p) )

/* Getc with full buffering
 *
 * This version of getc assumes that there is an input buffer
 * that is not empty and simply grabs the character from there.
 */
#define FGETC(p) ( UCHAR(*(p)->__rptr++) )

/* Putc for non buffered streams
 *
 * This version of putc is explicitly for unbuffered streams. A
 * call is made directly to the buffer flushing code.
 */
#define NPUTC(x,p) ( (*(p)->__flsbuf)((x),(p)) )

/* Putc with full buffering
 *
 * This version of putc() assumes that there is an output buffer
 * that is not full and simply dumps the character in there.
 */
#define FPUTC(x, p) ( UCHAR((*(p)->__wptr++ = (x))) )
 
/* Flush a stream
 *
 * Call the flush routine to clear the output buffer.
 */
#define FFLUSH(f) ( (*(f)->__flush)(f) )

/* Set __flsbuf
 *
 * Set the __flsbuf function pointer.
 */
#define SETFLSBUF(f,p) ( (f)->__flsbuf = (p) )

/* Set __filbuf
 *
 * Set the __filbuf function pointer.
 */
#define SETFILBUF(f,p) ( (f)->__filbuf = (p) )

/* Set __flush
 *
 * Set the __flush function pointer.
 */
#define SETFLUSH(f,p) ( (f)->__flush = (p) )

/* Initialise an output buffer
 *
 * This macro uses __base and __bufsiz to initialise __wptr and
 * __wend. __wptr will be set to point at the base of the
 * buffer. __wend will be set to point at one past the end of the
 * buffer if the stream is buffered otherwise it will point at
 * the base of the buffer. Line buffered streams are considered to
 * be fully buffered. This macro must not alter __base unless the
 * code in fflush() is changed.
 */
#define INITWRITEBUFFER(f) ( \
  (f)->__wend = ((f)->__wptr = (f)->__base) + \
                (TESTFLAG(f, _IONBF | _IOLBF) ? 0 : (f)->__bufsiz) \
)

/* Initialise an input buffer
 *
 * This macro empties an input buffer. It uses __base to initialise
 * __rptr and sets __rend to point to the high water mark of the buffer.
 */
#define INITREADBUFFER(f, v) ( \
  (f)->__rend = ((f)->__rptr = (f)->__base) + (v) \
)

/* Flush the next write
 *
 * This macro initialises the buffer by setting __wend and
 * __wptr to nil. This will force the next putc to call
 * __flsbuf.
 */
#define CHECKNEXTWRITE(f) ( (f)->__wend = (f)->__wptr = 0 )

/* Check a write
 *
 * If __wptr is zero, the write buffer is checked.
 */
#define CHECKWRITE(f) ( \
  (f)->__wptr == 0 ? ((f)->__wend = &(f)->__buf, (*(f)->__flsbuf)(0, f)) : 0 \
)

/* Check write status
 *
 * Return non-zero if a write check is scheduled, zero otherwise. This
 * assumes that the stream is writeable.
 */
#define ISCHECKWRITE(f) ( (f)->__wptr == 0 )

/* Flush the next read
 *
 * This macro initialises the buffer by setting __rend and
 * __rptr to nil. This will force the next next getc to call
 * __filbuf.
 */
#define CHECKNEXTREAD(f) ( (f)->__rend = (f)->__rptr = 0 )

/* Check a read
 *
 * If __rptr is zero, the read buffer is checked.
 */
#define CHECKREAD(f) ( \
  (f)->__rptr == 0 ? ((f)->__rend = &(f)->__buf, (*(f)->__filbuf)(f)) : 0 \
)

/* Check read status
 *
 * Return non-zero if a read check is scheduled, zero otherwise. This
 * assumes that the stream is readable.
 */
#define ISCHECKREAD(f) ( (f)->__rptr == 0 )

/* Buffer size
 *
 * Return the size of the buffer. This will return rubbish
 * for unbuffered streams. Line and fully buffered streams will
 * have the true buffer size returned.
 */
#define BUFFERSIZE(f) ( (f)->__bufsiz )

/* Bytes left in input buffer
 *
 * This macro returns the number of bytes left in an input buffer.
 * The result returned will be zero even in the case where the stream
 * has a check scheduled.
 */
#define BYTESINREADBUFFER(f) ( (f)->__rend - (f)->__rptr )

/* Bytes written in output buffer
 *
 * This macro returns the number of bytes left in an output buffer.
 * The result is not valid when a check has been scheduled.
 */
#define BYTESINWRITEBUFFER(f) ( (f)->__wptr - (f)->__base )

/* Unused bytes in output buffer
 *
 * This macro returns the number of unused bytes in an output buffer.
 * Unbuffered streams will return rubbish. Line and fully buffered streams
 * will have the amount of space remaining returned. In order to accommodate
 * line buffered streams, __wend cannot be used. The result is not valid
 * when a check has been scheduled.
 */
#define UNUSEDINWRITEBUFFER(f) ( (f)->__bufsiz - BYTESINWRITEBUFFER(f) )

/* Get pointer into write buffer
 *
 * This macro gets the pointer into the write buffer.
 */
#define GETWRITEPTR(f) ( (f)->__wptr )

/* Set pointer into write buffer
 *
 * This macro sets the pointer into the write buffer.
 */
#define SETWRITEPTR(f,p) ( (f)->__wptr = (p) )

/* Get pointer to end of write buffer
 *
 * This macro returns the end of write buffer pointer. This may not
 * point to the real end of the write buffer.
 */
#define GETWRITEEND(f) ( (f)->__wend )

/* Get pointer to limit of write buffer
 *
 * This macro returns the limit of write buffer pointer. This will
 * point to the real end of the write buffer.
 */
#define GETWRITELIMIT(f) ( (f)->__base + BUFFERSIZE(f) )

/* Get pointer into read buffer
 *
 * This macro gets the pointer into the read buffer.
 */
#define GETREADPTR(f) ( (f)->__rptr )

/* Set pointer into read buffer
 *
 * This macro sets the pointer into the read buffer.
 */
#define SETREADPTR(f,p) ( (f)->__rptr = (p) )

/* Get pointer to end of read buffer
 *
 * This macro returns the end of read buffer pointer. This may not
 * point to the real end of the read buffer.
 */
#define GETREADEND(f) ( (f)->__rend )

/* Get pointer to limit of read buffer
 *
 * This macro returns the limit of read buffer pointer. This will
 * point to the real end of the read buffer.
 */
#define GETREADLIMIT(f) ( (f)->__base + BUFFERSIZE(f) )

/* Check if buffering has been set
 *
 * Return true if buffering has already been set. A stream
 * set for unbuffered output is considered to have had
 * its buffering set.
 */
#define HASBUFFER(f) ( (f)->__base != 0 )

/* Set __wend for line buffering
 *
 * Set __wend for line buffering. This means that _wend is set to
 * point at __base.
 */
#define SETWRITELINEBUFFERING(f) ( (f)->__wend = (f)->__base )

/* Set __wend for full buffering
 *
 * Set __wend to __base + __bufsiz so that the entire buffer can be
 * used.
 */
#define SETWRITEFULLBUFFERING(f) ( (f)->__wend = (f)->__base + (f)->__bufsiz )

/* Unroll a loop
 *
 * Assume that the loop must execute at least once. The first argument
 * is the name of the loop control variable. The second argument is the
 * name of the loop control variable. The third argument is the expression
 * to be placed in the loop body. The control variable should be unsigned,
 * otherwise the right shift might propagate the sign bit. The caller must
 * also provide the name of a unique label.
 */
#define UNROLL_DO(l,v,x) { \
  char t = (v); \
  (v) = ((v)+1) >> 1; \
  if (t & 1) goto l; \
  do { x; l: x; } while (--(v)); \
}

#endif
