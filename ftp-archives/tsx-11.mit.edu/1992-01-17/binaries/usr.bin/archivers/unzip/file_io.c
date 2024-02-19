/*---------------------------------------------------------------------------

  file_io.c

  This file contains routines for doing direct input/output, file-related
  sorts of things.  Most of the system-specific code for unzip is contained
  here, including the non-echoing password code for decryption (bottom).

  ---------------------------------------------------------------------------*/


#define const
#include "unzip.h"


/************************************/
/*  File_IO Local Prototypes, etc.  */
/************************************/

#ifndef DOS_OS2
   static int dos2unix __((unsigned char *buf, int len));
   int CR_flag = 0;      /* when last char of buffer == CR (for dos2unix()) */
#endif

#ifdef OS2
   extern int   longname;          /* set in mapname.c */
   extern char  longfilename[];
#endif

#ifdef CRYPT
   typedef char  voidp;
#  if defined(DOS_OS2) || defined(VMS)
#    define MSVMS
#    ifdef DOS_OS2
#      include <conio.h>
#    else /* !DOS_OS2 */
#      define getch() getc(stderr)
#      define OFF 0   /* for echo control */
#      define ON 1
#      include <descrip.h>
#      include <iodef.h>
#      include <ttdef.h>
#      ifndef SS$_NORMAL
#        define SS$_NORMAL 1   /* only thing we need from <ssdef.h> */
#      endif
#    endif /* ?DOS_OS2 */
#  else /* !(DOS_OS2 || VMS) */
#    ifdef TERMIO	/* Amdahl, Cray, all SysV? */
#      ifdef CONVEX
#        include <sys/termios.h>
#	 include <sgtty.h>
#      else /* !CONVEX */
#        include <sys/termio.h>
#        define sgttyb termio
#        define sg_flags c_lflag
#      endif /* ?CONVEX */
       int ioctl OF((int, int, voidp *));
#      define GTTY(f,s) ioctl(f,TCGETA,(voidp *)s)
#      define STTY(f,s) ioctl(f,TCSETAW,(voidp *)s)
#    else /* !TERMIO */
#      ifndef MINIX
#	 include <sys/ioctl.h>
#      endif /* !MINIX */
#      include <sgtty.h>
       int gtty OF((int, struct sgttyb *));
       int stty OF((int, struct sgttyb *));
#      define GTTY gtty
#      define STTY stty
#    endif /* ?TERMIO */
     int isatty OF((int));
     char *ttyname OF((int));
     int open OF((char *, int, ...));
     int close OF((int));
     int read OF((int, voidp *, int));
#  endif /* ?(DOS_OS2 || VMS) */
#endif /* CRYPT */





/******************************/
/* Function open_input_file() */
/******************************/

int open_input_file()
{                               /* return non-0 if open failed */
    /*
     *  open the zipfile for reading and in BINARY mode to prevent cr/lf
     *  translation, which would corrupt the bitstreams
     */

#ifdef UNIX
    zipfd = open(zipfn, O_RDONLY);
#else
    zipfd = open(zipfn, O_RDONLY | O_BINARY);
#endif
    if (zipfd < 1) {
        fprintf(stderr, "error:  can't open zipfile [ %s ]\n", zipfn);
        return (1);
    }
    return 0;
}





/**********************/
/* Function readbuf() */
/**********************/

int readbuf(buf, size)
char *buf;
register unsigned size;
{                               /* return number of bytes read into buf */
    register int count;
    int n;

    n = size;
    while (size) {
        if (incnt == 0) {
            if ((incnt = read(zipfd, (char *)inbuf, INBUFSIZ)) <= 0)
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





#ifndef VMS   /* for VMS use code in vms.c (old VMS code below is retained
               * in case of problems...will be removed in a later release) */

/*********************************/
/* Function create_output_file() */
/*********************************/

int create_output_file()
{                               /* return non-0 if creat failed */


/*---------------------------------------------------------------------------
    Create the output file with appropriate permissions.  If we've gotten to
    this point and the file still exists, we have permission to blow it away.
  ---------------------------------------------------------------------------*/

#ifndef DOS_OS2
    CR_flag = 0;   /* hack to get CR at end of buffer working */
#endif

#if defined(UNIX) && !defined(AMIGA)
    {
        int mask;

#ifndef VMS
        if (!stat(filename, &statbuf) && (unlink(filename) < 0)) {
            fprintf(stderr, "\n%s:  cannot delete old copy\n", filename);
            return 1;
        }
#  define EXTRA_ARGS
#else /* VMS */
#  define EXTRA_ARGS   ,"rfm=stmlf","rat=cr"
#endif /* ?VMS */

        mask = umask(0);   /* now know we own it */
        outfd = creat(filename, 0777 & pInfo->unix_attr  EXTRA_ARGS);
        umask(mask);                                            /* VMS, Unix */
    }
#else /* !UNIX || AMIGA */  /* file permissions set after file closed */
#if defined(MACOS) && defined(MPW) && !defined(MCH_MACINTOSH)
    outfd = creat(filename);                         /* Mac MPW C compiler */
#else /* !(Macintosh MPW 3.2 C) */
    outfd = creat(filename, S_IWRITE | S_IREAD);     /* DOS, OS2, Mac, Amiga */
#endif /* ?(Macintosh MPW 3.2 C) */
#endif /* ?(UNIX && !AMIGA) */

    if (outfd < 1) {
        fprintf(stderr, "\n%s:  cannot create\n", filename);
        return 1;
    }

/*---------------------------------------------------------------------------
    If newly created file is in text mode and should be binary (to disable
    automatic CR/LF translations), either close it and reopen as binary or
    else change the mode to binary (DOS, OS/2).  If it is already binary and
    should be text, change the mode to text (Mac).
  ---------------------------------------------------------------------------*/

#ifndef UNIX
#ifdef THINK_C
    /*
     * THINKC's stdio routines have the horrible habit of
     * making any file you open look like generic files.
     * This code tells the OS that it's a text file.
     */
    if (aflag) {
        fileParam pb;
        OSErr err;

        CtoPstr(filename);
        pb.ioNamePtr = (StringPtr)filename;
        pb.ioVRefNum = 0;
        pb.ioFVersNum = 0;
        pb.ioFDirIndex = 0;
        err = PBGetFInfo(&pb,0);
        if (err == noErr) {
            pb.ioFlFndrInfo.fdCreator = '\?\?\?\?';
            pb.ioFlFndrInfo.fdType = 'TEXT';
            err = PBSetFInfo(&pb, 0);
        }
        PtoCstr(filename);
    }
#endif /* THINK_C */

    if (!aflag) {
#ifdef DOS_OS2
        if (setmode(outfd, O_BINARY) == -1) {
#else /* !DOS_OS2 */
        close(outfd);
        if ((outfd = open(filename, O_RDWR | O_BINARY)) < 1) {
#endif /* ?DOS_OS2 */
            fprintf(stderr, "Can't make output file binary:  %s\n", filename);
            return 1;
        }
    }
#endif /* !UNIX */

    return 0;
}

#endif /* !VMS */





/****************************/
/* Function FillBitBuffer() */
/****************************/

int FillBitBuffer()
{
    /*
     * Fill bitbuf, which is 32 bits.  This function is only used by the
     * READBIT and PEEKBIT macros (which are used by all of the uncompression
     * routines).
     */
    UWORD temp;

    zipeof = 1;
    while (bits_left < 25 && ReadByte(&temp) == 8)
    {
      bitbuf |= (ULONG)temp << bits_left;
      bits_left += 8;
      zipeof = 0;
    }
    return 0;
}





/***********************/
/* Function ReadByte() */
/***********************/

int ReadByte(x)
UWORD *x;
{
    /*
     * read a byte; return 8 if byte available, 0 if not
     */


    if (csize-- <= 0)
        return 0;

    if (incnt == 0) {
        if ((incnt = read(zipfd, (char *)inbuf, INBUFSIZ)) <= 0)
            return 0;
        /* buffer ALWAYS starts on a block boundary:  */
        cur_zipfile_bufstart += INBUFSIZ;
        inptr = inbuf;
#ifdef CRYPT
        if (pInfo->encrypted) {
            byte *p;
            int n, t;

	    for (n = incnt > csize + 1 ? (int)csize + 1 : incnt, p = inptr;
                 n--; p++)
                *p = DECRYPT(*p);
        }
#endif /* CRYPT */
    }
    *x = *inptr++;
    --incnt;
    return 8;
}





#ifndef VMS   /* for VMS use code in vms.c */

/**************************/
/* Function FlushOutput() */
/**************************/

int FlushOutput()
{
    /*
     * flush contents of output buffer; return PK-type error code
     */
#ifndef DOS_OS2
#   define CTRLZ  26
    int saved_ctrlZ = FALSE;
#endif /* !DOS_OS2 */
    int len;


    if (disk_full) {
        outpos += outcnt;   /* fake emptied buffer */
        outcnt = 0;
        outptr = outbuf;
        return 50;          /* ignore rest of this file */
    }

    if (outcnt) {
        UpdateCRC(outbuf, outcnt);

        if (!tflag) {
#ifndef DOS_OS2
            if (aflag) {
                if (outbuf[outcnt-1] == CTRLZ) {
                    --outcnt;
                    saved_ctrlZ = TRUE;
                }
                len = dos2unix(outbuf, outcnt);
            } else
#endif /* !DOS_OS2 */
                len = outcnt;
            if (write(outfd, (char *)outout, len) != len)
#ifdef DOS_OS2
                if (!cflag)           /* ^Z treated as EOF, removed with -c */
#else /* !DOS_OS2 */
#ifdef MINIX
                if (errno == EFBIG)
                    if (write(fd, outout, len/2) != len/2  ||
                        write(fd, outout+len/2, len/2) != len/2)
#endif /* MINIX */
#endif /* ?DOS_OS2 */
                {
                    /* GRR: add test for force_flag when has its own switch */
                    fprintf(stderr,
                      "\n%s:  write error (disk full?).  Continue? (y/n/^C) ",
                      filename);
                    FFLUSH   /* for Amiga and Mac MPW */
                    fgets(answerbuf, 9, stdin);
                    if (*answerbuf == 'y')   /* stop writing to this file */
                        disk_full = 1;       /*  (outfd bad?), but new OK */
                    else
                        disk_full = 2;       /* no:  exit program */
                    return 50;    /* 50:  disk full */
                }
        }
        outpos += outcnt;
        outcnt = 0;
        outptr = outbuf;
#ifndef DOS_OS2
        if (saved_ctrlZ) {
            *outptr++ = CTRLZ;
            ++outcnt;
        }
#endif /* !DOS_OS2 */
    }
    return (0);                 /* 0:  no error */
}

#endif /* !VMS */





#ifndef DOS_OS2   /* GRR:  rewrite this for generic text conversions */

/***********************/
/* Function dos2unix() */
/***********************/

static int dos2unix(buf, len)
unsigned char *buf;
int len;
{
    int new_len;
    int i;
    unsigned char *walker;

    new_len = len;
    walker = outout;
#ifdef MACOS
    /*
     * Mac wants to strip LFs instead CRs from CRLF pairs
     */
    if (CR_flag && *buf == LF) {
        buf++;
        new_len--;
        len--;
        CR_flag = buf[len] == CR;
    }
    else
        CR_flag = buf[len - 1] == CR;
    for (i = 0; i < len; i += 1) {
        *walker++ = ascii_to_native(*buf);
        if (*buf == LF) walker[-1] = CR;
        if (*buf++ == CR && *buf == LF) {
            new_len--;
            buf++;
            i++;
        }
    }
#else
    if (CR_flag && *buf != LF)
        *walker++ = ascii_to_native(CR);
    CR_flag = buf[len - 1] == CR;
    for (i = 0; i < len; i += 1) {
        *walker++ = ascii_to_native(*buf);
        if (*buf++ == CR && *buf == LF) {
            new_len--;
            walker[-1] = ascii_to_native(*buf++);
            i++;
        }
    }
    /*
     * If the last character is a CR, then "ignore it" for now...
     */
    if (walker[-1] == ascii_to_native(CR))
        new_len--;
#endif
    return new_len;
}

#endif /* !DOS_OS2 */





#ifdef DOS_OS2

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void set_file_time_and_close()
 /*
  * MS-DOS AND OS/2 VERSION (Mac, Unix/VMS versions are below)
  *
  * Set the output file date/time stamp according to information from the
  * zipfile directory record for this member, then close the file and set
  * its permissions (archive, hidden, read-only, system).  Aside from closing
  * the file, this routine is optional (but most compilers support it).
  */
{
/*---------------------------------------------------------------------------
    Allocate local variables needed by OS/2 and Turbo C.
  ---------------------------------------------------------------------------*/

#ifdef OS2              /* (assuming only MSC or MSC-compatible compilers
                         * for this part) */

    union {
        FDATE fd;               /* system file date record */
        UWORD zdate;            /* date word */
    } ud;

    union {
        FTIME ft;               /* system file time record */
        UWORD ztime;            /* time word */
    } ut;

    FILESTATUS fs;

#else                           /* !OS2 */
#ifdef __TURBOC__

    union {
        struct ftime ft;        /* system file time record */
        struct {
            UWORD ztime;        /* date and time words */
            UWORD zdate;        /* .. same format as in .ZIP file */
        } zt;
    } td;

#endif                          /* __TURBOC__ */
#endif                          /* !OS2 */

/*---------------------------------------------------------------------------
     Do not attempt to set the time stamp on standard output.
  ---------------------------------------------------------------------------*/

    if (cflag) {
        close(outfd);
        return;
    }

/*---------------------------------------------------------------------------
    Copy and/or convert time and date variables, if necessary; then set the
    file time/date.
  ---------------------------------------------------------------------------*/

#ifdef OS2
    DosQFileInfo(outfd, 1, &fs, sizeof(fs));
    ud.zdate = lrec.last_mod_file_date;
    ut.ztime = lrec.last_mod_file_time;
    fs.fdateLastWrite = ud.fd;
    fs.ftimeLastWrite = ut.ft;
    fs.attrFile = pInfo->dos_attr; /* hidden, system, archive, read-only */
    DosSetFileInfo(outfd, 1, (PBYTE) &fs, sizeof(fs));
#else /* !OS2 */
#ifdef __TURBOC__
    td.zt.ztime = lrec.last_mod_file_time;
    td.zt.zdate = lrec.last_mod_file_date;
    setftime(outfd, &td.ft);
#else /* !__TURBOC__ */
    _dos_setftime(outfd, lrec.last_mod_file_date, lrec.last_mod_file_time);
#endif /* ?__TURBOC__ */
#endif /* ?OS2 */

/*---------------------------------------------------------------------------
    And finally we can close the file...at least everybody agrees on how to
    do *this*.  I think...  Oh yeah, also change the mode according to the
    stored file attributes, since we didn't do that when we opened the dude.
  ---------------------------------------------------------------------------*/

    close(outfd);

#ifdef OS2
    if (longname)
        SetLongNameEA(filename, longfilename);
#else /* !OS2 */
#ifdef __TURBOC__
    if (_chmod(filename, 1, pInfo->dos_attr) != pInfo->dos_attr)
        fprintf(stderr, "\nwarning:  file attributes may not be correct\n");
#else /* !__TURBOC__ */
    _dos_setfileattr(filename, pInfo->dos_attr);
#endif /* ?__TURBOC__ */
#endif /* ?OS2 */

} /* end function set_file_time_and_close() (DOS, OS/2) */





#else                           /* !DOS_OS2 */
#ifdef MACOS                    /* Mac */

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void set_file_time_and_close()
 /*
  * MAC VERSION
  */
{
    long m_time;
    DateTimeRec dtr;
    ParamBlockRec pbr;
    OSErr err;

    if (outfd != 1) {
        close(outfd);

        /*
         * Macintosh bases all file modification times on the number of seconds
         * elapsed since Jan 1, 1904, 00:00:00.  Therefore, to maintain
         * compatibility with MS-DOS archives, which date from Jan 1, 1980,
         * with NO relation to GMT, the following conversions must be made:
         *      the Year (yr) must be incremented by 1980;
         *      and converted to seconds using the Mac routine Date2Secs(),
         *      almost similar in complexity to the Unix version :-)
         *                                     J. Lee
         */

        dtr.year = (((lrec.last_mod_file_date >> 9) & 0x7f) + 1980);
        dtr.month = ((lrec.last_mod_file_date >> 5) & 0x0f);
        dtr.day = (lrec.last_mod_file_date & 0x1f);

        dtr.hour = ((lrec.last_mod_file_time >> 11) & 0x1f);
        dtr.minute = ((lrec.last_mod_file_time >> 5) & 0x3f);
        dtr.second = ((lrec.last_mod_file_time & 0x1f) * 2);

        Date2Secs(&dtr, (unsigned long *)&m_time);
        CtoPstr(filename);
        pbr.fileParam.ioNamePtr = (StringPtr)filename;
        pbr.fileParam.ioVRefNum = pbr.fileParam.ioFVersNum =
          pbr.fileParam.ioFDirIndex = 0;
        err = PBGetFInfo(&pbr, 0L);
        pbr.fileParam.ioFlMdDat = pbr.fileParam.ioFlCrDat = m_time;
        if (err == noErr)
            err = PBSetFInfo(&pbr, 0L);
        if (err != noErr)
            printf("error:  can't set the time for %s\n", filename);

        /* set read-only perms if needed */
        if (err != noErr && pInfo->unix_attr != 0)
            err = SetFLock((ConstStr255Param)filename, 0);
        PtoCstr(filename);
    }
}





#else                                /* !MACOS... */
#if !defined(MTS) && !defined(VMS)   /* && !MTS (can't do) && !VMS: only one
                                      * left is UNIX (for VMS use code in
                                      * vms.c--old VMS code below is retained
                                      * in case of problems and will be removed
                                      * in a later release) */

/**************************************/
/* Function set_file_time_and_close() */
/**************************************/

void set_file_time_and_close()
 /*
  * UNIX AND VMS VERSION (MS-DOS & OS/2, Mac versions are above)
  */
{
    static short yday[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    long m_time;
    int yr, mo, dy, hh, mm, ss, leap, days=0;
#ifdef VMS
#   define YRBASE  0
    char timbuf[24];
    static char *month[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    struct VMStimbuf {
        char *actime;           /* VMS revision date, ASCII format */
        char *modtime;          /* VMS creation date, ASCII format */
    } ascii_times;
#else /* !VMS */
    struct utimbuf {
        time_t actime;          /* new access time */
        time_t modtime;         /* new modification time */
    } tp;
#ifdef AMIGA
#   define YRBASE  1978         /* in AmigaDos, counting begins 01-Jan-1978 */
    struct DateStamp myadate;
/*  extern char *_TZ;   no longer used? */
#else /* !AMIGA */
#   define YRBASE  1970
#ifdef BSD
    static struct timeb tbp;
#else /* !BSD */
    extern long timezone;
#endif /* ?BSD */
#endif /* ?AMIGA */
#endif /* ?VMS */


    /*
     * Close the file *before* setting its time under Unix, AmigaDos and VMS.
     */
#ifdef AMIGA
    if (cflag)                  /* can't set time on stdout */
        return;
    close(outfd);
#else /* !AMIGA */
    close(outfd);
    if (cflag)                  /* can't set time on stdout */
        return;
#endif /* ?AMIGA */

    /*
     * These date conversions look a little weird, so I'll explain.
     * UNIX bases all file modification times on the number of seconds
     * elapsed since Jan 1, 1970, 00:00:00 GMT.  Therefore, to maintain
     * compatibility with MS-DOS archives, which date from Jan 1, 1980,
     * with NO relation to GMT, the following conversions must be made:
     *      the Year (yr) must be incremented by 10;
     *      the Date (dy) must be decremented by 1;
     *      and the whole mess must be adjusted by TWO factors:
     *          relationship to GMT (ie.,Pacific Time adds 8 hrs.),
     *          and whether or not it is Daylight Savings Time.
     * Also, the usual conversions must take place to account for leap years,
     * etc.
     *                                     C. Seaman
     */

    /* dissect date */
    yr = ((lrec.last_mod_file_date >> 9) & 0x7f) + (1980 - YRBASE);
    mo = ((lrec.last_mod_file_date >> 5) & 0x0f) - 1;
    dy = (lrec.last_mod_file_date & 0x1f) - 1;

    /* dissect time */
    hh = (lrec.last_mod_file_time >> 11) & 0x1f;
    mm = (lrec.last_mod_file_time >> 5) & 0x3f;
    ss = (lrec.last_mod_file_time & 0x1f) * 2;

#ifdef VMS
    sprintf(timbuf, "%02d-%3s-%04d %02d:%02d:%02d.00", dy+1, month[mo],
      yr, hh, mm, ss);

    ascii_times.actime = timbuf;
    ascii_times.modtime = timbuf;

    if ((mm = VMSmunch(filename, SET_TIMES, &ascii_times)) != RMS$_NMF)
        fprintf(stderr, "error %d:  can't set the time for %s\n", mm, filename);

#else /* !VMS */
    /* leap = # of leap years from BASE up to but not including current year */
    leap = ((yr + YRBASE - 1) / 4);   /* leap year base factor */

    /* How many days from BASE to this year? (& add expired days this year) */
    days = (yr * 365) + (leap - 492) + yday[mo];

    /* if year is a leap year and month is after February, add another day */
    if ((mo > 1) && ((yr+YRBASE)%4 == 0) && ((yr+YRBASE) != 2100))
        ++days;                 /* OK through 2199 */

#ifdef AMIGA
/*  _TZ = getenv("TZ"); does Amiga not have TZ and tzset() after all? */
    myadate.ds_Days   =   days+dy-2;   /* off by one? */
    myadate.ds_Minute =   hh*60+mm;
    myadate.ds_Tick   =   ss*TICKS_PER_SECOND;

    if (!(SetFileDate(filename, &myadate)))
        fprintf(stderr, "error:  can't set the time for %s\n", filename);

#else /* !AMIGA */
    /* convert date & time to seconds relative to 00:00:00, 01/01/YRBASE */
    m_time = ((days + dy) * 86400) + (hh * 3600) + (mm * 60) + ss;

#ifdef BSD
    ftime(&tbp);
    m_time += tbp.timezone * 60L;
#else /* !BSD */
    tzset();                    /* set `timezone' */
    m_time += timezone;         /* account for timezone differences */
#endif /* ?BSD */

    if (localtime(&m_time)->tm_isdst)
        m_time -= 60L * 60L;    /* adjust for daylight savings time */

    tp.actime = m_time;         /* set access time */
    tp.modtime = m_time;        /* set modification time */

    /* set the time stamp on the file */
    if (utime(filename, &tp))
        fprintf(stderr, "error:  can't set the time for %s\n", filename);
#endif /* ?AMIGA */
#endif /* ?VMS */
}

#endif                          /* !MTS && !VMS */
#endif                          /* ?MACOS */
#endif                          /* ?DOS_OS2 */





/*******************************/
/*  Non-echoing password code  */
/*******************************/

#ifdef EMX32
#  undef DOS_OS2
#  undef MSVMS
#endif

#ifdef CRYPT
#ifndef DOS_OS2
#ifdef VMS

int echo(opt)
    int opt;
{
/*---------------------------------------------------------------------------
    Based on VMSmunch.c, which in turn was based on Joe Meadows' file.c code.
  ---------------------------------------------------------------------------
     * For VMS v5.x:
     *   IO$_SENSEMODE/SETMODE info:  Programming, Vol. 7A, System Programming,
     *     I/O User's: Part I, sec. 8.4.1.1, 8.4.3, 8.4.5, 8.6
     *   sys$assign(), sys$qio() info:  Programming, Vol. 4B, System Services,
     *     System Services Reference Manual, pp. sys-23, sys-379
     *   fixed-length descriptor info:  Programming, Vol. 3, System Services,
     *     Intro to System Routines, sec. 2.9.2
     * GRR, 15 Aug 91
  ---------------------------------------------------------------------------*/
    static struct dsc$descriptor_s DevDesc =
        {9, DSC$K_DTYPE_T, DSC$K_CLASS_S, "SYS$INPUT"};
     /* {dsc$w_length, dsc$b_dtype, dsc$b_class, dsc$a_pointer}; */
    static short           DevChan, iosb[4];
    static long            i, status;
    static unsigned long   oldmode[2], newmode[2];   /* each = 8 bytes */
  

/*---------------------------------------------------------------------------
    Assign a channel to standard input.
  ---------------------------------------------------------------------------*/

    status = sys$assign(&DevDesc, &DevChan, 0, 0);
#ifdef DEBUG
    printf("echo:  sys$assign returns status = %ld\n", status);
#endif /* DEBUG */
    if (!(status & 1)) return(status);

/*---------------------------------------------------------------------------
    Use sys$qio and the IO$_SENSEMODE function to determine the current tty
    status (for password reading, could use IO$_READVBLK function instead,
    but echo on/off will be more general).
  ---------------------------------------------------------------------------*/

    status = sys$qio(0, DevChan, IO$_SENSEMODE, &iosb, 0, 0,
                     oldmode, 8, 0, 0, 0, 0);
#ifdef DEBUG
    printf("echo:  sys$qio(IO$_SENSEMODE) returns status = %ld\n", status);
    printf("echo:  sys$qio(IO$_SENSEMODE) returns iosb status = %d\n", 
      iosb[0]);
#endif /* DEBUG */
    if (!(status & 1)) return(status);
    status = iosb[0];
    if (!(status & 1)) return(status);

/*---------------------------------------------------------------------------
    Copy old mode into new-mode buffer, then modify to be either NOECHO or
    ECHO (depending on function argument opt).
  ---------------------------------------------------------------------------*/

    newmode[0] = oldmode[0];
    newmode[1] = oldmode[1];
    if (opt == OFF)
        newmode[1] |= TT$M_NOECHO;                      /* set NOECHO bit */
    else
        newmode[1] &= ~((unsigned long) TT$M_NOECHO);   /* clear NOECHO bit */
#ifdef DEBUG
    printf("echo:  current terminal status = %lx\n", oldmode[1]);
    printf("echo:  current echo status = %s\n",
      (oldmode[1] & TT$M_NOECHO)? "noecho" : "echo");
    printf("echo:  new terminal status = %lx\n", newmode[1]);
    printf("echo:  new echo status = %s\n",
      (newmode[1] & TT$M_NOECHO)? "noecho" : "echo");
#endif /* DEBUG */

/*---------------------------------------------------------------------------
    Use the IO$_SETMODE function to change the tty status.
  ---------------------------------------------------------------------------*/

    status = sys$qio(0, DevChan, IO$_SETMODE, &iosb, 0, 0,
                     newmode, 8, 0, 0, 0, 0);
#ifdef DEBUG
    printf("echo:  sys$qio(IO$_SETMODE) returns status = %ld\n", status);
    printf("echo:  sys$qio(IO$_SETMODE) returns iosb status = %d\n", iosb[0]);
#endif /* DEBUG */
    if (!(status & 1)) return(status);
    status = iosb[0];
    if (!(status & 1)) return(status);

/*---------------------------------------------------------------------------
    Deassign the sys$input channel by way of clean-up, then exit happily.
  ---------------------------------------------------------------------------*/

    status = sys$dassgn(DevChan);
#ifdef DEBUG
    printf("echo:  sys$dassgn returns status = %ld\n\n", status);
#endif /* DEBUG */
    if (!(status & 1)) return(status);

    return SS$_NORMAL;   /* we be happy */

} /* end function echo() */





#else /* !VMS */

static int echofd = -1;	/* file descriptor whose echo is off */

void echoff(f)
int f;			/* file descriptor to turn echo off on */
/* Turn echo off for file descriptor f.  Assumes that f is a tty device. */
{
  struct sgttyb sg;	/* tty device structure */

  echofd = f;
  GTTY(f, &sg);					/* get settings */
  sg.sg_flags &= ~ECHO;				/* turn echo off */
  STTY(f, &sg);
}



void echon()
/* Turn echo back on for file descriptor echofd. */
{
  struct sgttyb sg;	/* tty device structure */

  if (echofd != -1)
  {
    GTTY(echofd, &sg);				/* get settings */
    sg.sg_flags |= ECHO;			/* turn echo on */
    STTY(echofd, &sg);
    echofd = -1;
  }
}

#endif /* ?VMS */
#endif /* !DOS_OS2 */





char *getp(m, p, n)
char *m;		/* prompt for password */
char *p;		/* return value: line input */
int n;			/* bytes available in p[] */
/* Get a password of length n-1 or less into *p using the prompt *m.
   The entered password is not echoed.  Return p on success, NULL on
   failure (can't get controlling tty). */
{
  char c;		/* one-byte buffer for read() to use */
  int i;		/* number of characters input */
  char *w;		/* warning on retry */

#ifndef DOS_OS2
#ifdef VMS
  echo(OFF);                                    /* turn echo off */
#else /* !VMS */
  int f;		/* file decsriptor for tty device */

  /* Turn off echo on tty */
  if (!isatty(2))
    return NULL;				/* error if not tty */
  if ((f = open(ttyname(2), 0, 0)) == -1)
    return NULL;
  echoff(f);					/* turn echo off */
#endif /* ?VMS */
#endif /* !DOS_OS2 */

  /* Get password */
  w = "";
  do {
#ifdef VMS   /* bug:  VMS adds '\n' to NULL fputs (apparently) */
    if (*w)
#endif /* VMS */
    fputs(w, stderr);				/* warning if back again */
    fputs(m, stderr);				/* prompt */
    fflush(stderr);
    i = 0;
    do {					/* read line, keeping n */
#ifdef MSVMS
      if ((c = (char)getch()) == '\r')
        c = '\n';
#else /* !MSVMS */
      read(f, &c, 1);
#endif /* ?MSVMS */
      if (i < n)
	p[i++] = c;
    } while (c != '\n');
    putc('\n', stderr);  fflush(stderr);
    w = "(line too long--try again)\n";
  } while (p[i-1] != '\n');
  p[i-1] = 0;					/* terminate at newline */

#ifndef DOS_OS2
#ifdef VMS
  echo(ON);                                     /* turn echo back on */
#else /* !VMS */
  /* Turn echo back on */
  echon();					/* turn echo back on */
  close(f);
#endif /* ?VMS */
#endif /* !DOS_OS2 */

  /* Return pointer to password */
  return p;
}

#endif /* CRYPT */
