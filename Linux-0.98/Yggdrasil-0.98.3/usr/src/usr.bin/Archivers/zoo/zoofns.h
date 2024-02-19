/* @(#) zoofns.h 2.5 88/01/16 19:03:13 */
/* @(#) zoofns.h 2.7 88/01/27 19:39:18 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/

/* Defines function declarations for all Zoo functions */

#ifndef PARMS
#ifdef LINT_ARGS
#define	PARMS(x)		x
#else
#define	PARMS(x)		()
#endif
#endif

/* 
:.,$s/(PARMS\(.*\));/PARMS\1;/
*/
#ifdef ANSI_HDRS
#include <stdlib.h>
#else
char *memset PARMS ((char *, int, unsigned));
#endif /* ANSI_HDRS */

long calc_ofs PARMS ((char *));
char *addext PARMS ((char *, char *));
char *combine PARMS ((char[], char *, char *));
VOIDPTR emalloc PARMS ((unsigned int));
VOIDPTR ealloc PARMS ((unsigned int));
VOIDPTR erealloc PARMS ((VOIDPTR, unsigned int));
char *findlast PARMS ((char *, char *));
char *fixfname PARMS ((char *));
char *getstdin PARMS ((void));
char *lastptr PARMS ((char *));
char *nameptr PARMS ((char *));
char *newcat PARMS ((char *, char *));
char *nextfile PARMS ((int, char *, int));
int cfactor PARMS ((long, long));
int chname PARMS ((char *, char *));
int cmpnum PARMS ((unsigned int, unsigned int, unsigned int, unsigned int));
T_SIGNAL ctrl_c PARMS ((void));
int exists PARMS ((char *));
int getfile PARMS ((ZOOFILE, ZOOFILE, long, int));
int getutime PARMS ((char *, unsigned *, unsigned *));
int gettime PARMS ((ZOOFILE, unsigned *, unsigned *));
T_SIGNAL handle_break PARMS ((void));

#ifdef USE_ASCII
int isupper PARMS ((int));
int isdigit PARMS ((int));
#endif /* USE_ASCII */

int kill_files PARMS ((char *[], int));
#ifdef UNBUF_IO
int lzc PARMS ((int, int));
int lzd PARMS ((int, int));
#else
int lzc PARMS ((ZOOFILE, ZOOFILE));
int lzd PARMS ((ZOOFILE, ZOOFILE));
#endif

int lzh_encode PARMS((FILE *infile, FILE *outfile));
int lzh_decode PARMS((FILE *infile, FILE *outfile));

int match_half PARMS ((char *, char *));
int samefile PARMS ((char *, char *));
int settime PARMS ((ZOOFILE, unsigned, unsigned));
int setutime PARMS ((char *, unsigned, unsigned));
int str_icmp PARMS ((char *, char *));

#ifdef USE_ASCII
int tolower PARMS ((int));
int toascii PARMS ((int));
#endif /* USE_ASCII */

void zooexit PARMS ((int));
long inlist PARMS ((char *, unsigned int *, unsigned int *, unsigned *,
					unsigned *, unsigned *, long *, int));
unsigned long space PARMS ((int, int *));
void addbfcrc PARMS ((char *, int));
void addfname PARMS ((char *, long, unsigned int, unsigned int, 
							unsigned, unsigned));
void add_version PARMS ((char *, struct direntry *));
void basename PARMS ((char *, char []));
void break_off PARMS ((void));
void close_file PARMS ((ZOOFILE));
void comment PARMS ((char *, char *));
void extension PARMS ((char *, char []));
void exit PARMS ((int));
void fixslash PARMS ((char *));
void makelist PARMS ((int, char *[], char *[], int, char *, char *, char *, int *));
void memerr PARMS ((unsigned int));
void prterror PARMS ((int, char *, ...));
void rootname PARMS ((char *, char *));
void skip_files PARMS ((ZOOFILE, unsigned int *, unsigned int *, int *,
                  char [], long *));
void writenull PARMS ((ZOOFILE, int));
void zooadd PARMS ((char *, int, char **, char *));
void zoodel PARMS ((char *, char *, int));
void zoofilt PARMS ((char *));
void zooext PARMS ((char *, char *));
void zoolist PARMS ((char **, char *, int));
void zoopack PARMS ((char *, char *));

char *str_dup PARMS ((char *));
char *str_lwr PARMS ((char *));

