/* ship.c -- Not copyrighted 1991 Mark Adler */

#define SHIPVER "ship version 1.1  November 4, 1991  Mark Adler"

/* Command for mailing (-m): the %s's are, in order, the subject prefix,
   the part number (always of the form "partnnnn"), the subject suffix
   (empty or " (last)" if the last part), the mailing address, and the
   name of the temporary file begin mailed.  The command "Mail" is for BSD
   systems.  You may need to change it to "mailx" for System V Unix, using
   the compile option "-DMAILX".  Also, on Sperry (Unisys?) SysV.3 systems,
   you might try the command name "v6mail". */

#ifdef DIRENT           /* If compiled with zip, DIRENT implies System V */
#  define MAILX
#endif /* DIRENT */

#ifdef sun              /* Except Sun's use DIRENT, but have Mail */
#  ifdef MAILX
#    undef MAILX
#  endif /* MAILX */
#endif /* sun */

#ifdef sgi              /* Silicon Graphics that way too */
#  ifdef MAILX
#    undef MAILX
#  endif /* MAILX */
#endif /* sgi */

#ifdef VMS
#  define TMPNAME "_SXXXXXX."
#  define MAILCMD "mail %s /subj=\"%s %s%s\" \"%s\""
#  define PATHCUT ']'
#else /* !VMS */
#  define TMPNAME "_SXXXXXX"
#  ifdef MAILX
#    define MAILCMD "mailx -s \"%s %s%s\" \"%s\" < %s"
#  else /* !MAILX */
#    ifdef M_XENIX
#      define MAILCMD "mail -s \"%s %s%s\" \"%s\" < %s"
#    else /* !M_XENIX */
#      define MAILCMD "Mail -s \"%s %s%s\" \"%s\" < %s"
#    endif /* ?M_XENIX */
#  endif /* ?MAILX */
#  ifdef MSDOS
#    define PATHCUT '\\'
#  else /* !MSDOS */
#    define PATHCUT '/'
#  endif /* ?MSDOS */
#endif /* ?VMS */

/*

SHIP -

  Ship is a program for sending binary files through email.  It is designed
  to supplant uuencode and uudecode.  Ship encodes approximately 6.32 bits
  per character mailed, compared to uuencode's 5.81 bits per character.

  Ship also has these features: a 32-bit CRC check on each file; automatic
  splitting of the ship output into multiple, smaller files for speedier
  mailing; automatic mailing of ship's output, with subject lines for
  multiple parts; and a check on the sequence of parts when unshipping.

  Usage:

       ship [-nnn] [-m address] [-s subject] file ...

  where nnn is the maximum number of K bytes for each output file, address
  is the address to send mail to, subject is a Subject" line prefix, and
  file ... is a list of files to ship.  If no options are given, ship
  outputs to stdout.  The simplest use is:

       ship foo > x

  where foo is converted into the mailable file, x.

  When -nnn is specified, but -m is not, ship writes to the files
  part0001, part0002, etc., where each file has nnn or less K bytes.  For
  example:

       ship -25 bigfoo

  will write however many 25K byte or less ship files is needed to contain
  bigfoo.  If, say, six files are needed, then the files part0001 to part0006
  will be written.

  When using -m, nothing is written, either to files or to stdout; rather,
  the output is mailed to the specified address.  If -nnn is also specified,
  then the parts are mailed separately with the subject lines part0001, etc.
  If -nnn is not specified, then only one part (the whole thing) is mailed
  with the subject line "part0001".  For example:

       ship -25 -m fred bigfoo

  will mail the six parts of bigfoo to fred.

  Any number of files can be shipped at once.  They become part of one long
  ship stream, so if, for example -25 is specified, all but the last part
  will have about 25K bytes.  For example:

       ship -25 -m fred fee fi fo fum

  will send the files fee, fi, fo, and fum to fred.

  Fred will get several mail messages with the subject lines part0001, etc.
  He can then save those messages as the files, say, p1, p2, p3, ...
  Then he can use the command:

       ship -u p?

  to recreate bigfoo, or fee fi fo and fum, depending on what he was sent.
  If Fred saved the wrong numbers, ship will detect this and report a
  sequence error.

  Note: there is enough information in the shipped parts to determine the
  correct sequence.  A future version of ship will prescan the files to
  determine the sequence, and then process them in the correct order.

  If a file being received already exists, ship -u will report an error
  and exit.  The -o option avoids this and allows ship to overwrite existing
  files.  The -o option must follow the -u option:

       ship -u -o p?

  In addition to the -u option, ship will unship if it sees that its name is
  unship.  On Unix systems, this can be done simply by linking the executable
  to unship:

       ln ship unship 

  Ship can also be used as a filter.  The special file name "-" means stdin.
  For example:

       tar covf - foodir | compress | ship -25 -m fred -

  will tar the directory foodir, compress it, and ship it to fred in 25K byte
  pieces.  Then, after Fred saves the files as p01, etc. at the other, end,
  he can:

       ship -u p? | zcat | tar xovf -

  which will recreate the directory foobar and its contents.  ship -u knows
  to write to stdout, since the original ship put the special file name "-"
  in the first part.

  Ship uses a base 85 coding that needs 32-bit multiplication and division.
  This can be slow on 16-bit machines, so ship provides a fast encoding
  method by specifying the -f option.  This method is somewhat faster even
  on 32-bit machines, and has approximately a 1% penalty in the size of the
  encoded result (-f gives 6.26 bits per character, on the average).  The -f
  option need only be used when shipping--unshipping (ship -u) automatically
  detects the encoding used.  For example:

       ship -f -25 -m fred foo

  will send foo to fred in 25K byte pieces using the fast encoding method.
  You don't need to tell Fred, since ship -u will figure that out for him.

  The fast encoding method is probabilistic, so it's possible for the size
  penalty to be worse than 1%, and it's also possible for the fast encoding
  to produce a smaller result than base 85 encoding would, all depending on
  the data.

  The -q option can be used with either ship or unship (ship -u) for quiet
  operation--informational messages are inhibited.

  You can find out the version of ship and get the command usage by using
  "ship -h" or "ship -?".  The version number and date and help will be
  printed, and ship will exit (the rest of the command line is ignored).

  Acknowledgements:

  The hard-arithmetic coding algorithm was blatantly stolen from Peter
  Gutmann's pgencode/pgdecode programs posted on comp.compression, with
  modifications to use 86 instead of 94 characters, and to make zeros encode
  better than, rather than worse than other bytes.  (As Stravinsky once said:
  "Mediocre composers plagiarize.  Great composers steal.")

*/

/* tailor.h -- Not copyrighted 1991 Mark Adler */

/* const's are inconsistently used across ANSI libraries--kill for all
   header files. */
#define const


/* Use prototypes and ANSI libraries if __STDC__ */
#ifdef __STDC__
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* __STDC__ */


/* Use prototypes and ANSI libraries if Silicon Graphics */
#ifdef sgi
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* sgi */


/* Define MSDOS for Turbo C as well as Microsoft C */
#ifdef __POWERC                 /* For Power C too */
#  define __TURBOC__
#endif /* __POWERC */
#ifdef __TURBOC__
#  ifndef MSDOS
#    define MSDOS
#  endif /* !MSDOS */
#endif /* __TURBOC__ */


/* Use prototypes and ANSI libraries if Microsoft or Borland C */
#ifdef MSDOS
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* MSDOS */


/* Turn off prototypes if requested */
#ifdef NOPROTO
#  ifdef PROTO
#    undef PROTO
#  endif /* PROTO */
#endif /* NOPROT */


/* Used to remove arguments in function prototypes for non-ANSI C */
#ifdef PROTO
#  define OF(a) a
#else /* !PROTO */
#  define OF(a) ()
#endif /* ?PROTO */


/* Allow far and huge allocation for small model (Microsoft C or Turbo C) */
#ifdef MSDOS
#  ifdef __TURBOC__
#    include <alloc.h>
#  else /* !__TURBOC__ */
#    include <malloc.h>
#    define farmalloc _fmalloc
#    define farfree   _ffree
#  endif /* ?__TURBOC__ */
#else /* !MSDOS */
#  define huge
#  define far
#  define near
#  define farmalloc malloc
#  define farfree   free
#endif /* ?MSDOS */


/* Define MSVMS if either MSDOS or VMS defined */
#ifdef MSDOS
#  define MSVMS
#else /* !MSDOS */
#  ifdef VMS
#    define MSVMS
#  endif /* VMS */
#endif /* ?MSDOS */


/* Define void, voidp, and extent (size_t) */
#include <stdio.h>
#ifdef MODERN
#  if !defined(M_XENIX) && !(defined(__GNUC__) && defined(sun))
#    include <stddef.h>
#  endif
#  include <stdlib.h>
   typedef size_t extent;
   typedef void voidp;
#else /* !MODERN */
   typedef unsigned int extent;
#  define void int
   typedef char voidp;
#endif /* ?MODERN */

/* Get types and stat */
#ifdef VMS
#  include <types.h>
#  include <stat.h>
#else /* !VMS */
#  include <sys/types.h>
#  include <sys/stat.h>
#endif /* ?VMS */


/* Cheap fix for unlink on VMS */
#ifdef VMS
#  define unlink delete
#endif /* VMS */


/* For Pyramid */
#ifdef pyr
#  define strrchr rindex
#  define ZMEM
#endif /* pyr */


/* File operations--use "b" for binary if allowed */
#ifdef MODERN
#  define FOPR "rb"
#  define FOPM "r+b"
#  define FOPW "w+b"
#else /* !MODERN */
#  define FOPR "r"
#  define FOPM "r+"
#  define FOPW "w+"
#endif /* ?MODERN */


/* Fine tuning */
#ifndef MSDOS
#   define BSZ 8192   /* Buffer size for files */
#else /* !MSDOS */
#   define BSZ 4096   /* Keep precious NEAR space */
    /* BSZ can't be 8192 even for compact model because of 64K limitation
     * in im_lmat.c. If you run out of memory when processing a large number
     * files, use the compact model and reduce BSZ to 2048 here and in
     * im_lm.asm.
     */
#endif /* ?MSDOS */

/* end of tailor.h */

#ifdef MODERN
#  include <string.h>
#else /* !MODERN */
   voidp *malloc();
   long atol();
   char *strcpy();
   char *strrchr();
#endif /* ?MODERN */

/* Library functions not in (most) header files */
char *mktemp OF((char *));
int unlink OF((char *));

#ifdef MSDOS            /* Use binary mode for binary files */
#  include <io.h>
#  include <fcntl.h>
#endif /* MSDOS */


#define LNSZ 1025       /* size of line buffer */

typedef unsigned long ulg;      /* 32-bit unsigned integer */

typedef struct {        /* associates a CRC with a file */
  FILE *f;              /* pointer to associated file stream */
  ulg c;                /* CRC register */
  ulg b;                /* four byte buffer */
  int n;                /* buffer count */
} cfile;


/* Function prototypes */
#ifdef MODERN
   void err(int, char *);
   cfile *chook(FILE *);
   char *nopath(char *);
   void newship(void);
   void endship(int);
   void newline(char *);
   void ship(char *, FILE *);
   void mkinv(void);
   void decode(unsigned char *, cfile *);
   void unship(char **, int, int);
   void help(void);
   void main(int, char **);
#endif /* MODERN */



/* Globals for ship() */
char sname[9];          /* current ship file name */
FILE *sfile;            /* current ship file */
ulg slns;               /* number of lines written to ship file */
ulg slmax;              /* maximum number of lines per ship file */
int fast;               /* true for arithmetic coding, else base 85 */
int mail;               /* true if mailing */
char mpspc[9];          /* prealloced space for prefix */
char *mprefix = mpspc;  /* identification for this mailing */
char *mdest;            /* mail destination */
char mname[10];         /* temporary file name if mailing */
ulg ccnt;               /* count of bytes read or written */
int noisy = 1;          /* false to inhibit informational messages */



/* Errors */
#define SE_ARG 1
#define SE_FIND 2
#define SE_NONE 3
#define SE_PART 4
#define SE_FORM 5
#define SE_CONT 6
#define SE_CRC 7
#define SE_MAIL 8
#define SE_OVER 9
#define SE_FULL 10
#define SE_MANY 11
#define SE_MEM 12
char *errors[] = {
  /* 1 */ "invalid argument ",
  /* 2 */ "could not find ",
  /* 3 */ "no files received",
  /* 4 */ "unfinished file ",
  /* 5 */ "invalid ship format in ",
  /* 6 */ "wrong sequence for ",
  /* 7 */ "CRC check failed on ",
  /* 8 */ "mail command failed: ",
  /* 9 */ "attempted to overwrite ",
  /* 10 */ "could not write to ",
  /* 11 */ "too many output files!",
  /* 12 */ "out of memory"
};


/* Set of 86 characters used for the base 85 digits (last one not used), and
   the 86 character arithmetic coding.  Selected to be part of both the ASCII
   printable characters, and the common EBCDIC printable characters whose
   ASCII translations are universal. */
unsigned char safe[] = {
        '{','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
        '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@',
        'A','B','C','D','E','F','G','H','I','J','K','L','M',
        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z','_',
        'a','b','c','d','e','f','g','h','i','j','k','l','m',
        'n','o','p','q','r','s','t','u','v','w','x','y','z','}'};

#define LOWSZ (sizeof(safe)-64)         /* low set size for fast coding */

/* Special replacement pairs--if first of each pair is received, it is
   treated like the second member of the pair.  You're probably
   wondering why.  The first pair is for compatibility with an
   earlier version of ship that used ! for the base 85 zero digit.
   However, there exist ASCII-EBCDIC translation tables that don't
   know about exclamation marks.  The second set has mysterious
   historical origins that are best left unspoken ... */
unsigned char aliases[] = {'!','{','|','+',0};

/* Inverse of safe[], filled in by mkinv() */
unsigned char invsafe[256];

/* Table of CRC-32's of all single byte values (made by makecrc.c) */
ulg crctab[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

/* Macro to update the CRC shift register one byte at a time */
#define CRC(c,b) (crctab[((int)(c)^(int)(b))&0xff]^((c)>>8))


char *errname = "ship error";
char *warname = "ship warning";

void err(n, m)
int n;                  /* error number */
char *m;                /* additional error information */
{
  if (n == SE_FIND || n == SE_FULL)
    perror(errname);
  fputs(errname, stderr);
  fputs(": ", stderr);
  fputs(errors[n - 1], stderr);
  fputs(m, stderr);
  putc('\n', stderr);
  if (*mname)
    unlink(mname);
#ifdef VMS
  exit(0);
#else /* !VMS */
  exit(n);
#endif /* ?VMS */
}


cfile *chook(f)
FILE *f;                /* file stream */
/* Inherit the file stream structure and add a CRC and buffer for appending
   a CRC on reads and checking the CRC on writes.  Return a pointer to the
   cfile structure, or NULL if the malloc() failed.  Also, if MSDOS, set the
   file mode to binary to avoid LF<->CRLF conversions. */
{
  cfile *c;             /* allocated cfile structure */

#ifdef MSDOS
  /* Set file mode to binary for MSDOS systems */
  setmode(fileno(f), O_BINARY);
#endif /* MSDOS */

  /* Allocate and fill structure */
  if ((c = (cfile *)malloc(sizeof(cfile))) != NULL)
  {
    c->f = f;                           /* file stream */
    c->b = 0;                           /* empty fifo (for output) */
    c->c = 0xffffffffL;                 /* preload CRC register */
    c->n = 0;                           /* fifo is empty (output) or */
  }                                     /*  no CRC bytes given (input) */
  return c;
}



/* cgetc(x)--like getc(f), but appends a 32-bit CRC to the end of the stream.
   Return the byte read (the last four of which will be the CRC) or EOF. */
#define cgete(x) (x->n==4?EOF:(x->c=x->n++?x->c>>8:~x->c,(int)x->c&0xff))
#define cgetc(x) (x->n==0&&(b=getc(x->f))!=EOF?(ccnt++,x->c=CRC(x->c,b),b):cgete(c))


/* cputc(d,x)--like putc(d,f), but delays four bytes and computes a CRC.
   x is a cfile *, and d is expected to be an ulg. */
#define cputf(x) (int)(x->c=CRC(x->c,x->b),putc((int)x->b&0xff,x->f),ccnt++)
#define cputc(d,x) (x->n!=4?x->n++:cputf(x),x->b=(x->b>>8)+((ulg)(d)<<24))


char *nopath(p)
char *p;                /* full file name */
/* Extract just the name of file--remove and subdirectories or devices */
{
#ifdef MSDOS
  char *q = "/\\:";     /* MSDOS delimiters */
#else /* !MSDOS */
#ifdef VMS
  char *q = "]:";       /* VMS delimiters */
#else /* !VMS */
  char *q = "/";        /* Unix delimiter */
#endif /* ?VMS */
#endif /* ?MSDOS */
  char *r;              /* result of strrchr() */

  while (*q)
    if ((r = strrchr(p, *q++)) != NULL)
      p = r + 1;
  return p;
}


void newship()
/* Open up a new ship file to write to */
{
  int i;                /* scans down name to increment */

  for (i = 7; i > 3; i--)
    if (++sname[i] > '9')
      sname[i] = '0';
    else
      break;
  if (i == 3)
    err(SE_MANY, "");
  if ((sfile = fopen(mail ? mktemp(strcpy(mname, TMPNAME)) : sname,
                     "w")) == NULL)
    err(SE_FULL, mail ? mname : sname);
  slns = 0;
}


void endship(e)
int e;          /* true if ending the last ship file */
/* Finish off current ship file */
{
  char *s;              /* malloc'd space for mail command */

  if (ferror(sfile) || fclose(sfile))
    err(SE_FULL, mail ? mname : sname);
  if (mail)
  {
    if ((s = malloc(strlen(MAILCMD)- 5*2 + strlen(mprefix) + strlen(sname) +
                    (e ? 7 : 0) + strlen(mdest) + strlen(mname) + 1)) == NULL)
      err(SE_MEM, "");
#ifdef VMS
    sprintf(s, MAILCMD, mname, mprefix, sname, e ? " (last)" : "", mdest);
    if (!system(s))             /* this string fits on one line */
      err(SE_MAIL, "system() call is not supported on this machine");
#else /* !VMS */
    sprintf(s, MAILCMD, mprefix, sname, e ? " (last)" : "", mdest, mname);
    if (system(s))
      err(SE_MAIL, s);
#endif /* ?VMS */
    free((voidp *)s);
    unlink(mname);
    *mname = 0;
  }
}


void newline(p)
char *p;                /* name of the input file */
/* Add a new line inside a ship file, possibly cut the file */
{
  putc('\n', sfile);
  slns++;
  if (slmax && slns >= slmax - 2)
  {
    putc('$', sfile);
    if (fast)
      fputs(" f", sfile);
    fputs("\nmore\n", sfile);
    endship(0);
    newship();
    fprintf(sfile, "$%s\ncont %lu %s\n", fast ? " f" : "", ccnt, nopath(p));
    slns += 2;
  }
}


/* Macro to avoid leading dots.  It assumes i==0 at the beginning of a line
   and that b is an available int.  c is only evaluated once. */
#define sputc(c,f) (i==0?((b=(c))=='.'?putc(' ',f):0,putc(b,f)):putc(c,f))


void ship(p, f)
char *p;                /* name of the input file */
FILE *f;                /* input file */
/* Encode the binary file f. */
{
  int b;                /* character just read */
  cfile *c;             /* checked file stream */
  int i;                /* how much is written on line so far */
  int j;                /* how much is in bit buffer */

  /* Set up output file if needed */
  if ((mail || slmax) && sfile == stdout)
  {
    strcpy(sname, "part0000");
    newship();
  }

  /* Write header */
  if ((c = chook(f)) == NULL)
    err(SE_MEM, "");
  ccnt = 0;
  if (slmax && slns >= slmax - 5) 
  {
    endship(0);
    newship();
  }
  fprintf(sfile, "$%s\nship %s\n", fast ? " f" : "", nopath(p));
  slns += 2;

  /* Encode the file, writing to sfile */
  if (fast)
  {
    int d;              /* accumulates bits (never more than 14) */

    d = j = i = 0;
    while ((b = cgetc(c)) != EOF)
    {
      d |= b << j;
      j += 8;
      if ((d & 0x3f) >= LOWSZ)
      {
        sputc((int)(safe[(d & 0x3f) + LOWSZ]), sfile);
        d >>= 6;
        j -= 6;
      }
      else
      {
        sputc((int)(safe[(d & 0x3f) + (d & 0x40 ? LOWSZ : 0)]), sfile);
        d >>= 7;
        j -= 7;
      }
      if (++i == 79)
      {
        newline(p);
        i = 0;
      }
      if (j >= 6 && (d & 0x3f) >= LOWSZ)
      {
        sputc((int)(safe[(d & 0x3f) + LOWSZ]), sfile);
        d >>= 6;
        j -= 6;
        if (++i == 79)
        {
          newline(p);
          i = 0;
        }
      }
      else if (j >= 7)
      {
        sputc((int)(safe[(d & 0x3f) + (d & 0x40 ? LOWSZ : 0)]), sfile);
        d >>= 7;
        j -= 7;
        if (++i == 79)
        {
          newline(p);
          i = 0;
        }
      }
    }
    free((voidp *)c);

    /* Write leftover bits */
    if (j)
    {
      sputc((int)(safe[d + (d < LOWSZ ? 0 : LOWSZ)]), sfile);
      putc('\n', sfile);
      slns++;
    }
    else if (i)
    {
      putc('\n', sfile);
      slns++;
    }
  }
  else
  {
    ulg d;              /* accumulates bytes */

    d = j = i = 0;
    while ((b = cgetc(c)) != EOF)
    {
      d += ((ulg)b) << j;
      if ((j += 8) == 32)
      {
        sputc((int)(safe[(int)(d % 85)]), sfile);  d /= 85;
        putc((int)(safe[(int)(d % 85)]), sfile);  d /= 85;
        putc((int)(safe[(int)(d % 85)]), sfile);  d /= 85;
        putc((int)(safe[(int)(d % 85)]), sfile);  d /= 85;
        putc((int)(safe[(int)d]), sfile);
        if (++i == 15)                  /* each line is <= 75 characters */
        {
          newline(p);
          i = 0;
        }
        d = j = 0;
      }
    }
    free((voidp *)c);
  
    /* Write leftover data */
    if (j)
    {
      j >>= 3;
      sputc((int)(safe[(int)(d % 85)]), sfile);
      while (j--)
      {
        d /= 85;
        putc((int)(safe[(int)(d % 85)]), sfile);
      }
      putc('\n', sfile);
      slns++;
    }
    else if (i)
    {
      putc('\n', sfile);
      slns++;
    }
  }
  putc('$', sfile);
  if (fast)
    fputs(" f", sfile);
  fputs("\nend\n", sfile);
  slns += 2;
  if (ferror(sfile) || fflush(sfile))
    err(SE_FULL, mail ? mname : sname);
  if (noisy)
    fprintf(stderr, "%s shipped\n", p);
}


void mkinv()
/* Build invsafe[], the inverse of safe[]. */
{
  int i;

  for (i = 0; i < 256; i++)
    invsafe[i] = 127;
  for (i = 0; i < sizeof(safe); i++)
    invsafe[safe[i]] = (char)i;
  for (i = 0; aliases[i]; i += 2)
    invsafe[aliases[i]] = invsafe[aliases[i + 1]];
}


unsigned int decb;      /* bit buffer for decode */
unsigned int decn;      /* number of bits in decb */

void decode(s, c)
unsigned char *s;       /* data to decode */
cfile *c;               /* binary output file */
/* Decode s, a string of base 85 digits or, if fast is true, a string of safe
   characters generated arithmetically, into its binary equivalent, writing
   the result to c, using cputc(). */
{
  int b;                /* state of line loop, next character */
  int k;                /* counts bits or digits read */
  /* powers of 85 table for decoding */
  static ulg m[] = {1L,85L,85L*85L,85L*85L*85L,85L*85L*85L*85L};

  if (fast)
  {
    unsigned int d;     /* disperses bits */

    d = decb;
    k = decn;
    while ((b = *s++) != 0)
      if ((b = invsafe[b]) < sizeof(safe))
      {
        if (b < LOWSZ)
        {
          d |= b << k;
          k += 7;
        }
        else if ((b -= LOWSZ) < LOWSZ)
        {
          d |= (b + 0x40) << k;
          k += 7;
        }
        else
        {
          d |= b << k;
          k += 6;
        }
        if (k >= 8)
        {
          cputc(d, c);
          d >>= 8;
          k -= 8;
        }
      }
    decb = d;
    decn = k;
  }
  else
  {
    ulg d;              /* disperses bytes */

    d = k = 0;
    while ((b = *s++) != 0)
      if ((b = invsafe[b]) < 85)
      {
        d += m[k] * b;
        if (++k == 5)
        {
          cputc(d, c);  d >>= 8;
          cputc(d, c);  d >>= 8;
          cputc(d, c);  d >>= 8;
          cputc(d, c);
          d = k = 0;
        }
      }
    if (--k > 0)
    {
      while (--k)
      {
        cputc(d, c);
        d >>= 8;
      }
      cputc(d, c);
    }
  }
}


void unship(v, g, o)
char **v;               /* arguments */
int g;                  /* number of arguments */
int o;                  /* overwrite flag */
/* Extract from the files named in the arguments the files that were
   encoded by ship.  If an argument is "-", then stdin is read. */
{
  int b;                /* state of line loop */
  cfile *c;             /* output binary file */
  FILE *f;              /* output file */
  char *h;              /* name of current ship file */
  char l[LNSZ];         /* line buffer on input */
  int n;                /* next argument to use for input */
  char *p;              /* modifies line buffer */
  char *q;              /* scans continuation line */
  char *r;              /* name of output binary file */
  FILE *s;              /* current ship file */
  int z;                /* true if zero files received */

  /* Build inverse table */
  mkinv();

  /* No input or output files initially */
  s = NULL;
  c = NULL;
  h = r = NULL;

  /* Loop on input files' lines */
  z = 1;                                /* none received yet */
  n = 0;                                /* start with file zero */
  b = 2;                                /* not in body yet */
  while (1)                             /* return on end of last file */
  {
    /* Get next line from list of files */
    while (s == NULL || fgets(l, LNSZ, s) == NULL)
    {
      if (s != NULL)
        fclose(s);
      if (n >= g)
      {
        if (c != NULL)
          err(SE_PART, r);
        else if (z)
          err(SE_NONE, "");
        return;
      }
      if (v[n][0] == '-')
        if (v[n][1])
          err(SE_ARG, v[n]);
        else
        {
          h = "stream stdin";
          s = stdin;
        }
      else
      {
        h = v[n];
        if ((s = fopen(h, "r")) == NULL)
          err(SE_FIND, h);
      }
      n++;
      b &= ~1;                          /* not in middle of line */
    }

    /* Strip control characters and leading blank space, if any */
    for (q = l; *q && *q <= ' ' && *q != '\n'; q++)
      ;
    for (p = l; *q; q++)
      if (*q >= ' ' || *q == '\n')
        *p++ = *q;
    *p = 0;

    /* Based on current state, end or start on terminator.  States are:
         b == 0:  at start of body or body terminator line
         b == 1:  in middle of body line
         b == 2:  at start of non-body line
         b == 3:  in middle of non-body line
         b == 4:  at information line
    */
    switch (b)
    {
    case 0:
      if ((!fast && strcmp(l, "$\n") == 0) ||
          (fast && strcmp(l, "$ f\n") == 0))
      {
        b = 4;
        break;
      }
      /* fall through to case 1 */
    case 1:
      decode((unsigned char *)l, c);
      b = l[strlen(l) - 1] != '\n';
      break;
    case 2:
      if (strcmp(l, "$\n") == 0 || strcmp(l, "$ f\n") == 0)
      {
        fast = l[1] == ' ';
        b = 4;
        break;
      }
      /* fall through to case 3 */
    case 3:
      b = l[strlen(l)-1] == '\n' ? 2 : 3;
      break;
    case 4:
      /* Possible information lines are ship, more, cont, and end */
      if (l[b = strlen(l) - 1] != '\n')
        err(SE_FORM, h);
      l[b] = 0;
      if (strncmp(l, "ship ", 5) == 0)
      {
        /* get name, open new output file */
        if (c != NULL)
          err(SE_FORM, h);
        if ((r = malloc(b - 4)) == NULL)
          err(SE_MEM, "");
        strcpy(r, l + 5);
        if (strcmp(r, "-") == 0)
          f = stdout;
#ifndef VMS     /* shouldn't have explicit version #, so VMS won't overwrite */
        else if (!o && (f = fopen(r, "r")) != NULL)
        {
          fclose(f);
          err(SE_OVER, r);
        }
#endif /* !VMS */
        else if ((f = fopen(r, "w")) == NULL)
          err(SE_FULL, r);
        if ((c = chook(f)) == NULL)
          err(SE_MEM, "");
        b = decb = decn = 0;
        ccnt = 0;
      }
      else if (strcmp(l, "more") == 0)
      {
        /* check if currently writing */
        if (c == NULL)
          err(SE_FORM, h);
        b = 2;
      }
      else if (strncmp(l, "cont ", 5) == 0)
      {
        /* check name and file offset */
        if (c == NULL)
          err(SE_FORM, h);
        for (q = l + 5; *q && *q != ' '; q++)
          ;
        if (*q == 0 || atol(l + 5) != ccnt + 4 + (decn != 0) ||
            strcmp(q + 1, r))
          err(SE_CONT, r);
        b = 0;
      }
      else if (strcmp(l, "end") == 0)
      {
        /* check crc, close output file */
        if (c == NULL)
          err(SE_FORM, h);
        if (c->n != 4 || c->b != ~c->c)
          err(SE_CRC, r);
        if (ferror(c->f) || fclose(c->f))
          err(SE_FULL, r);
        if (noisy)
          fprintf(stderr, "%s received\n", r);
        z = 0;
        free((voidp *)c);
        c = NULL;
        b = 2;
      }
      else
      {
        for (q = l; *q && *q != ' '; q++)
          ;
        *q = 0;
        fprintf(stderr, "%s: unsupported keyword '%s' ignored\n", warname, l);
        b = 4;
      }
      break;
    }
  }
}


void help()
{
  int i;
  static char *text[] = {
"Usage:",
"   ship [-f] [-q] [-nnn] [-m address] [-s subject] files...",
"",
"   ships the files to stdout.  -m sends the output via the mailer to",
"   address.  -nnn splits the output into pieces of nnnK bytes or less.",
"   if -nnn is used without -m, the output goes to the files partxxxx,",
"   where xxxx is 0001, 0002, etc.  If -0 is specified, the output goes",
"   entirely to the file part0001.  -f uses a fast method with slightly",
"   less performance.  If no files are given, stdin is used.  The special",
"   filename '-' also takes input from stdin.  Files shipped from stdin",
"   are unshipped to stdout.  This can be used to document a shipment.",
"   When mailing, -s gives a subject line prefix.  -q inhibits messages.",
"",
"   ship -u [-o] [-q] files...",
"   unship  [-o] [-q] files...",
"",
"   extracts the contents of the mail messages in files...  -o allows",
"   existing files to be overwritten.  -u is implied if the name of the",
"   executable is unship.  If no files are given, the input is from",
"   stdin.  If any of the files were shipped from stdin, then they are",
"   extracted to stdout."
  };

  puts(SHIPVER);
  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i]);
    putchar('\n');
  }
  exit(0);
}


void main(argc, argv)
int argc;               /* number of arguments */
char **argv;            /* table of argument strings */
{
  FILE *f;              /* input file */
  ulg k;                /* number of k requested per part */
  char *p;              /* temporary variable */
  int o;                /* overwrite flag */
  int r;                /* temporary variable */
  int s;                /* true if no names given */

  /* No temporary file yet (for err()) */
  *mname = 0;

  /* No subject prefix yet */
  *mprefix = 0;

  /* See if help requested */
  if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0))
    help();

  /* Unship */
  if ((p = strrchr(argv[0], PATHCUT)) == NULL)
    p = argv[0];
  else
    p++;
  r = 0;                                /* (make some compilers happier) */
  if ((r = strncmp(p, "unship", 6)) == 0 ||
      (r = strncmp(p, "UNSHIP", 6)) == 0 ||
      (argc > 1 && strcmp(argv[1], "-u") == 0))
  {
    errname = "unship error";
    warname = "unship warning";
    r = r ? 2 : 1;                      /* next arg */
    o = 0;                              /* disallow overwriting */
    if (r < argc && strcmp(argv[r], "-o") == 0)
    {
      r++;
      o = 1;                            /* allow overwriting */
    }
    if (r < argc && strcmp(argv[r], "-q") == 0)
    {
      r++;
      noisy = 0;                        /* inhibit messages */
    }
    if (r < argc)
      unship(argv + r, argc - r, o);    /* unship files in args */
    else
    {
      char *a[1];       /* short list of names (one) */

      a[0] = "-";
      unship(a, 1, o);                  /* no args--do stdin */
    }
  }

  /* Ship */
  else
  {
    mail = 0;                           /* not mailing */
    fast = 0;                           /* use base 85 encoding */
    s = 1;                              /* no names given yet */
    strcpy(sname, "-");                 /* output to stdout */
    sfile = stdout;
    slns = slmax = k = 0;
    for (r = 1; r < argc; r++)          /* go through args */
      if (argv[r][0] == '-')            /* option or stdin */
        if (argv[r][1])                 /* option */
        {
          if (argv[r][1] == 'm')        /* mail output */
          {
            mail = 1;
            mdest = NULL;               /* next arg is mail address */
          }
          else if (argv[r][1] == 's')   /* next arg is subject prefix */
            mprefix = NULL;
          else if (argv[r][1] == 'f')   /* fast arithmetic encoding */
          {
            fast = 1;
            if (k)                      /* recompute slmax if needed */
              slmax = 4 + (k * 1024L) / 81;
          }
          else if (argv[r][1] == 'q')   /* quiet operation */
            noisy = 0;
          else                          /* option is number of lines */
          {
            /* Check numeric option */
            for (p = argv[r] + 1; *p; p++)
              if (*p < '0' || *p > '9')
                break;
            if (*p || slmax)
              err(SE_ARG, argv[r]);
  
            /* Zero means infinity, else convert */
            if ((k = atol(argv[r] + 1)) == 0)
              slmax = -1L;
            else
              slmax = 4 + (k * 1024L) / (fast ? 81 : 77);
          }
        }
        else                            /* input file is stdin */
        {
          if (mail && mdest == NULL)
            err(SE_ARG, "- (no mail destination given)");
          s = 0;
          if (mail && !*mprefix)
            strcpy(mprefix, "(stdin)");
          ship("-", stdin);
        }
      else                              /* not option or stdin */
        if (mail && mdest == NULL)      /* arg is mail address */
          mdest = argv[r];
        else if (mprefix == NULL)       /* arg is subject prefix */
          mprefix = argv[r];
        else                            /* arg is file to ship */
        {
          s = 0;
          if ((f = fopen(argv[r], "r")) == NULL)
            err(SE_FIND, argv[r]);
          if (mail && !*mprefix)
          {
            int i;

            for (i = 0, p = nopath(argv[r]); i < 8 && *p; p++)
              if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') ||
                  (*p >= 'a' && *p <= 'z') || *p == '.' || *p == '_')
                mprefix[i++] = *p;
            mprefix[i] = 0;
          }
          ship(argv[r], f);
          fclose(f);
        }
    if (s)                              /* no names--act as filter */
      if (mail && mdest == NULL)
        err(SE_ARG, "-m (no mail destination given)");
      else if (mprefix == NULL)
        err(SE_ARG, "-s (no subject prefix given)");
      else
      {
        if (mail && !*mprefix)
          strcpy(mprefix, "(stdin)");
        ship("-", stdin);
      }
    endship(1);                         /* clean up */
    if (noisy && (mail || slmax))
      fprintf(stderr, "file%s%s %s\n",
              strcmp("part0001", sname) ? "s part0001.." : " ", sname,
              mail ? "mailed" : "written");
  }

  /* Done */
  exit(0);
}
