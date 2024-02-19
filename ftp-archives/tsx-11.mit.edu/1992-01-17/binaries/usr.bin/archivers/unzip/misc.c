/*---------------------------------------------------------------------------

  misc.c

  This file contains a number of useful but not particularly closely related
  functions; their main claim to fame is that they don't change much, so this
  file should rarely need to be recompiled.  The CRC-32 stuff is from crc32.c;
  do_string() is from nunzip.c; makeword() and makelong() are from unzip.c;
  memset() and memcpy() are from zmemset.c and zmemcpy.c, respectively; and
  dos_to_unix_time() is from set_file_time_and_close() in file_io.c.  ebcdic[],
  check_for_newer(), dateformat(), and return_VMS() are new.  Things lumped
  together here to cut down on the size of unzip.c and the number of associ-
  ated files floating around.

  ---------------------------------------------------------------------------

  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.

  ---------------------------------------------------------------------------*/


#include "unzip.h"



#ifndef ZIPINFO   /* no need to calculate CRCs */

/**************************/
/*  Function UpdateCRC()  */
/**************************/

 /*--------------------------------------------------------------------

   First, the polynomial itself and its table of feedback terms.  The
   polynomial is
   X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0

   Note that we take it "backwards" and put the highest-order term in
   the lowest-order bit.  The X^32 term is "implied"; the LSB is the
   X^31 term, etc.  The X^0 term (usually shown as "+1") results in
   the MSB being 1.

   Note that the usual hardware shift register implementation, which
   is what we're using (we're merely optimizing it by doing eight-bit
   chunks at a time) shifts bits into the lowest-order term.  In our
   implementation, that means shifting towards the right.  Why do we
   do it this way?  Because the calculated CRC must be transmitted in
   order from highest-order term to lowest-order term.  UARTs transmit
   characters in order from LSB to MSB.  By storing the CRC this way,
   we hand it to the UART in the order low-byte to high-byte; the UART
   sends each low-bit to hight-bit; and the result is transmission bit
   by bit from highest- to lowest-order term without requiring any bit
   shuffling on our part.  Reception works similarly.

   The feedback terms table consists of 256, 32-bit entries.  Notes:

       The table can be generated at runtime if desired; code to do so
       is shown later.  It might not be obvious, but the feedback
       terms simply represent the results of eight shift/xor opera-
       tions for all combinations of data and CRC register values.

       The values must be right-shifted by eight bits by the "updcrc"
       logic; the shift must be unsigned (bring in zeroes).  On some
       hardware you could probably optimize the shift in assembler by
       using byte-swap instructions.
       polynomial $edb88320

   --------------------------------------------------------------------*/

ULONG crc_32_tab[] =
{
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


void UpdateCRC(s, len)
register byte *s;
register int len;
 /* update running CRC calculation with contents of a buffer */
{
    register ULONG crcval = crc32val;


    while (len--)
        crcval = crc_32_tab[((byte) crcval ^ (*s++)) & 0xff] ^ (crcval >> 8);
    crc32val = crcval;
}

#endif /* !ZIPINFO */





/**************************/
/*  Function do_string()  */
/**************************/

int do_string(len, option)      /* return PK-type error code */
unsigned int len;               /* without prototype, UWORD converted to this */
int option;
{
    int block_length, error = 0;
    UWORD comment_bytes_left, extra_len;



/*---------------------------------------------------------------------------
    This function processes arbitrary-length (well, usually) strings.  Three
    options are allowed:  SKIP, wherein the string is skipped pretty logical,
    eh?); DISPLAY, wherein the string is printed to standard output after un-
    dergoing any necessary or unnecessary character conversions; and FILENAME,
    wherein the string is put into the filename[] array after undergoing ap-
    propriate conversions (including case-conversion, if that is indicated:
    see the global variable pInfo->lcflag).  The latter option should be OK,
    since filename is now dimensioned at 1025, but we check anyway.

    The string, by the way, is assumed to start at the current file-pointer
    position; its length is given by len.  So start off by checking length
    of string:  if zero, we're already set.
  ---------------------------------------------------------------------------*/

    if (!len)
        return (0);             /* 0:  no error */

    switch (option) {

    /*
     * First case:  print string on standard output.  First set loop vari-
     * ables, then loop through the comment in chunks of OUTBUFSIZ bytes,
     * converting formats and printing as we go.  The second half of the
     * loop conditional was added because the file might be truncated, in
     * which case comment_bytes_left will remain at some non-zero value for
     * all time.  outbuf is used as a scratch buffer because it is avail-
     * able (we should be either before or in between any file processing).
     * [The typecast in front of the min() macro was added because of the
     * new promotion rules under ANSI C; readbuf() wants an int, but min()
     * returns a signed long, if I understand things correctly.  The proto-
     * type should handle it, but just in case...]
     */

    case DISPLAY:
        comment_bytes_left = len;
        block_length = OUTBUFSIZ;    /* for the while statement, first time */
        while (comment_bytes_left > 0 && block_length > 0) {
            if ((block_length = readbuf((char *) outbuf,
                         (int) min(OUTBUFSIZ, comment_bytes_left))) <= 0)
                return (51);    /* 51:  unexpected EOF */
            comment_bytes_left -= block_length;
            NUKE_CRs(outbuf, block_length);     /* (modifies block_length) */

            /*  this is why we allocated an extra byte for outbuf: */
            outbuf[block_length] = '\0';        /* terminate w/zero:  ASCIIZ */

            A_TO_N(outbuf);     /* translate string to native */

            printf("%s", outbuf);
        }
        printf("\n", outbuf);   /* assume no newline at end */
        break;

    /*
     * Second case:  read string into filename[] array.  The filename should
     * never ever be longer than FILNAMSIZ-1 (1024), but for now we'll check,
     * just to be sure.
     */

    case FILENAME:
        extra_len = 0;
        if (len >= FILNAMSIZ) {
            fprintf(stderr, "warning:  filename too long--truncating.\n");
            error = 1;          /* 1:  warning error */
            extra_len = len - FILNAMSIZ + 1;
            len = FILNAMSIZ - 1;
        }
        if (readbuf(filename, len) <= 0)
            return (51);        /* 51:  unexpected EOF */
        filename[len] = '\0';   /* terminate w/zero:  ASCIIZ */

        A_TO_N(filename);       /* translate string to native */

#ifndef ZIPINFO
        if (pInfo->lcflag)
            TOLOWER(filename, filename);  /* replace with lowercase filename */
#endif

        if (!extra_len)         /* we're done here */
            break;

        /*
         * We truncated the filename, so print what's left and then fall
         * through to the SKIP routine.
         */
        fprintf(stderr, "[ %s ]\n", filename);
        len = extra_len;
        /*  FALL THROUGH...  */

    /*
     * Third case:  skip string, adjusting readbuf's internal variables
     * as necessary (and possibly skipping to and reading a new block of
     * data).
     */

    case SKIP:
        LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        break;

    /*
     * Fourth case:  assume we're at the start of an "extra field"; malloc
     * storage for it and read data into the allocated space.
     */

    case EXTRA_FIELD:
        if (extra_field != NULL)
            free(extra_field);
        if ((extra_field = (byte *)malloc(len)) == NULL) {
            fprintf(stderr,
              "warning:  extra field too long (%d).  Ignoring...\n", len);
            LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        } else
            if (readbuf((char *)extra_field, len) <= 0)
                return 51;      /* 51:  unexpected EOF */
        break;

    }                           /* end switch (option) */
    return error;

}                               /* end function do_string() */





#ifndef ZIPINFO
#ifndef VMS

/*********************************/
/*  Function dos_to_unix_time()  */
/*********************************/

time_t dos_to_unix_time(ddate, dtime)
unsigned ddate, dtime;
{
    static short yday[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int yr, mo, dy, hh, mm, ss, leap;
    long m_time, days=0;
#if !defined(MACOS) && !defined(MSC)
#if defined(BSD) || defined(MTS)
    static struct timeb tbp;
#else /* !(BSD || MTS) */
    extern long timezone;    /* declared in <time.h> for MSC (& Borland?) */
#endif /* ?(BSD || MTS) */
#endif /* !MACOS && !MSC (may need to change to DOS_OS2) */

#   define YRBASE  1970

    /* dissect date */
    yr = ((ddate >> 9) & 0x7f) + (1980 - YRBASE);
    mo = ((ddate >> 5) & 0x0f) - 1;
    dy = (ddate & 0x1f) - 1;

    /* dissect time */
    hh = (dtime >> 11) & 0x1f;
    mm = (dtime >> 5) & 0x3f;
    ss = (dtime & 0x1f) * 2;

    /* leap = # of leap years from BASE up to but not including current year */
    leap = ((yr + YRBASE - 1) / 4);   /* leap year base factor */

    /* How many days from BASE to this year? (& add expired days this year) */
    days = (yr * 365) + (leap - 492) + yday[mo];

    /* if year is a leap year and month is after February, add another day */
    if ((mo > 1) && ((yr+YRBASE)%4 == 0) && ((yr+YRBASE) != 2100))
        ++days;                 /* OK through 2199 */

    /* convert date & time to seconds relative to 00:00:00, 01/01/YRBASE */
    m_time = ((long)(days + dy) * 86400) + ((long) hh * 3600) + (mm * 60) + ss;
      /* - 1;   MS-DOS times always rounded up to nearest even second */

#if !defined(MACOS) && !defined(EMX32)
#if defined(BSD) || defined(MTS)
    ftime(&tbp);
    m_time += tbp.timezone * 60L;
#else /* !(BSD || MTS) */
    tzset();                    /* set `timezone' */
    m_time += timezone;         /* account for timezone differences */
#endif /* ?(BSD || MTS) */
#endif /* !MACOS && !EMX32 */

    if (localtime((time_t *)&m_time)->tm_isdst)
        m_time -= 60L * 60L;    /* adjust for daylight savings time */

    return m_time;

} /* end function dos_to_unix_time() */

#endif /* !VMS */





/********************************/
/*  Function check_for_newer()  */  /* could make this into a macro for Unix */
/********************************/

int check_for_newer(filename)   /* return 1 if existing file newer or equal; */
char *filename;                 /*  0 if older; -1 if doesn't exist yet */
{
#ifdef VMS
    int dy, mo, yr, hh, mm, ss, dy2, mo2, yr2, hh2, mm2, ss2;
    float sec;
    char mon[4];
    static char actimbuf[24], modtimbuf[24];
    static char *month[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    struct VMStimbuf {
        char *actime;           /* VMS revision date, ASCII format */
        char *modtime;          /* VMS creation date, ASCII format */
    } ascii_times={actimbuf,modtimbuf};


    if (stat(filename, &statbuf))
        return -1;

    if (VMSmunch(filename, GET_TIMES, &ascii_times) != RMS$_NMF)
        return 0;   /* exists but can't get the time:  assume older */

    sscanf(modtimbuf, "%2d-%3s-%04d %02d:%02d:%05f", &dy, mon,
      &yr, &hh, &mm, &sec);

    yr2 = ((lrec.last_mod_file_date >> 9) & 0x7f) + 1980;
    if (yr > yr2)
        return 1;
    else if (yr < yr2)
        return 0;

    for (mo = 0;  mo < 11;  ++mo)
        if (!strcmp(mon, month[mo]))
            break;
    mo2 = ((lrec.last_mod_file_date >> 5) & 0x0f) - 1;
    if (mo > mo2)
        return 1;
    else if (mo < mo2)
        return 0;

    dy2 = (lrec.last_mod_file_date & 0x1f);
    if (dy > dy2)
        return 1;
    else if (dy < dy2)
        return 0;

    hh2 = (lrec.last_mod_file_time >> 11) & 0x1f;
    if (hh > hh2)
        return 1;
    else if (hh < hh2)
        return 0;

    mm2 = (lrec.last_mod_file_time >> 5) & 0x3f;
    if (mm > mm2)
        return 1;
    else if (mm < mm2)
        return 0;

    /* round to nearest 2 secs--may become 60, but doesn't matter for compare */
    ss = (int)(sec + 1.) & -2;
    ss2 = (lrec.last_mod_file_time & 0x1f) * 2;
    if (ss >= ss2)
        return 1;

    return 0;

#else /* !VMS */        /* round up filetime to nearest 2 secs --v  */
    return stat(filename, &statbuf)?  -1 :
      ( ((statbuf.st_mtime & 1)? statbuf.st_mtime+1 : statbuf.st_mtime) >=
      dos_to_unix_time(lrec.last_mod_file_date, lrec.last_mod_file_time) );
#endif /* ?VMS */

} /* end function check_for_newer() */





/***************************/
/*  Function dateformat()  */
/***************************/

int dateformat()
{

/*-----------------------------------------------------------------------------
  For those operating systems which support it, this function returns a value
  which tells how national convention says that numeric dates are displayed.

  Return values are DF_YMD, DF_DMY and DF_MDY.  The meanings should be fairly
  obvious.
 -----------------------------------------------------------------------------*/

#ifdef OS2
    COUNTRYINFO    ctryi;
    COUNTRYCODE    ctryc;
#ifdef __32BIT__
    ULONG          cbCountryInfo;
#else
    USHORT         cbCountryInfo;
#endif


    ctryc.country = ctryc.codepage = 0;
    if (DosGetCtryInfo(sizeof ctryi, &ctryc, &ctryi, &cbCountryInfo) != NO_ERROR)
        return DF_MDY;
    else
        switch (ctryi.fsDateFmt) {
            case 0 /* DATEFMT_MM_DD_YY */ :
                return DF_MDY;
            case 1 /* DATEFMT_DD_MM_YY */ :
                return DF_DMY;
            case 2 /* DATEFMT_YY_MM_DD */ :
                return DF_YMD;
        }
#else /* !OS2 */
#ifdef MSDOS
    unsigned short int CountryInfo[18];
    union REGS regs;
    struct SREGS sregs;


    regs.x.ax = 0x3800;
    regs.x.dx = FP_OFF(CountryInfo);
    sregs.ds  = FP_SEG(CountryInfo);
    int86x(0x21, &regs, &regs, &sregs);
    switch(CountryInfo[0]) {
        case 0:
            return DF_MDY;
        case 1:
            return DF_DMY;
        case 2:
            return DF_YMD;
    }
#endif /* !MSDOS */
#endif /* ?OS2 */

    return DF_MDY;   /* default for Unix, VMS, etc. */
}                               /* end function dateformat() */

#endif /* !ZIPINFO */





#ifdef EBCDIC

/*
 * This is the MTS ASCII->EBCDIC translation table. It provides a 1-1
 * translation from ISO 8859/1 8-bit ASCII to IBM Code Page 37 EBCDIC.
 */

unsigned char ebcdic[] =
{
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,
    0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a, 0x1b,
    0x30, 0x31, 0x1a, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3a, 0x3b, 0x04, 0x14, 0x3e, 0xff,
    0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5, 0xbd, 0xb4, 0x9a, 0x8a, 0x5f, 0xca, 0xaf, 0xbc,
    0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3, 0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
    0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf, 0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xad, 0xae, 0x59,
    0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
    0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1, 0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf
};

#endif                          /* EBCDIC */





/*************************/
/*  Function makeword()  */
/*************************/

UWORD makeword(b)
byte *b;
 /*
  * Convert Intel style 'short' integer to non-Intel non-16-bit
  * host format.  This routine also takes care of byte-ordering.
  */
{
/*
    return  ( ((UWORD)(b[1]) << 8)  |  (UWORD)(b[0]) );
 */
    return ((b[1] << 8) | b[0]);
}





/*************************/
/*  Function makelong()  */
/*************************/

ULONG makelong(sig)
byte *sig;
 /*
  * Convert intel style 'long' variable to non-Intel non-16-bit
  * host format.  This routine also takes care of byte-ordering.
  */
{
    return (((ULONG) sig[3]) << 24)
        + (((ULONG) sig[2]) << 16)
        + (((ULONG) sig[1]) << 8)
        + ((ULONG) sig[0]);
}





#ifdef VMS

/***************************/
/*  Function return_VMS()  */
/***************************/

void return_VMS(zip_error)
int zip_error;
{
#ifdef RETURN_CODES
/*---------------------------------------------------------------------------
    Do our own, explicit processing of error codes and print message, since
    VMS misinterprets return codes as rather obnoxious system errors ("access
    violation," for example).
  ---------------------------------------------------------------------------*/

    switch (zip_error) {

    case 0:
        break;                  /* life is fine... */
    case 1:
        fprintf(stderr, "\n[return-code 1:  warning error \
(e.g., failed CRC or unknown compression method)]\n");
        break;
    case 2:
    case 3:
        fprintf(stderr, "\n[return-code %d:  error in zipfile \
(e.g., can't find local file header sig)]\n",
                zip_error);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        fprintf(stderr, "\n[return-code %d:  insufficient memory]\n",
                zip_error);
        break;
    case 9:
        fprintf(stderr, "\n[return-code 9:  zipfile not found]\n");
        break;
    case 10:                    /* this is the one that gives "access violation," I think */
        fprintf(stderr, "\n[return-code 10:  bad or illegal parameters \
specified on command line]\n");
        break;
    case 11:                    /* I'm not sure this one is implemented, but maybe soon? */
        fprintf(stderr, "\n[return-code 11:  no files found to \
extract/view/etc.]\n");
        break;
    case 50:
        fprintf(stderr, "\n[return-code 50:  disk full \
(or otherwise unable to open output file)]\n");
        break;
    case 51:
        fprintf(stderr, "\n[return-code 51:  unexpected EOF in zipfile \
(i.e., truncated)]\n");
        break;
    default:
        fprintf(stderr, "\n[return-code %d:  unknown return-code \
(who put this one in?  Wasn't me...)]\n",
                zip_error);
        break;
    }
#endif                          /* RETURN_CODES */

    exit(0);                    /* everything okey-dokey as far as VMS concerned */
}

#endif                          /* VMS */





#ifdef ZMEM                     /* memset, memcpy for systems without them */

/***********************/
/*  Function memset()  */
/***********************/

char *memset(buf, init, len)
register char *buf, init;       /* buffer loc and initializer */
register unsigned int len;      /* length of the buffer */
{
    char *start;

    start = buf;
    while (len--)
        *(buf++) = init;
    return (start);
}





/***********************/
/*  Function memcpy()  */
/***********************/

char *memcpy(dst, src, len)
register char *dst, *src;
register unsigned int len;
{
    char *start;

    start = dst;
    while (len-- > 0)
        *dst++ = *src++;
    return (start);
}

#endif                          /* ZMEM */
