#ifndef _STDIO_H
#define _STDIO_H

/*			s t d i o
 *
 *		Author: C. E. Chew
 *		Date:   August 1989
 *
 * (C) Copyright C E Chew
 *
 * Feel free to copy, use and distribute this software provided:
 *
 *	1. you do not pretend that you wrote it
 *	2. you leave this copyright notice intact.
 *
 * Definitions and user interface for the stream io package.
 *
 * Patchlevel 2.0
 *
 * Edit History:
 */

/* Site specific definitions */
/*+*/
/*efine	_STDIO_P_(x)*/
/*efine	_STDIO_VA_*/
/*efine	_STDIO_UCHAR_*/
/*efine	_STDIO_VA_LIST_*/
/*efine	_STDIO_SIZE_T_*/
/*efine _STDIO_USIZE_T_*/
/*efine _STDIO_NULL*/
/*efine NULL*/
/*-*/

/* Definitions based on ANSI compiler */
#ifdef		__STDC__
# ifndef	_STDIO_P_
#   define	_STDIO_P_(x)		x
# endif
# ifndef	_STDIO_VA_
#   define	_STDIO_VA_		, ...
# endif
# ifndef	_STDIO_UCHAR_
#   define	_STDIO_UCHAR_		0
# endif
#else
# ifndef	_STDIO_P_
#   define	_STDIO_P_(x)		()
# endif
# ifndef	_STDIO_VA_
#   define	_STDIO_VA_
# endif
# ifndef	_STDIO_UCHAR_
#   define	_STDIO_UCHAR_		(0xff)
# endif
#endif

#ifndef		_STDIO_VA_LIST_
#  define	_STDIO_VA_LIST_		void *
#endif

#ifndef		_STDIO_SIZE_T_
#  define	_STDIO_SIZE_T_		unsigned int
#endif

#ifndef		_STDIO_USIZE_T_
#  define	_STDIO_USIZE_T_		unsigned int
#endif

/* ANSI Definitions */
#define BUFSIZ 1024			/* default buffer size */

#ifndef	NULL
# define NULL		((void *) 0)	/* null pointer */
#endif

#define EOF		(-1)		/* eof flag */
#define FOPEN_MAX	16		/* minimum guarantee */
#define FILENAME_MAX	127		/* maximum length of file name */

#define SEEK_SET	0		/* seek from beginning */
#define SEEK_CUR	1		/* seek from here */
#define SEEK_END	2		/* seek from end */

#define TMP_MAX		(0xffff)	/* maximum number of temporaries */

#define L_tmpnam	(5 + 8 + 4 + 1 + 1) /* length of temporary file name */

#ifndef _FPOS_T
# define _FPOS_T
  typedef long fpos_t;			/* stream positioning */
#endif

#ifndef	_SIZE_T
# define _SIZE_T
  typedef _STDIO_SIZE_T_ size_t;	/* sizeof type */
#endif

#define _IOFBF		000000		/* fully buffered io */
#define _IOREAD		000001		/* opened for reading */
#define _IOWRITE	000002		/* opened for writing */
#define _IONBF		000004		/* unbuffered */
#define _IOMYBUF	000010		/* allocated buffer */
#define _IOPOOLBUF	000020		/* buffer belongs to pool */
#define _IOEOF		000040		/* eof encountered */
#define _IOERR		000100		/* error encountered */
#define _IOSTRING	000200		/* strings */
#define _IOLBF		000400		/* line buffered */
#define _IORW		001000		/* opened for reading and writing */
#define _IOAPPEND	002000		/* append mode */
#define _IOINSERT	004000		/* insert into __iop chain */
#define _IOSTDX		030000		/* standard stream */

#define _IOSTDIN	010000		/* stdin indication */
#define _IOSTDOUT	020000		/* stdout indication */
#define _IOSTDERR	030000		/* stderr indication */

#define _IORETAIN	(_IOSTDX | _IOINSERT)	/* flags to be retained */

/* Implementation Definitions */

typedef char __stdiobuf_t;		/* stdio buffer type */
typedef _STDIO_USIZE_T_ __stdiosize_t;	/* unsigned size_t */

typedef struct __iobuf {
  __stdiobuf_t *__rptr;			/* pointer into read buffer */
  __stdiobuf_t *__rend;			/* point at end of read buffer */
  __stdiobuf_t *__wptr;			/* pointer into write buffer */
  __stdiobuf_t *__wend;			/* point at end of write buffer */
  __stdiobuf_t *__base;			/* base of buffer */
  __stdiosize_t __bufsiz;		/* size of buffer */
  short __flag;				/* flags */
  char __file;				/* channel number */
  __stdiobuf_t __buf;			/* small buffer */
  int (*__filbuf) _STDIO_P_((struct __iobuf *));      /* fill input buffer */
  int (*__flsbuf) _STDIO_P_((int, struct __iobuf *)); /* flush output buffer */
  int (*__flush) _STDIO_P_((struct __iobuf *));	/* flush buffer */
  struct __iobuf *__next;		/* next in chain */
} FILE;

extern FILE __stdin;			/* stdin */
extern FILE __stdout;			/* stdout */
extern FILE __stderr;			/* stderr */

#define stdin		(&__stdin)
#define stdout		(&__stdout)
#define stderr		(&__stderr)

/* ANSI Stdio Requirements */

int	getc		_STDIO_P_((FILE *));
#if	_STDIO_UCHAR_
# define getc(p)	((p)->__rptr>=(p)->__rend\
			 ?(*(p)->__filbuf)(p)\
			 :(int)(*(p)->__rptr++&_STDIO_UCHAR_))
#else
# define getc(p)	((p)->__rptr>=(p)->__rend\
			 ?(*(p)->__filbuf)(p)\
			 :(int)((unsigned char)(*(p)->__rptr++)))
#endif

int	getchar		_STDIO_P_((void));
#define getchar()	getc(stdin)

int	putc		_STDIO_P_((int, FILE *));
#if	_STDIO_UCHAR_
# define putc(x,p)	((p)->__wptr>=(p)->__wend\
                         ?(*(p)->__flsbuf)((x),(p))\
	                 :(int)(*(p)->__wptr++=(x)&_STDIO_UCHAR_))
#else
# define putc(x,p)	((p)->__wptr>=(p)->__wend\
                         ?(*(p)->__flsbuf)((x),(p))\
	                 :(int)((unsigned char)(*(p)->__wptr++=(x))))
#endif

int	putchar		_STDIO_P_((int));
#define	putchar(x)	putc(x,stdout)

int	feof		_STDIO_P_((FILE *));
#define feof(p)		(((p)->__flag&_IOEOF)!=0)

int	ferror		_STDIO_P_((FILE *));
#define ferror(p)	(((p)->__flag&_IOERR)!=0)

void	clearerr	_STDIO_P_((FILE *));
#define clearerr(p)	((p)->__flag&=~(_IOEOF|_IOERR))

FILE 	*fopen		_STDIO_P_((const char *, const char *));
FILE	*freopen	_STDIO_P_((const char *, const char *, FILE *));
int	fflush		_STDIO_P_((FILE *));
int	fclose		_STDIO_P_((FILE *));

int	fgetpos		_STDIO_P_((FILE *, fpos_t *));
int	fsetpos		_STDIO_P_((FILE *, fpos_t *));
long	ftell		_STDIO_P_((FILE *));
int	fseek		_STDIO_P_((FILE *, long, int));
void	rewind		_STDIO_P_((FILE *));

int	fgetc		_STDIO_P_((FILE *));
int	fputc		_STDIO_P_((int, FILE *));
__stdiosize_t	fread	_STDIO_P_((void *, __stdiosize_t,
				   __stdiosize_t, FILE *));
__stdiosize_t	fwrite	_STDIO_P_((void *, __stdiosize_t,
				   __stdiosize_t, FILE *));

int	getw		_STDIO_P_((FILE *));
int	putw		_STDIO_P_((int, FILE *));
char	*gets		_STDIO_P_((char *));
char	*fgets		_STDIO_P_((char *, int, FILE *));
int	puts		_STDIO_P_((const char *));
int	fputs		_STDIO_P_((const char *, FILE *));

int	ungetc		_STDIO_P_((int, FILE *));

int	printf		_STDIO_P_((const char * _STDIO_VA_));
int	fprintf		_STDIO_P_((FILE *, const char * _STDIO_VA_));
int	sprintf		_STDIO_P_((char *, const char * _STDIO_VA_));
int	vprintf		_STDIO_P_((const char *, _STDIO_VA_LIST_));
int	vfprintf	_STDIO_P_((FILE *, const char *, _STDIO_VA_LIST_));
int	vsprintf	_STDIO_P_((char *, const char *, _STDIO_VA_LIST_));
int	scanf		_STDIO_P_((const char * _STDIO_VA_));
int	fscanf		_STDIO_P_((FILE *, const char * _STDIO_VA_));
int	sscanf		_STDIO_P_((const char *, const char * _STDIO_VA_));

void	setbuf		_STDIO_P_((FILE *, char *));
int	setvbuf		_STDIO_P_((FILE *, char *, int, __stdiosize_t));

int	rename		_STDIO_P_((const char *, const char *));
int	remove		_STDIO_P_((const char *));

void	perror		_STDIO_P_((const char *));

char *	tmpnam		_STDIO_P_((char *));
FILE *	tmpfile		_STDIO_P_((void));

/* Posix Definitions */
int	unlink		_STDIO_P_((const char *));
#define remove(x)	unlink((x))

#define L_ctermid	9
char *	ctermid		_STDIO_P_((char *s));

#define L_cuserid	9
char *	cuserid		_STDIO_P_((char *s));

FILE	*fdopen		_STDIO_P_((int, const char *));

int	fileno		_STDIO_P_((FILE *));
#define fileno(p)	((p)->__file)

#undef	_STDIO_P_
#undef	_STDIO_VA_
#undef	_STDIO_VA_LIST_
/*ndef	_STDIO_UCHAR_*/
#undef	_STDIO_SIZE_T_
#undef	_STDIO_USIZE_T_
#endif
