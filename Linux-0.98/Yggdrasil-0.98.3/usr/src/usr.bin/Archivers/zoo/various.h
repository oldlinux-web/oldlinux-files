/* @(#) various.h 2.3 87/12/27 14:44:34 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/

/*
This files gives definitions for most external functions used by Zoo.
If ANSI_PROTO is defined, ANSI-style function prototypes are used, else
normal K&R function declarations are used.

Note:  Always precede this file with an include of stdio.h because it uses
the predefined type FILE.
*/

#ifndef PARMS
#ifdef ANSI_PROTO
#define	PARMS(x)		x
#else
#define	PARMS(x)		()
#endif
#endif

#ifdef ANSI_HDRS /* if not defined in stdio.h */
# include <string.h>
# include <stdlib.h>
#else
FILE *fdopen PARMS ((int, char *));
FILE *fopen PARMS ((char *, char *));
char *fgets PARMS ((char *, int, FILE *));
char *gets PARMS ((char *));
VOIDPTR malloc PARMS ((unsigned int));
VOIDPTR realloc PARMS ((char *, unsigned int));
char *strcat PARMS ((char *, char *));
char *strchr PARMS ((char *, int));
char *strcpy PARMS ((char *, char *));
char *strncat PARMS ((char *, char *, unsigned int));
char *strncpy PARMS ((char *, char *, unsigned int));
char *strrchr PARMS ((char *, int));
int fclose PARMS ((FILE *));
int fflush PARMS ((FILE *));
int fgetc PARMS ((FILE *));
int fgetchar PARMS (());
int fprintf PARMS ((FILE *, char *, ...));
int fputchar PARMS ((int));
int fputs PARMS ((char *, FILE *));

#ifndef NO_STDIO_FN
# ifdef ALWAYS_INT
int fputc PARMS ((int, FILE *));
int fread PARMS ((VOIDPTR, int, int, FILE *));
int fwrite PARMS ((VOIDPTR, int, int, FILE *));
# else
int fputc PARMS ((char, FILE *));
int fread PARMS ((VOIDPTR, unsigned, unsigned, FILE *));
int fwrite PARMS ((VOIDPTR, unsigned, unsigned, FILE *));
# endif /* ALWAYS_INT */
#endif /* NO_STDIO_FN */

int fseek PARMS ((FILE *, long, int));
int printf PARMS ((char *, ...));
int rename PARMS ((char *, char *));
int setmode PARMS ((int, int));
int strcmp PARMS ((char *, char *));
int strncmp PARMS ((char *, char *, unsigned int));
int unlink PARMS ((char *));
long ftell PARMS ((FILE *));
unsigned int strlen PARMS ((char *));

#endif /* ! ANSI_HDRS */

