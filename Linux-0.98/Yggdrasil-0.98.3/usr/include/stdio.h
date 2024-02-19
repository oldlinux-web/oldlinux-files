/* This is part of the iostream/stdio library, providing -*- C -*- I/O.
   Define ANSI C stdio on top of C++ iostreams.
   Copyright (C) 1991 Per Bothner.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.


This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _STDIO_H
#define _STDIO_H

#include <traditional.h>

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef EOF
#define EOF (-1)
#endif
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

/* check streambuf.h */
#define STDIO_S_EOF_SEEN 16		/* _S_EOF_SEEN */
#define STDIO_S_ERR_SEEN 32		/* _S_ERR_SEEN */

#define _IOFBF 0 /* Fully buffered. */
#define _IOLBF 1 /* Line buffered. */
#define _IONBF 2 /* No buffering. */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#include <_G_config.h>
#ifndef _SIZE_T
#define _SIZE_T
typedef _G_size_t size_t;
#endif
#ifndef _FPOS_T
#define _FPOS_T
typedef _G_fpos_t fpos_t;
#endif

#ifdef linux
#define TMP_MAX         140608
#endif

#define FOPEN_MAX	_G_FOPEN_MAX
#define FILENAME_MAX	_G_FILENAME_MAX

#define L_ctermid     9
#define L_cuserid     9
#define P_tmpdir      "/usr/tmp/"
#define L_tmpnam      25

struct __FILE {
    /* NOTE: Must match (or be a prefix of) __streambuf! */
    int _flags;		/* High-order word is _IO_MAGIC; rest is flags. */
    char* _gptr;	/* Current get pointer */
    char* _egptr;	/* End of get area. */
    char* _eback;	/* Start of putback+get area. */
    char* _pbase;	/* Start of put area. */
    char* _pptr;	/* Current put pointer. */
    char* _epptr;	/* End of put area. */
    char* _base;	/* Start of reserve area. */
    char* _ebuf;	/* End of reserve area. */
    struct streambuf *_chain;
};

typedef struct __FILE FILE;
#if 1
extern FILE *stdin, *stdout, *stderr; /* For use by debuggers. */
extern struct _fake_filebuf _cin_sbuf, _cout_sbuf, _cerr_sbuf;
#define stdin ((FILE*)&_cin_sbuf)
#define stdout ((FILE*)&_cout_sbuf)
#define stderr ((FILE*)&_cerr_sbuf)
#else
extern FILE stdin[1], stdout[1], stderr[1];
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void	clearerr _ARGS((FILE*));
extern char*	ctermid _ARGS((char *));
extern char*	cuserid _ARGS((char *));
extern int	fclose _ARGS((FILE*));
extern int	feof _ARGS((FILE*));
extern int	ferror _ARGS((FILE*));
extern int	fflush _ARGS((FILE*));
extern int	fgetc _ARGS((FILE *));
extern int	fgetpos _ARGS((FILE *, fpos_t *));
extern char*	fgets _ARGS((char*, int, FILE*));
extern FILE*	fopen _ARGS((const char*, const char*));
extern int	fprintf _ARGS((FILE*, const char* __format, ...));
extern int	fputc _ARGS((int, FILE*));
extern int	fputs _ARGS((const char *str, FILE *fp));
extern size_t	fread _ARGS((void*, size_t, size_t, FILE*));
extern FILE*	freopen _ARGS((const char*, const char*, FILE*));
extern int	fscanf _ARGS((FILE *__fp, const char* __format, ...));
extern long int	fseek _ARGS((FILE* __fp, long int __offset, int __whence));
extern int	fsetpos _ARGS((FILE *, const fpos_t *));
extern long int	ftell _ARGS((FILE* fp));
extern size_t	fwrite _ARGS((const void*, size_t, size_t, FILE*));
extern int	getc _ARGS((FILE *stream));
extern int	getchar _ARGS((void));
extern char*	gets _ARGS((char*));
extern int	getw _ARGS((FILE*));
extern int	printf _ARGS((const char* __format, ...));
extern int	putc _ARGS((int __c, FILE*));
extern int	putchar _ARGS((int __c));
extern int	puts _ARGS((const char*));
extern int	putw _ARGS((int, FILE*));
extern int	remove _ARGS((const char*));
extern void	rewind _ARGS((FILE*));
extern int	scanf _ARGS((const char* __format, ...));
extern void	setbuf _ARGS((FILE*, char*));
extern void	setlinebuf _ARGS((FILE*));
extern void	setbuffer _ARGS((FILE*, char*, int));
extern int	setvbuf _ARGS((FILE*, char*, int __mode, size_t __size));
extern int	sprintf _ARGS((char*, const char* __format, ...));
extern int	sscanf _ARGS((const char* __string, const char* __format, ...));
extern char*	tempnam _ARGS((const char *__dir, const char *__pfx));
extern FILE*	tmpfile _ARGS((void));
extern char*	tmpnam  _ARGS((char *__s));
extern int	ungetc _ARGS((int __c, FILE* __fp));
extern int	vfprintf _ARGS((FILE *__fp, char const *__fmt0, _G_va_list));
extern int	vfscanf _ARGS((FILE *__fp, char const *__fmt0, _G_va_list));
extern int	vprintf _ARGS((char const *__fmt, _G_va_list));
extern int	vscanf _ARGS((char const *__fmt, _G_va_list));
extern int	vsprintf _ARGS((char* __string, const char* __format, _G_va_list));
extern int	vsscanf _ARGS((const char* __string, const char* __format, _G_va_list));

#if !defined(__STRICT_ANSI__) || defined(_POSIX_SOURCE)
extern FILE*	fdopen _ARGS((int, const char *));
extern int	fileno _ARGS((FILE*));
extern int	pclose _ARGS((FILE*));
extern FILE*	popen _ARGS((const char*, const char*));
#endif

extern int __underflow _ARGS((struct streambuf*));
extern int __overflow _ARGS((struct streambuf*, int));

#if defined(__OPTIMIZE__) && !defined(NO_STDIO_MACRO)

#define getc(fp)		((fp)->_gptr >= (fp)->_egptr && \
				__underflow((struct streambuf*)(fp)) \
				== EOF ? EOF \
				: *(unsigned char*)(fp)->_gptr++)
#define getchar()		getc(stdin)
#define putc(c,fp)		(((fp)->_pptr >= (fp)->_epptr) ? \
				__overflow((struct streambuf*)(fp), \
				(unsigned char) (c)) \
				: (unsigned char)(*(fp)->_pptr++ = (c)))
#define putchar(c)		putc(c, stdout)
#define	clearerr(stream)	((stream)->_flags &= \
				~(STDIO_S_ERR_SEEN | STDIO_S_EOF_SEEN))
#define	feof(stream)		(((stream)->_flags & \
				STDIO_S_EOF_SEEN) ? EOF : 0)
#define	ferror(stream)		(((stream)->_flags & \
				STDIO_S_ERR_SEEN) != 0)

#endif	/* not Optimizing */

#ifdef __cplusplus
}
#endif

#endif /*!_STDIO_H*/
