#ifndef LINT
/* derived from: @(#) zoopack.c 2.16 88/08/22 15:51:20 */
/*$Source: /usr/home/dhesi/zoo/RCS/zoopack.c,v $*/
/*$Id: zoopack.c,v 1.5 91/07/09 01:54:17 dhesi Exp $*/
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/zoopack.c,v $\n\
$Id: zoopack.c,v 1.5 91/07/09 01:54:17 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
/* Packs an archive.  The sequence is:
   1. Copy all files from current archive to new one.
   2. If the user didn't want a backup, delete the old archive
      else rename it to same name with extension of .BAK.
   3. Rename temporary archive to old name.
*/

/* define this to make packing noisless */
#define QUIETPACK 1


#include "portable.h"
#include "zooio.h"
#include "various.h"
#include "zoo.h"
#include "zoofns.h"
#include "errors.i"
#ifndef NOSIGNAL
#include <signal.h>
#endif

char *mktemp PARMS((char *));

struct zoo_header zoo_header = {
   TEXT,
   ZOO_TAG,
   (long) SIZ_ZOOH,
   (long) (-SIZ_ZOOH),
   MAJOR_VER,
   MINOR_VER,
	H_TYPE,
	0L,						/* comment position */
	0,							/* comment length */
	GEN_DEFAULT				/* generations */
};
char file_leader[] = FILE_LEADER;
extern int quiet;
int break_hit;

int ver_too_high PARMS((struct zoo_header *));

void zoopack(zoo_path, option)
char *zoo_path, *option;
{
char temp_file[PATHSIZE];
static char xes[]="XXXXXX";               /* template for temp file */

#ifndef NOSIGNAL
T_SIGNAL (*oldsignal)();
#endif
register ZOOFILE zoo_file;                /* archive */
ZOOFILE new_file;                         /* destination archive */
long next_ptr;                            /* pointer to within archive */
long new_dir_pos;                         /* ditto */
struct direntry direntry;                 /* directory entry */
struct zoo_header old_zoo_header;         /* just for reading old header */
int status;                               /* error status */
int nobackup = 0;                         /* keep backup */
int force = 0;                            /* force overwrite of old backup */
int extcount = 0;                         /* how many files moved */
char backup_name[PATHSIZE];               /* name of backup */
int bad_header = 0;                       /* if archive has bad header */
int latest_date = 0;                      /* latest date on any file moved */
int latest_time = 0;                      /*  ...likewise */
int curr_dir = 0;									/* create backup in curr dir */
static char partial_msg[] =
   "Partially packed archive left in %s.\n";

#ifdef FATTR
unsigned long zoofattr;							/* zoo archive protection */
int setfattr PARMS ((char *, unsigned long));
unsigned long getfattr							/* params below */
# ifdef FATTR_FNAME
  PARMS ((char *));
# else
  PARMS ((ZOOFILE));
# endif /* FATTR_FNAME */
#endif /* FATTR */

while (*option) {
   switch (*option) {
      case 'P': force++; break;
      case 'E': nobackup++; break;
      case 'q': quiet++; break;
		case '.': curr_dir++; break;
      default:
         prterror ('f', inv_option, *option);
   }
   option++;
}
if (force == 1)         /* force only if P was doubled */
   force--;

zoo_path = addext (zoo_path, EXT_DFLT);      /* add default extension */

/* Create a backup name by replacing any extension by backup extension. */
strcpy (backup_name, zoo_path);
{
   char *temp;
   if ((temp = strrchr (backup_name,EXT_CH)) != 0)  /* if dot found */
      strcpy (temp, BACKUP_EXT);                /* replace old extension */
   else
      strcat (backup_name, BACKUP_EXT);         /* else just append */
}

/*
Open original archive for read-write access.  Although we will only
read from it and never write to it, we want to avoid packing an
archive that is read-only, since presumably the user didn't want
to risk changing it in any way.
*/
zoo_file = zooopen(zoo_path, Z_RDWR);

if (zoo_file == NOFILE)
   prterror ('f', could_not_open, zoo_path);

/* If possible, save protection code of old archive for propagation to new */
#ifdef FATTR
# ifdef FATTR_FNAME
   zoofattr = getfattr (zoo_path);
# else
   zoofattr = getfattr (zoo_file);
# endif /* FATTR_FNAME */
#endif /* FATTR */

/* Read the header of the old archive. */
frd_zooh(&old_zoo_header, zoo_file);

if ((old_zoo_header.zoo_start + old_zoo_header.zoo_minus) != 0L) {
   prterror ('w', failed_consistency);
   ++bad_header;                    /* remember for future error message */
}

/* Refuse to pack it if its version number is higher than we can accept */
if (ver_too_high (&old_zoo_header))
   prterror ('f', wrong_version, old_zoo_header.major_ver,
                                    old_zoo_header.minor_ver);

/* Now see if the archive already exists with the backup extension.  If so,
   give an error message and abort.  However, we skip this test if the user
   specified overwriting the backup */

if (!force) {
   if (exists (backup_name))
      prterror ('f', "File %s already exists.  Delete it or use PP option.\n",
                      backup_name);
}

/*
Open the new archive by a temporary name.  If not otherwise specified,
we open the new archive in the same directory as the original.  But if
the curr_dir switch was given, we just put XXXXXX into temp_file.
*/
if (!curr_dir) {
	strcpy (temp_file, zoo_path);          /* original archive name */
	*nameptr (temp_file) = '\0';           /* ... minus original filename */
	strcat (temp_file, xes);               /* ... plus XXXXXX */
} else {
   strcpy (temp_file, xes);
}
mktemp (temp_file);                    /* ... and make unique */
new_file = zoocreate (temp_file);
if (new_file == NOFILE)
   prterror ('f', "Could not create temporary file %s.\n", temp_file);

/*
If old_zoo_header greater than type 0, we update zoo_header as follows:  
new archive comment will be just after archive header;  zoo_start will 
point to just beyond archive comment.  But if old_zoo_header is of 
type 0, we leave zoo_header unchanged.  However, we always 
unconditionally update the header type to be type H_TYPE.  
(Note:  zoo_header.type is initialized to H_TYPE in the
global declaration of zoo_header.)
*/
if (old_zoo_header.type > 0) {
	zoo_header.zoo_start = SIZ_ZOOH + old_zoo_header.acmt_len;
	zoo_header.zoo_minus = -zoo_header.zoo_start;
	zoo_header.acmt_pos = SIZ_ZOOH;	/* new comment just after header */
	zoo_header.acmt_len = old_zoo_header.acmt_len;
	zoo_header.vdata	  = old_zoo_header.vdata;
} else /* keep generations off if using old format archive */
	zoo_header.vdata &=  (~VFL_ON);

/* Write the header of the new archive, updated with our own data */
fwr_zooh (&zoo_header, new_file);

/* copy archive comment */
if (old_zoo_header.acmt_len != 0) {
	zooseek (zoo_file, old_zoo_header.acmt_pos, 0);	/* find archive comment */
	getfile (zoo_file, new_file, (long) zoo_header.acmt_len, 0); /* copy it */
}

/* WARNING: CHECK FOR SEEK BEYOND END OF FILE */
zooseek (new_file, zoo_header.zoo_start, 0);       /* position to add files */

zooseek (zoo_file, old_zoo_header.zoo_start, 0); /* seek to where data begins */

/* Now we loop through the old archive's files and add each to the new
archive.  The only changes needed are to update the .next and .offset
fields of the directory entry. */

while (1) {
   frd_dir(&direntry, zoo_file);
   if (direntry.zoo_tag != ZOO_TAG) {
      long currpos, zoolength;
      prterror ('F', bad_directory);
      if (bad_header) {    /* bad headers means don't save temp file */
         zooclose (new_file);
         unlink (temp_file);
      } else {
         writenull (new_file, MAXDIRSIZE);    /* write final null entry */
         printf (partial_msg, temp_file);
         if ((currpos = ftell (zoo_file)) != -1L)
            if (zooseek (zoo_file, 0L, 2) == 0)
               if ((zoolength = ftell (zoo_file)) != -1L)
                  printf (cant_process, zoolength - currpos);
      }
      zooexit (1);
   }
   if (direntry.next == 0L) {                /* END OF CHAIN */
      break;                                 /* EXIT on end of chain */
   }
   next_ptr = direntry.next;                 /* ptr to next dir entry */

   if (!direntry.deleted) {
#ifdef QUIETPACK
/* nothing */
#else
      prterror ('m', "%-14s -- ",
         direntry.namlen > 0 ? direntry.lfname : direntry.fname);
#endif

      if (zooseek (zoo_file, direntry.offset, 0) == -1L) {
         prterror ('f', "Could not seek to file data.\n");
      } else {
         extcount++;          /* update count of files extracted */

         /* write a directory entry for this file */
         new_dir_pos = zootell (new_file); /* new direntry pos in new archive */

         /*
         Write a null directory entry to preserve integrity in case of
         program being interrupted.  Note:  I don't think it is
         necessary to save direntry.next but I haven't checked.
         */
         {
            long oldnext;
            oldnext = direntry.next;
            direntry.next = 0L;
            fwr_dir(&direntry, new_file);
            direntry.next = oldnext;
         }

         zooseek (zoo_file, direntry.offset, 0);  /* where to start copying */
         /* Write file leader and remember position of new file data */
         (void) zoowrite (new_file, file_leader, SIZ_FLDR);
         direntry.offset = zootell (new_file);
         status = getfile (zoo_file, new_file, direntry.size_now, 0);
         /* if no error copy any comment attached to file */
         if (status == 0 && direntry.cmt_size != 0) {
            zooseek (zoo_file, direntry.comment, 0);  /* seek to old comment  */
            direntry.comment = zootell (new_file); /* location of new comment */
            status = getfile (zoo_file, new_file,
                                 (long) direntry.cmt_size, 0);
         }

         if (status != 0) {
            if (status == 1) {
               memerr(0);
            } else
               if (status == 2 || status == 3) {
                  prterror ('F', disk_full);
                  printf (partial_msg, temp_file);
                  zooexit (1);
               } else
                  prterror ('f', internal_error);
         } else {
            if (latest_date < direntry.date ||
                     (latest_date == direntry.date &&
                           latest_time < direntry.time))  {
               latest_date = direntry.date;
               latest_time = direntry.time;
            }
         }
         direntry.next = zootell (new_file);
         zooseek (new_file, new_dir_pos, 0);  /* position to write direntry */

         break_hit = 0;
#ifndef NOSIGNAL
         oldsignal = signal (SIGINT, SIG_IGN);
         if (oldsignal != SIG_IGN)
            signal (SIGINT, handle_break);
#endif

			/* Bug fix thanks to Mark Alexander */
         if (fwr_dir (&direntry, new_file) != -1 &&
            zoowrite (new_file, file_leader, SIZ_FLDR) == SIZ_FLDR) {
#ifdef QUIETPACK
            /* prterror ('M', "."); */ ;
#else
            prterror ('M', "moved\n");
#endif
         } else
            prterror ('f', "Write to temporary packed archive %s failed.\n", temp_file);
#ifndef NOSIGNAL
         signal (SIGINT, oldsignal);
#endif
         if (break_hit)
            zooexit (1);
         zooseek (new_file, direntry.next, 0);  /* back to end of new archive */
      }  /* end if (lseek ... */
   } /* end if (!direntry.deleted) */

zooseek (zoo_file, next_ptr, 0);   /* ..seek to next dir entry */
} /* end while */

zooclose (zoo_file);

/* write a final null entry */
writenull (new_file, MAXDIRSIZE);

#ifdef NIXTIME
zooclose (new_file);
setutime (temp_file, latest_date, latest_time);
#else
settime (new_file, latest_date, latest_time);    /* adjust its time */
zooclose (new_file);
#endif

/* Important note:  At this point, it is assumed that the archive was
   packed and written to a new file without error.  If control reaches
   here without the new archive having been written properly, then
   loss of data due to deletion of the original file could occur.  In
   other words, if something went wrong, execution MUST NOT reach here. */

if (extcount == 0) {
   unlink (temp_file);
   prterror ('m', "No files moved.\n");
} else {
   /* (a) if user requested, delete original, else rename it to
   *.bak.  (b) rename temp file to same base name as zoo_file. */

#ifdef QUIETPACK
   /* prterror('M', "\n"); */
#endif

   unlink (backup_name);    /* remove any previous backup in any case */
   if (nobackup)
      unlink (zoo_path);
   else
      chname (backup_name, zoo_path);

	/* if we are packing into current directory, we will rename temp file
		to same basename but without the preceding pathname */
	if (curr_dir)
		zoo_path = nameptr (zoo_path);		/* strip pathname */


   if (chname (zoo_path, temp_file)) {
      prterror ('w', "Renaming error.  Packed archive is now in %s.\n", temp_file);
      zooexit (1);
   }

/*
Set protection on packed archive -- after renaming, since some
OSs might not allow renaming of read-only files
*/
#ifdef FATTR
	setfattr (zoo_path, zoofattr);
#endif /* FATTR */

} /* end if */

} /* end zoopack() */

/* handle_break() */
/* Sets break_hit to 1 when called */
T_SIGNAL handle_break()
{
#ifndef NOSIGNAL
   signal (SIGINT, SIG_IGN);     /* ignore future control ^Cs for now */
   break_hit = 1;
#endif
}
