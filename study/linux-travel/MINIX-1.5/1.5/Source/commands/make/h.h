/*************************************************************************
 *
 *  m a k e :   h . h
 *
 *  include file for make
 *========================================================================
 * Edition history
 *
 *  #    Date                         Comments                       By
 * --- -------- ---------------------------------------------------- ---
 *   1    ??                                                         ??
 *   2 23.08.89 LZ increased,N_EXISTS added,suffix as macro added    RAL
 *   3 30.08.89 macro flags added, indention changed                 PSH,RAL
 *   4 03.09.89 fixed LZ eliminated, struct str added,...            RAL
 *   5 06.09.89 TABCHAR,M_MAKE added                                 RAL
 *   6 09.09.89 tos support added, EXTERN,INIT,PARMS added           PHH,RAL
 *   7 17.09.89 __STDC__ added, make1 decl. fixed , N_EXEC added     RAL
 * ------------ Version 2.0 released ------------------------------- RAL
 *
 *************************************************************************/

#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <utime.h>
#endif

#ifdef eon
#include <sys/stat.h>
#include <sys/err.h>
#endif

#ifdef os9
#include <time.h>
#include <os9.h>
#include <modes.h>
#include <direct.h>
#include <errno.h>
#endif

#ifdef tos
struct DOSTIME {short time,date; };     /* time structure of TOS */

#ifdef LATTICE
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <osbind.h>
#endif LATTICE

#ifdef TURBO
#include <tos.h>
#include <errno.h>
#include <string.h>
#endif TURBO

#endif tos

#include <ctype.h>
#include <stdio.h>

#ifdef eon
#define MNOENT ER_NOTF
#else
#define MNOENT ENOENT
#endif

#ifndef uchar
#ifdef os9
#define uchar  char
#define void   int
#define fputc  putc
#else
#define uchar  unsigned char
#endif
#endif

#define bool   uchar
#ifndef time_t
#define time_t long
#endif
#define TRUE   (1)
#define FALSE  (0)
#define max(a,b) ((a)>(b)?(a):(b))

#ifdef unix
#define DEFN1   "makefile"
#define DEFN2   "Makefile"
#endif
#ifdef eon
#define DEFN1   "makefile"
#define DEFN2   "Makefile"
#endif
#ifdef tos
#define DEFN1   "makefile."
#define DEFN2   (char *)0
#endif
#ifdef os9
#define DEFN1   "makefile"
#define DEFN2   (char *)0
#endif


#ifdef os9
#define TABCHAR ' '
#else
#define TABCHAR '\t'
#endif

#define LZ1	(2048)		/*  Initial input/expand string size  */
#define LZ2	(256)		/*  Initial input/expand string size  */



/*
 *	A name.  This represents a file, either to be made, or existant
 */

struct name
{
  struct name  *n_next;		/* Next in the list of names */
  char         *n_name;		/* Called */
  struct line  *n_line;		/* Dependencies */
  time_t        n_time;		/* Modify time of this name */
  uchar         n_flag;		/* Info about the name */
};

#define N_MARK    0x01			/* For cycle check */
#define N_DONE    0x02			/* Name looked at */
#define N_TARG    0x04			/* Name is a target */
#define N_PREC    0x08			/* Target is precious */
#define N_DOUBLE  0x10			/* Double colon target */
#define N_EXISTS  0x20			/* File exists */
#define N_ERROR   0x40			/* Error occured */
#define N_EXEC    0x80			/* Commands executed */

/*
 *	Definition of a target line.
 */
struct	line
{
  struct line    *l_next;		/* Next line (for ::) */
  struct depend  *l_dep;		/* Dependents for this line */
  struct cmd     *l_cmd;		/* Commands for this line */
};


/*
 *	List of dependents for a line
 */
struct	depend
{
  struct depend  *d_next;		/* Next dependent */
  struct name    *d_name;		/* Name of dependent */
};


/*
 *	Commands for a line
 */
struct	cmd
{
  struct cmd  *c_next;		/* Next command line */
  char        *c_cmd;		/* Command line */
};


/*
 *	Macro storage
 */
struct	macro
{
  struct macro *m_next;	/* Next variable */
  char *m_name;		/* Called ... */
  char *m_val;		/* Its value */
  uchar m_flag;		/* Infinite loop check */
};


#define M_MARK		0x01	/* for infinite loop check */
#define M_OVERRIDE	0x02	/* command-line override */
#define M_MAKE		0x04	/* for MAKE macro */

/*
 *	String
 */
struct	str
{
  char **ptr;		/* ptr to real ptr. to string */
  int    len;		/* length of string */
  int    pos;		/* position */
};


/* Declaration, definition & initialization of variables */

#ifndef EXTERN
#define EXTERN extern
#endif

#ifndef INIT
#define INIT(x)
#endif

extern int    errno;
extern char **environ;

EXTERN char *myname;
EXTERN bool  domake   INIT(TRUE);  /*  Go through the motions option  */
EXTERN bool  ignore   INIT(FALSE); /*  Ignore exit status option      */
EXTERN bool  conterr  INIT(FALSE); /*  continue on errors  */
EXTERN bool  silent   INIT(FALSE); /*  Silent option  */
EXTERN bool  print    INIT(FALSE); /*  Print debuging information  */
EXTERN bool  rules    INIT(TRUE);  /*  Use inbuilt rules  */
EXTERN bool  dotouch  INIT(FALSE); /*  Touch files instead of making  */
EXTERN bool  quest    INIT(FALSE); /*  Question up-to-dateness of file  */
EXTERN bool  useenv   INIT(FALSE); /*  Env. macro def. overwrite makefile def.*/
EXTERN bool  dbginfo  INIT(FALSE); /*  Print lot of debugging information */
EXTERN bool  ambigmac INIT(FALSE); /*  guess undef. ambiguous macros (*,<) */
EXTERN struct name  *firstname;
EXTERN char         *str1;
EXTERN char         *str2;
EXTERN struct str    str1s;
EXTERN struct str    str2s;
EXTERN struct name **suffparray; /* ptr. to array of ptrs. to name chains */
EXTERN int           sizesuffarray INIT(20); /* size of suffarray */
EXTERN int           maxsuffarray INIT(0);   /* last used entry in suffarray */
EXTERN struct macro *macrohead;
EXTERN bool          expmake; /* TRUE if $(MAKE) has been expanded */
EXTERN int           lineno;

#ifdef tos
#ifdef LATTICE
EXTERN int _mneed INIT(60000);    /* VERY important for TOS with LATTICE C*/
#endif LATTICE
#endif tos
#ifdef eon
#define MEMSPACE  (16384)
EXTERN unsigned  memspace = MEMSPACE;
#endif

/* Defines function declarations for all functions */

#ifdef __STDC__
#define CONST const
#define LINT_ARGS
#else
#define CONST /* */
#endif

#ifndef PARMS
#ifdef  LINT_ARGS
#define PARMS(x)   x
#else
#define PARMS(x)   ()
#endif
#endif

/*
:.,$s/(PARMS\(.*\));/PARMS\1;/
*/

extern time_t  time PARMS(( time_t *));
extern char   *ctime PARMS (( CONST time_t *));
extern char   *getenv PARMS (( CONST char *));
extern char   *fgets PARMS (( char *, int, FILE *));
extern char   *strchr PARMS (( CONST char *, int));
extern char   *strrchr PARMS (( CONST char *, int));
#ifndef __STDC__
extern char   *malloc PARMS (( unsigned));
extern char   *realloc PARMS (( char *, unsigned)); /* OS9 ? */
#else
extern void   *malloc PARMS (( unsigned));
extern void   *realloc PARMS (( void *, unsigned)); /* OS9 ? */
#endif

/* main.c */
void           main PARMS (( int, char **));
void           setoption PARMS ((char));
void           usage PARMS (());
void           fatal PARMS (( char *, char *, int));
/* check.c */
void           prt PARMS (());
void           check PARMS (( struct name *));
void           circh PARMS (());
void           precious PARMS (());
/* input.c */
void           init PARMS (());
void           strrealloc PARMS (( struct str *));
struct name   *newname  PARMS (( char *));
struct name   *testname PARMS (( char *));
struct depend *newdep PARMS (( struct name *, struct depend *));
struct cmd    *newcmd PARMS (( char *, struct cmd *));
void           newline PARMS (( struct name *, struct depend *, struct cmd *, int));
void           input PARMS (( FILE *));
/* macro.c */
struct macro  *getmp PARMS (( char *));
char          *getmacro PARMS (( char *));
struct macro  *setmacro PARMS (( char *, char *));
void           setDFmacro PARMS (( char *, char *));
void           doexp PARMS (( struct str *, char *));
void           expand PARMS (( struct str *));
/* make.c */
int            dosh PARMS (( char *, char *));
void           docmds1 PARMS (( struct name *, struct line *));
void           docmds PARMS (( struct name *));
#ifdef tos
int            Tosexec PARMS (( char *));
time_t         mstonix PARMS (( struct DOSTIME *));
#endif
#ifdef os9
void           getmdate PARMS (( int, struct sgtbuf *));
time_t         cnvtime PARMS (( struct sgtbuf *));
void           time PARMS (( time_t *));
#endif
void           modtime PARMS (( struct name *));
void           touch PARMS (( struct name *));
int            make PARMS (( struct name *, int));
void           make1 PARMS (( struct name *, struct line *, struct depend *,char *, char *));
void           implmacros PARMS (( struct name *, struct line *, char **,char **));
void           dbgprint PARMS (( int, struct name *, char *));
/* reader.c */
void           error PARMS (( char *, char *));
bool           getline PARMS (( struct str *, FILE *));
char          *gettok PARMS (( register char **));
/* rules.c */
bool           dyndep PARMS (( struct name  *, char **, char **));
void           makerules PARMS (());


/*
 *	Return a pointer to the suffix of a name
 */
#define  suffix(name)   strrchr(name,(int)'.')

EXTERN int _ctypech;
#define mylower(x)  (islower(_ctypech=(x)) ? _ctypech :tolower(_ctypech))
#define myupper(x)  (isupper(_ctypech=(x)) ? _ctypech :toupper(_ctypech))

