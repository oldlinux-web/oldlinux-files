/*
 *   bin86/ld/type.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* type.h - types for linker */

typedef unsigned bool_pt;
typedef unsigned char bool_t;

typedef unsigned short u2_t;
typedef unsigned u2_pt;
typedef unsigned long u4_t;
typedef unsigned long u4_pt;

#ifdef OBJ_H			/* obj.h is included */

typedef unsigned flags_t;	/* unsigned makes shifts logical */

#ifdef LONG_OFFSETS
typedef unsigned long offset_t;
#else
typedef unsigned offset_t;
#endif

struct entrylist		/* list of entry symbols */
{
    struct entrylist *elnext;	/* next on list */
    struct symstruct *elsymptr;	/* entry on list */
};

struct modstruct		/* module table entry format */
{
    char *filename;		/* file containing this module */
    char *archentry;		/* name of component file for archives */
    char *modname;		/* name of module */
    long textoffset;		/* offset to module text in file */
    char class;			/* class of module */
    char loadflag;		/* set if module to be loaded */
    char segmaxsize[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for seg max size */
				/* 00 = 1, 01 = 2, 10 = 3, 11 = 4 */
    char segsizedesc[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for #bytes for size */
				/* 00 = 0, 01 = 1, 10 = 2, 11 = 4 */
    struct symstruct **symparray;	/* ^array of ptrs to referenced syms */
    struct modstruct *modnext;	/* next module in order of initial reading */
    char segsize[1];		/* up to 64 size bytes begin here */
};				/* careful with sizeof( struct modstruct )!! */

struct redlist			/* list of redefined (exported) symbols */
{
    struct redlist *rlnext;	/* next on list */
    struct symstruct *rlsymptr;	/* to symbol with same name, flags */
    struct modstruct *rlmodptr;	/* module for this redefinition */
    offset_t rlvalue;		/* value for this redefinition */
};

struct symstruct		/* symbol table entry format */
{
    struct modstruct *modptr;	/* module where symbol is defined */
    offset_t value;		/* value of symbol */
    flags_t flags;		/* see below (unsigned makes shifts logical) */
    struct symstruct *next;	/* next symbol with same hash value */
    char name[1];		/* name is any string beginning here */
};				/* don't use sizeof( struct symstruct )!! */

#endif				/* obj.h is included */

/* prototypes */

#if defined(PROTO) || __STDC__
#define P(x)	x
#else
#define P(x)	()
#endif

/* dump.c */
void dumpmods P((void));
void dumpsyms P((void));

/* io.c */
void ioinit P((char *progname));
void closein P((void));
void closeout P((void));
void errtrace P((char *name, int level));
void executable P((void));
void flusherr P((void));
void openin P((char *filename));
void openout P((char *filename));
void putstr P((char *message));
#ifdef OBJ_H
void put08x P((offset_t num));
void put08lx P((offset_t num));
#endif
void putbstr P((unsigned width, char *str));
void putbyte P((int ch));
int readchar P((void));
void readin P((char *buf, unsigned count));
bool_pt readineofok P((char *buf, unsigned count));
void seekin P((long offset));
void seekout P((long offset));
void seektrel P((long offset));
void writechar P((int c));
void writedrel P((char *buf, unsigned count));
void writeout P((char *buf, unsigned count));
void writetrel P((char *buf, unsigned count));
void fatalerror P((char *message));
void inputerror P((char *message));
void input1error P((char *message));
void outofmemory P((void));
void prematureeof P((void));
void redefined P((char *name, char *message, char *archentry,
		  char *deffilename, char *defarchentry));
void reserved P((char *name));
#ifdef OBJ_H
void size_error P((int seg, offset_t count, offset_t size));
#endif
void undefined P((char *name));
void usage P((void));
void use_error P((char *message));

/* ld.c */
int main P((int argc, char **argv));

/* readobj.c */
void objinit P((void));
void readsyms P((char *filename, bool_pt trace));
#ifdef OBJ_H
void entrysym P((struct symstruct *symptr));
offset_t readconvsize P((unsigned countindex));
offset_t readsize P((unsigned count));
unsigned segsizecount P((unsigned seg, struct modstruct *modptr));
#endif

/* table.c */
void syminit P((void));
struct symstruct *addsym P((char *name));
struct symstruct *findsym P((char *name));
char *moveup P((unsigned nbytes));
char *ourmalloc P((unsigned nbytes));
void ourfree P((char *cptr));
char *readstring P((void));
void release P((char *cptr));
char *stralloc P((char *s));

/* typeconvert.c */
u2_pt c2u2 P((char *buf));
u4_t c4u4 P((char *buf));
u2_pt cnu2 P((char *buf, unsigned count));
u4_t cnu4 P((char *buf, unsigned count));
void u2c2 P((char *buf, u2_pt offset));
void u4c4 P((char *buf, u4_t offset));
void u2cn P((char *buf, u2_pt offset, unsigned count));
void u4cn P((char *buf, u4_t offset, unsigned count));
bool_pt typeconv_init P((bool_pt big_endian, bool_pt long_big_endian));

/* writebin.c */
void writebin P((char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp));
void linksyms P((bool_pt argreloc_output));

/* library - fcntl.h */
#undef NULL
#include <fcntl.h>
/*
int creat P((const char *_path, int _mode));
int open P((const char *_path, int _oflag, ...));
*/

/* library - sys/stat.h */
#include <sys/stat.h>
#ifdef MY_STAT_H
int chmod P((const char *_path, int _mode));
int fstat P((int _fd, struct stat *_statbuf));
unsigned short umask P((int _oldmask));
				/* it should be mode_t but it's hard to
				 * decide which systems define it, and where */
#endif

/* library - stdlib.h */
#include <stdlib.h>
/*
void exit P((int _status));
void *malloc P((unsigned _nbytes));
unsigned long strtoul P((const char *_nptr, char **_endptr, int _base));
*/

/* library - string.h */
#include <string.h>
/*
void *memset P((void *_s, int _c, unsigned _nbytes));
char *strcat P((char *_target, const char *_source));
char *strchr P((const char *_s, int _ch));
int strcmp P((const char *_s1, const char *_s2));
char *strcpy P((char *_target, const char *_source));
unsigned strlen P((const char *_s));
char *strncat P((char *_target, const char *_source, unsigned _maxlength));
int strncmp P((const char *_s1, const char *_s2, unsigned _nbytes));
char *strncpy P((char *_target, const char *_source, unsigned _maxlength));
char *strrchr P((const char *_s, int _ch));
*/

/* library - unistd.h */
#include <unistd.h>
/*
int access P((char *_path, int _amode));
int close P((int _fd));
long lseek P((int _fd, long _offset, int _whence));
int read P((int _fd, char *_buf, unsigned _nbytes));
int write P((int _fd, char *_buf, unsigned _nbytes));
*/
