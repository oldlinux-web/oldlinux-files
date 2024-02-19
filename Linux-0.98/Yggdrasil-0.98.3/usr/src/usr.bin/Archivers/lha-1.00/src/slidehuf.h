/*
 * slidehuf.h -- part of LHa for UNIX
 *
 *	modified: Feb. 26, 1992, Masaru Oki.
 */

#include <stdio.h>
#include <errno.h>

#if defined(__STDC__) || defined(AIX)
#include <limits.h>
#else
#ifndef CHAR_BIT
#define CHAR_BIT  8
#endif
#ifndef UCHAR_MAX
#define UCHAR_MAX ((1<<(sizeof(unsigned char)*8))-1)
#endif
#ifndef USHRT_MAX
#define USHRT_MAX ((1<<(sizeof(unsigned short)*8))-1)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX ((1<<(sizeof(short)*8-1))-1)
#endif
#ifndef SHRT_MIN
#define SHRT_MIN (SHRT_MAX-USHRT_MAX)
#endif
#endif /* not __STDC__ */

/* #include "errmes.h" */
/* Your C-Compiler has no 'void' (from lharc.h) */
#if !defined(__STDC__) && !defined(AIX)  /* NO_VOID */
#define void
#endif

struct encode_option {
#if defined(__STDC__) || defined(AIX)
	void (*output)();
	void (*encode_start)();
	void (*encode_end)();
#else
	int (*output)();
	int (*encode_start)();
	int (*encode_end)();
#endif
};

struct decode_option {
	unsigned short (*decode_c)();
	unsigned short (*decode_p)();
#if defined(__STDC__) || defined(AIX)
	void (*decode_start)();
#else
	int (*decode_start)();
#endif
};

/* from slide.c */
#define MAX_DICBIT    13 
#define MAX_DICSIZ (1 << MAX_DICBIT)
#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256    /* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3    /* choose optimal value */

extern int errno;
extern int unpackable;
extern unsigned long origsize, compsize;
extern unsigned short dicbit;
extern unsigned short maxmatch;
extern unsigned long count;
extern unsigned short loc;
extern unsigned char *text;

/* from huf.c */
#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
			/* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define USHRT_BIT 16	/* (CHAR_BIT * sizeof(ushort)) */

extern unsigned short	left[], right[];
extern unsigned char	c_len[], pt_len[];
extern unsigned short	c_freq[], c_table[], c_code[],
			p_freq[], pt_table[], pt_code[],
			t_freq[];

extern void output_st1();
extern unsigned char *alloc_buf(void);
extern void encode_start_st1(void);
extern void encode_end_st1(void);
extern unsigned short decode_c_st1(void);
extern unsigned short decode_p_st1(void);
extern void decode_start_st1(void);

/* from shuf.c */
extern void decode_start_st0(void);
extern void encode_p_st0(/*unsigned short j*/);
extern void encode_start_fix(void);
extern void decode_start_fix(void);
extern unsigned short decode_c_st0(void);
extern unsigned short decode_p_st0(void);

/* from dhuf.c */
extern unsigned int n_max;

extern void start_c_dyn(void);
extern void decode_start_dyn(void);
extern unsigned short decode_c_dyn(void);
extern unsigned short decode_p_dyn(void);
extern void output_dyn(/*int code, unsigned int pos*/);
extern void encode_end_dyn(void);

/* from larc.c */
extern unsigned short decode_c_lzs(void);
extern unsigned short decode_p_lzs(void);
extern unsigned short decode_c_lz5(void);
extern unsigned short decode_p_lz5(void);
extern void decode_start_lzs(void);
extern void decode_start_lz5(void);

/* from maketbl.c */
/* void make_table(short nchar, uchar bitlen[],
		   short tablebits, ushort table[]); */
extern void make_table(/*int nchar, uchar bitlen[],
			 int tablebits, ushort table[]*/);

/* from maketree.c */
/* void make_code(short n, uchar len[], ushort code[]);
short make_tree(short nparm, ushort freqparm[], 
                uchar lenparm[], ushort codeparam[]); */
extern void make_code(/*int n, uchar len[], ushort code[]*/);
extern short make_tree(/*int nparm, ushort freqparm[], 
			 uchar lenparm[], ushort codeparam[]*/);

/* from crcio.c */
extern FILE *infile, *outfile;
extern unsigned short crc, bitbuf;
extern int dispflg;

extern void make_crctable(void);
extern unsigned short calccrc(/*uchar *p, uint n*/);
extern void fillbuf(/*uchar n*/);
extern unsigned short getbits(/*uchar n*/);
extern void putcode(/*uchar n, ushort x*/);
extern void putbits(/*uchar n, ushort x*/);
extern int fread_crc(/*uchar *p, int n, FILE *f*/);
extern void fwrite_crc(/*uchar *p, int n, FILE *f*/);
extern void init_getbits(void);
extern void init_putbits(void);
