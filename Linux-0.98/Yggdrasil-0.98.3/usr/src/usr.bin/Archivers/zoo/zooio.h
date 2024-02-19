/* @(#) zooio.h 2.7 88/01/27 19:39:24 */

/*
Declarations for portable I/O

The contents of this file are hereby placed in the public domain.

											-- Rahul Dhesi 1988/01/24
*/
#ifndef	OK_STDIO
#include <stdio.h>
#define	OK_STDIO
#endif

#ifndef PARMS
#ifdef LINT_ARGS
#define	PARMS(x)		x
#else
#define	PARMS(x)		()
#endif
#endif

/*
In theory, all I/O using buffered files could be replaced with unbuffered
I/O simply by changing the following definitions.  This has not been tried
out yet, and there may be some remaining holes in the scheme.  On systems
with limited memory, it might prove necessary to use unbuffered I/O
only.
*/
typedef FILE *ZOOFILE;
/* try undefining this for LINUX */
#ifdef	LINUX
#undef	NOFILE
#endif	/* LINUX */
#define NOFILE		((ZOOFILE) 0)
#define NULLFILE	((ZOOFILE) -1)		/* or any unique value */
#define STDOUT		stdout

#ifdef FILTER
#define STDIN		stdin
#endif

#ifdef IO_MACROS
#define zooread(file, buffer, count)		fread (buffer, 1, count, file)
#define zoowrite(file, buffer, count) \
	(file == NULLFILE ? count : fwrite (buffer, 1, count, file))
#define zooseek(file, offset, whence)		fseek (file, offset, whence)
#define zootell(file)							ftell (file)
#else
int zooread PARMS((ZOOFILE, char *, int));
int zoowrite PARMS((ZOOFILE, char *, int));
long zooseek PARMS((ZOOFILE, long, int));
long zootell PARMS((ZOOFILE));
#endif /* IO_MACROS */

ZOOFILE zooopen PARMS((char *, char *));
ZOOFILE zoocreate PARMS((char *));
int zooclose PARMS((ZOOFILE));
int zootrunc PARMS((ZOOFILE));

char *choosefname PARMS((struct direntry *));
char *fullpath PARMS((struct direntry *));
int frd_zooh PARMS((struct zoo_header *, ZOOFILE));
int frd_dir PARMS((struct direntry *, ZOOFILE));
int fwr_dir PARMS((struct direntry *, ZOOFILE));
int fwr_zooh PARMS((struct zoo_header *, ZOOFILE));
int readdir PARMS((struct direntry *, ZOOFILE, int));
void rwheader PARMS((struct zoo_header *, ZOOFILE, int));
void newdir PARMS((struct direntry *));
void writedir PARMS((struct direntry *, ZOOFILE));
