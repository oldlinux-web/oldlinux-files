/* @(#) options.h 2.22 88/08/24 15:27:36 */

/*
The contents of this file are hereby released to the public domain.
                           -- Rahul Dhesi 1991/07/06

For documentation about this file, see options.doc.
*/

#define ZOO				/* always defined */
#define PORTABLE		/* always defined */
#define ZOOCOMMENT	/* always defined */


/***********************************************************************/
/* SYSTEM V (should be compatible with most releases)                  */
/***********************************************************************/

#ifdef SYS_V
#define FILTER
#define IO_MACROS
#define EXISTS(f)		(access(f, 00) == 0)
#define FNLIMIT 14
#define CHEKDIR
#define NIXTIME
#define NIXFNAME
#define NEEDCTYP
#define NOENUM
#define REN_LINK
#define SETBUF
#define GETTZ
#define FATTR
#define T_SIGNAL	void
#define VARARGS
#define NEED_MEMMOVE
/* #define NEED_MEMCPY */
#define T_UINT16		unsigned short		/* must be 16 bit unsigned */
#define HAVE_ISATTY
/* #define NEED_VPRINTF */
#endif /* SYS_V */

/***********************************************************************/
/* Turbo C++ 1.0 under MS-DOS                                          */
/***********************************************************************/

#ifdef TURBOC
#undef PORTABLE
#define ANSI_HDRS
#define USE_ASCII
#define SPECINIT
#define SPECEXIT
#define PURIFY
#define DISK_CH ':'
#define IGNORECASE
#define WILDCARD "*.*"
#define FOLD
#define FORCESLASH
#define FNLIMIT 12
#define CUR_DIR "."
#define PATH_SEP ":/\\"
#define EXT_SEP  ":/\\."
#define SETMODE
/* 0x8000 and 0x4000 taken from <fcntl.h> for Turbo C */
#define MODE_BIN(f)      setmode(fileno(f), 0x8000)
#define MODE_TEXT(f)     setmode(fileno(f), 0x4000)
#define NEED_STDIO
#define ANSI_PROTO
#define VOIDPTR		void *
#define REN_STDC
#define STDARG
#define T_UINT16		unsigned short		/* must be 16 bit unsigned */
/* #define UNBUF_IO */
/* #define UNBUF_LIMIT	512 */
#define  T_SIGNAL void
#define DIRECT_CONVERT
#define STDARG
#define CHECK_BREAK
#define check_break kbhit
#define HAVE_ISATTY
#ifdef  PORTABLE		/* for testing only */
# define SPECNEXT
# define NIXTIME
# undef  WILDCARD
#endif
#endif /* TURBOC */

/***********************************************************************/
/* Older BSD 4.3 and most derivatives                                  */
/***********************************************************************/

#ifdef BSD4_3
#define NOSTRCHR /* not really needed for 4.3BSD */
#define FILTER
#define IO_MACROS
#define EXISTS(f)		(access(f, 00) == 0)
#define FNLIMIT 1023
#define CHEKDIR
#define NIXTIME
#define NIXFNAME
#define NEEDCTYP
#define NOENUM
#define REN_STDC
#define SETBUF
#define GETTZ
#define FATTR
#define T_SIGNAL	int
#define VARARGS
#define NEED_MEMMOVE
#define T_UINT16		unsigned short		/* must be 16 bit unsigned */
#define HAVE_ISATTY
#define NEED_VPRINTF		/* older BSDs only; newer ones have vprintf */
#endif /* BSD4_3 */

/*  Ultrix 4.1 */
#ifdef ULTRIX
#define NO_STDIO_FN	/* avoid declaring certain stdio functions */
#define NOSTRCHR /* needed? */
#define FILTER
#define IO_MACROS
#define EXISTS(f)		(access(f, 00) == 0)
#define FNLIMIT 1023
#define CHEKDIR
#define NIXTIME
#define NIXFNAME
#define NEEDCTYP
#define NOENUM
#define REN_STDC
#define SETBUF
#define GETTZ
#define FATTR
#define T_SIGNAL	void
#define VARARGS
#define NEED_MEMMOVE
#define T_UINT16	unsigned short	/* must be 16 bit unsigned */
#define HAVE_ISATTY
/* #define NEED_VPRINTF */
#define BSD4_3		/* for I/O definitions */
#endif /* ULTRIX */

/***********************************************************************/
/* Newer BSD 4.4 (projected)                                           */
/***********************************************************************/

#ifdef BSD4_4
/* #define NOSTRCHR */
#define FILTER
#define IO_MACROS
#define EXISTS(f)		(access(f, 00) == 0)
#define FNLIMIT 1023
#define CHEKDIR
#define NIXTIME
#define NIXFNAME
#define NEEDCTYP
/* #define NOENUM */
#define REN_STDC
#define SETBUF
#define GETTZ
#define FATTR
#define T_SIGNAL	void
/* #define VARARGS */
/* #define NEED_MEMMOVE */
#define T_UINT16		unsigned short		/* must be 16 bit unsigned */
#define HAVE_ISATTY
/* #define NEED_VPRINTF */
#endif /* BSD4_4 */

/***********************************************************************/
/* VAX/VMS version 5.3 or so                                           */
/***********************************************************************/

#ifdef VMS

/*
Select VMS pre-4.6 or later next line.  Pre-4.6 library does not have
rename() and memset() so zoo defines its own;  4.6 has these, so we
must use them, else VMS library functions will conflict with our
own.
*/
# if 0		/* Make this 1 for VMS version 4.5 or earlier */
#  define NEED_VMS_RENAME	/* used in vms.c */
#  define NEED_MEMSET
# endif
#define REN_STDC
#define IO_MACROS
#define SPEC_WILD
#define EXT_ANYWAY
#define VER_CH ';'
#define SPECEXIT
#define CHEKUDIR
#define FNLIMIT 78
#define DIR_SEP '.'  /* separates dir fields */
#define DISK_CH ':'
#define DIR_LBRACK "[" /* left bracketing symbol dir dir name */
#define PATH_CH "]"
#define PATH_SEP ":]"
#define EXT_SEP ":]."
#define CUR_DIR "."
#define NIXTIME
#define NEEDCTYP
#define NOENUM
#define IGNORECASE
#define SPECMOD
#define SPECNEXT
#define WILDCARD "*.*"
#define FOLD
#define NO_STDIO_FN
#define T_SIGNAL	void
#define T_UINT16		unsigned short		/* must be 16 bit unsigned */
#define VARARGS
#endif /* VMS */

/***********************************************************************/
/* AMIGA, SOME VERSION -- NOT TESTED, MAY NEED PORTING                 */
/***********************************************************************/

#ifdef MCH_AMIGA
#define PURIFY
#define DISK_CH ':'
#define SPECNEXT
#define WILDCARD "*"
#define IGNORECASE
#define FNLIMIT 30
#define NEEDCTYP
#define CUR_DIR "."
#define PATH_SEP ":/"
#define EXT_SEP  ":/."
#define NOSIGNAL
#define REN_STDC
#define NOENUM
#define SETBUF
#define CHEKUDIR
#define GETUTIME
#define NIXTIME
#endif

/***********************************************************************/
/* GENERIC **IX SYSTEM -- GOOD STARTING POINT FOR YOURS                */
/***********************************************************************/

#ifdef GENERIC
/* #define SPECNEXT */
/* #define IGNORECASE */
#define FNLIMIT 14
#define NEEDCTYP
#define CUR_DIR "."
#define PATH_SEP "/"
#define EXT_SEP  "/."
/* #define NOSIGNAL */
/* REN_LINK is UNIX-specific.  Can't find a generic rename() function */
#define REN_LINK
#define NOENUM
/* #define SETBUF */
#define CHEKDIR
#define NIXTIME
#define HAVE_ISATTY
#define NEED_MEMMOVE
#endif /* GENERIC */


/***********************************************************************/
/* REST OF THIS FILE SHOULD NOT NEED ANY CHANGES                       */
/***********************************************************************/

/***********************************************************************/
/*  Common filename conventions for **IX systems                       */
/***********************************************************************/

#ifdef NIXFNAME
#define CUR_DIR "."
#define PATH_SEP "/"
#define EXT_CH '.'
#define EXT_SEP  "/."
#define EXT_DFLT ".zoo"
#endif

/* Compensate for strchr/index differences */
#ifdef NOSTRCHR
#define	strchr	index
#define	strrchr	rindex
#endif

/* let non-**IX lints under **IX work (see makefile) */
#ifdef CROSS_LINT
# undef ANSI_HDRS
# undef ANSI_PROTO
# ifdef STDARG
#  undef STDARG
#  define VARARGS
# endif /* STDARG */
#endif

/* assume certain defaults */
#ifndef VOIDPTR
# define VOIDPTR   char *
#endif

#ifndef VER_DISPLAY
# define VER_DISPLAY ";"
#endif
#ifndef VER_INPUT
# define VER_INPUT ":;"
#endif
#ifndef PATH_CH
# define PATH_CH "/"
#endif
#ifndef EXT_CH
# define EXT_CH '.'
#endif
#ifndef EXT_DFLT
# define EXT_DFLT ".zoo"
#endif

#ifndef STDARG
# ifndef VARARGS
#  define VARARGS
# endif
#endif

#ifndef T_SIGNAL
# define T_SIGNAL		int
#endif

#ifdef STDARG
# ifdef VARARGS
# include "DO NOT DEFINE BOTH STDARG AND VARARGS"
# endif
#endif

/* We supply a default for T_UINT16 if it is not defined.  But this
value is critical, so we compile in a runtime check. */

#ifndef T_UINT16
# define T_UINT16	unsigned short
# define CHECK_TUINT	/* will do runtime check for correct size */
#endif

/* ANSI compatibility in declarations -- see zoofns.h for usage */
#ifndef PARMS
# ifdef ANSI_PROTO
#  define PARMS(x)		x
# else
#  define PARMS(x)		()
# endif
#endif
