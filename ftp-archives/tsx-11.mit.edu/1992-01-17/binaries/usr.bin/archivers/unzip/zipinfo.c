/*--------------------------------------------------------------------------

  zipinfo.c

  This program reads all sorts of totally nifty information, including the
  central directory stuff, from a ZIP archive ("zipfile" for short).  It
  started as just a testbed for fooling with zipfiles, but at this point
  it's actually a moderately useful utility.  It also became the basis
  for the rewrite of unzip (3.16 -> 4.0), using the central directory for
  processing rather than the individual (local) file headers.

  For myself, I find it convenient to define an alias "ii" (under Unix and
  VMS) or to rename the executable to "ii.exe" (OS/2 and DOS).  This nicely
  complements my Unix long-listing "ll" alias (ls -lF), since zipinfo's de-
  fault action is to produce a Unix-like listing of the archive's contents.
  "ii zipfile" is easier to type than "zipinfo zipfile"...

  Another dandy product from your buddies at Newtware!

  --------------------------------------------------------------------------

  To compile (partial instructions; some of this stuff doesn't exist yet):

     under Unix (cc):  make zipinfo

     under MS-DOS (TurboC):  make -fMKZIPINF.DOS   (edit appropriately)

     under MS-DOS (MSC):  make MKZIPINF.DOS
       (or use Makefile if you have MSC 6.0:  "nmake zi_dos")

     under OS/2 (MSC):  make MKZIPINF.DOS   (edit appropriately)
       (or use Makefile if you have MSC 6.0:  "nmake zi_os2")

     under Atari OS:  beats me...

     under VMS:  @MAKE_ZIPINFO     (see also VMSNOTES)
                 ZIPINFO == $DISKNAME:[DIRECTORY]ZIPINFO.EXE

     under Macintosh OS:  who knows?

  --------------------------------------------------------------------------

  Version:    unzip42.zip (.tar.Z, etc.) for Unix, VMS, OS/2 and MS-DOS
  Source:     wuarchive.wustl.edu (128.252.135.4) in /mirrors/misc/unix
              wsmr-simtel20.army.mil (192.88.110.20) in pd1:[misc.unix]
  Author:     Greg Roelofs, roelofs@amelia.nas.nasa.gov, 23 August 1990
  Copyright:  none (except that some of the file input/output code comes
              from unzip, which has its own copyrights, sort of...MY stuff
              is in the public domain, however)

  --------------------------------------------------------------------------*/





#define ZIPINFO
#include "unzip.h"

#define VERSION  "v0.96k BETA of 20 Mar 92"





/**********************/
/*  Global Variables  */
/**********************/

#ifdef EBCDIC
   int  aflag=1;    /* this is so you can read it on the screen  */
#else               /* (basically, entire program is "unzip -c") */
   int  aflag=0;
#endif
int lflag=2;        /* for "ls -l" type listing */


byte *inbuf, *inptr;    /* input buffer (any size is legal) and pointer */
int incnt;

int zipfd;                      /* zipfile file handle */
char zipfn[FILNAMSIZ];

char local_hdr_sig[5] = "\120";    /* remaining signature bytes come later:  */
char central_hdr_sig[5] = "\120";  /*  must initialize at runtime so zipinfo */
char end_central_sig[5] = "\120";  /*  executable won't look like a zipfile  */

cdir_file_hdr crec;             /* used in zipinfo.c, misc.c */
local_file_hdr lrec;
ecdir_rec ecrec;
struct stat statbuf;            /* used by main() */

int process_all_files;
longint extra_bytes=0;          /* used in zipinfo.c, misc.c */
longint cur_zipfile_bufstart;   /* find_end_central_dir, readbuf */

min_info info, *pInfo=(&info);

byte *extra_field = NULL;       /* currently used by VMS version only */


byte *outbuf;                   /* buffer for rle look-back, zipfile comment */
byte *outout;                   /* scratch pad for ASCII-native trans */

char filename[FILNAMSIZ];
char sig[5];


char *fnames[2] = {"*", NULL};    /* default filenames vector */
char **fnv = fnames;

static byte *hold;
static longint ziplen;
static UWORD hostnum;
static UWORD methnum;
static UWORD extnum;
/*
    UWORD j, yr, mo, dy, hh, mm, members = 0;
    ULONG tot_csize = 0L, tot_ucsize = 0L;
 */


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

main(argc, argv)
    int    argc;
    char   *argv[];
{
    char   *s;
    int    c, error=FALSE;



/*---------------------------------------------------------------------------
    Everybody is now "NOTINT16," but this is a nice little piece of code, so
    just comment it out for future reference. :-)
  ---------------------------------------------------------------------------*/

#if 0
# ifndef KNOW_IT_WORKS  /* define this to save space, if things already work */
# ifndef DOS_OS2        /* already works (no RISCy OS/2's yet...) */
# ifndef NOTINT16       /* whole point is to see if this NEEDS defining */
    {
        int error=0;
        long testsig;
        static char *mach_type[3] = {"big-endian", "structure-padding",
                                     "big-endian and structure-padding"};

        strcpy((char *)&testsig,"012");
        if (testsig != 0x00323130)
            error = 1;
        if (sizeof(cdir_file_hdr) != CREC_SIZE)
            error += 2;
        if (error--)
            fprintf(stderr, "It appears that your machine is %s.  If errors\n\
occur, please try recompiling with \"NOTINT16\" defined (read the\n\
Makefile, or try \"make zipinfo\").\n\n", mach_type[error]);
    }
# endif /* !NOTINT16 */
# endif /* !DOS_OS2 */
# endif /* !KNOW_IT_WORKS */
#endif /* 0 */

/*---------------------------------------------------------------------------
    Rip through any command-line options lurking about...
  ---------------------------------------------------------------------------*/

    while (--argc > 0 && (*++argv)[0] == '-') {
        s = argv[0] + 1;
        while ((c = *s++) != 0) {    /* "!= 0":  prevent Turbo C warning */
            switch (c) {
                case ('1'):    /* minimum listing:  just names */
                    lflag = 0;
                    break;
                case ('l'):    /* default:  "ls -l" type listing */
                    lflag = 2;
                    break;
                case ('v'):    /* turbo-verbose listing */
                    lflag = 10;
                    break;
                default:
                    error = TRUE;
                    break;
            }
        }
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

    if (stat(zipfn, &statbuf)) {       /* try again */
        fprintf(stderr, "error:  can't find zipfile [ %s ]\n", zipfn);
        RETURN(9);                     /* 9:  file not found */
    } else
        ziplen = statbuf.st_size;

    if (argc != 0) {
        fnv = argv;
        process_all_files = FALSE;
    } else
        process_all_files = TRUE;      /* for speed */

/*---------------------------------------------------------------------------
    Okey dokey, we have everything we need to get started.  Let's roll.
  ---------------------------------------------------------------------------*/

    inbuf = (byte *) (malloc(INBUFSIZ + 4));    /* 4 extra for hold[] (below) */
    outbuf = (byte *) (malloc(OUTBUFSIZ + 1));  /* 1 extra for string termin. */
    if (aflag)                  /* if need an ascebc scratch, */
        outout = (byte *) (malloc(OUTBUFSIZ));
    else                        /*  allocate it... */
        outout = outbuf;        /*  else just point to outbuf */

    if ((inbuf == NULL) || (outbuf == NULL) || (outout == NULL)) {
        fprintf(stderr, "error:  can't allocate zipinfo buffers\n");
        RETURN(4);              /* 4-8:  insufficient memory */
    }
    hold = &inbuf[INBUFSIZ];    /* to check for boundary-spanning signatures */

    RETURN(process_zipfile());  /* keep passing errors back... */

}       /* end main() */





/**********************/
/*  Function usage()  */
/**********************/

int usage(error)
    int error;
{
    FILE *usagefp;


/*---------------------------------------------------------------------------
    If user requested usage, send it to stdout; else send to stderr.
  ---------------------------------------------------------------------------*/

    if (error)
        usagefp = (FILE *) stderr;
    else
        usagefp = (FILE *) stdout;

    fprintf(usagefp, "\
   ZipInfo:  Zipfile Information Utility %s\n\
   (brought to you by Newtware, Inc., and the fine folks at Info-ZIP)\n\n\
   Usage:  zipinfo [-1lv] file[.zip] [filespec...]\n", VERSION);
    fprintf(usagefp, "\
     -1  list filenames only, one per line (useful for pipes)\n\
     -l  list files in Unix \"ls -l\" format:  default\n\
     -v  list files in verbose, multi-page format\n");
/*
     -p  disable automatic \"more\" function (for pipes) [not implemented]\n");
 */

#ifdef VMS
    fprintf(usagefp, "\nRemember that non-lowercase filespecs must be quoted\
 in VMS (e.g., \"Makefile\").\n");
#endif

    if (error)
        return 10;    /* 10:  bad or illegal parameters specified */
    else
        return 0;     /* just wanted usage screen: no error */

} /* end function usage() */





/********************************/
/*  Function process_zipfile()  */
/********************************/

int process_zipfile()   /* return PK-type error code */
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
     record format.  (Both bilf.c and make_bilf.com are included\n\
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
    Reconstruct the various PK signature strings; find and process the end-
    of-central-directory header and the central directory; and close the
    zipfile.
  ---------------------------------------------------------------------------*/

    strcat(local_hdr_sig, LOCAL_HDR_SIG);
    strcat(central_hdr_sig, CENTRAL_HDR_SIG);
    strcat(end_central_sig, END_CENTRAL_SIG);

    if (find_end_central_dir()) /* not found; nothing to do */
        return (2);             /* 2:  error in zipfile */

    real_ecrec_offset = cur_zipfile_bufstart + (inptr-inbuf);
#ifdef TEST
    printf("\n  found end-of-central-dir signature at offset %ld (%.8lXh)\n",
      real_ecrec_offset, real_ecrec_offset);
    printf("    from beginning of file; offset %d (%.4Xh) within block\n",
      inptr-inbuf, inptr-inbuf);
#endif

    if ((error_in_archive = process_end_central_dir()) > 1)
        return (error_in_archive);

    if (ecrec.number_this_disk == ecrec.num_disk_with_start_central_dir) {
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
                error_in_archive = 2;   /* 2:  (weak) error in zipfile */
            } else {
                fprintf(stderr, "\nwarning:  extra %ld bytes at beginning or\
 within zipfile\n          (attempting to process anyway)\n\n", extra_bytes);
                error_in_archive = 1;   /* 1:  warning error */
            }
        }
        LSEEK( ecrec.offset_start_central_directory )
        if ((error = process_central_dir()) > error_in_archive)
            error_in_archive = error;    /* don't overwrite stronger error */
        if (lflag > 9)
            printf("\n");
    } else {
        fprintf(stderr, "\n\
     Zipfile is part of a multi-disk archive, and this is not the disk on\
     which the central zipfile directory begins.\n");
        error_in_archive = 11;  /* 11:  no files found */
    }

    close(zipfd);
#ifdef VMS
    VMSmunch(zipfn, RESTORE_RTYPE, NULL);
#endif
    return (error_in_archive);

}       /* end function process_zipfile() */





/*************************************/
/*  Function find_end_central_dir()  */
/*************************************/

int find_end_central_dir()   /* return 0 if found, 1 otherwise */
{
    int       i, numblks;
    longint   tail_len;



/*---------------------------------------------------------------------------
    Treat case of short zipfile separately.
  ---------------------------------------------------------------------------*/

    if (ziplen <= INBUFSIZ) {
        lseek(zipfd, 0L, SEEK_SET);
        if ((incnt = read(zipfd,inbuf,(unsigned int)ziplen)) == ziplen)

            /* 'P' must be at least 22 bytes from end of zipfile */
            for ( inptr = inbuf+ziplen-22  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp(inptr, end_central_sig, 4) ) {
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
            if ((incnt = read(zipfd,inbuf,(unsigned int)tail_len)) != tail_len)
                goto fail;      /* shut up, it's expedient. */

            /* 'P' must be at least 22 bytes from end of zipfile */
            for ( inptr = inbuf+tail_len-22  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp(inptr, end_central_sig, 4) ) {
                    incnt -= inptr - inbuf;
                    return(0);  /* found it! */
                }               /* ...otherwise search next block */
            strncpy(hold, inbuf, 3);    /* sig may span block boundary */

        } else {
            cur_zipfile_bufstart = ziplen - tail_len;
        }

        /*
         * Loop through blocks of zipfile data, starting at the end and going
         * toward the beginning.  Need only check last 65557 bytes of zipfile:
         * comment may be up to 65535 bytes long, end-of-central-directory rec-
         * ord is 18 bytes (shouldn't hardcode this number, but what the hell:
         * already did so above (22=18+4)), and sig itself is 4 bytes.
         * 
         * zipinfo:  check the whole file, just in case some transfer protocol
         * has appended a whole bunch of garbage at the end of the archive.
         */

#ifndef ZIPINFO
        /*         ==amt to search==   ==done==   ==rounding==    =blksiz= */
        numblks = (min(ziplen,65557) - tail_len + (INBUFSIZ-1)) / INBUFSIZ;
#else
        numblks = (    ziplen        - tail_len + (INBUFSIZ-1)) / INBUFSIZ;
#endif

        for ( i = 1  ;  i <= numblks  ;  ++i ) {
            cur_zipfile_bufstart -= INBUFSIZ;
            lseek(zipfd, cur_zipfile_bufstart, SEEK_SET);
            if ((incnt = read(zipfd,inbuf,INBUFSIZ)) != INBUFSIZ)
                break;          /* fall through and fail */

            for ( inptr = inbuf+INBUFSIZ-1  ;  inptr >= inbuf  ;  --inptr )
                if ( (ascii_to_native(*inptr) == 'P')  &&
                      !strncmp(inptr, end_central_sig, 4) ) {
                    incnt -= inptr - inbuf;
                    return(0);  /* found it! */
                }
            strncpy(hold, inbuf, 3);    /* sig may span block boundary */
        }

    } /* end if (ziplen > INBUFSIZ) */

/*---------------------------------------------------------------------------
    Searched through whole region where signature should be without finding
    it.  Print informational message and die a horrible death.
  ---------------------------------------------------------------------------*/

fail:

    fprintf(stderr, "\n\
     End-of-central-directory signature not found.  Either this file is not\n\
     a zipfile, or it constitutes one disk of a multi-part archive.  In the\n\
     latter case the central directory and zipfile comment will be found on\n\
     the last disk(s) of this archive.\n");
    return(1);          /* failed */

}       /* end function find_end_central_dir() */





/****************************************/
/*  Function process_end_central_dir()  */
/****************************************/

int process_end_central_dir()   /* return PK-type error code */
{
    ec_byte_rec   byterec;
    int           error=0;


/*--------------------------------------------------------------------------
    Read the end-of-central-directory record and do any necessary machine-
    type conversions (byte ordering, structure padding compensation) by
    copying character array to struct.
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
    Print out various interesting things about the zipfile.
  ---------------------------------------------------------------------------*/

    if (lflag == 2) {
        if (process_all_files)
           /* fits on one line, for anything up to 10GB and 10000 files */
           printf((strlen(zipfn)<39)? "Archive:  %s   %ld bytes   %d file%s\n"
             : "Archive:  %s   %ld   %d\n", zipfn, ziplen,
             ecrec.total_entries_central_dir,
             (ecrec.total_entries_central_dir==1)? "":"s");
    } else if (lflag > 9) {   /* verbose format */
        printf("\nEnd-of-central-directory record:\n");
        printf("-------------------------------\n\n");

        if (ecrec.number_this_disk == 0) {
            printf("\
  This zipfile constitutes the sole disk of a single-part archive; its\n\
  central directory contains %u %s.  The central directory is %lu\n\
  (%.8lXh) bytes long, and its offset in bytes from the beginning of\n\
  the zipfile is %lu (%.8lXh).\n\n",
              ecrec.total_entries_central_dir,
              (ecrec.total_entries_central_dir == 1)? "entry" : "entries",
              ecrec.size_central_directory, ecrec.size_central_directory,
              ecrec.offset_start_central_directory,
              ecrec.offset_start_central_directory);
        } else {
            printf("\
  This zipfile constitutes disk %u of a multi-part archive.  The central\n\
  directory starts on disk %u; %u of its entries %s contained within\n\
  this zipfile, out of a total of %u %s.  The entire central\n\
  directory is %lu (%.8lXh) bytes long, and its offset in bytes from\n\
  the beginning of the zipfile in which it begins is %lu (%.8lXh).\n\n",
              ecrec.number_this_disk,
              ecrec.num_disk_with_start_central_dir,
              ecrec.num_entries_centrl_dir_ths_disk,
              (ecrec.num_entries_centrl_dir_ths_disk == 1)? "is" : "are",
              ecrec.total_entries_central_dir,
              (ecrec.total_entries_central_dir == 1) ? "entry" : "entries",
              ecrec.size_central_directory, ecrec.size_central_directory,
              ecrec.offset_start_central_directory,
              ecrec.offset_start_central_directory);
        }

    /*-----------------------------------------------------------------------
        Get the zipfile comment, if any, and print it out.  (Comment may be
        up to 64KB long.  May the fleas of a thousand camels infest the arm-
        pits of anyone who actually takes advantage of this fact.)
      -----------------------------------------------------------------------*/

        if (!ecrec.zipfile_comment_length)
            printf("  There is no zipfile comment.\n");
        else {
            printf("  The zipfile comment is %u bytes long and contains the following text:\n\n",
              ecrec.zipfile_comment_length );
            printf("======================== zipfile comment begins ==========================\n");
            if (do_string(ecrec.zipfile_comment_length, DISPLAY))
                error = 1;          /* 1:  warning error */
            printf("\n========================= zipfile comment ends ===========================\n");
            if (error)
                printf("\n  The zipfile comment is truncated.\n");
        } /* endif (comment exists) */

    } /* endif (verbose) */

    return error;

}       /* end function process_end_central_dir() */





/******************************************/
/*  Function process_central_directory()  */
/******************************************/

int process_central_dir()   /* return PK-type error code */
{
    char    **fnamev;
    int     do_this_file=FALSE, none_found=TRUE, error, error_in_archive=0;
    UWORD   j, members=0;
    ULONG   tot_csize=0L, tot_ucsize=0L;


/*---------------------------------------------------------------------------
    Set file pointer to start of central directory, then loop through cen-
    tral directory entries.  Check that directory-entry signature bytes are
    actually there (just a precaution), then process the entry.  We know
    the entire central directory is on this disk:  we wouldn't have any of
    this information unless the end-of-central-directory record was on this
    disk, and we wouldn't have gotten to this routine unless this is also
    the disk on which the central directory starts.  In practice, this had
    better be the *only* disk in the archive, but maybe someday we'll add
    multi-disk support.
  ---------------------------------------------------------------------------*/

    pInfo->lcflag = 0;   /* match(), do_string():  never TRUE in zipinfo */

    for (j = 0;  j < ecrec.total_entries_central_dir;  ++j) {
        if (readbuf(sig, 4) <= 0)
            return (51);        /* 51:  unexpected EOF */
        if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
            fprintf(stderr, CentSigMsg, j);  /* sig not found */
            return (3);         /* 3:  error in zipfile */
        }
        if ((error = process_cdir_file_hdr()) != 0)
            return (error);     /* only 51 (EOF) defined */
        if ((error = do_string(crec.filename_length, FILENAME)) != 0) {
          error_in_archive = error;   /* might be warning */
          if (error > 1)        /* fatal */
              return (error);
        }

        if (!process_all_files) {   /* check if specified on command line */
            do_this_file = FALSE;
            fnamev = fnv;       /* don't destroy permanent filename ptr */
            for (--fnamev;  *++fnamev; )
                if (match(filename, *fnamev)) {
                    do_this_file = TRUE;
                    none_found = FALSE;
                    break;      /* found match, so stop looping */
                }
        }

    /*-----------------------------------------------------------------------
        If current file was specified on command line, or if no names were
        specified, do the listing for this file.  Otherwise, get rid of the
        file comment and go back for the next file.
      -----------------------------------------------------------------------*/

        if (process_all_files || do_this_file) {
            switch (lflag) {
                case 0:
                    printf("%s\n", filename);
                    SKIP_(crec.extra_field_length)
                    SKIP_(crec.file_comment_length)
                    break;

                case 2:
                    if ((error = short_info()) != 0) {
                        error_in_archive = error;   /* might be warning */
                        if (error > 1)              /* fatal */
                            return (error);
                    }
                    break;

                case 10:
#ifdef VMS   /* GRR: FIX THIS (no pipes:  add cbreak-style "more" function) */
                    printf("\nCentral directory entry #%d:\n", j);
#else /* !VMS */
                    /* formfeed/CR for piping to "more": */
                    printf("%s\nCentral directory entry #%d:\n", "\014", j);
#endif /* ?VMS */
                    printf("---------------------------\n\n");

                    if ((error = long_info()) != 0) {
                      error_in_archive = error;   /* might be warning */
                      if (error > 1)              /* fatal */
                          return (error);
                    }
                    break;

                default:
                    SKIP_(crec.extra_field_length)
                    SKIP_(crec.file_comment_length)
                    break;

            } /* end switch (lflag) */

            tot_ucsize += crec.uncompressed_size;
            tot_csize += crec.compressed_size;
            ++members;

        } else {   /* not listing */
            SKIP_(crec.extra_field_length)
            SKIP_(crec.file_comment_length)

        } /* end if (list member?) */

    } /* end for-loop (j: member files) */

/*---------------------------------------------------------------------------
    Double check that we're back at the end-of-central-directory record.
  ---------------------------------------------------------------------------*/

    readbuf(sig, 4);
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
        error_in_archive = 1;        /* 1:  warning error */
    }

/*---------------------------------------------------------------------------
    Check that we actually found requested files.
  ---------------------------------------------------------------------------*/

    if (none_found && !process_all_files) {
        fnamev = fnv;       /* don't destroy permanent filename ptr */
        for (--fnamev;  *++fnamev; )
            printf("zipinfo:  %s not found in %s\n", *fnamev, zipfn);
    }

    return (error_in_archive);
}





/**************************************/
/*  Function process_cdir_file_hdr()  */
/**************************************/

int process_cdir_file_hdr()   /* return PK-type error code */
{
    cdir_byte_hdr   byterec;


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
        makelong(&byterec[C_EXTERNAL_FILE_ATTRIBUTES]); /* LONG, not word! */
    crec.relative_offset_local_header =
        makelong(&byterec[C_RELATIVE_OFFSET_LOCAL_HEADER]);

    return (0);

} /* end function process_cdir_file_hdr() */




/**************************/
/*  Function long_info()  */
/**************************/

int long_info()   /* return PK-type error code */
{
    int           error, error_in_archive=0;
    UWORD         hostver, extver, xattr;
    char          workspace[12], attribs[22];
    static char   unkn[16];
    static char   *os[NUM_HOSTS+1] = {"MS-DOS or OS/2 FAT", "Amiga", "VAX VMS",
                      "Unix", "VM/CMS", "Atari ST", "OS/2 HPFS", "Macintosh",
                      "Z-System", "CP/M", "unknown" };
    static char   *method[NUM_METHODS+1] = {"none (stored)", "shrunk",
                      "reduced (factor 1)", "reduced (factor 2)",
                      "reduced (factor 3)", "reduced (factor 4)",
                      "imploded", "tokenized", "deflated", unkn};
    static char   *dtype[4] = {"normal", "maximum", "fastest", "undefined"};


/*---------------------------------------------------------------------------
    Print out various interesting things about the compressed file.
  ---------------------------------------------------------------------------*/

    hostnum = min(crec.version_made_by[1], NUM_HOSTS);
    hostver = crec.version_made_by[0];
    extnum = min(crec.version_needed_to_extract[1], NUM_HOSTS);
    extver = crec.version_needed_to_extract[0];
    methnum = min(crec.compression_method, NUM_METHODS);
    if (methnum == NUM_METHODS)
        sprintf(unkn, "unknown (%d)", crec.compression_method);

    printf("  %s\n", filename);

    printf("\n  host operating system (created on):               %s\n",
      os[hostnum]);
    printf("  version of encoding software:                     %d.%d\n",
      hostver/10, hostver%10);
    printf("  minimum operating system compatibility required:  %s\n",
      os[extnum]);
    printf("  minimum software version required to extract:     %d.%d\n",
      extver/10, extver%10);
    printf("  compression method:                               %s\n",
      method[methnum]);
    if (methnum == IMPLODED) {
        printf("  size of sliding dictionary (implosion):           %cK\n",
          (crec.general_purpose_bit_flag & 2)? '8' : '4');
        printf("  number of Shannon-Fano trees (implosion):         %c\n",
          (crec.general_purpose_bit_flag & 4)? '3' : '2');
    } else if (methnum == DEFLATED) {
        UWORD  dnum=(crec.general_purpose_bit_flag>>1) & 3;
        printf("  compression sub-type (deflation):                 %s\n",
          dtype[dnum]);
    }
    printf("  file security status:                             %sencrypted\n",
      (crec.general_purpose_bit_flag & 1)? "" : "not ");
    printf("  extended local header:                            %s\n",
      (crec.general_purpose_bit_flag & 8)? "yes" : "no");
    /* print upper 3 bits for amusement? */
    printf("  file last modified on:                            %s\n",
      zipinfo_time(&crec.last_mod_file_date, &crec.last_mod_file_time));
    printf("  32-bit CRC value (hex):                           %.8lx\n",
      crec.crc32);
    printf("  compressed size:                                  %lu bytes\n",
      crec.compressed_size);
    printf("  uncompressed size:                                %lu bytes\n",
      crec.uncompressed_size);
    printf("  length of filename:                               %u characters\n",
      crec.filename_length);
    printf("  length of extra field:                            %u bytes\n",
      crec.extra_field_length);
    printf("  length of file comment:                           %u characters\n",
      crec.file_comment_length);
    printf("  disk number on which file begins:                 disk %u\n",
      crec.disk_number_start);
    printf("  apparent file type:                               %s\n",
      (crec.internal_file_attributes & 1)? "text" : "binary");
/*
    printf("  external file attributes (hex):                   %.8lx\n",
      crec.external_file_attributes);
 */
    xattr = (crec.external_file_attributes >> 16) & 0xFFFF;
    if (hostnum == VMS_) {
        char   *p=attribs, *q=attribs+1;
        int    i, j, k;

        for (k = 0;  k < 12;  ++k)
            workspace[k] = 0;
        if (xattr & S_IRUSR)
            workspace[0] = 'R';
        if (xattr & S_IWUSR) {
            workspace[1] = 'W';
            workspace[3] = 'D';
        }
        if (xattr & S_IXUSR)
            workspace[2] = 'E';
        if (xattr & S_IRGRP)
            workspace[4] = 'R';
        if (xattr & S_IWGRP) {
            workspace[5] = 'W';
            workspace[7] = 'D';
        }
        if (xattr & S_IXGRP)
            workspace[6] = 'E';
        if (xattr & S_IROTH)
            workspace[8] = 'R';
        if (xattr & S_IWOTH) {
            workspace[9] = 'W';
            workspace[11] = 'D';
        }
        if (xattr & S_IXOTH)
            workspace[10] = 'E';

        *p++ = '(';
        for (k = j = 0;  j < 3;  ++j) {    /* loop over groups of permissions */
            for (i = 0;  i < 4;  ++i, ++k)  /* loop over perms within a group */
                if (workspace[k])
                    *p++ = workspace[k];
            *p++ = ',';                      /* group separator */
            if (j == 0)
                while ((*p++ = *q++) != ','); /* system, owner perms are same */
        }
        *p-- = 0;
        *p = ')';   /* overwrite last comma */
        printf("  VMS file attributes (%06o octal):               %s\n",
          xattr, attribs);

    } else if ((hostnum != DOS_OS2_FAT_) && (hostnum != OS2_HPFS_)) {
        /* assume Unix-like */
        switch (xattr & S_IFMT) {
            case S_IFREG:   attribs[0] = '-';  break;
            case S_IFLNK:   attribs[0] = 'l';  break;
            case S_IFBLK:   attribs[0] = 'b';  break;
            case S_IFCHR:   attribs[0] = 'c';  break;
            case S_IFIFO:   attribs[0] = 'p';  break;
            case S_IFSOCK:  attribs[0] = 's';  break;
            case S_IFDIR:   attribs[0] = 'd';  break;
            default:        attribs[0] = '?';  break;
        }
        if (xattr & S_IRUSR)        /* no read-permission: user */
            attribs[1] = 'r';
        else
            attribs[1] = '-';
        if (xattr & S_IWUSR)        /* no write-permission: user */
            attribs[2] = 'w';
        else
            attribs[2] = '-';
        if (xattr & S_IXUSR)        /* no execute-permission: user */
            if (xattr & S_ISUID)
                attribs[3] = 's';
            else
                attribs[3] = 'x';
        else
            if (xattr & S_ISUID)
                attribs[3] = 'S';   /* undefined state */
            else
                attribs[3] = '-';
        if (xattr & S_IRGRP)        /* no read-permission: group */
            attribs[4] = 'r';
        else
            attribs[4] = '-';
        if (xattr & S_IWGRP)        /* no write-permission: group */
            attribs[5] = 'w';
        else
            attribs[5] = '-';
        if (xattr & S_IXGRP)        /* no execute-permission: group */
            if (xattr & S_ISGID)
                attribs[6] = 's';
            else
                attribs[6] = 'x';
        else
            if (xattr & S_ISGID)    /* or could use S_ENFMT (same) */
                attribs[6] = 'l';
            else
                attribs[6] = '-';
        if (xattr & S_IROTH)        /* no read-permission: other */
            attribs[7] = 'r';
        else
            attribs[7] = '-';
        if (xattr & S_IWOTH)        /* no write-permission: other */
            attribs[8] = 'w';
        else
            attribs[8] = '-';
        if (xattr & S_IXOTH)        /* no execute-permission: other */
            if (xattr & S_ISVTX)    /* "sticky bit" */
                attribs[9] = 't';
            else
                attribs[9] = 'x';
        else
            if (xattr & S_ISVTX)
                attribs[9] = 'T';   /* undefined state */
            else
                attribs[9] = '-';
        attribs[10] = 0;
        printf("  Unix file attributes (%06o octal):              %s\n",
          xattr, attribs);

    } /* endif (hostnum: external attributes format) */

    if ((xattr=crec.external_file_attributes & 0xFF) == 0)
        printf("  MS-DOS file attributes (%02X hex):                  none\n",
          xattr);
    else if (xattr == 1)
        printf(
          "  MS-DOS file attributes (%02X hex):                  read-only\n",
          xattr);
    else
        printf(
         "  MS-DOS file attributes (%02X hex):                  %s%s%s%s%s%s\n",
          xattr, (xattr&1)?"rdo ":"", (xattr&2)?"hid ":"", (xattr&4)?"sys ":"",
          (xattr&8)?"lab ":"", (xattr&16)?"dir ":"", (xattr&32)?"arc":"");
    printf(
     "  offset of local header from start of archive:     %lu (%.8lXh) bytes\n",
      crec.relative_offset_local_header, crec.relative_offset_local_header);

/*---------------------------------------------------------------------------
    Skip the extra field, if any, and print the file comment, if any (the
    filename has already been printed, above).  That finishes up this file
    entry...
  ---------------------------------------------------------------------------*/

    if (crec.extra_field_length > 0) {
        printf("\n  There is an extra field (skipping).\n");
        SKIP_(crec.extra_field_length)
    } else
        printf("\n");

    if (!crec.file_comment_length)
        printf("  There is no file comment.\n");
    else {
        printf("\
------------------------- file comment begins ----------------------------\n");
        if ((error = do_string(crec.file_comment_length, DISPLAY)) != 0) {
          error_in_archive = error;   /* might be warning */
          if (error > 1)      /* fatal */
              return (error);
        }
        printf("\n\
-------------------------- file comment ends -----------------------------\n");
    }

    return (error_in_archive);

} /* end function long_info() */





/***************************/
/*  Function short_info()  */
/***************************/

int short_info()   /* return PK-type error code */
{
    int           k, error, error_in_archive=0;
    UWORD         hostver, xattr;
    char          workspace[12], attribs[16];
    static char   impl[5]="i#:#", defl[5]="def#", unkn[8];
    static char   dtype[5]="NXF?";  /* normal, maximum, fastest, undefined */
    static char   *os[NUM_HOSTS+1] = {"dos", "ami", "vms", "unx", "cms",
                      "atr", "os2", "mac", "zzz", "cpm", "???" };
    static char   *method[NUM_METHODS+1] = {"stor", "shrk", "re:1", "re:2",
                      "re:3", "re:4", impl, "tokn", defl, unkn};


/*---------------------------------------------------------------------------
    Print out various interesting things about the compressed file.
  ---------------------------------------------------------------------------*/

    methnum = min(crec.compression_method, NUM_METHODS);
    hostnum = min(crec.version_made_by[1], NUM_HOSTS);
    hostver = crec.version_made_by[0];
/*
    extnum = min(crec.version_needed_to_extract[1], NUM_HOSTS);
    extver = crec.version_needed_to_extract[0];
 */

    if (methnum == IMPLODED) {
        impl[1] = (crec.general_purpose_bit_flag & 2)? '8' : '4';
        impl[3] = (crec.general_purpose_bit_flag & 4)? '3' : '2';
    } else if (methnum == DEFLATED) {
        UWORD  dnum=(crec.general_purpose_bit_flag>>1) & 3;
        defl[3] = dtype[dnum];
    } else if (methnum == NUM_METHODS) {   /* unknown */
        sprintf(unkn, "u%03d", crec.compression_method);
    }

    for (k = 0;  k < 15;  ++k)
        attribs[k] = ' ';
    attribs[15] = 0;

    xattr = (crec.external_file_attributes >> 16) & 0xFFFF;
    switch (hostnum) {
      case VMS_:
          {   char   *p=attribs;
              int    i, j;

              for (k = 0;  k < 12;  ++k)
                  workspace[k] = 0;
              if (xattr & S_IRUSR)
                  workspace[0] = 'R';
              if (xattr & S_IWUSR) {
                  workspace[1] = 'W';
                  workspace[3] = 'D';
              }
              if (xattr & S_IXUSR)
                  workspace[2] = 'E';
              if (xattr & S_IRGRP)
                  workspace[4] = 'R';
              if (xattr & S_IWGRP) {
                  workspace[5] = 'W';
                  workspace[7] = 'D';
              }
              if (xattr & S_IXGRP)
                workspace[6] = 'E';
              if (xattr & S_IROTH)
                  workspace[8] = 'R';
              if (xattr & S_IWOTH) {
                  workspace[9] = 'W';
                  workspace[11] = 'D';
              }
              if (xattr & S_IXOTH)
                  workspace[10] = 'E';

              for (k = j = 0;  j < 3;  ++j) {     /* groups of permissions */
                  for (i = 0;  i < 4;  ++i, ++k)  /* perms within a group */
                      if (workspace[k])
                          *p++ = workspace[k];
                  *p++ = ',';                     /* group separator */
              }
              *--p = ' ';   /* overwrite last comma */
              if ((p - attribs) < 12)
                  sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
          }
          break;

      case DOS_OS2_FAT_:
      case OS2_HPFS_:
          xattr = crec.external_file_attributes & 0xFF;
          sprintf(attribs, "%s,%s,%s,%s", (xattr&32)?"arc":"",
            (xattr&2)?"hid":"", (xattr&1)?"rdo":"rw", (xattr&4)?"sys":"");
          if ((k = strlen(attribs)) < 15)
              attribs[k] = ' ';   /* overwrite '\0' */
          if (k < 12)
              sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
          break;

      default:   /* assume Unix-like */
          switch (xattr & S_IFMT) {
              case S_IFREG:   attribs[0] = '-';  break;
              case S_IFLNK:   attribs[0] = 'l';  break;
              case S_IFBLK:   attribs[0] = 'b';  break;
              case S_IFCHR:   attribs[0] = 'c';  break;
              case S_IFIFO:   attribs[0] = 'p';  break;
              case S_IFSOCK:  attribs[0] = 's';  break;
              case S_IFDIR:   attribs[0] = 'd';  break;
              default:        attribs[0] = '?';  break;
          }
          if (xattr & S_IRUSR)        /* no read-permission: user */
              attribs[1] = 'r';
          else
              attribs[1] = '-';
          if (xattr & S_IWUSR)        /* no write-permission: user */
              attribs[2] = 'w';
          else
              attribs[2] = '-';
          if (xattr & S_IXUSR)        /* no execute-permission: user */
              if (xattr & S_ISUID)
                  attribs[3] = 's';
              else
                  attribs[3] = 'x';
          else
              if (xattr & S_ISUID)
                  attribs[3] = 'S';   /* undefined state */
              else
                  attribs[3] = '-';
          if (xattr & S_IRGRP)        /* no read-permission: group */
              attribs[4] = 'r';
          else
              attribs[4] = '-';
          if (xattr & S_IWGRP)        /* no write-permission: group */
              attribs[5] = 'w';
          else
              attribs[5] = '-';
          if (xattr & S_IXGRP)        /* no execute-permission: group */
              if (xattr & S_ISGID)
                  attribs[6] = 's';
              else
                  attribs[6] = 'x';
          else
              if (xattr & S_ISGID)    /* or could use S_ENFMT (same) */
                  attribs[6] = 'l';
              else
                  attribs[6] = '-';
          if (xattr & S_IROTH)        /* no read-permission: other */
              attribs[7] = 'r';
          else
              attribs[7] = '-';
          if (xattr & S_IWOTH)        /* no write-permission: other */
              attribs[8] = 'w';
          else
              attribs[8] = '-';
          if (xattr & S_IXOTH)        /* no execute-permission: other */
              if (xattr & S_ISVTX)    /* "sticky bit" */
                  attribs[9] = 't';
              else
                  attribs[9] = 'x';
          else
              if (xattr & S_ISVTX)
                  attribs[9] = 'T';   /* undefined state */
              else
                  attribs[9] = '-';
          sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
          break;

    } /* end switch (hostnum: external attributes format) */

    printf("%s %s %7lu %c%c %7lu %s %s %s\n", attribs, os[hostnum], 
      crec.uncompressed_size, (crec.general_purpose_bit_flag & 1)?
      ((crec.internal_file_attributes & 1)? 'T' : 'B') :   /* encrypted */
      ((crec.internal_file_attributes & 1)? 't' : 'b'),    /* plaintext */
      (crec.general_purpose_bit_flag & 8)? (crec.extra_field_length? 'X' : 'l')
                                         : (crec.extra_field_length? 'x' : '-'),
      crec.compressed_size, method[methnum],
      zipinfo_time(&crec.last_mod_file_date, &crec.last_mod_file_time),
      filename);

/*---------------------------------------------------------------------------
    Skip the extra field and/or the file comment, if any (the filename has
    already been printed, above).  That finishes up this file entry...
  ---------------------------------------------------------------------------*/

    SKIP_(crec.extra_field_length)
    SKIP_(crec.file_comment_length)

    return (error_in_archive);

} /* end function short_info() */





/*****************************/
/*  Function zipinfo_time()  */
/*****************************/

char *zipinfo_time(datez, timez)
    UWORD   *datez, *timez;
{
    UWORD         yr, mo, dy, hh, mm, ss;
    static char   d_t_str[21];
    static char   *month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};



/*---------------------------------------------------------------------------
    Convert the file-modification date and time info to a string of the form 
    "23 Feb 1990 17:15:00" or "23-Feb-91 17:15," depending on value of lflag.
  ---------------------------------------------------------------------------*/

    yr = ((*datez >> 9) & 0x7f) + 80;     /* dissect date */
    mo = ((*datez >> 5) & 0x0f) - 1;
    dy = *datez & 0x1f;

    hh = (*timez >> 11) & 0x1f;           /* dissect time */
    mm = (*timez >> 5) & 0x3f;
    ss = (*timez & 0x1f) * 2;

    if (lflag == 2)
        sprintf(d_t_str, "%2u-%s-%u %02u:%02u", dy, month[mo], yr, hh, mm);
    else if (lflag > 9)  /* verbose listing format */
        sprintf(d_t_str, "%u %s %u %02u:%02u:%02u", dy, month[mo], yr+1900,
          hh, mm, ss);

    return(d_t_str);

} /* end function zipinfo_time() */





/********************************/
/*  Function open_input_file()  */
/********************************/

int open_input_file()
{                               /* return non-0 if open failed */
    /*
     *  open the zipfile for reading and in BINARY mode to prevent cr/lf
     *  translation, which would corrupt the bitstreams
     */

#ifndef UNIX
    zipfd = open(zipfn, O_RDONLY | O_BINARY);
#else
    zipfd = open(zipfn, O_RDONLY);
#endif
    if (zipfd < 1) {
        fprintf(stderr, "error:  can't open zipfile [ %s ]\n", zipfn);
        return (1);
    }
    return 0;
}





/************************/
/*  Function readbuf()  */
/************************/

int readbuf(buf, size)
    char *buf;
    register unsigned size;
{                               /* return number of bytes read into buf */
    register int count;
    int n;

    n = size;
    while (size) {
        if (incnt == 0) {
            if ((incnt = read(zipfd, inbuf, INBUFSIZ)) <= 0)
                return (n-size);
            /* buffer ALWAYS starts on a block boundary:  */
            cur_zipfile_bufstart += INBUFSIZ;
            inptr = inbuf;
        }
        count = min(size, incnt);
        memcpy(buf, inptr, count);
        buf += count;
        inptr += count;
        incnt -= count;
        size -= count;
    }
    return (n);
}
