/*---------------------------------------------------------------------------

  extract.c

  This file contains the high-level routines ("driver routines") for extrac-
  ting and testing zipfile members.  It calls the low-level routines in files
  inflate.c, unimplod.c, unreduce.c and unshrink.c.

  ---------------------------------------------------------------------------*/


#include "unzip.h"


/************************************/
/*  Extract Local Prototypes, etc.  */
/************************************/

static int store_info __((void));
static int extract_or_test_member __((void));
#ifdef CRYPT
   static int decrypt_member __((void));
#endif

static char *VersionMsg =
  " skipping: %-22s  need %s compat. v%u.%u (can do v%u.%u)\n";
static char *ComprMsg =
  " skipping: %-22s  compression method %d\n";
static char *FilNamMsg =
  "%s:  bad filename length (%s)\n";
static char *ExtFieldMsg =
  "%s:  bad extra field length (%s)\n";
static char *OffsetMsg =
  "file #%d:  bad zipfile offset (%s)\n";





/**************************************/
/*  Function extract_or_test_files()  */
/**************************************/

int extract_or_test_files()    /* return PK-type error code */
{
    char **fnamev;
    byte *cd_inptr;
    int cd_incnt, error, error_in_archive=0;
    int renamed, query, len, filnum=(-1), blknum=0;
    UWORD i, j, members_remaining, num_skipped=0, num_bad_pwd=0;
    longint cd_bufstart, bufstart, inbuf_offset, request;
    min_info info[DIR_BLKSIZ];


/*---------------------------------------------------------------------------
    The basic idea of this function is as follows.  Since the central di-
    rectory lies at the end of the zipfile and the member files lie at the
    beginning or middle or wherever, it is not very desirable to simply
    read a central directory entry, jump to the member and extract it, and
    then jump back to the central directory.  In the case of a large zipfile
    this would lead to a whole lot of disk-grinding, especially if each mem-
    ber file is small.  Instead, we read from the central directory the per-
    tinent information for a block of files, then go extract/test the whole
    block.  Thus this routine contains two small(er) loops within a very
    large outer loop:  the first of the small ones reads a block of files
    from the central directory; the second extracts or tests each file; and
    the outer one loops over blocks.  There's some file-pointer positioning
    stuff in between, but that's about it.  Btw, it's because of this jump-
    ing around that we can afford to be lenient if an error occurs in one of
    the member files:  we should still be able to go find the other members,
    since we know the offset of each from the beginning of the zipfile.

    Begin main loop over blocks of member files.  We know the entire central
    directory is on this disk:  we would not have any of this information un-
    less the end-of-central-directory record was on this disk, and we would
    not have gotten to this routine unless this is also the disk on which
    the central directory starts.  In practice, this had better be the ONLY
    disk in the archive, but maybe someday we'll add multi-disk support.
  ---------------------------------------------------------------------------*/

    pInfo = info;
    members_remaining = ecrec.total_entries_central_dir;

    while (members_remaining) {
        j = 0;

        /*
         * Loop through files in central directory, storing offsets, file
         * attributes, and case-conversion flags until block size is reached.
         */

        while (members_remaining && (j < DIR_BLKSIZ)) {
            --members_remaining;
            pInfo = &info[j];

            if (readbuf(sig, 4) <= 0) {
                error_in_archive = 51;  /* 51:  unexpected EOF */
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
                fprintf(stderr, CentSigMsg, j);  /* sig not found */
                fprintf(stderr, ReportMsg);   /* check binary transfers */
                error_in_archive = 3;   /* 3:  error in zipfile */
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            /* process_cdir_file_hdr() sets pInfo->hostnum, pInfo->lcflag */
            if ((error = process_cdir_file_hdr()) != 0) {
                error_in_archive = error;   /* only 51 (EOF) defined */
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            if ((error = do_string(crec.filename_length, FILENAME)) != 0) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > 1) {  /* fatal:  no more left to do */
                    fprintf(stderr, FilNamMsg, filename, "central");
                    members_remaining = 0;
                    break;
                }
            }
            if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0)
            {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > 1) {  /* fatal */
                    fprintf(stderr, ExtFieldMsg, filename, "central");
                    members_remaining = 0;
                    break;
                }
            }
            if ((error = do_string(crec.file_comment_length, SKIP)) != 0) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > 1) {  /* fatal */
                    fprintf(stderr, "\n%s:  bad file comment length\n",
                            filename);
                    members_remaining = 0;
                    break;
                }
            }
            if (process_all_files) {
                if (store_info())
                    ++num_skipped;
                else
                    ++j;  /* file is OK: save info[] and continue with next */
            } else {
                fnamev = fnv;   /* don't destroy permanent filename pointer */
                for (--fnamev; *++fnamev;)
                    if (match(filename, *fnamev)) {
                        if (store_info())
                            ++num_skipped;
                        else
                            ++j;   /* file is OK */
                        break;  /* found match for filename, so stop looping */
                    } /* end if (match), for-loop (fnamev) */
            } /* end if (process_all_files) */

        } /* end while-loop (adding files to current block) */

        /* save position in central directory so can come back later */
        cd_bufstart = cur_zipfile_bufstart;
        cd_inptr = inptr;
        cd_incnt = incnt;

    /*-----------------------------------------------------------------------
        Second loop:  process files in current block, extracting or testing
        each one.
      -----------------------------------------------------------------------*/

        for (i = 0; i < j; ++i) {
            filnum = i + blknum*DIR_BLKSIZ;
            pInfo = &info[i];
            /*
             * if the target position is not within the current input buffer
             * (either haven't yet read far enough, or (maybe) skipping back-
             * ward) skip to the target position and reset readbuf().
             */
            /* LSEEK(pInfo->offset):  */
            request = pInfo->offset + extra_bytes;
            inbuf_offset = request % INBUFSIZ;
            bufstart = request - inbuf_offset;

            if (request < 0) {
                fprintf(stderr, SeekMsg, ReportMsg);
                error_in_archive = 3;       /* 3:  severe error in zipfile, */
                continue;                   /*  but can still go on */
            } else if (bufstart != cur_zipfile_bufstart) {
                cur_zipfile_bufstart = lseek(zipfd, bufstart, SEEK_SET);
                if ((incnt = read(zipfd,(char *)inbuf,INBUFSIZ)) <= 0) {
                    fprintf(stderr, OffsetMsg, filnum, "lseek");
                    error_in_archive = 3;   /* 3:  error in zipfile, but */
                    continue;               /*  can still do next file   */
                }
                inptr = inbuf + inbuf_offset;
                incnt -= inbuf_offset;
            } else {
                incnt += (inptr-inbuf) - inbuf_offset;
                inptr = inbuf + inbuf_offset;
            }

            /* should be in proper position now, so check for sig */
            if (readbuf(sig, 4) <= 0) {  /* bad offset */
                fprintf(stderr, OffsetMsg, filnum, "EOF");
                error_in_archive = 3;    /* 3:  error in zipfile */
                continue;       /* but can still try next one */
            }
            if (strncmp(sig, local_hdr_sig, 4)) {
                fprintf(stderr, OffsetMsg, filnum,
                        "can't find local header sig");   /* bad offset */
                error_in_archive = 3;
                continue;
            }
            if ((error = process_local_file_hdr()) != 0) {
                fprintf(stderr, "\nfile #%d:  bad local header\n", filnum);
                error_in_archive = error;       /* only 51 (EOF) defined */
                continue;       /* can still try next one */
            }
            if ((error = do_string(lrec.filename_length, FILENAME)) != 0) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > 1) {
                    fprintf(stderr, FilNamMsg, filename, "local");
                    continue;   /* go on to next one */
                }
            }
            if ((error = do_string(lrec.extra_field_length, EXTRA_FIELD)) != 0)
            {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > 1) {
                    fprintf(stderr, ExtFieldMsg, filename, "local");
                    continue;   /* go on */
                }
            }

            /*
             * just about to extract file:  if extracting to disk, check if
             * already exists, and if so, take appropriate action according to
             * fflag/uflag/overwrite_all/etc. (we couldn't do this in upper
             * loop because we don't store the possibly renamed filename[] in
             * info[])
             */
            if (!tflag && !cflag) {
                renamed = FALSE;   /* user hasn't renamed output file yet */
startover:
                query = FALSE;
                /* mapname can create dirs if not freshening or if renamed */
                if ((error = mapname(!fflag || renamed)) > 1) {    /* skip */
                    if ((error > 2) && (error_in_archive < 2))
                        error_in_archive = 2;   /* (weak) error in zipfile */
                    continue;   /* go on to next file */
                }

                switch (check_for_newer(filename)) {
                    case DOES_NOT_EXIST:
                        if (fflag && !renamed)  /* don't skip if just renamed */
                            continue;   /* freshen (no new files):  skip */
                        break;
                    case EXISTS_AND_OLDER:
                        if (overwrite_none)
                            continue;   /* never overwrite:  skip file */
                        if (!overwrite_all && !force_flag)
                            query = TRUE;
                        break;
                    case EXISTS_AND_NEWER:             /* (or equal) */
                        if (overwrite_none || (uflag && !renamed))
                            continue;  /* skip if update/freshen & orig name */
                        if (!overwrite_all && !force_flag)
                            query = TRUE;
                        break;
                }
#ifndef VMS     /* VMS creates higher version number instead of overwriting
                 * (will have to modify for VMS-style names with specific
                 *  version numbers:  just check V_flag?  don't use stat?) */
                if (query) {
                    fprintf(stderr,
                      "replace %s? [y]es, [n]o, [A]ll, [N]one, [r]ename: ",
                      filename);
                    FFLUSH   /* for Amiga and Mac MPW */
                    fgets(answerbuf, 9, stdin);
                    switch (*answerbuf) {
                        case 'A':   /* dangerous option:  force caps */
                            overwrite_all = TRUE;
                            overwrite_none = FALSE;  /* just to make sure */
                            break;
                        case 'r':
                        case 'R':
                            do {
                                fprintf(stderr, "new name: ");
                                FFLUSH   /* for AMIGA and Mac MPW */
                                fgets(filename, FILNAMSIZ, stdin);
                                /* usually get \n here: better check for it */
                                len = strlen(filename);
                                if (filename[len-1] == '\n')
                                    filename[--len] = 0;
                            } while (len == 0);
                            renamed = TRUE;
                            goto startover;   /* sorry for a goto */
                        /*  break;   (never reached--dpk) */
                        case 'y':
                        case 'Y':
                            break;
                        case 'N':
                            overwrite_none = TRUE;
                            overwrite_all = FALSE;  /* make sure */
                            force_flag = FALSE;     /* ditto */
                            /* FALL THROUGH, skip */
                        case 'n':
                        default:
                            continue;   /* skip file */
                    } /* end switch (*answerbuf) */
                } /* end if (query) */
#endif /* !VMS */
            } /* end if (extracting to disk) */

#ifdef CRYPT
            if (pInfo->encrypted && ((error = decrypt_member()) != 0)) {
                fprintf(stderr, " skipping: %-22s  incorrect password\n",
                  filename);
                ++num_bad_pwd;
            /*  if (error > error_in_archive)   >>until fix up error return<<
                    error_in_archive = error;    unable to test file... */
                continue;   /* go on to next file */
                /* GRR:  add loop here (2 or 3 times) for bad passwords */
            }
#endif /* CRYPT */
            disk_full = 0;
            if ((error = extract_or_test_member()) != 0) {
                if (error > error_in_archive)
                    error_in_archive = error;       /* ...and keep going */
                if (disk_full > 1)
                    return error_in_archive;        /* (unless disk full) */
            }
        } /* end for-loop (i:  files in current block) */


        /*
         * Jump back to where we were in the central directory, then go and do
         * the next batch of files.
         */

        cur_zipfile_bufstart = lseek(zipfd, cd_bufstart, SEEK_SET);
        read(zipfd, (char *)inbuf, INBUFSIZ);  /* were there b4 ==> no error */
        inptr = cd_inptr;
        incnt = cd_incnt;
        ++blknum;

#ifdef TEST
        printf("\ncd_bufstart = %ld (%.8lXh)\n", cd_bufstart, cd_bufstart);
        printf("cur_zipfile_bufstart = %ld (%.8lXh)\n", cur_zipfile_bufstart,
          cur_zipfile_bufstart);
        printf("inptr-inbuf = %d\n", inptr-inbuf);
        printf("incnt = %d\n\n", incnt);
#endif

    } /* end while-loop (blocks of files in central directory) */

/*---------------------------------------------------------------------------
    Double-check that we're back at the end-of-central-directory record, and
    print quick summary of results, if we were just testing the archive.  We
    send the summary to stdout so that people doing the testing in the back-
    ground and redirecting to a file can just do a "tail" on the output file.
  ---------------------------------------------------------------------------*/

    readbuf(sig, 4);
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
        fprintf(stderr, ReportMsg);  /* check binary transfers */
        if (!error_in_archive)       /* don't overwrite stronger error */
            error_in_archive = 1;    /* 1:  warning error */
    }
    if (tflag && (quietflg == 1)) {
        int num=filnum+1 - num_bad_pwd;

        if (error_in_archive)
            printf("At least one error was detected in %s.\n", zipfn);
        else if (num == 0)
            printf("Caution:  zero files tested in %s.\n", zipfn);
        else if (process_all_files && (num_skipped+num_bad_pwd == 0))
            printf("No errors detected in %s.\n", zipfn);
        else
            printf("No errors detected in %s for the %d file%s tested.\n",
              zipfn, num, (num==1)? "":"s");
        if (num_skipped > 0)
            printf("%d file%s skipped because of unsupported compression or\
 encoding.\n",
              num_skipped, (num_skipped==1)? "":"s");
#ifdef CRYPT
        if (num_bad_pwd > 0)
            printf("%d file%s skipped because of incorrect password.\n",
              num_bad_pwd, (num_bad_pwd==1)? "":"s");
#endif /* CRYPT */
    }
    if ((num_skipped > 0) && !error_in_archive)   /* files not tested or  */
        error_in_archive = 1;                     /*  extracted:  warning */
#ifdef CRYPT
    if ((num_bad_pwd > 0) && !error_in_archive)   /* files not tested or  */
        error_in_archive = 1;                     /*  extracted:  warning */
#endif /* CRYPT */

    return (error_in_archive);

} /* end function extract_or_test_files() */





/***************************/
/*  Function store_info()  */
/***************************/

static int store_info()   /* return 1 if skipping, 0 if OK */
{
    ULONG tmp;

#ifdef INFLATE
#  define UNKN_COMPR \
   (crec.compression_method>IMPLODED && crec.compression_method!=DEFLATED)
#else
#  define UNKN_COMPR   (crec.compression_method>IMPLODED)
#endif


/*---------------------------------------------------------------------------
    Check central directory info for version/compatibility requirements.
  ---------------------------------------------------------------------------*/

    pInfo->encrypted = crec.general_purpose_bit_flag & 1;    /* bit field */
    pInfo->ExtLocHdr = (crec.general_purpose_bit_flag & 8) == 8;  /* bit */
    pInfo->text = crec.internal_file_attributes & 1;         /* bit field */
/*  pInfo->crc = crec.crc32;  (no longer used) */

    if (crec.version_needed_to_extract[1] == VMS_) {
        if (crec.version_needed_to_extract[0] > VMS_VERSION) {
            fprintf(stderr, VersionMsg, filename, "VMS",
              crec.version_needed_to_extract[0] / 10,
              crec.version_needed_to_extract[0] % 10,
              VMS_VERSION / 10, VMS_VERSION % 10);
            return 1;
        }
#ifndef VMS   /* won't be able to use extra field, but still have data */
        else if (!tflag && !force_flag) {  /* if forcing, extract regardless */
            fprintf(stderr,
              "\n%s:  stored in VMS format.  Extract anyway? (y/n) ",
              filename);
            FFLUSH   /* for Amiga and Mac MPW */
            fgets(answerbuf, 9, stdin);
            if ((*answerbuf != 'y') && (*answerbuf != 'Y'))
                return 1;
        }
#endif /* !VMS */
    /* usual file type:  don't need VMS to extract */
    } else if (crec.version_needed_to_extract[0] > UNZIP_VERSION) {
        fprintf(stderr, VersionMsg, filename, "PK",
          crec.version_needed_to_extract[0] / 10,
          crec.version_needed_to_extract[0] % 10,
          UNZIP_VERSION / 10, UNZIP_VERSION % 10);
        return 1;
    }

    if UNKN_COMPR {
        fprintf(stderr, ComprMsg, filename, crec.compression_method);
        return 1;
    }
#ifndef CRYPT
    if (pInfo->encrypted) {
        fprintf(stderr, " skipping: %-22s  encrypted (not supported)\n",
          filename);
        return 1;
    }
#endif /* !CRYPT */

/*---------------------------------------------------------------------------
    Store some central-directory information (encryption, file attributes,
    offsets) for later use.
  ---------------------------------------------------------------------------*/

    tmp = crec.external_file_attributes;

    pInfo->dos_attr = 32;   /* set archive bit:  file is not backed up */
    switch (pInfo->hostnum) {
        case UNIX_:
        case VMS_:
            pInfo->unix_attr = tmp >> 16;
            break;
        case DOS_OS2_FAT_:
        case OS2_HPFS_:
            pInfo->dos_attr = (unsigned) tmp;
            tmp = (!(tmp & 1)) << 1;   /* read-only bit */
            pInfo->unix_attr = 0444 | (tmp<<6) | (tmp<<3) | tmp;
#ifdef UNIX
            umask( (int)(tmp=umask(0)) );
            pInfo->unix_attr &= ~tmp;
#endif
            break;
        case MAC_:
            tmp &= 1;   /* read-only bit */
            pInfo->unix_attr = tmp;
            break;
        default:
            pInfo->unix_attr = 0666;
            break;
    } /* end switch (host-OS-created-by) */

    pInfo->offset = (longint) crec.relative_offset_local_header;
    return 0;

} /* end function store_info() */





/***************************************/
/*  Function extract_or_test_member()  */
/***************************************/

static int extract_or_test_member()    /* return PK-type error code */
{
#ifdef S_IFLNK
    int symlnk=FALSE;
#endif /* S_IFLNK */
    int error=0;
    UWORD b;



/*---------------------------------------------------------------------------
    Initialize variables, buffers, etc.
  ---------------------------------------------------------------------------*/

    bits_left = 0;
    bitbuf = 0L;
    outpos = 0L;
    outcnt = 0;
    outptr = outbuf;
    zipeof = 0;
    crc32val = 0xFFFFFFFFL;

#ifdef S_IFLNK
    if ((pInfo->unix_attr & S_IFMT) == S_IFLNK  &&  (pInfo->hostnum == UNIX_)
        && !tflag && !cflag)
        symlnk = TRUE;
#endif /* S_IFLNK */

    memset(outbuf, 0, OUTBUFSIZ);
#ifndef DOS_OS2
    if (aflag)                  /* if we have a scratchpad, clear it out */
        memset(outout, 0, OUTBUFSIZ);
#endif /* !DOS_OS2 */

    if (tflag) {
        if (!quietflg) {
            fprintf(stdout, "  Testing: %-22s ", filename);
            fflush(stdout);
        }
    } else {
        if (cflag) {            /* output to stdout (copy of it) */
#if defined(MACOS) || defined(AMIGA)
            outfd = 1;
#else /* !(MACOS || AMIGA) */
            outfd = dup(1);     /* GRR: change this to #define for Mac/Amiga */
#endif /* ?(MACOS || AMIGA) */
#ifdef DOS_OS2
            if (!aflag)
                setmode(outfd, O_BINARY);
#endif /* DOS_OS2 */
#ifdef VMS
            if (create_output_file())   /* VMS version required for stdout! */
                return 50;      /* 50:  disk full (?) */
#endif
        } else
#ifdef S_IFLNK
        if (!symlnk)    /* symlink() takes care of file creation */
#endif /* !S_IFLNK */
        {
            if (create_output_file())
                return 50;      /* 50:  disk full (?) */
        }
    } /* endif (!tflag) */

/*---------------------------------------------------------------------------
    Unpack the file.
  ---------------------------------------------------------------------------*/

    switch (lrec.compression_method) {

    case STORED:
        if (!tflag && QCOND) {
            fprintf(stdout, " Extracting: %-22s ", filename);
            if (cflag)
                fprintf(stdout, "\n");
            fflush(stdout);
        }
#ifdef S_IFLNK
        /*
         * If file came from Unix and is a symbolic link and we are extracting
         * to disk, allocate a storage area, put the data in it, and create the
         * link.  Since we know it's a symbolic link to start with, shouldn't
         * have to worry about overflowing unsigned ints with unsigned longs.
         * (This doesn't do anything for compressed symlinks, but that can be
         * added later...it also doesn't set the time or permissions of the
         * link, but does anyone really care?)
         */
        if (symlnk) {
#if defined(MTS) || defined(MACOS)
            fprintf(stdout, "\n  warning:  symbolic link ignored\n");
            error = 1;          /* 1:  warning error */
#else /* !(MTS || MACOS) */
            char *orig = (char *)malloc((unsigned)lrec.uncompressed_size+1);
            char *p = orig;

            while (ReadByte(&b))
                *p++ = b;
            *p = 0;   /* terminate string */
            UpdateCRC((unsigned char *)orig, p-orig);
            if (symlink(orig, filename))
                if ((errno == EEXIST) && overwrite_all) {  /* OK to overwrite */
                    unlink(filename);
                    if (symlink(orig, filename))
                        perror("symlink error");
                } else
                    perror("symlink error");
            free(orig);
#endif /* ?(MTS || MACOS) */
        } else
#endif /* S_IFLNK */
        while (ReadByte(&b) && !disk_full)
            OUTB(b)
        break;

    case SHRUNK:
        if (!tflag && QCOND) {
            fprintf(stdout, "UnShrinking: %-22s ", filename);
            if (cflag)
                fprintf(stdout, "\n");
            fflush(stdout);
        }
#ifdef S_IFLNK   /* !!! This code needs to be added to unShrink, etc. !!! */
        if (symlnk) {
            fprintf(stdout, "\n  warning:  symbolic link ignored\n");
            error = 1;          /* 1:  warning error */
        }
#endif /* S_IFLNK */
        unShrink();
        break;

    case REDUCED1:
    case REDUCED2:
    case REDUCED3:
    case REDUCED4:
        if (!tflag && QCOND) {
            fprintf(stdout, "  Expanding: %-22s ", filename);
            if (cflag)
                fprintf(stdout, "\n");
            fflush(stdout);
        }
#ifdef S_IFLNK   /* !!! This code needs to be added to unShrink, etc. !!! */
        if (symlnk) {
            fprintf(stdout, "\n  warning:  symbolic link ignored\n");
            error = 1;          /* 1:  warning error */
        }
#endif /* S_IFLNK */
        unReduce();
        break;

    case IMPLODED:
        if (!tflag && QCOND) {
            fprintf(stdout, "  Exploding: %-22s ", filename);
            if (cflag)
                fprintf(stdout, "\n");
            fflush(stdout);
        }
#ifdef S_IFLNK   /* !!! This code needs to be added to unShrink, etc. !!! */
        if (symlnk) {
            fprintf(stdout, "\n  warning:  symbolic link ignored\n");
            error = 1;          /* 1:  warning error */
        }
#endif /* S_IFLNK */
        unImplode();
        break;

#ifdef INFLATE
    case DEFLATED:
        if (!tflag && QCOND) {
            fprintf(stdout, "  Deflating: %-22s ", filename);
            if (cflag)
                fprintf(stdout, "\n");
            fflush(stdout);
        }
#ifdef S_IFLNK   /* !!! This code needs to be added to unShrink, etc. !!! */
        if (symlnk) {
            fprintf(stdout, "\n  warning:  symbolic link ignored\n");
            error = 1;          /* 1:  warning error */
        }
#endif /* S_IFLNK */
        inflate();
        break;
#endif /* INFLATE */

    default:   /* should never get to this point */
        fprintf(stderr, "%s:  unknown compression method\n", filename);
        /* close and delete file before return? */
        return 1;               /* 1:  warning error */

    } /* end switch (compression method) */

    if (disk_full) {            /* set by FlushOutput()/OUTB() macro */
        if (disk_full > 1)
            return 50;          /* 50:  disk full */
        error = 1;              /* 1:  warning error */
    }

/*---------------------------------------------------------------------------
    Write the last partial buffer, if any; set the file date and time; and
    close the file (not necessarily in that order).  Then make sure CRC came
    out OK and print result.  [Note:  crc32val must be logical-ANDed with
    32 bits of 1's, or else machines whose longs are bigger than 32 bits will
    report bad CRCs (because of the upper bits being filled with 1's instead
    of 0's).]
  ---------------------------------------------------------------------------*/

#ifdef S_IFLNK
    if (!symlnk) {
#endif /* S_IFLNK */
    if (!disk_full && FlushOutput())
        if (disk_full > 1)
            return 50;          /* 50:  disk full */
        else {                  /* disk_full == 1 */
            fprintf(stderr, "%s:  probably corrupt\n", filename);
            error = 1;          /* 1:  warning error */
        }

    if (!tflag)
#ifdef VMS
        CloseOutputFile();
#else /* !VMS */
#ifdef MTS                      /* MTS can't set file time */
        close(outfd);
#else /* !MTS */
        set_file_time_and_close();
#endif /* ?MTS */
#endif /* ?VMS */

#ifdef S_IFLNK
    } /* endif (!symlnk) */
#endif /* S_IFLNK */

    if ((crc32val = ((~crc32val) & 0xFFFFFFFFL)) != lrec.crc32) {
        /* if quietflg is set, we haven't output the filename yet:  do it */
        if (quietflg)
            printf("%-22s: ", filename);
        fprintf(stdout, " Bad CRC %08lx  (should be %08lx)\n", crc32val,
                lrec.crc32);
        error = 1;              /* 1:  warning error */
    } else if (tflag) {
        if (!quietflg)
            fprintf(stdout, " OK\n");
    } else {
        if (QCOND && !error)
            fprintf(stdout, "\n");
    }

    return error;

}       /* end function extract_or_test_member() */





#ifdef CRYPT

/*******************************/
/*  Function decrypt_member()  */
/*******************************/

static int decrypt_member()   /* return 10 if out of memory or can't get */
{                             /*  tty; -1 if bad password; 0 if checks out */
    UWORD b, c;
    byte *p;
    int n, t;

    /* set keys */
    if (key == NULL && ((key = malloc(PWLEN+1)) == NULL ||
         getp("Enter password: ", key, PWLEN+1) == NULL))
        return 10;        /* what to return here? */
    init_keys(key);

    /* decrypt current buffer contents, set flag to re-read if necessary */
    for (n = incnt > csize ? (int)csize : incnt, p = inptr; n--; p++)
        *p = DECRYPT(*p);

    /* decrypt header */
    for (c = 0; c < 10; c++)
        ReadByte(&b);
    ReadByte(&c);
    ReadByte(&b);
#ifdef CRYPT_DEBUG
    printf("   lrec.crc = %08lx   crec.crc = [ ? ]   pInfo->ExtLocHdr = %s\n",
      lrec.crc32, /* pInfo->crc, */ pInfo->ExtLocHdr? "true":"false");
    printf("   incnt = %d   unzip offset into zipfile = %ld\n", incnt,
      cur_zipfile_bufstart+(inptr-inbuf));
    printf("   (c | (b<<8)) = %04x   (crc >> 16) = %04x   lrec.time = %04x\n",
      (UWORD)(c | (b<<8)), (UWORD)(lrec.crc32 >> 16), lrec.last_mod_file_time);
#endif /* CRYPT_DEBUG */
    /* same test as in zipbare() in crypt.c */
    if ((UWORD)(c | (b<<8)) != (pInfo->ExtLocHdr? lrec.last_mod_file_time :
        (UWORD)(lrec.crc32 >> 16)))
        return -1;
    return 0;
}

#endif /* CRYPT */
