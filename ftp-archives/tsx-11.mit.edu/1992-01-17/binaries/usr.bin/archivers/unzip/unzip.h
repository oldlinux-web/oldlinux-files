/*---------------------------------------------------------------------------

  unzip.h

  This header file is used by all of the unzip source files.  Its contents
  are divided into seven more-or-less separate sections:  predefined macros,
  OS-dependent includes, (mostly) OS-independent defines, typedefs, function 
  prototypes (or "prototypes," in the case of non-ANSI compilers), macros, 
  and global-variable declarations.

  ---------------------------------------------------------------------------*/



/*****************************************/
/*  Predefined, Machine-specific Macros  */
/*****************************************/

#if defined(ultrix) || defined(bsd4_2) || defined(sun)
#  if !defined(BSD) && !defined(__SYSTEM_FIVE)
#    define BSD
#  endif /* !BSD && !__SYSTEM_FIVE */
#endif /* ultrix || bsd4_2 || sun */

/* [1992.1.21:  ALL machines now "NOTINT16"; NOTINT16 and ULONG_ removed] */
#if 0
#  if defined(mips) || defined(sun) || defined(CRAY)
#    ifndef NOTINT16
#      define NOTINT16
#    endif /* !NOTINT16 */
#  endif /* mips || sun || CRAY */

#  if defined(vax) || defined(MSDOS) || defined(__MSDOS__)
#    ifdef NOTINT16
#      undef NOTINT16
#    endif /* NOTINT16 */
#  endif /* vax || MSDOS || __MSDOS__ */
#endif /* 0 */

#ifdef CRAY
#  ifdef ZMEM
#    undef ZMEM
#  endif /* ZMEM */
#  ifndef TERMIO
#    define TERMIO
#  endif /* !TERMIO */
#endif /* CRAY */

#ifdef CONVEX
#  ifndef TERMIO
#    define TERMIO
#  endif /* !TERMIO */
#endif /* CONVEX */

/* the i386 test below is to catch SCO Unix (which has redefinition
 * warnings if param.h is included), but it probably doesn't hurt if
 * other 386 Unixes get nailed, too--are there any 386 BSD systems?
 * param.h is mostly included for "BSD", I think...
 */
#if defined(MINIX) || (defined(i386) && defined(unix))
#  define NO_PARAM_H
#endif /* MINIX || (i386 && unix) */

#if defined(__IBMC__) && defined(__32BIT__) && !defined(IBMC32)
#  define IBMC32
#endif /* __IBMC__ && __32BIT__ && !IBMC32 */





/***************************/
/*  OS-Dependent Includes  */
/***************************/

#ifndef MINIX            /* Minix needs it after all the other includes (?) */
#  include <stdio.h>     /* this is your standard header for all C compiles */
#endif
#include <ctype.h>
#include <errno.h>       /* used in mapname() */
#define DECLARE_ERRNO    /* everybody except MSC 6.0 */
#ifdef VMS               /* sigh...you just KNEW someone had to break this.  */
#  include <types.h>     /*  (placed up here instead of in VMS section below */
#  include <stat.h>      /*   because types.h is used in some other headers) */
#else  /* almost everybody */
#  if defined(THINK_C) || defined(MPW) /* for Macs */
#    include <stddef.h>
#  else
#    include <sys/types.h> /* off_t, time_t, dev_t, ... */
#    include <sys/stat.h>  /* Everybody seems to need this. */
#  endif
#endif


/*---------------------------------------------------------------------------
    Next, a word from our Unix (mostly) sponsors:
  ---------------------------------------------------------------------------*/

#ifdef UNIX
#  ifdef AMIGA
#    include <libraries/dos.h>
#  else /* !AMIGA */
#    ifndef NO_PARAM_H
#if 0  /* [GRR: this is an alternate fix for SCO's redefinition bug] */
#      ifdef NGROUPS_MAX
#        undef NGROUPS_MAX     /* SCO bug:  defined again in <param.h> */
#      endif /* NGROUPS_MAX */
#endif /* 0 */
#      include <sys/param.h>   /* conflict with <sys/types.h>, some systems? */
#    endif /* !NO_PARAM_H */
#  endif /* ?AMIGA */

#  ifndef BSIZE
#    ifdef MINIX
#      define BSIZE   1024
#    else /* !MINIX */
#      define BSIZE   DEV_BSIZE  /* assume common for all Unix systems */
#    endif /* ?MINIX */
#  endif

#  ifndef BSD
#    if !defined(AMIGA) && !defined(MINIX)
#      define NO_MKDIR           /* for mapname() */
#    endif /* !AMIGA && !MINIX */
#    ifndef SKIP_TIME_H          /* temporary fix for VMS gcc and Amdahl cc */
#      include <time.h>          /*  in decryption version */
#    endif /* !SKIP_TIME_H */
     struct tm *gmtime(), *localtime();
#  else   /* BSD */
#    ifndef SKIP_TIME_H          /* temporary fix for Sun 3 cc, etc. */
#      include <sys/time.h>      /*  in decryption version */
#    endif /* !SKIP_TIME_H */
#    include <sys/timeb.h>
#  endif

#else   /* !UNIX */
#  define BSIZE   512               /* disk block size */
#endif /* ?UNIX */

#if defined(V7) || defined(BSD)
#  define strchr    index
#  define strrchr   rindex
#endif

/*---------------------------------------------------------------------------
    And now, our MS-DOS and OS/2 corner:
  ---------------------------------------------------------------------------*/

#ifdef __TURBOC__
#  define DOS_OS2             /* Turbo C under DOS, MSC under DOS or OS2    */
#  include <sys/timeb.h>      /* for structure ftime                        */
#  ifndef __BORLANDC__        /* There appears to be a bug (?) in Borland's */
#    include <mem.h>          /*   MEM.H related to __STDC__ and far poin-  */
#  endif                      /*   ters. (dpk)  [mem.h included for memcpy] */
#else                         /* NOT Turbo C...                             */
#  ifdef MSDOS                /*   but still MS-DOS, so we'll assume it's   */
#    ifndef MSC               /*   Microsoft's compiler and fake the ID, if */
#      define MSC             /*   necessary (it is in 5.0; apparently not  */
#    endif                    /*   in 5.1 and 6.0)                          */
#    include <dos.h>          /* _dos_setftime()                            */
#  endif
#  ifdef OS2                  /* stuff for DOS and OS/2 family version */
#    if !defined(IBMC32) && !defined(MSC) && !defined(__BORLANDC__)
#      define MSC             /* assume Microsoft if not ID'd already */
#    endif
#    define INCL_BASE
#    define INCL_NOPM
#    include <os2.h>          /* DosQFileInfo(), DosSetFileInfo()? */
#  endif
#endif

#ifdef IBMC32
#  define DOS_OS2
#  define S_IFMT 0xF000
#  define timezone _timezone
#endif

#ifdef EMX32
#  define DOS_OS2
#  define __32BIT__
#  define DosGetCtryInfo   DosQueryCtryInfo
#  define DosQFileInfo     DosQueryFileInfo
#  define far
#endif
#ifdef MSC                    /* defined for all versions of MSC now         */
#  define DOS_OS2             /* Turbo C under DOS, MSC under DOS or OS/2    */
#  ifndef __STDC__            /* MSC 5.0 and 5.1 aren't truly ANSI-standard, */
#    define __STDC__ 1        /*   but they understand prototypes...so       */
#  endif                      /*   they're close enough for our purposes     */
#  if defined(_MSC_VER) && (_MSC_VER >= 600)      /* new with 5.1 or 6.0 ... */
#    undef DECLARE_ERRNO      /* errno is now a function in a dynamic link   */
#  endif                      /*   library (or something)--incompatible with */
#endif                        /*   the usual "extern int errno" declaration  */

#ifdef DOS_OS2                /* defined for both Turbo C, MSC */
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  ifndef SKIP_TIME_H         /* also included in crypt.c */
#    include <time.h>         /* localtime() */
#  endif /* !SKIP_TIME_H */
#endif

/*---------------------------------------------------------------------------
    Followed by some VMS (mostly) stuff:
  ---------------------------------------------------------------------------*/

#ifdef VMS
#  ifndef SKIP_TIME_H  /* temporary bugfix for VMS gcc in decryption version */
#    include <time.h>           /* the usual non-BSD time functions */
#  endif /* !SKIP_TIME_H */
#  include <file.h>             /* same things as fcntl.h has */
#  include <rmsdef.h>           /* RMS error codes */
#  include "fatdef.h"           /* RMS-related things used by VMSmunch */
#  include "VMSmunch.h"         /* VMSmunch argument definitions */
#  define UNIX                  /* can share most of same code from now on */
#  define RETURN   return_VMS   /* VMS interprets return codes incorrectly */
#else /* !VMS */
#  define RETURN   return       /* only used in main() */
#  ifdef V7
#    define O_RDONLY  0
#    define O_WRONLY  1
#    define O_RDWR    2
#  else /* !V7 */
#    ifdef MTS
#      include <sys/file.h>     /* MTS uses this instead of fcntl.h */
#      include <timeb.h>
#      ifndef SKIP_TIME_H       /* also included in crypt.c */
#        include <time.h>
#      endif /* !SKIP_TIME_H */
#    else /* !MTS */
#      ifdef COHERENT           /* Coherent 3.10/Mark Williams C */
#        include <sys/fcntl.h>
#        define SHORT_NAMES
#        define tzset  settz
#      else /* !COHERENT */
#        include <fcntl.h>      /* #define O_BINARY 0x8000 (no CR/LF */
#      endif /* ?COHERENT */    /*   translation), as used in open() */
#    endif /* ?MTS */
#  endif /* ?V7 */
#endif /* ?VMS */

/*---------------------------------------------------------------------------
    And some Mac stuff for good measure:
  ---------------------------------------------------------------------------*/

#ifdef THINK_C
#  define MACOS
#  ifndef __STDC__            /* If Think C hasn't defined __STDC__ ... */
#    define __STDC__ 1        /* make sure it's defined: it needs it */
#  else /* __STDC__ defined */
#    if !__STDC__             /* Sometimes __STDC__ is defined as 0. */
#      undef __STDC__         /* It needs to be 1 or required header */
#      define __STDC__ 1      /* files are not properly included. */
#    endif /* !__STDC__ */
#  endif /* ?defined(__STDC__) */
#  include <unix.h>
#  include "macstat.h"
#endif

#ifdef MPW
#  define MACOS
#  include <Files.h>
#  include "macstat.h"
#  define CtoPstr c2pstr
#  define PtoCstr p2cstr
#  ifndef TRUE
#    define TRUE     1
#  endif
#  ifndef FALSE
#    define FALSE    0
#  endif
#  ifdef CR
#    undef  CR
#  endif
#endif

/*---------------------------------------------------------------------------
    And finally, some random extra stuff:
  ---------------------------------------------------------------------------*/

#if __STDC__
#  include <stdlib.h>      /* standard library prototypes, malloc(), etc. */
#  include <string.h>      /* defines strcpy, strcmp, memcpy, etc. */
#else
   char *malloc();
   char *strchr(), *strrchr();
   long lseek();
#endif

#ifdef MINIX
#  include <stdio.h>
#endif

#ifdef SHORT_NAMES         /* Mark Williams C, ...? */
#  define extract_or_test_files    xtr_or_tst_files
#  define extract_or_test_member   xtr_or_tst_member
#endif





/*************/
/*  Defines  */
/*************/

#define INBUFSIZ          BUFSIZ   /* same as stdio uses */
#define DIR_BLKSIZ        64       /* number of directory entries per block
                                    *  (should fit in 4096 bytes, usually) */
/*
 * If <limits.h> exists on most systems, should include that, since it may
 * define some or all of the following:  NAME_MAX, PATH_MAX, _POSIX_NAME_MAX,
 * _POSIX_PATH_MAX.
 */
#ifdef DOS_OS2
#  include <limits.h>
#endif /* ?DOS_OS2 */

#ifdef _MAX_PATH
#  define FILNAMSIZ       (_MAX_PATH)
#else /* !_MAX_PATH */
#  define FILNAMSIZ       1025
#endif /* ?_MAX_PATH */

#ifndef PATH_MAX
#  ifdef MAXPATHLEN                /* defined in <sys/param.h> some systems */
#    define PATH_MAX      MAXPATHLEN
#  else
#    if FILENAME_MAX > 255         /* used like PATH_MAX on some systems */
#      define PATH_MAX    FILENAME_MAX
#    else
#      define PATH_MAX    (FILNAMSIZ - 1)
#    endif
#  endif /* ?MAXPATHLEN */
#endif /* !PATH_MAX */

#ifdef ZIPINFO
#  define OUTBUFSIZ       BUFSIZ   /* zipinfo needs less than unzip does    */
#else
#  define OUTBUFSIZ       0x2000   /* unImplode needs power of 2, >= 0x2000 */
#endif

#define ZSUFX             ".zip"
#define CENTRAL_HDR_SIG   "\113\001\002"   /* the infamous "PK" signature */
#define LOCAL_HDR_SIG     "\113\003\004"   /*  bytes, sans "P" (so unzip not */
#define END_CENTRAL_SIG   "\113\005\006"   /*  mistaken for zipfile itself) */

#define SKIP              0    /* choice of activities for do_string() */
#define DISPLAY           1
#define FILENAME          2
#define EXTRA_FIELD       3

#define DOES_NOT_EXIST    -1   /* return values for check_for_newer() */
#define EXISTS_AND_OLDER  0
#define EXISTS_AND_NEWER  1

#define DOS_OS2_FAT_      0    /* version_made_by codes (central dir) */
#define AMIGA_            1
#define VMS_              2    /* MAKE SURE THESE ARE NOT DEFINED ON */
#define UNIX_             3    /* THE RESPECTIVE SYSTEMS!!  (like, for */
#define VM_CMS_           4    /* instance, "VMS", or "UNIX":  CFLAGS = */
#define ATARI_            5    /*  -O -DUNIX) */
#define OS2_HPFS_         6
#define MAC_              7
#define Z_SYSTEM_         8
#define CPM_              9
/* #define TOPS20_   10?  (TOPS20_ is to be defined in PKZIP 2.0...)  */
#define NUM_HOSTS         10   /* index of last system + 1 */

#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define NUM_METHODS       9    /* index of last method + 1 */
/* don't forget to update list_files() appropriately if NUM_METHODS changes */

#define DF_MDY            0    /* date format 10/26/91 (USA only) */
#define DF_DMY            1    /* date format 26/10/91 (most of the world) */
#define DF_YMD            2    /* date format 91/10/26 (a few countries) */

#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_VERSION       42   /* if OS-needed-to-extract is VMS:  can do */

/*---------------------------------------------------------------------------
    True sizes of the various headers, as defined by PKWare--so it is not
    likely that these will ever change.  But if they do, make sure both these
    defines AND the typedefs below get updated accordingly.
  ---------------------------------------------------------------------------*/

#define LREC_SIZE     26    /* lengths of local file headers, central */
#define CREC_SIZE     42    /*  directory headers, and the end-of-    */
#define ECREC_SIZE    18    /*  central-dir record, respectively      */


#define MAX_BITS      13                 /* used in unShrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */

#define LF   10   /* '\n' on ASCII machines.  Must be 10 due to EBCDIC */
#define CR   13   /* '\r' on ASCII machines.  Must be 13 due to EBCDIC */

#ifdef EBCDIC
#  define ascii_to_native(c)   ebcdic[(c)]
#  define NATIVE    "EBCDIC"
#endif

#ifdef AMIGA
#  define FFLUSH    fflush(stderr);
#else /* !AMIGA */
#  if MPW
#    define FFLUSH    putc('\n',stderr);
#  else
#    define FFLUSH
#  endif
#endif /* ?AMIGA */

#ifdef CRYPT
#  define OF __
#  define PWLEN        80
#  define DECRYPT(b)   (update_keys(t=((b)&0xff)^decrypt_byte()),t)
#endif /* CRYPT */

#ifdef QQ  /* Newtware version */
#  define QCOND    (quietflg < 2)    /* -xq only kills comments */
#  define QCOND2   (!quietflg)       /* for no file comments with -vq[q] */
#else      /* (original) Bill Davidsen version  */
#  define QCOND    (!quietflg)       /* -xq[q] both kill "extracting:" msgs */
#  define QCOND2   (which_hdr)       /* file comments with -v, -vq, -vqq */
#endif

#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#  define FALSE     0
#endif

#ifndef SEEK_SET        /* These should all be declared in stdio.h!  But   */
#  define SEEK_SET  0   /*  since they're not (in many cases), do so here. */
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif

#ifndef S_IRUSR
#  define S_IRWXU       00700       /* read, write, execute: owner */
#  define S_IRUSR       00400       /* read permission: owner */
#  define S_IWUSR       00200       /* write permission: owner */
#  define S_IXUSR       00100       /* execute permission: owner */
#  define S_IRWXG       00070       /* read, write, execute: group */
#  define S_IRGRP       00040       /* read permission: group */
#  define S_IWGRP       00020       /* write permission: group */
#  define S_IXGRP       00010       /* execute permission: group */
#  define S_IRWXO       00007       /* read, write, execute: other */
#  define S_IROTH       00004       /* read permission: other */
#  define S_IWOTH       00002       /* write permission: other */
#  define S_IXOTH       00001       /* execute permission: other */
#endif /* !S_IRUSR */

#ifdef ZIPINFO      /* these are individually checked because SysV doesn't */
#  ifndef S_IFBLK   /*  have some of them, Microsoft C others, etc. */
#    define   S_IFBLK     0060000     /* block special */
#  endif
#  ifndef S_IFIFO  /* in Borland C, not MSC */
#    define   S_IFIFO     0010000     /* fifo */
#  endif
#  ifndef S_IFLNK  /* in BSD, not SysV */
#    define   S_IFLNK     0120000     /* symbolic link */
#  endif
#  ifndef S_IFSOCK  /* in BSD, not SysV */
#    define   S_IFSOCK    0140000     /* socket */
#  endif
#  ifndef S_ISUID
#    define S_ISUID       04000       /* set user id on execution */
#  endif
#  ifndef S_ISGID
#    define S_ISGID       02000       /* set group id on execution */
#  endif
#  ifndef S_ISVTX
#    define S_ISVTX       01000       /* directory permissions control */
#  endif
#  ifndef S_ENFMT
#    define S_ENFMT       S_ISGID     /* record locking enforcement flag */
#  endif
#endif /* ZIPINFO */





/**************/
/*  Typedefs  */
/**************/

#ifndef _BULL_SOURCE                /* Bull has it defined somewhere already */
   typedef unsigned char  byte;     /* code assumes UNSIGNED bytes */
#endif /* !_BULL_SOURCE */

typedef char              boolean;
typedef long              longint;
typedef unsigned short    UWORD;
#if !defined(IBMC32) && !defined(EMX32)
   typedef unsigned long  ULONG;
#endif

typedef struct min_info {
    unsigned unix_attr;
    unsigned dos_attr;
    int hostnum;
    longint offset;
/*  ULONG crc;  */           /* did need to check decryption (use local now) */
    unsigned encrypted : 1;  /* file encrypted: decrypt before uncompressing */
    unsigned ExtLocHdr : 1;  /* use time instead of CRC for decrypt check */
    unsigned text : 1;       /* file is text or binary */
    unsigned lcflag : 1;     /* convert filename to lowercase */
} min_info;

/*---------------------------------------------------------------------------
    Zipfile layout declarations.  If these headers ever change, make sure the
    xxREC_SIZE defines (above) change with them!
  ---------------------------------------------------------------------------*/

   typedef byte   local_byte_hdr[ LREC_SIZE ];
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

   typedef byte   cdir_byte_hdr[ CREC_SIZE ];
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

   typedef byte   ec_byte_rec[ ECREC_SIZE+4 ];
/*     define SIGNATURE                         0   space-holder only */
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10
#      define SIZE_CENTRAL_DIRECTORY            12
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20


   typedef struct local_file_header {                 /* LOCAL */
       byte version_needed_to_extract[2];
       UWORD general_purpose_bit_flag;
       UWORD compression_method;
       UWORD last_mod_file_time;
       UWORD last_mod_file_date;
       ULONG crc32;
       ULONG compressed_size;
       ULONG uncompressed_size;
       UWORD filename_length;
       UWORD extra_field_length;
   } local_file_hdr;

   typedef struct central_directory_file_header {     /* CENTRAL */
       byte version_made_by[2];
       byte version_needed_to_extract[2];
       UWORD general_purpose_bit_flag;
       UWORD compression_method;
       UWORD last_mod_file_time;
       UWORD last_mod_file_date;
       ULONG crc32;
       ULONG compressed_size;
       ULONG uncompressed_size;
       UWORD filename_length;
       UWORD extra_field_length;
       UWORD file_comment_length;
       UWORD disk_number_start;
       UWORD internal_file_attributes;
       ULONG external_file_attributes;
       ULONG relative_offset_local_header;
   } cdir_file_hdr;

   typedef struct end_central_dir_record {            /* END CENTRAL */
       UWORD number_this_disk;
       UWORD num_disk_with_start_central_dir;
       UWORD num_entries_centrl_dir_ths_disk;
       UWORD total_entries_central_dir;
       ULONG size_central_directory;
       ULONG offset_start_central_directory;
       UWORD zipfile_comment_length;
   } ecdir_rec;





/*************************/
/*  Function Prototypes  */
/*************************/

#ifndef __              /* This is a common macro, but the name "__" was */
#  if __STDC__          /*  swiped from the fine folks at Cray Research, */
#    define __(X)   X   /*  Inc. (thanks, guys!).  Keep interior stuff   */
#  else                 /*  parenthesized...                             */
#    define __(X)   ()
#  endif
#endif

/*---------------------------------------------------------------------------
    Functions in unzip.c and/or zipinfo.c:
  ---------------------------------------------------------------------------*/

int    usage                     __( (int error) );
int    process_zipfile           __( (void) );
int    find_end_central_dir      __( (void) );
int    process_end_central_dir   __( (void) );
int    list_files                __( (void) );
int    process_cdir_file_hdr     __( (void) );
int    process_local_file_hdr    __( (void) );
int    process_central_dir       __( (void) );
int    long_info                 __( (void) );
int    short_info                __( (void) );
char   *zipinfo_time             __( (UWORD *datez, UWORD *timez) );

/*---------------------------------------------------------------------------
    Functions in extract.c:
  ---------------------------------------------------------------------------*/

int    extract_or_test_files     __( (void) );
/*
 * static int   store_info               __( (void) );
 * static int   extract_or_test_member   __( (void) );
 */

/*---------------------------------------------------------------------------
    Functions in file_io.c and crypt.c:
  ---------------------------------------------------------------------------*/

int    open_input_file           __( (void) );
int    readbuf                   __( (char *buf, register unsigned size) );
int    create_output_file        __( (void) );
int    FillBitBuffer             __( (void) );
int    ReadByte                  __( (UWORD *x) );
int    FlushOutput               __( (void) );
/*
 * static int   dos2unix         __( (unsigned char *buf, int len) );
 */
void   set_file_time_and_close   __( (void) );
char   *getp                     __( (char *, char *, int) );

int    decrypt_byte              __( (void) );
void   update_keys               __( (int) );
void   init_keys                 __( (char *) );

/*---------------------------------------------------------------------------
    Macintosh file_io functions:
  ---------------------------------------------------------------------------*/

#ifdef MACOS
/*
 * static int   IsHFSDisk        __( (int wAppVRefNum) );
 */
void   macfstest                 __( (int vrefnum) );
int    mkdir                     __( (char *path) );
#ifndef MCH_MACINTOSH            /* The next prototype upsets Aztec C. */
void   SetMacVol                 __( (char *pch, short wVRefNum) );
#endif
#endif

/*---------------------------------------------------------------------------
    Uncompression functions (all internal compression routines, enclosed in
    comments below, are prototyped in their respective files and are invisi-
    ble to external functions):
  ---------------------------------------------------------------------------*/

void   inflate                  __( (void) );                  /* inflate.c */
/*
 */

void   unImplode                __( (void) );                  /* unimplod.c */
/*
 * static void   ReadLengths     __( (sf_tree *tree) );
 * static void   SortLengths     __( (sf_tree *tree) );
 * static void   GenerateTrees   __( (sf_tree *tree, sf_node *nodes) );
 * static void   LoadTree        __( (sf_tree *tree, int treesize, sf_node *nodes) );
 * static void   LoadTrees       __( (void) );
 * static void   ReadTree        __( (register sf_node *nodes, int *dest) );
 */

void   unReduce                 __( (void) );                  /* unreduce.c */
/*
 * static void   LoadFollowers   __( (void) );
 */

void   unShrink                 __( (void) );                  /* unshrink.c */
/*
 * static void   partial_clear   __( (void) );
 */

/*---------------------------------------------------------------------------
    Functions in match.c, mapname.c, misc.c, etc.:
  ---------------------------------------------------------------------------*/

int      match             __( (char *string, char *pattern) );   /* match.c */
/*
 * static BOOLEAN   do_list      __( (register char *string, char *pattern) );
 * static void      list_parse   __( (char **patp, char *lowp, char *highp) );
 * static char      nextch       __( (char **patp) );
 */

int      mapname           __( (int create_dirs) );             /* mapname.c */

void     UpdateCRC         __( (register unsigned char *s, register int len) );
int      do_string         __( (unsigned int len, int option) );   /* misc.c */
time_t   dos_to_unix_time  __( (unsigned ddate, unsigned dtime) ); /* misc.c */
int      check_for_newer   __( (char *filename) );                 /* misc.c */
int      dateformat        __( (void) );                           /* misc.c */
UWORD    makeword          __( (byte *b) );                        /* misc.c */
ULONG    makelong          __( (byte *sig) );                      /* misc.c */
void     return_VMS        __( (int zip_error) );                  /* misc.c */
#ifdef ZMEM
   char  *memset           __( (register char *buf, register char init, register unsigned int len) );
   char  *memcpy           __( (register char *dst, register char *src, register unsigned int len) );
#endif      /* These guys MUST be ifdef'd because their definition  */
            /*  conflicts with the standard one.  Others (makeword, */
            /*  makelong, return_VMS) don't matter.                 */

int      VMSmunch          __( (char *filename, int action, char *extra) );
#ifdef AMIGA
   int   utime             __( (char *file, time_t timep[]) );
#endif





/************/
/*  Macros  */
/************/

#ifndef min    /* MSC defines this in stdlib.h */
#  define min(a,b)   ((a) < (b) ? (a) : (b))
#endif


#define LSEEK(abs_offset) {longint request=(abs_offset)+extra_bytes,\
   inbuf_offset=request%INBUFSIZ, bufstart=request-inbuf_offset;\
   if(request<0) {fprintf(stderr, SeekMsg, ReportMsg); return(3);}\
   else if(bufstart!=cur_zipfile_bufstart)\
   {cur_zipfile_bufstart=lseek(zipfd,bufstart,SEEK_SET);\
   if((incnt=read(zipfd,(char *)inbuf,INBUFSIZ))<=0) return(51);\
   inptr=inbuf+inbuf_offset; incnt-=inbuf_offset;}\
   else {incnt+=(inptr-inbuf)-inbuf_offset; inptr=inbuf+inbuf_offset;}}

/*
 *  Seek to the block boundary of the block which includes abs_offset,
 *  then read block into input buffer and set pointers appropriately.
 *  If block is already in the buffer, just set the pointers.  This macro
 *  is used by process_end_central_dir (unzip.c) and do_string (misc.c).
 *  A slightly modified version is embedded within extract_or_test_files
 *  (unzip.c).  ReadByte and readbuf (file_io.c) are compatible.
 *
 *  macro LSEEK(abs_offset)
 *      longint   abs_offset;
 *  {
 *      longint   request = abs_offset + extra_bytes;
 *      longint   inbuf_offset = request % INBUFSIZ;
 *      longint   bufstart = request - inbuf_offset;
 *
 *      if (request < 0) {
 *          fprintf(stderr, SeekMsg, ReportMsg);
 *          return(3);             /-* 3:  severe error in zipfile *-/
 *      } else if (bufstart != cur_zipfile_bufstart) {
 *          cur_zipfile_bufstart = lseek(zipfd, bufstart, SEEK_SET);
 *          if ((incnt = read(zipfd,inbuf,INBUFSIZ)) <= 0)
 *              return(51);        /-* 51:  unexpected EOF *-/
 *          inptr = inbuf + inbuf_offset;
 *          incnt -= inbuf_offset;
 *      } else {
 *          incnt += (inptr-inbuf) - inbuf_offset;
 *          inptr = inbuf + inbuf_offset;
 *      }
 *  }
 *
 */


#define SKIP_(length) if(length&&((error=do_string(length,SKIP))!=0))\
  {error_in_archive=error; if(error>1) return error;}

/*
 *  Skip a variable-length field, and report any errors.  Used in zipinfo.c
 *  and unzip.c in several functions.
 *
 *  macro SKIP_(length)
 *      UWORD   length;
 *  {
 *      if (length && ((error = do_string(length, SKIP)) != 0)) {
 *          error_in_archive = error;   /-* might be warning *-/
 *          if (error > 1)              /-* fatal *-/
 *              return (error);
 *      }
 *  }
 *
 */


#define OUTB(intc) {*outptr++=intc; if (++outcnt==OUTBUFSIZ) FlushOutput();}

/*
 *  macro OUTB(intc)
 *  {
 *      *outptr++ = intc;
 *      if (++outcnt == OUTBUFSIZ)
 *          FlushOutput();
 *  }
 *
 */


#define READBIT(nbits,zdest) {if(nbits>bits_left) FillBitBuffer();\
  zdest=(int)(bitbuf&mask_bits[nbits]); bitbuf>>=nbits; bits_left-=nbits;}

/*
 * macro READBIT(nbits,zdest)
 *  {
 *      if (nbits > bits_left)
 *          FillBitBuffer();
 *      zdest = (int)(bitbuf & mask_bits[nbits]);
 *      bitbuf >>= nbits;
 *      bits_left -= nbits;
 *  }
 *
 */


#define PEEKBIT(nbits) ( nbits > bits_left ? (FillBitBuffer(), bitbuf & mask_bits[nbits]) : bitbuf & mask_bits[nbits] )


#define NUKE_CRs(buf,len) {register int i,j; for (i=j=0; j<len; (buf)[i++]=(buf)[j++]) if ((buf)[j]=='\r') ++j; len=i;}

/*
 *  Remove all the ASCII carriage returns from buffer buf (length len),
 *  shortening as necessary (note that len gets modified in the process,
 *  so it CANNOT be an expression).  This macro is intended to be used
 *  BEFORE A_TO_N(); hence the check for CR instead of '\r'.  NOTE:  The
 *  if-test gets performed one time too many, but it doesn't matter.
 *
 *  macro NUKE_CRs( buf, len )
 *    {
 *      register int   i, j;
 *
 *      for ( i = j = 0  ;  j < len  ;  (buf)[i++] = (buf)[j++] )
 *        if ( (buf)[j] == CR )
 *          ++j;
 *      len = i;
 *    }
 *
 */


#define TOLOWER(str1,str2) {char *ps1,*ps2; ps1=(str1)-1; ps2=(str2); while(*++ps1) *ps2++=(isupper(*ps1))?tolower(*ps1):*ps1; *ps2='\0';}

/*
 *  Copy the zero-terminated string in str1 into str2, converting any
 *  uppercase letters to lowercase as we go.  str2 gets zero-terminated
 *  as well, of course.  str1 and str2 may be the same character array.
 *
 *  macro TOLOWER( str1, str2 )
 *    {
 *      register char   *ps1, *ps2;
 *
 *      ps1 = (str1) - 1;
 *      ps2 = (str2);
 *      while ( *++ps1 )
 *        *ps2++ = (isupper(*ps1)) ?  tolower(*ps1)  :  *ps1;
 *      *ps2='\0';
 *    }
 *
 *  NOTES:  This macro makes no assumptions about the characteristics of
 *    the tolower() function or macro (beyond its existence), nor does it
 *    make assumptions about the structure of the character set (i.e., it
 *    should work on EBCDIC machines, too).  The fact that either or both
 *    of isupper() and tolower() may be macros has been taken into account;
 *    watch out for "side effects" (in the C sense) when modifying this
 *    macro.
 */


#ifndef ascii_to_native

#  define ascii_to_native(c)   (c)
#  define A_TO_N(str1)

#else

#  ifndef NATIVE
#    define NATIVE     "native chars"
#  endif
#  define A_TO_N(str1) { register unsigned char *ps1; for (ps1 = str1; *ps1; ps1++) *ps1 = (ascii_to_native(*ps1)); }

/*
 *   Translate the zero-terminated string in str1 from ASCII to the native
 *   character set. The translation is performed in-place and uses the
 *   ascii_to_native macro to translate each character.
 *
 *   macro A_TO_N( str1 )
 *     {
 *       register unsigned char *ps1;
 *
 *       for ( ps1 = str1; *ps1; ps1++ )
 *         *ps1 = ( ascii_to_native( *ps1 ) );
 *     }
 *
 *   NOTE: Using the ascii_to_native macro means that is it the only part of
 *     unzip which knows which translation table (if any) is actually in use
 *     to produce the native character set. This makes adding new character
 *     set translation tables easy insofar as all that is needed is an
 *     appropriate ascii_to_native macro definition and the translation
 *     table itself. Currently, the only non-ASCII native character set
 *     implemented is EBCDIC but this may not always be so.
 */

#endif





/*************/
/*  Globals  */
/*************/

   extern int       aflag;
/* extern int       bflag;   reserved */
   extern int       cflag;
   extern int       fflag;
   extern int       jflag;
   extern int       overwrite_none;
   extern int       overwrite_all;
   extern int       force_flag;
   extern int       quietflg;
#ifdef DOS_OS2
   extern int       sflag;
#endif
   extern int       tflag;
   extern int       uflag;
   extern int       V_flag;
#ifdef VMS
   extern int       secinf;
#endif
#ifdef MACOS
   extern int       hfsflag;
#endif
   extern int       process_all_files;
   extern longint   csize;
   extern longint   ucsize;
   extern char      *fnames[];
   extern char      **fnv;
   extern char      sig[];
   extern char      answerbuf[];
   extern min_info  *pInfo;
   extern char      *key;
   extern ULONG     keys[];

   extern short     prefix_of[];
#ifdef MACOS
   extern byte      *suffix_of;
   extern byte      *stack;
#else
   extern byte      suffix_of[];
   extern byte      stack[];
#endif
   extern ULONG     crc32val;
   extern ULONG     mask_bits[];

   extern byte      *inbuf;
   extern byte      *inptr;
   extern int       incnt;
   extern ULONG     bitbuf;
   extern int       bits_left;
   extern boolean   zipeof;
   extern int       zipfd;
   extern char      zipfn[];
   extern longint   extra_bytes;
   extern longint   cur_zipfile_bufstart;
   extern byte      *extra_field;
   extern char      local_hdr_sig[];
   extern char      central_hdr_sig[];
   extern char      end_central_sig[];
   extern local_file_hdr  lrec;
   extern cdir_file_hdr   crec;
   extern ecdir_rec       ecrec;
   extern struct stat     statbuf;

   extern byte      *outbuf;
   extern byte      *outptr;
   extern byte      *outout;
   extern longint   outpos;
   extern int       outcnt;
   extern int       outfd;
   extern int       disk_full;
   extern char      filename[];

   extern char      *EndSigMsg;
   extern char      *CentSigMsg;
   extern char      *SeekMsg;
   extern char      *ReportMsg;

#ifdef DECLARE_ERRNO
   extern int       errno;
#endif

#ifdef EBCDIC
   extern byte      ebcdic[];
#endif
