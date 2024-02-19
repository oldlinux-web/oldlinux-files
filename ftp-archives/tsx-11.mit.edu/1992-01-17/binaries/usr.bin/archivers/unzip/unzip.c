/*---------------------------------------------------------------------------

  unzip.c

  UnZip - a zipfile extraction utility.  See below for make instructions, or
  read the comments in Makefile for a more detailed explanation.  To join 
  Info-ZIP, send a message to info-zip-request@cs.ucla.edu.

  UnZip 4.x is nearly a complete rewrite of version 3.x, mainly to allow 
  access to zipfiles via the central directory (and hence to the OS bytes, 
  so we can make intelligent decisions about what to do with the extracted 
  files).  Based on unzip.c 3.15+ and zipinfo.c 0.90.  For a complete revi-
  sion history, see UnzpHist.zip at Info-ZIP headquarters (below).  For a 
  (partial) list of the many (near infinite) contributors, see "CONTRIBS" in
  the UnZip source distribution.

  ---------------------------------------------------------------------------

  To compile (partial instructions):

     under Unix (cc):  make <system name>
       (type "make list" for a list of valid names, or read Makefile for 
        details.  "make unzip" works for most systems.  If you have a NEW
        system, not covered by any of the existing targets, send FULL infor-
        mation--hardware, OS, versions, etc.--to zip-bugs@cs.ucla.edu)

     under MS-DOS (TurboC):  make -fMAKEFILE.DOS  for command line compiles
       (or use the integrated environment and the included files TCCONFIG.TC
        and UNZIP.PRJ.  Tweak for your environment.)

     under MS-DOS (MSC):  make MAKEFILE.DOS
       (or use Makefile if you have MSC 6.0:  "nmake msc_dos")

     under OS/2 (MSC):  make MAKEFILE.DOS   (edit appropriately)
       (or use Makefile if you have MSC 6.0:  "nmake msc_os2")

     under Atari OS:  needs a little work yet...

     under VMS:  DEFINE LNK$LIBRARY SYS$LIBRARY:VAXCRTL.OLB   (see VMSNOTES)
                 CC UNZIP,FILE_IO,MAPNAME,MATCH,...,UNSHRINK
                 LINK UNZIP,FILE_IO,MAPNAME,MATCH,...,UNSHRINK
                 UNZIP :== $DISKNAME:[DIRECTORY]UNZIP.EXE

     under Macintosh OS:   Double click on unzip.make.  Press <Command>-M.

  ---------------------------------------------------------------------------

  Version:  unzip42.{arc | tar.Z | zip | zoo} for Unix, VMS, OS/2, MS-DOS,
              Mac & Amiga.
  Source:  valeria.cs.ucla.edu (131.179.64.36) in /pub
           wuarchive.wustl.edu (128.252.135.4) in /mirrors/misc/unix
           wsmr-simtel20.army.mil (192.88.110.20) in pd1:[misc.unix]
  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.

  ---------------------------------------------------------------------------*/





#include "unzip.h"              /* includes, defines, and macros */

/* #define VERSION  "v4.20x BETA of 3-20-92"  internal beta level */
#define VERSION  "v4.2 of 20 March 1992"
#define PAKFIX   /* temporary solution to PAK-created zipfiles */





/**********************/
/*  Global Variables  */
/**********************/

int aflag;            /* -a: do ASCII to EBCDIC translation, or CR-LF  */
/* int bflag; RESERVED for -b: extract as binary */
int cflag;            /* -c: output to stdout */
int fflag;            /* -f: "freshen" (extract only newer files) */
int jflag;            /* -j: junk pathnames */
int overwrite_none;   /* -n: never overwrite files (no prompting) */
int overwrite_all;    /* -o: OK to overwrite files without prompting */
int force_flag;       /* (shares -o for now): force to override errors, etc. */
int quietflg;         /* -q: produce a lot less output */
#ifdef DOS_OS2          /*    to CR or LF conversion of extracted files */
   int sflag;         /* -s: allow spaces (blanks) in filenames */
#endif /* DOS_OS2 */
int tflag;            /* -t: test */
int uflag;            /* -u: "update" (extract only newer & brand-new files) */
static int U_flag;    /* -U: leave filenames in upper or mixed case */
static int vflag;     /* -v: view directory (only used in unzip.c) */
int V_flag;           /* -V: don't strip VMS version numbers */
#ifdef VMS
   int secinf;        /* -X: keep owner/protection */
#endif /* VMS */
int zflag;            /* -z: display only the archive comment */
int process_all_files;

longint csize;        /* used by list_files(), ReadByte(): must be signed */
longint ucsize;       /* used by list_files(), unReduce(), unImplode() */

char *fnames[2] = {"*", NULL};   /* default filenames vector */
char **fnv = fnames;
char sig[5];
char answerbuf[10];

min_info info[DIR_BLKSIZ], *pInfo=info;

#ifdef OS2
   int longname;           /* used only in mapname.c and file_io.c */
   char longfilename[FILNAMSIZ];
#endif /* OS2 */

#ifdef CRYPT
   char *key = NULL;       /* password with which to decrypt data, or NULL */
#endif /* CRYPT */

/*---------------------------------------------------------------------------
    unShrink/unReduce/unImplode working storage:
  ---------------------------------------------------------------------------*/

/* prefix_of (for unShrink) is biggest storage area, esp. on Crays...space */
/*  is shared by lit_nodes (unImplode) and followers (unReduce) */

short prefix_of[HSIZE + 1];     /* (8193 * sizeof(short)) */
#ifdef MACOS
   byte *suffix_of;
   byte *stack;
#else
   byte suffix_of[HSIZE + 1];   /* also s-f length_nodes (smaller) */
   byte stack[HSIZE + 1];       /* also s-f distance_nodes (smaller) */
#endif

ULONG crc32val;

ULONG mask_bits[] =
{0x00000000L,
 0x00000001L, 0x00000003L, 0x00000007L, 0x0000000fL,
 0x0000001fL, 0x0000003fL, 0x0000007fL, 0x000000ffL,
 0x000001ffL, 0x000003ffL, 0x000007ffL, 0x00000fffL,
 0x00001fffL, 0x00003fffL, 0x00007fffL, 0x0000ffffL,
 0x0001ffffL, 0x0003ffffL, 0x0007ffffL, 0x000fffffL,
 0x001fffffL, 0x003fffffL, 0x007fffffL, 0x00ffffffL,
 0x01ffffffL, 0x03ffffffL, 0x07ffffffL, 0x0fffffffL,
 0x1fffffffL, 0x3fffffffL, 0x7fffffffL, 0xffffffffL};

/*---------------------------------------------------------------------------
    Input file variables:
  ---------------------------------------------------------------------------*/

byte *inbuf, *inptr;     /* input buffer (any size is legal) and pointer */
int incnt;

ULONG bitbuf;
int bits_left;
boolean zipeof;

int zipfd;               /* zipfile file handle */
char zipfn[FILNAMSIZ];

char local_hdr_sig[5] = "\120";    /* remaining signature bytes come later   */
char central_hdr_sig[5] = "\120";  /*  (must initialize at runtime so unzip  */
char end_central_sig[5] = "\120";  /*  executable won't look like a zipfile) */

cdir_file_hdr crec;      /* used in unzip.c, extract.c, misc.c */
local_file_hdr lrec;     /* used in unzip.c, extract.c */
ecdir_rec ecrec;         /* used in unzip.c, extract.c */
struct stat statbuf;     /* used by main(), mapped_name(), check_for_newer() */

longint extra_bytes = 0;        /* used in unzip.c, misc.c */
longint cur_zipfile_bufstart;   /* extract_or_test_files, readbuf, ReadByte */

byte *extra_field = NULL;       /* currently used by VMS version only */

/*---------------------------------------------------------------------------
    Output stream variables:
  ---------------------------------------------------------------------------*/

byte *outbuf;                   /* buffer for rle look-back */
byte *outptr;
byte *outout;                   /* scratch pad for ASCII-native trans */
longint outpos;                 /* absolute position in outfile */
int outcnt;                     /* current position in outbuf */
int outfd;
int disk_full;
char filename[FILNAMSIZ];

/*---------------------------------------------------------------------------
    unzip.c static global variables (visible only within this file):
  ---------------------------------------------------------------------------*/

static byte *hold;
static char unkn[10];
static longint ziplen;
static UWORD methnum;

/*---------------------------------------------------------------------------
    unzip.c repeated error messages (we use all of these at least twice)
  ---------------------------------------------------------------------------*/

char *EndSigMsg = "\nwarning:\
  didn't find end-of-central-dir signature at end of central dir.\n";
char *CentSigMsg =
  "error:  expected central file header signature not found (file #%u).\n";
char *SeekMsg =
  "error:  attempt to seek before beginning of zipfile\n%s";

#ifdef VMS
char *ReportMsg = "\
  (please check that you have transferred or created the zipfile in the\n\
  appropriate BINARY mode--this includes ftp, Kermit, AND unzip'd zipfiles)\n";
#else /* !VMS */
char *ReportMsg = "\
  (please check that you have transferred or created the zipfile in the\n\
  appropriate BINARY mode and that you have compiled unzip properly)\n";
#endif /* ?VMS */




/******************/
/*  Main program  */
/******************/

main(argc, argv)        /* return PK-type error code (except under VMS) */
int argc;
char *argv[];
{
    char *s;
    int c, error=FALSE;


/*---------------------------------------------------------------------------
    Macintosh initialization code.
  ---------------------------------------------------------------------------*/

#ifdef MACOS
#ifdef THINK_C
    #include <console.h>
    static char *argstr[30], args[30*64];

    Point   p;
    SFTypeList  sfT;
    int a;
    EventRecord theEvent;
    short   eMask;
    SFReply  fileRep;
#endif /* THINK_C */

    typedef struct sf_node {        /* node in a true shannon-fano tree */
        UWORD left;                 /* 0 means leaf node */
        UWORD right;                /*   or value if leaf node */
    } sf_node;
 
    extern sf_node *lit_nodes, *length_nodes, *distance_nodes;

#ifdef MCH_MACINTOSH
    defSpin(0x20);
#endif

    suffix_of = (byte *)calloc(HSIZE+1, sizeof(byte));
    stack = (byte *)calloc(HSIZE+1, sizeof(byte));
    length_nodes = (sf_node *) suffix_of;  /* 2*LENVALS nodes */
    distance_nodes = (sf_node *) stack;    /* 2*DISTVALS nodes */

#ifdef THINK_C   
    for (a=0; a<30; a+=1)
        argstr[a] = &args[a*64];
start:
    tflag=vflag=cflag=aflag=jflag=U_flag=quietflg=fflag=uflag=zflag = 0;
    local_hdr_sig[1]=central_hdr_sig[1]=end_central_sig[1]='\0';

    argc = ccommand(&argv);
    SetPt(&p, 40,40);

    SFGetFile(p, "\pSpecify ZIP file:", 0L, -1, sfT, 0L, &fileRep);
    if (!fileRep.good)
        exit(1);
    macfstest(fileRep.vRefNum);
    SetMacVol(NULL, fileRep.vRefNum);
    for (a=1; a<argc; a+=1)
        if (argv[a][0] == '-')
            BlockMove(argv[a], argstr[a], (strlen(argv[a])>63) ? 64 : strlen(argv[a])+1);
        else
            break;
    PtoCstr((char *)fileRep.fName);
    strcpy(argstr[a], (char *)fileRep.fName);
    for (;a<argc; a+=1)
        BlockMove(argv[a], argstr[a+1], (strlen(argv[a])>63) ? 64 : strlen(argv[a])+1);
    argc+=1;
    argv = argstr;

    if (hfsflag == FALSE)   /* can't support directories:  junk pathnames */
        jflag = 1;
#endif /* THINK_C */
#endif /* MACOS */

/*---------------------------------------------------------------------------
    Debugging info for checking on structure padding:
  ---------------------------------------------------------------------------*/

#ifdef DEBUG_STRUC
    printf("local_file_hdr size: %X\n",
           sizeof(local_file_hdr));
    printf("local_byte_hdr size: %X\n",
           sizeof(local_byte_hdr));
    printf("actual size of local headers: %X\n", LREC_SIZE);

    printf("central directory header size: %X\n",
           sizeof(cdir_file_hdr));
    printf("central directory byte header size: %X\n",
           sizeof(cdir_byte_hdr));
    printf("actual size of central dir headers: %X\n", CREC_SIZE);

    printf("end central dir record size: %X\n",
           sizeof(ecdir_rec));
    printf("end central dir byte record size: %X\n",
           sizeof(ec_byte_rec));
    printf("actual size of end-central-dir record: %X\n", ECREC_SIZE);
#endif /* DEBUG_STRUC */

/*---------------------------------------------------------------------------
    Rip through any command-line options lurking about...
  ---------------------------------------------------------------------------*/

    while (--argc > 0 && (*++argv)[0] == '-') {
        s = argv[0] + 1;
        while ((c = *s++) != 0) {    /* "!= 0":  prevent Turbo C warning */
            switch (c) {
            case ('a'):
                ++aflag;
                break;
#if 0
            case ('s'):
                ++sflag;
                break;
#endif
            case ('c'):
                ++cflag;
#ifdef NATIVE
                ++aflag;   /* this is so you can read it on the screen */
#endif
                break;
            case ('d'):    /* re-create directory structure (now by default) */
                break;
            case ('e'):    /* just ignore -e, -x options (extract) */
                break;
            case ('f'):    /* "freshen" (extract only newer files) */
                ++fflag;
                ++uflag;
                break;
            case ('j'):    /* junk pathnames/directory structure */
                ++jflag;
                break;
         /* case ('l') is below, after fall-through for 'v' */
            case ('n'):    /* don't overwrite any files */
                overwrite_none = TRUE;
                break;
            case ('o'):    /* OK to overwrite files without prompting */
                overwrite_all = TRUE;
                force_flag = TRUE;  /* (share -o for now): force to continue */
                break;
            case ('p'):
                ++cflag;
#if defined(NATIVE) && !defined(DOS_OS2)
                ++aflag;
#endif
                quietflg += 99;
                break;
            case ('q'):
                ++quietflg;
                break;
#ifdef DOS_OS2
            case ('s'):
                ++sflag;
                break;
#endif
            case ('t'):
                ++tflag;
                break;
            case ('U'):    /* Uppercase flag (i.e., don't convert to lower) */
                ++U_flag;
                break;
            case ('u'):    /* "update" (extract only new and newer files) */
                ++uflag;
                break;
            case ('V'):    /* Version flag:  retain VMS/DEC-20 file versions */
                ++V_flag;
                break;
            case ('v'):
                ++vflag;
                /* fall thru */
            case ('l'):
                ++vflag;
                break;
#ifdef VMS
            case ('X'):    /* restore owner/protection info (may need privs) */
                secinf = TRUE;
                break;
#endif /* VMS */
            case ('x'):    /* extract:  default */
                break;
            case ('z'):    /* display only the archive comment */
                ++zflag;
                break;
            default:
                error = TRUE;
                break;
            }
        }
    }

/*---------------------------------------------------------------------------
    Make sure we aren't trying to do too many things here.  [This seems like
    kind of a brute-force way to do things; but aside from that, isn't the
    -a option useful when listing the directory (i.e., for reading zipfile
    comments)?  It's a modifier, not an action in and of itself, so perhaps
    it should not be included in the test--certainly, in the case of zipfile
    testing, it can just be ignored.]
  ---------------------------------------------------------------------------*/

    if ((aflag && tflag) || (aflag && vflag) || (cflag && tflag) ||
        (cflag && uflag) || (cflag && vflag) || (tflag && uflag) ||
        (tflag && vflag) || (uflag && vflag) || (fflag && overwrite_none)) {
        fprintf(stderr, "error:\
  -at, -av, -ct, -cu, -cv, -fn, -tu, -tv, -uv combinations not allowed\n");
        error = TRUE;
    }
    if (quietflg && zflag)
        quietflg = 0;
    if (overwrite_all && overwrite_none) {
        fprintf(stderr, "caution:  both -n and -o specified; ignoring -o\n");
        overwrite_all = FALSE;
    }
    if ((argc-- == 0) || error)
        RETURN(usage(error));

/*---------------------------------------------------------------------------
    Now get the zipfile name from the command line and see if it exists as a
    regular (non-directory) file.  If not, append the ".zip" suffix.  We don't
    immediately check to see if this results in a good name, but we will do so
    later.  In the meantime, see if there are any member filespecs on the com-
    mand line, and if so, set the filename pointer to point at them.
  ---------------------------------------------------------------------------*/

    strcpy(zipfn, *argv++);
    if (stat(zipfn, &statbuf) || (statbuf.st_mode & S_IFMT) == S_IFDIR)
        strcat(zipfn, ZSUFX);
#if defined(UNIX) && !defined(VMS)   /* Unix executables have no extension-- */
    else if (statbuf.st_mode & S_IEXEC)  /* might find zip, not zip.zip; etc */
        fprintf(stderr, "\nnote:  file [ %s ] may be an executable\n\n", zipfn);
#endif /* UNIX && !VMS */

    if (stat(zipfn, &statbuf)) {/* try again */
        fprintf(stderr, "error:  can't find zipfile [ %s ]\n", zipfn);
        RETURN(9);              /* 9:  file not found */
    } else
        ziplen = statbuf.st_size;

    if (argc != 0) {
        fnv = argv;
        process_all_files = FALSE;
    } else
        process_all_files = TRUE;       /* for speed */

/*---------------------------------------------------------------------------
    Okey dokey, we have everything we need to get started.  Let's roll.
  ---------------------------------------------------------------------------*/

    inbuf = (byte *) malloc(INBUFSIZ + 4);     /* 4 extra for hold[] (below) */
    outbuf = (byte *) malloc(OUTBUFSIZ + 1);   /* 1 extra for string termin. */
#ifndef DOS_OS2
    if (aflag)                  /* if need an ascebc scratch, */
        outout = (byte *) malloc(OUTBUFSIZ);
    else                        /*  allocate it... */
#endif /* !DOS_OS2 */
        outout = outbuf;        /*  else just point to outbuf */

    if ((inbuf == NULL) || (outbuf == NULL) || (outout == NULL)) {
        fprintf(stderr, "error:  can't allocate unzip buffers\n");
        RETURN(4);              /* 4-8:  insufficient memory */
    }
    hold = &inbuf[INBUFSIZ];    /* to check for boundary-spanning signatures */

#ifdef THINK_C
    if (!process_zipfile())
        goto start;
#else
    RETURN(process_zipfile());  /* keep passing errors back... */
#endif

}       /* end main() */





/**********************/
/*  Function usage()  */
/**********************/

int usage(error)   /* return PK-type error code */
    int error;
{
#ifdef NATIVE
#ifdef EBCDIC
    char *astring = "-a  convert ASCII to EBCDIC";
#else /* !EBCDIC */
    char *astring = "-a  convert ASCII to native chars";
#endif /* ?EBCDIC *?
/*  char *astring = "-a  convert ASCII to " NATIVE;  (ANSI C concatenation)  */
    char *loc_str = "";
#else /* !NATIVE */
#ifdef DOS_OS2
    char *astring = "-a  convert text (LF => CR LF)";
    char *loc_str = "-s  allow spaces in filenames";
#else /* !DOS_OS2 */
#ifdef MACOS
    char *astring = "-a  convert text (CR LF => CR)";
    char *loc_str = "";
#else /* !MACOS:  UNIX, VMS */
    char *astring = "-a  convert text (CR LF => LF)";
#ifdef VMS
    char *loc_str = "-X  restore owner/protection info";
#else /* !VMS */
    char *loc_str = "";
#endif /* ?VMS */
#endif /* ?MACOS */
#endif /* ?DOS_OS2 */
#endif /* ?NATIVE */
    FILE *usagefp;


/*---------------------------------------------------------------------------
    If user requested usage, send it to stdout; else send to stderr.
  ---------------------------------------------------------------------------*/

    if (error)
        usagefp = (FILE *) stderr;
    else
        usagefp = (FILE *) stdout;

    fprintf(usagefp, "\
UnZip:  Zipfile Extract %s;  (c) 1989 S.H.Smith and others\n\
Versions 3.0 and later by Info-ZIP.  Bug reports ONLY to zip-bugs@cs.ucla.edu\
\n\n", VERSION);

    fprintf(usagefp, "\
Usage: unzip [ -options[modifiers] ] file[.zip] [filespec...]\n\
  -x  extract files (default)                -l  list files (short format)\n\
  -c  extract files to stdout/screen (CRT)   -v  list files (verbose format)\n\
  -f  freshen existing files, create none    -p  extract to pipe, no messages\n\
  -u  update files, create if necessary      -t  test archive integrity\n\
                                             -z  display archive comment\n\
modifiers:\n\
  -n  never overwrite existing files         %s\n", loc_str);
    fprintf(usagefp, "\
  -o  overwrite files WITHOUT prompting      %s\n\
  -j  junk paths (don't make directories)    -U  don't make names lowercase\n\
  -q  quiet mode (-qq => quieter)            -V  retain VMS version numbers\
\n\n\
Examples: (See manual for more information)\n\
  unzip data1 Readme   => extracts file Readme from zipfile data1.zip\n\
  unzip -p foo | more  => send contents of foo.zip via pipe into program more\n\
  unzip -fo foo        => quietly replace existing files if archive files newer\
\n", astring);

#ifdef VMS
    fprintf(usagefp, "\
  unzip \"-V\" foo \"Bar\" => must quote uppercase options and filenames in VMS\
\n");
#endif

    if (error)
        return 10;    /* 10:  bad or illegal parameters specified */
    else
        return 0;     /* just wanted usage screen: no error */

}       /* end function usage() */





/********************************/
/*  Function process_zipfile()  */
/********************************/

int process_zipfile()    /* return PK-type error code */
{
    int error=0, error_in_archive;
    longint real_ecrec_offset, expect_ecrec_offset;


/*---------------------------------------------------------------------------
    Open the zipfile for reading and in BINARY mode to prevent CR/LF trans-
    lation, which would corrupt the bitstreams.
  ---------------------------------------------------------------------------*/

#ifdef VMS
    {
        int rtype;

        VMSmunch(zipfn, GET_RTYPE, (char *)&rtype);
        if (rtype == FAT$C_VARIABLE) {
            fprintf(stderr,
     "\n     Error:  zipfile is in variable-length record format.  Please\n\
     run \"bilf l %s\" to convert the zipfile to stream-LF\n\
     record format.  (Bilf.exe, bilf.c and make_bilf.com are included\n\
     in the VMS unzip distribution.)\n\n", zipfn);
            return 2;           /* 2:  error in zipfile */
        }
        rtype = FAT$C_STREAMLF; /* Unix I/O loves it */
        VMSmunch(zipfn, CHANGE_RTYPE, (char *)&rtype);
    }
#endif
    if (open_input_file())      /* this should never happen, given the */
        return (9);             /*   stat() test in main(), but... */

/*---------------------------------------------------------------------------
    Reconstruct the various PK signature strings; find and process the cen-
    tral directory; list, extract or test member files as instructed; and
    close the zipfile.
  ---------------------------------------------------------------------------*/

    strcat(local_hdr_sig, LOCAL_HDR_SIG);
    strcat(central_hdr_sig, CENTRAL_HDR_SIG);
    strcat(end_central_sig, END_CENTRAL_SIG);

    if (find_end_central_dir()) /* not found; nothing to do */
        return (2);             /* 2:  error in zipfile */

    real_ecrec_offset = cur_zipfile_bufstart+(inptr-inbuf);
#ifdef TEST
    printf("\n  found end-of-central-dir signature at offset %ld (%.8lXh)\n",
      real_ecrec_offset, real_ecrec_offset);
    printf("    from beginning of file; offset %d (%.4Xh) within block\n",
      inptr-inbuf, inptr-inbuf);
#endif

    if ((error_in_archive = process_end_central_dir()) > 1)
        return (error_in_archive);

    if (zflag)
        return (0);

#ifndef PAKFIX
    if (ecrec.number_this_disk == 0) {
#else /* PAKFIX */
    if ((ecrec.number_this_disk == 0)  ||
        (error = ((ecrec.number_this_disk == 1) &&
                  (ecrec.num_disk_with_start_central_dir == 1)) )) {

        if (error) {
            fprintf(stderr,
     "\n     Warning:  zipfile claims to be disk 2 of a two-part archive;\n\
     attempting to process anyway.  If no further errors occur, this\n\
     archive was probably created by PAK v2.5 or earlier.  This bug\n\
     was reported to NoGate and should have been fixed by mid-1991.\n\n");
            error_in_archive = 1;  /* 1:  warning */
        }
#endif /* ?PAKFIX */
        expect_ecrec_offset = ecrec.offset_start_central_directory +
                              ecrec.size_central_directory;
        if ((extra_bytes = real_ecrec_offset - expect_ecrec_offset) < 0) {
            fprintf(stderr, "\nerror:  missing %ld bytes in zipfile (\
attempting to process anyway)\n\n", -extra_bytes);
            error_in_archive = 2;       /* 2:  (weak) error in zipfile */
        } else if (extra_bytes > 0) {
            if ((ecrec.offset_start_central_directory == 0) &&
                (ecrec.size_central_directory != 0))   /* zip 1.5 -go bug */
            {
                fprintf(stderr, "\nerror:  NULL central directory offset (\
attempting to process anyway)\n\n");
                ecrec.offset_start_central_directory = extra_bytes;
                extra_bytes = 0;
                error_in_archive = 2;   /* 2:  (weak) error in zipfile */
            } else {
                fprintf(stderr, "\nwarning:  extra %ld bytes at beginning or\
 within zipfile\n          (attempting to process anyway)\n\n", extra_bytes);
                error_in_archive = 1;   /* 1:  warning error */
            }
        }
        LSEEK( ecrec.offset_start_central_directory )
        if (vflag)
            error = list_files();               /* LIST 'EM */
        else
            error = extract_or_test_files();    /* EXTRACT OR TEST 'EM */
        if (error > error_in_archive)   /* don't overwrite stronger error */
            error_in_archive = error;   /*  with (for example) a warning */
    } else {
        fprintf(stderr, "\nerror:  zipfile is part of multi-disk archive \
(sorry, not supported).\n");
        fprintf(stderr, "Please report to zip-bugs@cs.ucla.edu\n");
        error_in_archive = 11;  /* 11:  no files found */
    }

    close(zipfd);
#ifdef VMS
    VMSmunch(zipfn, RESTORE_RTYPE, NULL);
#endif
    return (error_in_archive);

}       /* end function process_zipfile() */





/************************************/
/*  Function find_end_central_dir() */
/************************************/

int find_end_central_dir()
/* return 0 if found, 1 otherwise */
{
    int i, numblks;
    longint tail_len;



/*---------------------------------------------------------------------------
    Treat case of short zipfile separately.
  ---------------------------------------------------------------------------*/

    if (ziplen <= INBUFSIZ) {
        lseek(zipfd, 0L, SEEK_SET);
        if ((incnt = read(zipfd,(char *)inbuf,(unsigned int)ziplen)) == ziplen)

            /* 'P' must be at least 22 bytes from end of zipfile */
            for ( inptr = inbuf+ziplen-22  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp((char *)inptr, end_central_sig, 4) ) {
                    incnt -= inptr - inbuf;
                    return(0);  /* found it! */
                }               /* ...otherwise fall through & fail */

/*---------------------------------------------------------------------------
    Zipfile is longer than INBUFSIZ:  may need to loop.  Start with short
    block at end of zipfile (if not TOO short).
  ---------------------------------------------------------------------------*/

    } else {
        if ((tail_len = ziplen % INBUFSIZ) > ECREC_SIZE) {
            cur_zipfile_bufstart = lseek(zipfd, ziplen-tail_len, SEEK_SET);
            if ((incnt = read(zipfd,(char *)inbuf,(unsigned int)tail_len)) != tail_len)
                goto fail;      /* shut up, it's expedient. */

            /* 'P' must be at least 22 bytes from end of zipfile */
            for ( inptr = inbuf+tail_len-22  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp((char *)inptr, end_central_sig, 4) ) {
                    incnt -= inptr - inbuf;
                    return(0);  /* found it! */
                }               /* ...otherwise search next block */
            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
                                                           boundary */

        } else {
            cur_zipfile_bufstart = ziplen - tail_len;
        }

        /*
         * Loop through blocks of zipfile data, starting at the end and going
         * toward the beginning.  Need only check last 65557 bytes of zipfile:
         * comment may be up to 65535 bytes long, end-of-central-directory rec-
         * ord is 18 bytes (shouldn't hardcode this number, but what the hell:
         * already did so above (22=18+4)), and sig itself is 4 bytes.
         */

        /*          ==amt to search==   ==done==   ==rounding==     =blksiz= */
        numblks = ( min(ziplen,65557) - tail_len + (INBUFSIZ-1) ) / INBUFSIZ;

        for ( i = 1  ;  i <= numblks  ;  ++i ) {
            cur_zipfile_bufstart -= INBUFSIZ;
            lseek(zipfd, cur_zipfile_bufstart, SEEK_SET);
            if ((incnt = read(zipfd,(char *)inbuf,INBUFSIZ)) != INBUFSIZ)
                break;          /* fall through and fail */

            for ( inptr = inbuf+INBUFSIZ-1  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp((char *)inptr, end_central_sig, 4) ) {
                    incnt -= inptr - inbuf;
                    return(0);  /* found it! */
                }
            strncpy((char *)hold, (char *)inbuf, 3);    /* sig may span block
                                                           boundary */
        }

    } /* end if (ziplen > INBUFSIZ) */

/*---------------------------------------------------------------------------
    Searched through whole region where signature should be without finding
    it.  Print informational message and die a horrible death.
  ---------------------------------------------------------------------------*/

fail:

    fprintf(stderr, "\nFile:  %s\n\n\
     End-of-central-directory signature not found.  Either this file is not\n\
     a zipfile, or it constitutes one disk of a multi-part archive.  In the\n\
     latter case the central directory and zipfile comment will be found on\n\
     the last disk(s) of this archive.\n", zipfn);
    return(1);

}       /* end function find_end_central_dir() */





/***************************************/
/*  Function process_end_central_dir() */
/***************************************/

int process_end_central_dir()    /* return PK-type error code */
{
    ec_byte_rec byterec;
    int error=0;


/*---------------------------------------------------------------------------
    Read the end-of-central-directory record and do any necessary machine-
    type conversions (byte ordering, structure padding compensation) by
    reading data into character array, then copying to struct.
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, ECREC_SIZE+4) <= 0)
        return (51);

    ecrec.number_this_disk =
        makeword(&byterec[NUMBER_THIS_DISK]);
    ecrec.num_disk_with_start_central_dir =
        makeword(&byterec[NUM_DISK_WITH_START_CENTRAL_DIR]);
    ecrec.num_entries_centrl_dir_ths_disk =
        makeword(&byterec[NUM_ENTRIES_CENTRL_DIR_THS_DISK]);
    ecrec.total_entries_central_dir =
        makeword(&byterec[TOTAL_ENTRIES_CENTRAL_DIR]);
    ecrec.size_central_directory =
        makelong(&byterec[SIZE_CENTRAL_DIRECTORY]);
    ecrec.offset_start_central_directory =
        makelong(&byterec[OFFSET_START_CENTRAL_DIRECTORY]);
    ecrec.zipfile_comment_length =
        makeword(&byterec[ZIPFILE_COMMENT_LENGTH]);

/*---------------------------------------------------------------------------
    Get the zipfile comment, if any, and print it out.  (Comment may be up
    to 64KB long.  May the fleas of a thousand camels infest the armpits of
    anyone who actually takes advantage of this fact.)  Then position the
    file pointer to the beginning of the central directory and fill buffer.
  ---------------------------------------------------------------------------*/

    if (ecrec.zipfile_comment_length && !quietflg) {
        if (!zflag)
          printf("[%s] comment:\n", zipfn);
        if (do_string(ecrec.zipfile_comment_length,DISPLAY)) {
            fprintf(stderr, "\ncaution:  zipfile comment truncated\n");
            error = 1;          /* 1:  warning error */
        }
#if 0
        if (!zflag)
          printf("\n\n");       /* what the heck is this doing here?! */
#endif
    }

    return (error);

}       /* end function process_end_central_dir() */





/**************************/
/*  Function list_files() */
/**************************/

int list_files()    /* return PK-type error code */
{
    char **fnamev;
    int do_this_file=FALSE, ratio, error, error_in_archive=0;
    int which_hdr=(vflag>1);
    UWORD j, yr, mo, dy, hh, mm, members=0;
    ULONG tot_csize=0L, tot_ucsize=0L;
    min_info info;
    static char *method[NUM_METHODS+1] =
        {"Stored", "Shrunk", "Reduce1", "Reduce2", "Reduce3", "Reduce4",
         "Implode", "Token", "Deflate", unkn};
    static char *Headers[][2] = {
        {" Length    Date    Time    Name",
         " ------    ----    ----    ----"},
        {" Length  Method   Size  Ratio   Date    Time   CRC-32     Name",
         " ------  ------   ----  -----   ----    ----   ------     ----"}
    };



/*---------------------------------------------------------------------------
    Unlike extract_or_test_files(), this routine confines itself to the cen-
    tral directory.  Thus its structure is somewhat simpler, since we can do
    just a single loop through the entire directory, listing files as we go.

    So to start off, print the heading line and then begin main loop through
    the central directory.  The results will look vaguely like the following:

  Length  Method   Size  Ratio   Date    Time   CRC-32     Name ("^" ==> case
  ------  ------   ----  -----   ----    ----   ------     ----   conversion)
   44004  Implode  13041  71%  11-02-89  19:34  8b4207f7   Makefile.UNIX
    3438  Shrunk    2209  36%  09-15-90  14:07  a2394fd8  ^dos-file.ext
  ---------------------------------------------------------------------------*/

    pInfo = &info;

    if (quietflg < 2)
        if (U_flag)
            printf("%s\n%s\n", Headers[which_hdr][0], Headers[which_hdr][1]);
        else
            printf("%s (\"^\" ==> case\n%s   conversion)\n", 
              Headers[which_hdr][0], Headers[which_hdr][1]);

    for (j = 0; j < ecrec.total_entries_central_dir; ++j) {

        if (readbuf(sig, 4) <= 0)
            return (51);        /* 51:  unexpected EOF */
        if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
            fprintf(stderr, CentSigMsg, j);  /* sig not found */
            fprintf(stderr, ReportMsg);   /* check binary transfers */
            return (3);         /* 3:  error in zipfile */
        }
        if ((error = process_cdir_file_hdr()) != 0)  /* (sets pInfo->lcflag) */
            return (error);     /* only 51 (EOF) defined */

        /*
         * We could DISPLAY the filename instead of storing (and possibly trun-
         * cating, in the case of a very long name) and printing it, but that
         * has the disadvantage of not allowing case conversion--and it's nice
         * to be able to see in the listing precisely how you have to type each
         * filename in order for unzip to consider it a match.  Speaking of
         * which, if member names were specified on the command line, check in
         * with match() to see if the current file is one of them, and make a
         * note of it if it is.
         */

        if ((error = do_string(crec.filename_length, FILENAME)) != 0) {
            error_in_archive = error;  /*             ^--(uses pInfo->lcflag) */
            if (error > 1)      /* fatal:  can't continue */
                return (error);
        }
        if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0) {
            error_in_archive = error;  
            if (error > 1)      /* fatal:  can't continue */
                return (error);
        }
        if (!process_all_files) {   /* check if specified on command line */
            do_this_file = FALSE;
            fnamev = fnv;       /* don't destroy permanent filename ptr */
            for (--fnamev; *++fnamev;)
                if (match(filename, *fnamev)) {
                    do_this_file = TRUE;
                    break;      /* found match, so stop looping */
                }
        }
        /*
         * If current file was specified on command line, or if no names were
         * specified, do the listing for this file.  Otherwise, get rid of the
         * file comment and go back for the next file.
         */

        if (process_all_files || do_this_file) {

            yr = (((crec.last_mod_file_date >> 9) & 0x7f) + 80) % (unsigned)100;
            mo = (crec.last_mod_file_date >> 5) & 0x0f;
            dy = crec.last_mod_file_date & 0x1f;

            /* twist date so it displays according to national convention */
            switch (dateformat()) {
                case DF_YMD:
                    hh = mo; mo = yr; yr = dy; dy = hh; break;
                case DF_DMY:
                    hh = mo; mo = dy; dy = hh;
            }
            hh = (crec.last_mod_file_time >> 11) & 0x1f;
            mm = (crec.last_mod_file_time >> 5) & 0x3f;

            csize = (longint) crec.compressed_size;
            ucsize = (longint) crec.uncompressed_size;
            if (crec.general_purpose_bit_flag & 1)
                csize -= 12;    /* if encrypted, don't count encrypt hdr */

            ratio = (ucsize == 0) ? 0 :   /* .zip can have 0-length members */
                ((ucsize > 2000000) ?     /* risk signed overflow if mult. */
                (int) ((ucsize-csize) / (ucsize/1000L)) + 5 :   /* big */
                (int) ((1000L*(ucsize-csize)) / ucsize) + 5);   /* small */

            switch (which_hdr) {
                case 0:   /* short form */
                    printf("%7ld  %02u-%02u-%02u  %02u:%02u  %c%s\n",
                      ucsize, mo, dy, yr, hh, mm, (pInfo->lcflag?'^':' '),
                      filename);
                    break;
                case 1:   /* verbose */
                    printf(
              "%7ld  %-7s%7ld %3d%%  %02u-%02u-%02u  %02u:%02u  %08lx  %c%s\n",
                      ucsize, method[methnum], csize, ratio/10, mo, dy, yr,
                      hh, mm, crec.crc32, (pInfo->lcflag?'^':' '), filename);
            }

            error = do_string(crec.file_comment_length, (QCOND2? DISPLAY:SKIP));
            if (error) {
                error_in_archive = error;  /* might be just warning */
                if (error > 1)  /* fatal */
                    return (error);
            }
            tot_ucsize += (ULONG) ucsize;
            tot_csize += (ULONG) csize;
            ++members;

        } else {        /* not listing this file */
            SKIP_(crec.file_comment_length)
        }
    }                   /* end for-loop (j: files in central directory) */

/*---------------------------------------------------------------------------
    Print footer line and totals (compressed size, uncompressed size, number
    of members in zipfile).
  ---------------------------------------------------------------------------*/

    ratio = (tot_ucsize == 0) ? 
        0 : ((tot_ucsize > 4000000) ?    /* risk unsigned overflow if mult. */
        (int) ((tot_ucsize - tot_csize) / (tot_ucsize/1000L)) + 5 :
        (int) ((tot_ucsize - tot_csize) * 1000L / tot_ucsize) + 5);

    if (quietflg < 2) {
        switch (which_hdr) {
        case 0:         /* short */
            printf("%s\n%7lu                    %-7u\n",
                   " ------                    -------",
                   tot_ucsize, members);
            break;
        case 1:         /* verbose */
            printf(
              "%s\n%7lu         %7lu %3d%%                              %-7u\n",
              " ------          ------  ---                              -------",
              tot_ucsize, tot_csize, ratio / 10, members);
        }
    }
/*---------------------------------------------------------------------------
    Double check that we're back at the end-of-central-directory record.
  ---------------------------------------------------------------------------*/

    readbuf(sig, 4);
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
/*      fprintf(stderr, ReportMsg);   */
        error_in_archive = 1;        /* 1:  warning error */
    }
    return (error_in_archive);

}       /* end function list_files() */





/**************************************/
/*  Function process_cdir_file_hdr()  */
/**************************************/

int process_cdir_file_hdr()    /* return PK-type error code */
{
    cdir_byte_hdr byterec;


/*---------------------------------------------------------------------------
    Read the next central directory entry and do any necessary machine-type
    conversions (byte ordering, structure padding compensation--do so by
    copying the data from the array into which it was read (byterec) to the
    usable struct (crec)).
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, CREC_SIZE) <= 0)
        return (51);            /* 51:  unexpected EOF */

    crec.version_made_by[0] = byterec[C_VERSION_MADE_BY_0];
    crec.version_made_by[1] = byterec[C_VERSION_MADE_BY_1];
    crec.version_needed_to_extract[0] = byterec[C_VERSION_NEEDED_TO_EXTRACT_0];
    crec.version_needed_to_extract[1] = byterec[C_VERSION_NEEDED_TO_EXTRACT_1];

    crec.general_purpose_bit_flag =
        makeword(&byterec[C_GENERAL_PURPOSE_BIT_FLAG]);
    crec.compression_method =
        makeword(&byterec[C_COMPRESSION_METHOD]);
    crec.last_mod_file_time =
        makeword(&byterec[C_LAST_MOD_FILE_TIME]);
    crec.last_mod_file_date =
        makeword(&byterec[C_LAST_MOD_FILE_DATE]);
    crec.crc32 =
        makelong(&byterec[C_CRC32]);
    crec.compressed_size =
        makelong(&byterec[C_COMPRESSED_SIZE]);
    crec.uncompressed_size =
        makelong(&byterec[C_UNCOMPRESSED_SIZE]);
    crec.filename_length =
        makeword(&byterec[C_FILENAME_LENGTH]);
    crec.extra_field_length =
        makeword(&byterec[C_EXTRA_FIELD_LENGTH]);
    crec.file_comment_length =
        makeword(&byterec[C_FILE_COMMENT_LENGTH]);
    crec.disk_number_start =
        makeword(&byterec[C_DISK_NUMBER_START]);
    crec.internal_file_attributes =
        makeword(&byterec[C_INTERNAL_FILE_ATTRIBUTES]);
    crec.external_file_attributes =
        makelong(&byterec[C_EXTERNAL_FILE_ATTRIBUTES]);  /* LONG, not word! */
    crec.relative_offset_local_header =
        makelong(&byterec[C_RELATIVE_OFFSET_LOCAL_HEADER]);

    pInfo->hostnum = min(crec.version_made_by[1], NUM_HOSTS);
/*  extnum = min( crec.version_needed_to_extract[1], NUM_HOSTS ); */
    methnum = min(crec.compression_method, NUM_METHODS);
    if (methnum == NUM_METHODS)
        sprintf(unkn, "Unk:%03d", crec.compression_method);

/*---------------------------------------------------------------------------
    Set flag for lowercase conversion of filename, depending on which OS the
    file is coming from.  This section could be ifdef'd if some people have
    come to love DOS uppercase filenames under Unix...but really, guys, get
    a life. :)  NOTE THAT ALL SYSTEM NAMES NOW HAVE TRAILING UNDERSCORES!!!
    This is to prevent interference with compiler command-line defines such
    as -DUNIX, for example, which are then used in "#ifdef UNIX" constructs.
  ---------------------------------------------------------------------------*/

    pInfo->lcflag = 0;
    if (!U_flag)   /* as long as user hasn't specified case-preservation */
        switch (pInfo->hostnum) {
            case DOS_OS2_FAT_:
            case VMS_:
            case VM_CMS_:           /* all caps? */
            case CPM_:              /* like DOS, right? */
        /*  case ATARI_:            ? */
        /*  case Z_SYSTEM_:         ? */
        /*  case TOPS20_:           (if we had such a thing...) */
                pInfo->lcflag = 1;  /* convert filename to lowercase */
                break;

            default:                /* AMIGA_, UNIX_, (ATARI_), OS2_HPFS_, */
                break;              /*   MAC_, (Z_SYSTEM_):  no conversion */
        }

    return 0;

}       /* end function process_cdir_file_hdr() */





/***************************************/
/*  Function process_local_file_hdr()  */
/***************************************/

int process_local_file_hdr()    /* return PK-type error code */
{
    local_byte_hdr byterec;


/*---------------------------------------------------------------------------
    Read the next local file header and do any necessary machine-type con-
    versions (byte ordering, structure padding compensation--do so by copy-
    ing the data from the array into which it was read (byterec) to the
    usable struct (lrec)).
  ---------------------------------------------------------------------------*/

    if (readbuf((char *) byterec, LREC_SIZE) <= 0)
        return (51);            /* 51:  unexpected EOF */

    lrec.version_needed_to_extract[0] = byterec[L_VERSION_NEEDED_TO_EXTRACT_0];
    lrec.version_needed_to_extract[1] = byterec[L_VERSION_NEEDED_TO_EXTRACT_1];

    lrec.general_purpose_bit_flag = makeword(&byterec[L_GENERAL_PURPOSE_BIT_FLAG]);
    lrec.compression_method = makeword(&byterec[L_COMPRESSION_METHOD]);
    lrec.last_mod_file_time = makeword(&byterec[L_LAST_MOD_FILE_TIME]);
    lrec.last_mod_file_date = makeword(&byterec[L_LAST_MOD_FILE_DATE]);
    lrec.crc32 = makelong(&byterec[L_CRC32]);
    lrec.compressed_size = makelong(&byterec[L_COMPRESSED_SIZE]);
    lrec.uncompressed_size = makelong(&byterec[L_UNCOMPRESSED_SIZE]);
    lrec.filename_length = makeword(&byterec[L_FILENAME_LENGTH]);
    lrec.extra_field_length = makeword(&byterec[L_EXTRA_FIELD_LENGTH]);

    csize = (longint) lrec.compressed_size;
    ucsize = (longint) lrec.uncompressed_size;

    return (0);                 /* 0:  no error */

}       /* end function process_local_file_hdr() */
