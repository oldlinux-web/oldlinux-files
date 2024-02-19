#ifndef LINT
/* derived from: zooadd.c 2.34 88/08/15 10:53:11 */
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/zooadd.c,v $\n\
$Id: zooadd.c,v 1.10 91/07/08 23:48:39 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
(C) Copyright 1991 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
/* Adds files specified in parameter-list to archive zoo_path. */

#define LONGEST	20					/* assumed length of longest filename */
#include "zoomem.h"             /* to define MAXADD */
#include "zoo.h"
#include "zooio.h"
#include "various.h"
#include "parse.h"
#include "debug.h"

#include "portable.h"

#include "zoofns.h"
#include "errors.i"
extern int break_hit;
extern int quiet;

void show_comment PARMS ((struct direntry *, ZOOFILE, int, char *));
void dosname PARMS ((char *, char *));
void modpath PARMS ((char *));
void opts_add PARMS ((char *, int *, int *, int *, int *, int *, int *,
               int *, int *, int *, int *, int *, int *, int *, int *));
int ver_too_high PARMS ((struct zoo_header *));
void get_comment PARMS ((struct direntry *, ZOOFILE, char *));
void copyfields PARMS ((struct direntry *, struct tiny_header *));
void storefname PARMS ((struct direntry *, char *, int));
char *choosefname PARMS ((struct direntry *));

extern struct zoo_header zoo_header;

extern char file_leader[];
extern unsigned int crccode;

void zooadd(zoo_path, argc, argv, option)
char *zoo_path;      /* pathname of zoo archive to add to */
int argc;            /* how many filespecs supplied */
char **argv;         /* array of pointers to filespecs */
char *option;        /* option string */
{
char *whichname;                          /* which name to show user */
char **flist;                      			/* list of ptrs to input fnames */
int fptr;                                 /* will point to within flist */
ZOOFILE this_file;                        /* file to add */
char zoo_fname[LFNAMESIZE];               /* basename of archive itself */
char zoo_bak[LFNAMESIZE];                 /* name of archive's backup */
char this_fname[LFNAMESIZE];              /* just filename of file to add */
char latest_name[LFNAMESIZE];             /* latest name in archive */
long last_old = 0L;                       /* last direntry in old chain */
ZOOFILE zoo_file;                         /* stream for open archive */
char *this_path;                          /* pathname of file to add */

#ifdef NOENUM
#define NEW_ZOO 1
#define OLD_ZOO 2
int zoo_status;
#else
enum {NEW_ZOO, OLD_ZOO} zoo_status;       /* newly created or not */
#endif

long this_dir_offset;                     /* pointers to within archive */
long save_position;                       /* pointer to within archive */
long prev_pos;                            /* posn of prev file of same name */
struct direntry direntry;                 /* directory entry */
struct direntry dir2entry;                /* spare */
int status;                               /* error status */
int success;                              /* successful addition of file? */
int addcount = 0;                         /* number added */
int update=0;                             /* only files already in archive */
int suppress=0;                           /* suppress compression */
int new=0;                                /* add only files not in archive */
int zootime = 0;                          /* just set archive time */
int add_comment = 0;                      /* add comment */
int add_global_comment = 0;					/* archive comment */
int pack = 0;                             /* pack after adding */
int need_dir = 1;                         /* store directories too */
int delcount = 0;                         /* count of deleted entries */
int exit_status = 0;                      /* exit status to set */

unsigned int latest_date = 0;             /* to set time on archive itself */
unsigned int latest_time = 0;             /* .. same */
int move = 0;                             /* delete after adding to archive */
int longest;                              /* length of longest pathname added */
int firstfile = 1;                        /* first file being added? */
int z_fmt = 0;                            /* look for Z format files? */
int inargs = 0;									/* read filenames from stdin? */

#ifndef PORTABLE
struct tiny_header tiny_header;           /* for Z format archives */
#endif

unsigned this_version_no;							/* version no. of old file */
unsigned  high_vflag;								/* version flag of old file */
unsigned high_version_no;							/* highest version no of this file */
long high_pos;										/* offset of file w/highest ver no */
unsigned int fgens;								/* gens. to preserve -- file */
unsigned int zgens;								/* gens. to preserve -- archive */
long oldcmtpos;									/* to save old comment */
unsigned int oldcmtsiz;							/* to save old comment */
int genson = 0;									/* whether to turn generations on */

int use_lzh = 0;									/* whether to use lzh compression */

/* on entry option points to first letter */

opts_add (option, &zootime, &quiet, &suppress, &move, &new, &pack,
          &update, &add_comment, &z_fmt, &need_dir, &inargs, &genson,
			 &use_lzh, &add_global_comment);

/* POSSIBLE RACE CONDITION BETWEEN TESTING EXISTENCE AND CREATING FILE */
if (exists (zoo_path)) {
	zoo_file = zooopen (zoo_path, Z_RDWR);
   zoo_status = OLD_ZOO;
} else {
   if (!zootime)
		zoo_file = zoocreate (zoo_path);
   else
      zoo_file = NOFILE;     /* don't create if just setting time */
   zoo_status = NEW_ZOO;
}

if (zoo_file == NOFILE)
   prterror ('f', could_not_open, zoo_path);
basename(zoo_path, zoo_fname);      /* get basename of archive */
rootname (zoo_path, zoo_bak);       /* name without extension */
strcat (zoo_bak, BACKUP_EXT);       /* name of backup of this archive */

/* Now we prepare the archive for adding one or more files.  If the archive
has just been created, we write the archive header */

addfname ("",0L,0,0,0,0); /* initialize table of files already in archive */
if (zoo_status == NEW_ZOO) {                 /* newly-created archive */
	if (genson)											/* if no generations needed */
		zoo_header.vdata = (VFL_ON|GEN_DEFAULT); /* generations on */
   fwr_zooh (&zoo_header, zoo_file);
	zgens = GEN_DEFAULT;
   zooseek (zoo_file, zoo_header.zoo_start, 0); /* seek to where data begins */
} else {
   /* read header and rewrite with updated version numbers, preserving
	   header type */
   rwheader (&zoo_header, zoo_file, 1);
	zgens = zoo_header.vdata & VFL_GEN;			/* get archive generations */
   /* initialize latest_name to null string */
	/* NOTE:  latest_name is not currently used for anything, but
		may be used in the future for inserting files into the
		archive in alphabetic order. */
   *latest_name = '\0';

   /* Skip existing files but add them to a list.  The variable last_old 
   gets the tail of the old chain of directory entries */
   skip_files (zoo_file, &latest_date, &latest_time, &delcount, 
               latest_name, &last_old);
}
/* The file pointer is now positioned correctly to add a file to archive, 
unless the null directory entry is too short.  This will be fixed below. */

/* If we are just setting time, do it and run. */
if (zootime) {
#ifdef NIXTIME
   zooclose (zoo_file);
   setutime (zoo_path, latest_date, latest_time);
#else
   settime (zoo_file, latest_date, latest_time);
   zooclose (zoo_file);
#endif
   prterror ('m', "Archive time adjusted.\n");
   zooexit (0);
}

/* make list of files, excluding archive and its backup */
longest = LONGEST;
flist = (char **) ealloc(MAXADD);
if (!inargs) {
   makelist(argc, argv, flist, MAXADD-2, zoo_fname, zoo_bak, ".", &longest);
   /*                                    ^^         ^^       ^^ exclude */
}

fptr = 0;	/* ready to get filename (if makelist() was called) or to
					begin adding filenames (if reading them from stdin) */

while (1) {
   unsigned int this_date, this_time;
   int INLIST; /* boolean */
   int RECENT; /* boolean */
   int danger; /* if update requested and disk copy is out of date */
	if (inargs) {
	again: /* loop back if filename was same as archive name or its backup */
		this_path = getstdin();			/* pathname from stdin, in static area */
		if (this_path != NULL) {
			if (samefile (nameptr(zoo_fname),nameptr(this_path)) ||
						samefile (nameptr(zoo_bak),nameptr(this_path)))
				goto again; 				/* don't add archive to itself */
			modpath (this_path);
		/* if moving files, add to list for later deletion;  if list overflows,
			terminate addition loop and give warning message */
			if (move) {
				if (fptr >= MAXADD-2) {
					prterror ('w', too_many_files, MAXADD-2);
					this_path = NULL;
				} else
					flist[fptr++] = str_dup (this_path);
			}
		}
	} else  {
		this_path = flist[fptr++];
	}
	/* exit the addition loop when no more pathnames are left */
	if (this_path == NULL) {
		/* in case stdin was being read, make sure flist is NULL-terminated */
		flist[fptr] = NULL;
		break;
	}

   basename (this_path, this_fname);   /* get just filename for later */

   this_file = zooopen(this_path, Z_READ);
   if (this_file == NOFILE) {
      prterror ('e', could_not_open, this_path);
      exit_status++;
      continue;
   }

#ifndef PORTABLE
   /* Test to see if this is a Z format file.  We assume the file is Z format
      if (a) tag is correct and (b) type is 1 and (c) embedded filename
      is not longer than FNAMESIZE.  
   */
   if (z_fmt) {
      zooread (this_file, (char *) &tiny_header, sizeof(tiny_header));
      if (tiny_header.tinytag == TINYTAG && tiny_header.type == 1 &&
                        strlen (tiny_header.fname) <= FNAMESIZE)
          /* ok */ ;
      else {
         zooclose (this_file);
         prterror ('e', "File %s does not have Z format.\n", this_fname);
         exit_status++;
         continue;
      }
   }
#endif

   /* get file time;  also fix name */
#ifndef PORTABLE
   if (z_fmt) {
      direntry.date = tiny_header.date;
      direntry.time = tiny_header.time;
      strcpy (direntry.fname, tiny_header.fname);
      direntry.dirlen = direntry.namlen = 0;
   } else {
#endif

      /* Get timstamp of file being added */
#ifdef GETUTIME
      getutime (this_path, &direntry.date, &direntry.time);
#else
      gettime (this_file, &direntry.date, &direntry.time);
#endif

		/* save file attributes */
#ifdef FATTR
		/* we expect getfattr() to set all attr. bits;  currently
			only the portable format is recognized */
		{
# ifdef FATTR_FNAME
			unsigned long getfattr PARMS ((char *);
			direntry.fattr = getfattr (this_path);
# else
			unsigned long getfattr PARMS ((ZOOFILE));
			direntry.fattr = getfattr (this_file);
# endif /* FATTR_FNAME */
		}
#else
			direntry.fattr = NO_FATTR;	/* none */
#endif /* FATTR */

#ifdef FOLD
      str_lwr(this_fname);
#endif
      dosname (this_fname, direntry.fname);  /* MSDOS filename */

   /*
   Store long filename into direntry.lfname iff it is different from MSDOS
   filename.  Also store directory name if need_dir is true.  Moved out of 
   zooadd() so zooadd() doesn't get too big for optimization.
   */
   storefname (&direntry, this_path, need_dir);

#ifndef PORTABLE
   }
#endif

#ifdef DEBUG
printf ("zooadd:  direntry.lfname = [%s]  direntry.dirname = [%s]\n",
                  direntry.lfname, direntry.dirname);
#endif

   /* if update option, then we add file if it is already in the archive 
      AND the archived file is older */

   /* The following logic was derived from a Karnaugh map so it may
      be hard to understand.  Essentially, if U=update requested,
      N=new files requested, I=file is already in archive, and
      R=file being archived is more recent than file already in
      archive, then the boolean equation is:

      add = U' (N' + I') + U (IR  + I'N)
   */

   /* Get the filename to use for this addition.  */
   whichname = choosefname(&direntry);

   /* Get position in archive of any old file of same name, ignoring
		any directory prefix if need_dir is not true.  Also get its
		date, time, version flag, and version number. */
   prev_pos = inlist (fullpath (&direntry), &this_date, &this_time, 
				&this_version_no, &high_vflag, &high_version_no, 
				&high_pos, !need_dir);

/* define DBG_INLIST for debugging by printing values returned by inlist() */
#ifdef DBG_INLIST
	printf ("FROM inlist(): prev_pos=%ld, high_pos=%ld\n", prev_pos, high_pos);
	printf ("this_version_no=%u, high_vflag=%4x, high_version_no=%u\n",
				this_version_no,    high_vflag,     high_version_no);
#endif

   INLIST = prev_pos > 0;  /* already in archive if positive value */
   if (INLIST) {
      int result;
      result = cmpnum (direntry.date, direntry.time, this_date, this_time);
      RECENT = result > 0;
      danger = result < 0;
   } else
      danger = 0; /* And RECENT is undefined and should not be used */

   if (
         !update && (!new || !INLIST) ||
         update && (INLIST && RECENT || !INLIST && new)
      )
         ;  /* then continue and add file */
   else {
      if (update && danger)
         prterror ('w', "Archived copy of %s is newer.\n", whichname);
      zooclose (this_file);
      continue;   /* cycle back, skip this file */
   }

#ifdef CHEKDIR
   /* Don't add if this is a directory */
   if (isadir (this_file)) {
      zooclose (this_file);
      continue;
   }
#else
# ifdef CHEKUDIR
   /* Don't add if this is a directory */
   if (isuadir (this_path)) {
      zooclose (this_file);
      continue;
   }
# endif /* CHEKUDIR */
#endif /* CHEKDIR */

   /* Create directory entry for new file (but don't add just yet) */
   /* NOTE:  we already got file date and time above for update option */
	/* add tag, type, timezone, struc, system_id, and var_dir_len */
	newdir (&direntry);

	if (!genson && zoo_status == NEW_ZOO || 
				(zoo_header.vdata & VFL_ON) == 0) {
		direntry.vflag = 0;
		direntry.version_no = 0;
	}

   /* 
   Write a null direntry entry.  Thus, if an error occurs or the program
   is interrupted, the end of the archive will still be meaningful.
   Special check needed for first one written.
   */

   direntry.next = direntry.offset = 0L;     /* trailing null entry */
   this_dir_offset = zootell (zoo_file);
   if (!firstfile) {
      writedir (&direntry, zoo_file);
   } else {
      /*
      Before adding the first file to the archive, we must make sure that
      the previous directory chain (if any) is properly terminated with a
      null entry of the right size.  If this is a new archive, we simply
      write a new null entry of the right size.  If this is an existing
      archive, we must check the size of the previous trailing null entry. 
      If it is too small, we will back up to the most recent real directory
      entry and change its .next field to point to end of file.  
      */

      if (zoo_status == NEW_ZOO) {
         writedir (&direntry, zoo_file);        /* write null dir entry */
      } else {
         struct direntry tmpentry;
         long tmppos;
         int oldlen, newlen;
         tmppos = zootell (zoo_file);
         frd_dir (&tmpentry, zoo_file);
#define  DIRLEN(x)   ((x.type<2) ? SIZ_DIR : (SIZ_DIRL+x.var_dir_len))
         oldlen = DIRLEN(tmpentry);             /* get length of direntry */
         newlen = DIRLEN(direntry);             /* ditto */

         if (newlen > oldlen) {                 /* trouble */
            zooseek (zoo_file, last_old, 0);    /* back to previous entry */
            frd_dir (&tmpentry, zoo_file);
            zooseek (zoo_file, 0L, 2);          /* get EOF position */
            tmpentry.next = zootell (zoo_file);    /* point to EOF */
            zooseek (zoo_file, last_old, 0);    /* back to previous entry */
            writedir (&tmpentry, zoo_file);     /* update it */
            zooseek (zoo_file, 0L, 2);          /* to EOF ... */
            this_dir_offset = zootell (zoo_file);
            writedir (&direntry, zoo_file);     /* ...write null dir entry */
         } else
            zooseek (zoo_file, tmppos, 0);      /* long enough -- let it be */
      } /* if (zoo_status == NEW_ZOO) ... */
   } /* if (!firstfile) ... */

   /* Now `this_dir_offset' is where the next directory entry will go */

   /* first file added goes at EOF to avoid overwriting comments */
   if (firstfile) {
      zooseek (zoo_file, 0L, 2);                   /* EOF */
      direntry.offset = zootell (zoo_file) + SIZ_FLDR;
   } else {
      direntry.offset = this_dir_offset + SIZ_DIRL + 
         direntry.var_dir_len + SIZ_FLDR;
   }

	if (use_lzh) {
		direntry.major_ver = MAJOR_LZH_VER;    /* minimum version number needed */
		direntry.minor_ver = MINOR_LZH_VER;    /* .. to extract */
	} else {
		direntry.major_ver = MAJOR_EXT_VER;    /* minimum version number needed */
		direntry.minor_ver = MINOR_EXT_VER;    /* .. to extract */
	}
   direntry.deleted = 0;               /* not deleted, naturally */
   direntry.comment = 0L;              /* no comment (yet) */
   direntry.cmt_size = 0;          /* .. so no size either */

   save_position = direntry.offset;          /* save position in case of error */

   (void) zooseek (zoo_file, direntry.offset - SIZ_FLDR, 0);
   (void) zoowrite (zoo_file, file_leader, SIZ_FLDR);

#ifdef PORTABLE
   prterror ('m', "%-*s -- ", longest, this_path);
#else
   if (z_fmt)
      prterror ('m', "%-12s <== %-*s -- ", 
         direntry.fname, longest, this_path);
   else
      prterror ('m', "%-*s -- ", longest, this_path);

#endif /* PORTABLE */

   crccode = 0;
#ifndef PORTABLE
   if (z_fmt) 
	{
      direntry.packing_method = tiny_header.packing_method;
      zooseek (this_file, (long) (sizeof(tiny_header)+tiny_header.cmt_size), 0);
      status = getfile (this_file, zoo_file, tiny_header.size_now, 1);
   } else 
#endif
	if (suppress) {                    /* suppress compression */
      direntry.packing_method = 0;           /* no compression */
      status = getfile (this_file, zoo_file, -1L, 1);
   } else {
#ifdef UNBUF_IO	/* unbuffered I/O */
		long lseek PARMS ((int, long, int));
		long tell PARMS ((int));
		int this_fd, zoo_fd;
#endif
		if (use_lzh)
			direntry.packing_method = 2;
		else
			direntry.packing_method = 1;
#ifdef UNBUF_IO
#include "UNBUF_IO not currently supported"
		this_fd = fileno (this_file);						/* get ..					*/
		zoo_fd = fileno (zoo_file);						/* ... file descriptors	*/
		zooseek (zoo_file, zootell (zoo_file), 0);	/* synch */
		zooseek (this_file, zootell (this_file), 0);	/* synch */
      status = lzc(this_fd, zoo_fd);					/* add with compression */
		zooseek (zoo_file, tell (zoo_fd), 0);			/* resynch	*/
		zooseek (this_file, tell (this_fd), 0);		/* resynch	*/
#else
		if (use_lzh)
			status = lzh_encode(this_file, zoo_file);
		else
			status = lzc(this_file, zoo_file);
#endif /* UNBUF_IO */

   }
   if (status != 0) { /* if I */
      ++exit_status;                         /* remember error */
      if (status == 1)
         prterror ('F', no_memory);
      else if (status == 2)
         prterror ('F', disk_full);
      else if (status == 3)
         prterror ('F', "Read error.\n");
      else
         prterror ('F', internal_error);
      success = 0;
   } else {
      direntry.next  = zootell (zoo_file);
      direntry.size_now = direntry.next - direntry.offset;

      /* find and store original size of file just compressed */
/*DEBUG VMS*/ zooseek (this_file, 0L, 2);	/* seek to EOF */

      direntry.org_size = zootell (this_file);  /* should be EOF already */

      /* If the compressed one is bigger, just copy */

      if (direntry.size_now >= direntry.org_size &&   /* if II */
            direntry.packing_method != 0) {
         zooseek (zoo_file, save_position, 0);  /* ..restore file pointer */
         zootrunc (zoo_file);                   /* ..truncate file */
         direntry.packing_method = 0;           /* ..and just copy */
         zooseek (this_file, 0L, 0);            /* (but rewind first!) */
         crccode = 0;                           /* re-start crc from 0 */
         status = getfile (this_file, zoo_file, -1L, 1);
         if (status != 0) {  /* if III */
            success = 0;
            printf (disk_full);
            exit_status++;
         } else {
            success = 1;
            direntry.next  = zootell (zoo_file);
            direntry.size_now = direntry.next - direntry.offset;
         } /* end if III */
      } else {
         success = 1;
      } /* end if II */

   } /* end if I */

   if (success) {                               /* file successfully added */
      addcount++;                               /* how many added */
      direntry.file_crc = crccode;

      /* remember most recent date and time */
      if (cmpnum (direntry.date,direntry.time,latest_date,latest_time) > 0) {
            latest_date = direntry.date;
            latest_time = direntry.time;
      }

#if 0
      /* mark any previous version of this file in archive as deleted */
      dir2entry.comment = 0L;       /* for later use assigning to direntry */
      dir2entry.cmt_size = 0;
#endif

      if (!z_fmt)
         prterror ('M', " (%2d%%) ", cfactor (direntry.org_size, direntry.size_now));

		oldcmtsiz = 0;								/* assume no old comment */
		oldcmtpos = 0L;

      if (prev_pos > 0) {										/* in archive */
			int delold = 0;										/* delete old? */
			/* if versions active both archive-wide and for file */
			if ((zoo_header.vdata & VFL_ON) && (high_vflag & VFL_ON)) {
				/* next test is optimization, to avoid redundant I/O */
				if (high_pos != prev_pos || this_version_no == 1) {
					/* prev highest is no longer highest so adjust vflag */
					long save_pos = zootell (zoo_file);			/*DEBUG*/
					zooseek (zoo_file, high_pos, 0);
					readdir (&dir2entry, zoo_file, 1);
					oldcmtpos = dir2entry.comment;
					oldcmtsiz = dir2entry.cmt_size;
					dir2entry.vflag &= (~VFL_LAST);				/* no longer highest */
					zooseek (zoo_file, high_pos, 0);
					writedir (&dir2entry, zoo_file);
					zooseek (zoo_file, save_pos, 0);				/*DEBUG*/
				}

				direntry.version_no = high_version_no + 1; /* ..one higher */
				direntry.vflag = high_vflag;
				/* now see if we need to delete older version */
				fgens = high_vflag & VFL_GEN;
				if (fgens == 0)
					fgens = zgens;
				if (zgens != 0 && zgens < fgens)
					fgens = zgens;
				if (fgens != 0 && direntry.version_no - this_version_no >= fgens) {
					delold = 1;
					prterror ('M', "replaced+\n");
				} else
					prterror ('M', "added+\n");
			} else {
				prterror ('M', "replaced\n");
				delold = 1;
			}

			if (delold) {											/* deleting old file */
				long save_pos = zootell (zoo_file);			/*DEBUG*/
				++delcount;											/* remember to pack */
				zooseek (zoo_file, prev_pos, 0);
				readdir (&dir2entry, zoo_file, 1);
				if (dir2entry.cmt_size != 0) {		/* propagate latest comment */
					oldcmtpos = dir2entry.comment;
					oldcmtsiz = dir2entry.cmt_size;
				}
				dir2entry.deleted = 1;							/* mark as deleted */
				/* following line is optimization if only 1 generation */
				dir2entry.vflag &= (~VFL_LAST);				/* no longer highest */
				zooseek (zoo_file, prev_pos, 0);
				writedir (&dir2entry, zoo_file);
				zooseek (zoo_file, save_pos, 0);				/*DEBUG*/
			}
      } else 														/* not in archive */
			prterror ('M', "added\n");

      /* Preserve any old comment if we replaced or superseded the file */
      direntry.comment = oldcmtpos;
      direntry.cmt_size = oldcmtsiz;

#ifndef PORTABLE
      /* Copy comment if any from Z format file */
      if (z_fmt && tiny_header.cmt_size != 0) {
         zooseek (this_file, (long) sizeof(tiny_header), 0); /* to comment */
         direntry.comment = zootell (zoo_file);
         direntry.cmt_size = tiny_header.cmt_size;
         /* 4th param is 0 for no CRC */
         getfile (this_file, zoo_file, (long) tiny_header.cmt_size, 0);
         direntry.next = zootell (zoo_file);
      } 
#endif

      /* if user requested comments, any previous comment in a Z format
         file may now be manually overwritten */
      if (add_comment && !feof (stdin)) {
         show_comment (&direntry, zoo_file, 1, whichname);
         get_comment (&direntry, zoo_file, this_path);
         direntry.next = zootell (zoo_file);    /* update .next ptr */
      } /* end if */

#ifndef PORTABLE
      /* if adding Z format archive, copy relevant fields from its header */
      if (z_fmt) {   /* moved out to shorten code & allow optimizer to work */
         copyfields (&direntry, &tiny_header);
      }
#endif

      debug((printf ("zooadd:  our new .next = [%lx].\n", direntry.next)))

      {
         long savepos = zootell (zoo_file);    /* save position */
         zooseek (zoo_file, this_dir_offset, 0);
         writedir (&direntry, zoo_file);
         zooseek (zoo_file, savepos, 0);    /* restore position */
      }

   } else {                               /* file was not properly added */
      zooseek (zoo_file, save_position, 0);  /* ..restore file pointer */
      zootrunc (zoo_file);                   /* ..truncate file */
   } /* end if */
   zooclose (this_file);
if (!success)
   break;
firstfile = 0;
} /* end for */

save_position = zootell (zoo_file);

/* Write a null direntry entry */
zooseek (zoo_file, save_position, 0);
writenull (zoo_file, MAXDIRSIZE);
zootrunc (zoo_file);  /* truncate */

#ifdef NIXTIME
zooclose (zoo_file);
setutime (zoo_path, latest_date, latest_time);
#else
settime (zoo_file, latest_date, latest_time);
zooclose (zoo_file);
#endif

if (!addcount) {                    /* no files added */
   prterror ('m', "No files added.\n");
   if (zoo_status == NEW_ZOO)
      unlink (zoo_path);
} else {
   if (delcount && pack) { /* pack if user asked and found deleted entries */
      prterror ('M', "-----\nPacking...");
      zoopack (zoo_path, "PP");
      prterror ('M', "done\n");
   }

   /* If files to move & we added some and no error so far, delete originals */
   if (move && !exit_status)
      if (kill_files (flist, longest) != 0)
         exit_status++;
}

/* right here we handle archive comment */
if (add_global_comment) {
	comment(zoo_path, "_A");
	add_global_comment = 0;
}

if (exit_status)
   zooexit (1);
} /* end zoo_add */
