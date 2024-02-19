/*$Source: /usr/home/dhesi/zoo/RCS/lzh.h,v $*/
/*$Id: lzh.h,v 1.3 91/07/09 01:39:23 dhesi Exp $*/

/*
Adapted from "ar" archiver written by Haruhiko Okumura.
*/

/* Define some things if they aren't defined in header files */
#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

#ifndef UCHAR_MAX
# define UCHAR_MAX 255
#endif

/* io.c */

extern FILE *arcfile, *lzh_infile, *lzh_infile;
extern t_uint16 bitbuf;
#define BITBUFSIZ (CHAR_BIT * sizeof bitbuf)

/* encode.c and decode.c */

#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256    /* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3    /* choose optimal value */
#define PERC_FLAG ((unsigned) 0x8000)

/* huf.c */

#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
	/* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define CODE_BIT  16  /* codeword length */

extern ushort left[], right[];
