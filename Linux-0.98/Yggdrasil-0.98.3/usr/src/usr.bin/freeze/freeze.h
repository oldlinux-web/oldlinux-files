#include <stdio.h>

#ifdef SUN4
#include <sys/stdtypes.h>
#else   /* SUN4 */
# ifndef getc
#   define getc(p)         (--(p)->_cnt < 0 ? _filbuf(p) : (int) *(p)->_ptr++)
# endif
# ifndef putc
#   define putc(x, p)      (--(p)->_cnt < 0 ? _flsbuf((unsigned char) (x), (p)) : (int) (*(p)->_ptr++ = (unsigned char) (x)))
# endif
#endif  /* SUN4 */

#if !defined(MSDOS) && defined(__MSDOS__)
#define MSDOS
#endif

#ifdef MSDOS
#define DOS
# include <fcntl.h>
#endif  /* MSDOS */

#ifdef TOS
#define DOS
#define O_TEXT		0x01
#define O_BINARY	0x02
#endif

#include <ctype.h>
#include <signal.h>

#ifndef TOS
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <tos.h>
#include <types.h>
#endif

#if !defined(__TURBOC__) && (!defined(M_XENIX) || defined(M_I386))
#ifdef UTIMES
#include <sys/time.h>
#else
#include <utime.h>      /* Some systems have sys/utime.h instead of this */
#endif /* UTIMES */
#else
#ifdef M_XENIX
struct utimbuf {
	time_t actime;
	time_t modtime;
};

extern int utime();
#endif
#endif

/* for MAXNAMLEN only !!! */
#ifdef unix
#ifndef BSD
#include <dirent.h>
#else
#include <sys/dir.h>
#endif /* BSD */
#endif /* unix */

#if !defined(MAXNAMLEN)
#define MAXNAMLEN       255
#else
#if MAXNAMLEN < 255
#undef MAXNAMLEN
#define MAXNAMLEN       255
#endif
#endif

#ifdef DEBUG
#include <assert.h>
#endif  /* DEBUG */

#ifdef DOS
# include <stdlib.h>
#endif  /* DOS */

#ifdef __TURBOC__
# ifdef MSDOS
#  include <io.h>
#  include <alloc.h>
# else /* TOS */
#  include <ext.h>
# endif  /* MSDOS */
#if !defined(SIGTYPE)
#define SIGTYPE void
#endif
#endif /* __TURBOC__ */

typedef unsigned short us_t;
typedef unsigned char uc_t;
typedef unsigned long ul_t;

#if defined(BITS) && BITS > 14
typedef ul_t hash_t;
#else
typedef us_t hash_t;
#endif  /* BITS */

#ifdef  lint
#define N2              256
#else   /* lint */
#define N2              8192    /* buffer size */
#endif  /* lint */

#define F2              256     /* pre-sence buffer size */
#define THRESHOLD	2

#define N_CHAR2         (256 - THRESHOLD + F2 + 1) /* code: 0 .. N_CHARi - 1 */
#define T2              (N_CHAR2 * 2 - 1)       /* size of table */

#define ENDOF           256                     /* pseudo-literal */

extern uc_t    Table2[];

extern long     in_count, bytes_out, file_length;

extern uc_t      text_buf[];

extern long     indc_threshold, indc_count;

extern short    do_melt, topipe, greedy, quiet, force;  /* useful flags */

#define MAGIC1          ((uc_t)'\037')
#define MAGIC2_1        ((uc_t)'\236')          /* freeze vers. 1.X */
#define MAGIC2_2        ((uc_t)'\237')

extern int exit_stat;

#ifdef DEBUG
extern short debug;
extern short verbose;
extern char * pr_char();
extern long symbols_out, refers_out;
#endif /* DEBUG */

#ifdef GATHER_STAT
extern long node_steps, node_matches;
#endif

extern short DecodeChar(), DecodePosition(), GetNBits();
extern void melt2(), (*meltfunc)(), writeerr(), prratio(), prbits(), freeze();

#ifdef COMPAT
#include "compat.h"
#endif

#define INDICATOR \
if (quiet < 0 && (in_count > indc_count)) {\
	if (ferror(stdout))\
		writeerr();\
	if (file_length) {\
		static int percents, old_percents = -1;\
		if ((percents = ftell(stdin) * 100 / file_length) !=\
			old_percents) {\
			fprintf(stderr, " %2d%%\b\b\b\b", percents);\
			old_percents = percents;\
		}\
		indc_count += indc_threshold;\
	} else {\
		fprintf(stderr, " %5ldK\b\b\b\b\b\b\b", in_count / 1024);\
		indc_count += indc_threshold;\
		indc_threshold += 1024;\
	}\
	fflush (stderr);\
}

#ifdef BSD
#define strchr index
#define strrchr rindex
#endif

extern char *strchr(), *strrchr();
