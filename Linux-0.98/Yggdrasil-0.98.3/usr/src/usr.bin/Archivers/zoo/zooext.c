#ifndef LINT
/* derived from: zooext.c 2.21 88/08/24 02:39:04 */
/*$Source: /usr/home/dhesi/zoo/RCS/zooext.c,v $*/
/*$Id: zooext.c,v 1.9 91/07/09 01:54:13 dhesi Exp $*/
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/zooext.c,v $\n\
$Id: zooext.c,v 1.9 91/07/09 01:54:13 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
(C) Copyright 1991 Rahul Dhesi -- All rights reserved
*/
/* Extract file from archive.  Extracts files specified in parameter-list
   from archive zoo_path.  If none specified, extracts all files from
   archive. */

#include "options.h"
#include "zoo.h"
#include "parse.h"      /* defines struct for parse() */

#include "portable.h"   /* portable I/O definitions */
#include "machine.h"    /* machine-specific declarations */

#include "zooio.h"
#include "various.h"

#ifndef NOSIGNAL
#include <signal.h>
#endif

#include "zoofns.h"

#ifdef MODE_BIN		/* will need fileno() from stdio.h */
# include <stdio.h>
#endif

void makepath PARMS((char *));
int needed PARMS((char *, struct direntry *, struct zoo_header *));
void putstr PARMS((char *));

#ifdef FATTR
int setfattr PARMS ((char *, unsigned long));
#endif /* FATTR */

extern int quiet;

#include "errors.i"

/* Following two are used by ctrl_c() also, hence declared here */
char extfname[LFNAMESIZE];             /* filename of extracted file */
char prtfname[LFNAMESIZE];             /* name of extracted file on screen */
static ZOOFILE this_file;              /* file to extract */

static int tofile;                     /* true if not pipe or null device */
extern unsigned int crccode;
extern char *out_buf_adr;              /* address of output buffer */

void zooext(zoo_path, option)
char *zoo_path, *option;
{
char *whichname;                          /* which name to extract */
char matchname[PATHSIZE];                 /* for pattern matching only */
#ifndef NOSIGNAL
T_SIGNAL (*oldsignal)();        /* to save previous SIGINT handler */
#endif
ZOOFILE zoo_file;                         /* open archive */
long next_ptr;                            /* pointer to within archive */
struct zoo_header zoo_header;             /* header for archive */
int status;                               /* error status */
int exit_status = 0;								/* exit status */
int error_message;								/* Whether to give error message */
unsigned long disk_space;                 /* disk space left */
int matched = 0;                          /* Any files matched? */
int overwrite = 0;                        /* force overwrite of files? */
int supersede = 0;								/* supersede newer files? */
int needdel = 0;                          /* extract deleted files too */
int usepath = 2;                          /* use path for extraction */
int todot = 0;                            /* extract relative to . */
int badcrc_count = 0;                     /* how many files with bad CRC */
int bad_header = 0;                       /* to avoid spurious messages later */
long fiz_ofs = 0;                         /* offset where to start */
long dat_ofs = 0;									/* .. and offset of file data */
int pipe = 0;                             /* are we piping output? */
int null_device = 0;                      /* are we sending to null device? */
#ifndef PORTABLE
int fast_ext = 0;                         /* fast extract as *.?Z? */
int alloc_size;                           /* disk allocation unit size */
#endif
struct direntry direntry;                 /* directory entry */
int first_dir = 1;								/* first dir entry seen? */

static char extract_ver[] = "Zoo %d.%d is needed to extract %s.\n";
static char no_space[] = "Insufficient disk space to extract %s.\n";

while (*option) {
   switch (*option) {
#ifndef PORTABLE
      case 'z': fast_ext++; break;
#endif
      case 'x':
      case 'e': break;
      case 'N': null_device++; break;
      case 'O': overwrite += 2; break;
      case 'o': overwrite++; break;
      case 'p': pipe++; break;
		case 'S': supersede++; break;
      case 'd': needdel++; break;
      case 'q': quiet++; break;
		case ':': usepath = 0; break;
      case '/': usepath++; break;
      case '.': todot++; break;
      case '@': 	/* if @m,n specified, fiz_ofs = m, dat_ofs = n */
			{
				char *comma_pos;
				++option;
				comma_pos = strchr(option, ',');
				if (comma_pos != NULL) {
					dat_ofs = calc_ofs (comma_pos + 1);
					*comma_pos = '\0';
				}
				fiz_ofs = calc_ofs(option); 
				goto no_more;
			}
      default:
         prterror ('f', inv_option, *option);
         /* break; */
   }
   option++;
}

no_more: /* come from exit in while loop above */


if (overwrite == 1)                 /* must be at least 2 to begin with */
   overwrite--;

if (null_device && pipe) {
   prterror ('f', inv_option, 'p');
   pipe = 0;
}

if (overwrite && pipe)
   prterror ('w', option_ignored, 'O');

#ifndef PORTABLE
if (null_device && fast_ext) {
   prterror ('w', inv_option, 'N');
   null_device = 0;
}
#endif

tofile = !pipe && !null_device;     /* sending to actual file */

zoo_file = zooopen(zoo_path, Z_READ);

if (zoo_file == NOFILE)
   prterror ('f', could_not_open, zoo_path);

if (fiz_ofs != 0L) {                /* if offset specified, start there */
	prterror ('m', start_ofs, fiz_ofs, dat_ofs);
   zooseek (zoo_file, fiz_ofs, 0);
} else {
   /* read header */
   frd_zooh (&zoo_header, zoo_file);
   if ((zoo_header.zoo_start + zoo_header.zoo_minus) != 0L) {
      prterror ('w', failed_consistency);
      bad_header++;
		exit_status = 1;
   }
   zooseek (zoo_file, zoo_header.zoo_start, 0); /* seek to where data begins */
}

#ifndef PORTABLE
disk_space = space (0, &alloc_size);         /* remember disk space left */
#else
disk_space = MAXLONG;              /* infinite disk space */
#endif

/* if piping output we open the output device just once */
if (null_device) {
   this_file = NULLFILE;
} else if (pipe)
   this_file = STDOUT;    /* standard output */

while (1) {
   frd_dir (&direntry, zoo_file);
   if (direntry.zoo_tag != ZOO_TAG) {
      long currpos, zoolength;
		prterror ('F', invalid_header);

		/* Note:  if header was bad, there's no point trying to find
			how many more bytes aren't processed -- our seek position is
			likely very wrong */

		if (!bad_header)
			if ((currpos = zootell (zoo_file)) != -1L)
				if (zooseek (zoo_file, 0L, 2) != -1)
					if ((zoolength = zootell (zoo_file)) != -1L)
						printf (cant_process, zoolength - currpos);              
		zooexit (1);
   }
   if (direntry.next == 0L) {                /* END OF CHAIN */
      break;                                 /* EXIT on end of chain */
   }
	/* when first direntry read, change dat_ofs from abs. pos. to rel. offset */
	if (first_dir && dat_ofs != 0) {
		dat_ofs -= direntry.offset;
		first_dir = 0;
	}
   next_ptr = direntry.next + dat_ofs;       /* ptr to next dir entry */

   whichname = choosefname(&direntry);       /* which filename */
	whichname = str_dup(whichname);				/* bug fix */
   fixfname(whichname);                      /* fix syntax */
	strcpy (matchname, fullpath (&direntry));	/* get full pathname */
	if (zoo_header.vdata & VFL_ON)
		add_version (matchname, &direntry);		/* add version suffix */

/* if extraction to subtree rooted at curr dir, modify pathname */
#if 0
#ifdef DIR_LBRACK
   if (todot && direntry.dirname[0] == *DIR_LBRACK &&
                direntry.dirname[1] != *CUR_DIR)      {
      char tmpstr[PATHSIZE];
      strcpy (tmpstr, DIR_LBRACK);
      strcat (tmpstr, CUR_DIR);
      strcat (tmpstr, &direntry.dirname[1]);
      strcpy (direntry.dirname, tmpstr);
   }
#endif
#endif

   /* hard-coded '/' should be eventually removed */
   if (todot && *direntry.dirname == '/') { 
      char tmpstr[PATHSIZE];
      strcpy(tmpstr, direntry.dirname);
      strcpy(direntry.dirname,CUR_DIR);
      strcat(direntry.dirname, tmpstr);
   }

   /* matchname now holds the full pathname for pattern matching */

   if ( ( (needdel && direntry.deleted) ||
            (needdel < 2 && !direntry.deleted)
        ) && needed(matchname, &direntry, &zoo_header)) {
      matched++;           /* update count of files extracted */

      if (direntry.major_ver > MAJOR_LZH_VER ||
         (direntry.major_ver == MAJOR_LZH_VER && 
            direntry.minor_ver > MINOR_LZH_VER)) {
            prterror ('e', extract_ver, direntry.major_ver, 
                           direntry.minor_ver, whichname);
				exit_status = 1;
            goto loop_again;
      }

      /* 
      If extracting to null device, or if user requested extraction
      of entire path, include any directory name in filename.
      If extraction to current directory requested, and if extfname
      begins with path separator, fix it */

      strcpy (extfname, whichname);
      if ((usepath || null_device) && direntry.dirlen != 0) {
         combine(extfname, direntry.dirname, whichname);
         if (usepath > 1 && !null_device)
            makepath(direntry.dirname);         /* make dir prefix */
      }

		strcpy(prtfname, extfname);
		if (zoo_header.vdata & VFL_ON)
			add_version (prtfname, &direntry);

      if (tofile) {
         int present = 0;

#ifndef PORTABLE
         /* 
         if Z format (fast) extraction, extension is created as
         follows:  for no current extension, new extension is "zzz";
         for current extension "a", new extension is "azz";  for 
         current extension "ab", new extension is "azb";  and for
         current extension "abc", new extension is "azc".
         */
           
         if (fast_ext) {
            int length;
            struct path_st path_st;
            parse (&path_st, extfname);         /* split filename */
            strcpy (extfname, path_st.fname);   /* just root filename */
            length = strlen (path_st.ext);
            strcat (extfname, ".");
            if (length == 0)
               strcat (extfname, "zzz");        /* no ext -> .zzz */
            else if (length == 1) {
               strcat (extfname, path_st.ext);
               strcat (extfname, "zz");         /* *.?    -> *.?zz */
            } else { /* length is 2 or 3 */
               if (length == 2)                 /* allow .aa, .ab, etc. */
                  path_st.ext[2] = path_st.ext[1];
               path_st.ext[1] = 'z';
               strcat (extfname, path_st.ext);  /* *.??   -> *.?z? */
            }
				strcpy(prtfname, direntry.fname);
				add_version (prtfname, &direntry);
         }
#endif   /* ifndef PORTABLE */

			/* don't extract if archived file is older than disk copy */
			if (!supersede && exists(extfname)) {
				unsigned int ddate, dtime;
#ifdef GETUTIME
				getutime (extfname, &ddate, &dtime);
#else
				ZOOFILE tfile;
				ddate = dtime = 0xffff;					/* assume maximum */
				tfile = zooopen(extfname, Z_READ);
				if (tfile == NOFILE)
					goto loop_again;
				gettime (tfile, &ddate, &dtime);
				zooclose (tfile);
#endif
				if (cmpnum (direntry.date, direntry.time, ddate, dtime) <= 0) {
					prterror ('m', "%-14s -- skipped\n", prtfname);
					goto loop_again;
				}
			}

         if (overwrite) {
            this_file = zoocreate (extfname);
#ifdef FATTR
				/* if can't open file, and OO option, make it writable first */
				if (this_file == NOFILE && overwrite >= 4 && 
						(direntry.fattr >> 22) == 1 && exists(extfname)) {
					setfattr (extfname, (unsigned long) (1L << 7) | direntry.fattr);
					this_file = zoocreate (extfname);
				}
#endif /* FATTR */
         } else {
            if (exists (extfname)) {
               present = 1;
               this_file = NOFILE;
            } else
               this_file = zoocreate (extfname);
         }
			error_message = 1;
         if (this_file == NOFILE) {
            if (present == 1) {      /* if file exists already */
					char ans[20];          /* answer to "Overwrite?" */
               do {
#ifdef EXT_ANYWAY
                  printf ("%s exists; extract anyway? [Yes/No/All] ",
                           extfname);
#else
                  printf ("Overwrite %s (Yes/No/All)? ", extfname);
#endif
                  fflush (stdin);
                  fgets (ans, sizeof(ans), stdin);
                  str_lwr (ans);
               } while (*ans != 'y' && *ans != 'n' && *ans != 'a');
   
               if (*ans == 'a')
                  overwrite++;
               if (*ans == 'y' || *ans == 'a') {
                  this_file = zoocreate(extfname);
                  error_message = 1; /* give error message if open fails */
               } else {
                  error_message = 0; /* user said 'n', so no error message */
               }
            } else {
               error_message = 1;   /* Real error -- give error message */
            }
         } /* end if */
      } /* end if */

      if (this_file == NOFILE) {         /* file couldn't be opened */
         if (error_message == 1) {
            prterror ('e', "Can't open %s for output.\n", extfname);
				exit_status = 1;

#ifndef PORTABLE
            /* if error was due to full disk, abort */
            if (space(0, &alloc_size) < alloc_size)
               prterror ('f', disk_full);
#endif

         }
      } else if (zooseek (zoo_file, (direntry.offset + dat_ofs), 0) == -1L) {
         prterror ('e', "Could not seek to file data.\n");
			exit_status = 1;
         close_file (this_file);
      } else {
#ifndef PORTABLE
         /* check msdos's free disk space if we seem to be running low 
            (within 1 cluster of being full) */
         if (tofile && disk_space < direntry.org_size + alloc_size) {
            disk_space = space (0, &alloc_size);
            if (disk_space < alloc_size) {
               close_file (this_file);
               unlink (extfname);
               prterror ('f', disk_full);
            }              
         }
#endif
         if (tofile && disk_space < direntry.org_size) {
#ifdef PORTABLE
            ;
#else
				prterror ('e', no_space, prtfname);
            unlink (extfname);               /* delete any created file */
#endif   /* portable */

         } else { 

#ifndef PORTABLE
            if (fast_ext) {            /* fast ext -> create header */
               void make_tnh PARMS((struct tiny_header *, struct direntry *));
               struct tiny_header tiny_header;
               make_tnh(&tiny_header, &direntry);
               zoowrite (this_file, (char *) &tiny_header, sizeof(tiny_header));

               if (direntry.cmt_size != 0) { /* copy comment */
                  long save_pos;
                  save_pos = zootell (zoo_file);
                  zooseek (zoo_file, direntry.comment, 0);
                  getfile (zoo_file, this_file, 
                          (long) direntry.cmt_size, 0);
                  zooseek (zoo_file, save_pos, 0);
               }
            }
#endif /* ifndef PORTABLE */

            crccode = 0;      /* Initialize CRC before extraction */
               if (!pipe) {
#ifdef PORTABLE
                  prterror ('m', "%-14s -- ", prtfname);
#else
                  if (fast_ext)
                     prterror ('m', "%-12s ==> %-12s -- ", 
                        prtfname,  extfname);
                  else
                     prterror ('m', "%-12s -- ", prtfname);
#endif /* PORTABLE */

               } else {            /* must be pipe */
                  prterror ('M',"\n\n********\n%s\n********\n",prtfname);

#ifdef SETMODE
                  MODE_BIN(this_file);           /* make std output binary so
                                                   ^Z won't cause error */
#endif
               }
#ifndef NOSIGNAL
            if (tofile)
               {
                  oldsignal = signal (SIGINT, SIG_IGN);
                  if (oldsignal != SIG_IGN) 
                     signal (SIGINT, ctrl_c); /* Trap ^C & erase partial file */
               }
#endif /* not NOSIGNAL */

            if (direntry.packing_method == 0)
               /* 4th param 1 means CRC update */
               status = getfile (zoo_file, this_file, direntry.size_now, 1);

#ifndef PORTABLE
            else if (fast_ext)
               /* 4th param 0 means no CRC update */
               status = getfile (zoo_file, this_file, direntry.size_now, 0);
#endif

            else if (direntry.packing_method == 1) {
#ifdef UNBUF_IO
#include "ERROR"
					/* NOT PORTABLE -- DO NOT TRY THIS AT HOME */
					long lseek PARMS ((int, long, int));
					long tell PARMS ((int));
					int this_fd, zoo_fd;
			
					/* get file descriptors */
					this_fd = null_device ? -2 : fileno (this_file);
					zoo_fd = fileno (zoo_file);

					zooseek (zoo_file, zootell (zoo_file), 0);	/* synch */
					lseek (zoo_fd, zootell (zoo_file), 0);			/* ..again */
					if (!null_device) {
						zooseek (this_file, zootell (this_file), 0);	/* synch */
						lseek (this_fd, zootell (this_file), 0);		/* ..again */
					}
			      status = lzd(zoo_fd, this_fd);			/* uncompress */
					zooseek (zoo_file, tell (zoo_fd), 0);	/* resynch	*/
					if (!null_device)
						zooseek (this_file, tell (this_fd), 0);/* resynch	*/
#else
               status = lzd (zoo_file, this_file); 	/* uncompress */
#endif
				} else if (direntry.packing_method == 2) {
               status = lzh_decode (zoo_file, this_file);
            } else {
               prterror ('e', "File %s:  impossible packing method.\n",
                  whichname);
                  unlink(extfname);
                  goto loop_again;
            }


#ifndef NOSIGNAL
            if (tofile)
               signal (SIGINT, oldsignal);
#endif /* not NOSIGNAL */

#ifdef SETMODE
            if (pipe)
               MODE_TEXT(this_file);          /* restore text mode */
#endif
   
            if (tofile) {
               /* set date/time of file being extracted */
#ifdef GETTZ
					void tzadj();
					/* adjust for original timezone */
					tzadj (&direntry);
#endif
#ifdef NIXTIME
               close_file (this_file);
               setutime (extfname, direntry.date, direntry.time);
#else
               settime (this_file, direntry.date, direntry.time);
               close_file (this_file);
#endif
#ifdef FATTR
/* Restore file attributes. Bit 23==1 means system-specific; we currently 
don't recognize this.  Bit 23==0 means use portable format, in which case 
bit 22==0 means ignore attributes.  Thus attributes are ignored if both 
bits 23 and 22 are zero, which is the effect of a zero-filled file 
attribute field.  Currently we restore file attributes if and only if
bit 23==0 and bit 22==1. */

					if ((direntry.fattr >> 22) == 1) {
						setfattr (extfname, direntry.fattr);
					}
#endif /* FATTR */
            } /* end of if (tofile) ... */
            if (status != 0) {
					exit_status = 1;
               if (tofile)
                  unlink (extfname);
               if (status == 2) {	/* was 1 (wrong) */
                  memerr(0);
               /* To avoid spurious errors due to ^Z being sent to screen,
                  we don't check for I/O error if output was piped */
               } else if (!pipe && (status == 2 || status == 3)) {
                     prterror ('e', no_space, prtfname);
               }
            } else {
               /* file extracted, so update disk space.  */
               /* we subtract the original size of the file, rounded
                  UP to the nearest multiple of the disk allocation
                  size. */
#ifndef PORTABLE
               {
                  unsigned long temp;
                  temp = (direntry.org_size + alloc_size) / alloc_size;
                  disk_space -= temp * alloc_size;
               }
#endif

               if (
#ifndef PORTABLE
					      !fast_ext && 
#endif
							direntry.file_crc != crccode
						) {
                  badcrc_count++;
						exit_status = 1;
                  if (!pipe) {
                     if (!null_device)
                        prterror ('M', "extracted   ");
                     prterror ('w', bad_crc, prtfname);
                  }
                  else {   /* duplicate to standard error */
                     static char stars[] = "\n******\n";
                     putstr (stars);
                     prterror ('w', bad_crc, prtfname);
                     putstr (stars);
                     fprintf (stderr, "WARNING:  ");
                     fprintf (stderr, bad_crc, prtfname);
                  }
               } else
                  if (!pipe)
                     prterror ('M', null_device ? "OK\n" : "extracted\n");

            } /* end if */
         } /* end if */
      } /* end if */
   } /* end if */

loop_again:
   zooseek (zoo_file, next_ptr, 0); /* ..seek to next dir entry */
} /* end while */

close_file (zoo_file);
if (!matched)
   putstr (no_match);

if (badcrc_count) {
   prterror ('w', "%d File(s) with bad CRC.\n", badcrc_count);
} else if (null_device)
   prterror ('m', "Archive seems OK.\n");

zooexit (exit_status);

} /* end zooext */

/* close_file() */
/* closes a file if and only if we aren't sending output to 
   a pipe or to the null device */

void close_file (file)
ZOOFILE file;
{
   if (tofile)
      zooclose (file);
}

/* Ctrl_c() is called if ^C is hit while a file is being extracted.
   It closes the files, deletes it, and exits. */
T_SIGNAL ctrl_c()
{
#ifndef NOSIGNAL
   signal (SIGINT, SIG_IGN);     /* ignore any more */
#endif
   zooclose (this_file);
   unlink (extfname);
   zooexit (1);
}

#ifndef PORTABLE
/* make_tnh copies creates a tiny_header */
void make_tnh (tiny_header, direntry)
struct tiny_header *tiny_header;
struct direntry *direntry;
{
   tiny_header->tinytag = TINYTAG;
   tiny_header->type = 1;
   tiny_header->packing_method = direntry->packing_method;
   tiny_header->date = direntry->date;
   tiny_header->time = direntry->time;
   tiny_header->file_crc = direntry->file_crc;
   tiny_header->org_size = direntry->org_size;
   tiny_header->size_now = direntry->size_now;
   tiny_header->major_ver = direntry->major_ver;
   tiny_header->minor_ver = direntry->minor_ver;
   tiny_header->cmt_size = direntry->cmt_size;
   strcpy (tiny_header->fname, direntry->fname);
} 
#endif /* ifndef PORTABLE */
