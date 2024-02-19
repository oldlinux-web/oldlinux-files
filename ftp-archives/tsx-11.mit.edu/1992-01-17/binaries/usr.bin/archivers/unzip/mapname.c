/*---------------------------------------------------------------------------

  mapname.c

  This routine changes DEC-20, VAX/VMS, and DOS-style filenames into normal
  Unix names (and vice versa, in some cases); it also creates any necessary 
  directories, if the -d switch was specified.

  ---------------------------------------------------------------------------

  Notes:

     - Unix allows multiple dots in directory names; MS-DOS and OS/2 FAT
       allow one; VMS does not allow any.  Things are almost as bad with
       regular filenames (VMS allows a single dot but TOPS-20 allows two,
       if you count the one in front of the version number).  As of v4.04,
       mapname converts directory-name dots to underscores on VMS, but it
       otherwise leaves the dots alone.  Since it is now possible to create
       zipfiles under Unix, this whole routine pretty much needs to be
       rewritten (different routines for each output OS, and different 
       rules for different parts of the path name).
     - If each zip program stores local-format names (like the VMS one did
       at one time), it would probably be best to convert to an intermedi-
       ate format first (assuming we're not extracting under the same OS 
       as that under which the zipfile was created), then from that to the 
       current operating system's format.
     - The strcpy and strcat operations on both cdp and filename may over-
       write memory, since they don't check lengths.  With a kilobyte in
       which to work, this is probably not that big a deal, but it could
       cause problems eventually.

  ------------------------------------------------------------------------- */


#include "unzip.h"


/*******************/
/* Mapname Defines */
/*******************/

#ifdef VMS
#  define PERMS   0
#else
#  define PERMS   0777
#endif

#ifndef NO_MKDIR
#  ifdef DOS_OS2
#    if (_MSC_VER >= 600)       /* have special MSC mkdir prototype */
#      include <direct.h>
#    else                       /* own prototype because dir.h conflicts? */
       int mkdir(const char *path);
#    endif /* ?(MSC 6.0 or later) */
#    define MKDIR(path,mode)   mkdir(path)
#  else /* !DOS_OS2 */
#    ifdef MACOS
#      define MKDIR(path,mode)   mkdir(path)
#    else /* !MACOS */
#      define MKDIR(path,mode)   mkdir(path,mode)
#    endif /* ?MACOS */
#  endif /* ?DOS_OS2 */
#endif /* !NO_MKDIR */




/************************/
/*  Function mapname()  */
/************************/

int mapname(create_dirs)   /* return 0 if no error, 1 if caution (filename */
    int create_dirs;       /*  truncated), 2 if warning (skip file because */
{                          /*  dir doesn't exist), 3 if error (skip file) */
#ifdef NO_MKDIR
    char command[FILNAMSIZ+40]; /* buffer for system() call */
#endif
#ifdef VMS
    int stat_val;               /* temp. holder for stat() return value */
    char *dp, *xp;              /* pointers to directory name */
#endif
#ifdef OS2
    char *last;
    extern int longname;        /* used also in file_io.c:  set EAs */
    extern char longfilename[]; /*  AFTER file created and closed */
#endif
    char name[FILNAMSIZ];       /* file name buffer */
    char *pp, *cp, *cdp;        /* character pointers */
    char delim = '\0';          /* directory delimiter */
    int dc = 0;                 /* counters */
    int quote = FALSE;          /* flags */
    int indir = FALSE;
    int done = FALSE;
    register int workch;        /* hold the character being tested */


/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

#ifdef MAP_DEBUG
    fprintf(stderr, "%s ", filename);   /* echo name of this file */
#endif
    pp = name;                  /* Point to translation buffer */
    *name = '\0';               /* Initialize buffer */

    if (!jflag) {               /* -j => junk pathnames */
        cdp = (char *) malloc(strlen(filename) + 3);  /* place for holding */
        if (cdp == NULL) {                            /*  directory name */
            fprintf(stderr, "mapname:  out of memory [%s]\n", filename);
            return 3;
        }
#ifdef VMS
        *cdp++ = '[';
        xp = cdp;               /* always points to last non-NULL char */
        *cdp++ = '.';
#endif /* VMS */
#ifdef MACOS
        *cdp = ':';             /* the Mac uses ':' as a directory separator */
        cdp[1] = '\0';
#else /* !MACOS */
        *cdp = '\0';
#endif /* ?MACOS */
    }
    dc = 0;                     /* Filename dot counter */

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    for (cp = filename; (workch = *cp++) != 0  &&  !done;) {

        if (quote) {            /* If this char quoted... */
            *pp++ = workch;     /*  include it literally. */
            quote = FALSE;
        } else if (indir) {     /* If in directory name... */
            if (workch == delim)
                indir = FALSE;  /*  look for end delimiter. */
        } else
            switch (workch) {
            case '<':           /* Discard DEC-20 directory name */
                indir = TRUE;
                delim = '>';
                break;
            case '[':           /* Discard VMS directory name */
                indir = TRUE;
                delim = ']';
                break;
            case '/':           /* Discard Unix path name... */
            case '\\':          /*  or MS-DOS path name...
                                 *  IF -j flag was given. */
                /*
                 * Special processing case:  if -j flag was not specified on
                 * command line and create_dirs is TRUE, create any necessary
                 * directories included in the pathname.  Creation of dirs is
                 * straightforward on BSD and MS-DOS machines but requires use
                 * of the system() command on SysV systems (or any others which
                 * don't have mkdir()).  The stat() check is necessary with
                 * MSC because it doesn't have an EEXIST errno, and it saves
                 * the overhead of multiple system() calls on SysV machines.
                 */

                if (!jflag) {
                    *pp = '\0';
#ifdef VMS
                    dp = name;
                    while (*++xp = *dp++)   /* copy name to cdp, while */
                        if (*xp == '.')     /*   changing all dots... */
                            *xp = '_';      /*   ...to underscores */
                    strcpy(xp, ".dir");    /* add extension for stat check */
                    stat_val = stat(cdp, &statbuf);
                    *xp = '\0';         /* remove extension for all else */
                    if (stat_val) {     /* doesn't exist, so create */
#else /* !VMS */
                    strcat(cdp, name);
#ifdef OS2
                    if (longname = !IsFileNameValid(cdp)) {
                        last = strrchr(cdp, '/');
                        strcpy(longfilename, last ? last + 1 : cdp);
                        fprintf(stderr, "caution:  renaming directory \"%s\"",
                          cdp);
                        ChangeNameForFAT(cdp);
                        fprintf(stderr, " to \"%s\"\n", cdp);
                    }
#endif /* OS2 */
                    if (stat(cdp, &statbuf)) {  /* doesn't exist, so create */
#endif /* ?VMS */
                        if (!create_dirs) /* told not to create (freshening) */
                            return 2;
#ifdef NO_MKDIR
                        sprintf(command,
                          "IFS=\" \t\n\" /bin/mkdir %s 2>/dev/null", cdp);
                        if (system(command)) {
#else /* !NO_MKDIR */
                        if (MKDIR(cdp, PERMS) == -1) {
#endif /* ?NO_MKDIR */
                            perror(cdp);
                            free(cdp);
                            fprintf(stderr, "mapame:  unable to process [%s]\n",
                              filename);
                            return 3;
                        }
#ifdef OS2
                        if (longname)
                            SetLongNameEA(cdp, longfilename);
#endif /* OS2 */
                    } else if (!(statbuf.st_mode & S_IFDIR)) {
                        fprintf(stderr,
                          "mapname:  %s exists but is not a directory\n", cdp);
                        free(cdp);
                        fprintf(stderr, "mapame:  unable to process [%s]\n",
                          filename);
                        return 3;
                    }
#ifdef VMS
                    *xp = '/';  /* for now... (mkdir()) */
#else /* !VMS */
#ifdef MACOS
                    strcat(cdp, ":");
#else /* !MACOS */
                    strcat(cdp, "/");
#endif /* ?MACOS */
#endif /* ?VMS */
                }
                pp = name;
                break;
            case ':':
#ifdef UNIX                       /* colon is a valid character in Unix */
                *pp++ = workch;   /*  filenames, so keep it; anywhere else, */
#else /* !UNIX */                 /*  change it to an underscore (should  */
                *pp++ = '_';      /*  NOT have stored drive/node names!!) */
#endif /* ?UNIX */
             /* pp = name;  (OLD) discard DEC dev: or node:: name */
                break;
            case '.':                   /* DEC-20 generation number
                                         * or MS-DOS type */
#ifdef NUKE_DOTS
                if (++dc == 1)          /* Keep first dot */
                    *pp++ = workch;
#else /* !NUKE_DOTS */
                ++dc;                   /* Not used, but what the hell. */
                *pp++ = workch;
#endif /* ?NUKE_DOTS */
                break;
            case ';':                   /* VMS generation or DEC-20 attrib */
                if (V_flag)             /* If requested, save VMS ";##" */
                    *pp++ = workch;     /*  version info; else discard */
                else                    /*  everything starting with */
                    done = TRUE;        /*  semicolon.  (Worry about */
                break;                  /*  DEC-20 later.) */
            case '\026':                /* Control-V quote for special chars */
                quote = TRUE;           /* Set flag for next time. */
                break;
            case ' ':
#if defined(VMS) || defined(MTS)
                *pp++ = '_';            /* change spaces to underscore */
#else /* !(VMS || MTS) */               /*  under VMS and MTS, and under DOS */
#ifdef DOS_OS2                          /*  and OS/2 if -s not specified. */
                if (!sflag)
                    *pp++ = '_';
                else
#endif /* DOS_OS2 */
                *pp++ = workch;         /* otherwise, leave as spaces */
#endif /* ?(VMS || MTS) */
                break;
            default:
                if (isprint(workch))    /* other printable, just keep */
                    *pp++ = workch;
            }                   /* end switch */
    }                           /* end for loop */
    *pp = '\0';                 /* done with name:  terminate it */

/*---------------------------------------------------------------------------
    We COULD check for existing names right now, create a "unique" name, etc.
    At present, we do this in extract_or_test_files() (immediately after we
    return from here).  If conversion went bad, the name'll either be nulled
    out (in which case we'll return non-0), or following procedures won't be
    able to create the extracted file and other error msgs will result.
  ---------------------------------------------------------------------------*/

    if (*name == '\0') {
        fprintf(stderr, "mapname:  conversion of [%s] failed\n", filename);
        return 3;
    }

#ifdef OS2   /* (if necessary, can use outbuf for temp filename holder) */
    if (longname = !IsFileNameValid(name)) {  /* THIS time, save for file_io */
        last = strrchr(name, '/');
        last = last ? last + 1 : name;  /* should ALWAYS be name... */
        strcpy(longfilename, last);
        fprintf(stderr, "caution:  renaming \"%s\"", name);
        ChangeNameForFAT(last);
        fprintf(stderr, " to \"%s\"\n", name);
    }
#endif /* OS2 */

    if (!jflag) {
#ifdef VMS
        *xp++ = ']';            /* proper end-of-dir-name delimiter */
        if (xp == cdp) {        /* no path-name stuff, so... */
            strcpy(filename, name);  /* copy file name into global */
            cdp -= 2;           /*   prepare to free malloc'd space */
        } else {                /* we've added path-name stuff... */
            *xp = '\0';         /*   so terminate... */
            dp = cdp;           /*   and convert to VMS subdir separators:  */
            while (*++dp)       /*   (skip first char:  better not be "/") */
                if (*dp == '/') /*   change all slashes */
                    *dp = '.';  /*     to dots */
            cdp -= 2;           /*   include leading bracket and dot */
            strcpy(filename, cdp);   /* copy VMS-style path name into global */
            strcat(filename, name);  /* concatenate file name to global */
        }
#else /* !VMS */
        strcpy(filename, cdp);  /* Either "" or slash-terminated path */
        strcat(filename, name); /* append file name to path name */
#endif /* ?VMS */
        free(cdp);
    } else
        strcpy(filename, name); /* copy converted name into global */

#if PATH_MAX < (FILNAMSIZ - 1)
    /* check the length of the file name and truncate if necessary */
    if (PATH_MAX < strlen(filename)) {
        fprintf(stderr, "caution:  truncating filename.\n");
        filename[PATH_MAX] = '\0';
        fprintf(stderr, "[ %s ]\n", filename);
        return 1;             /* 1:  warning error */
    }
#endif

    return 0;
}
