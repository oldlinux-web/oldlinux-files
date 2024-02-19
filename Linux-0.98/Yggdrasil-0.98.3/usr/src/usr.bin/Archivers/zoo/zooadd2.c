#ifndef LINT
/* derived from: zooadd2.c 2.14 88/01/27 10:40:32 */
static char sccsid[]="$Id: zooadd2.c,v 1.5 91/07/04 13:33:55 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
#include "zoo.h"
#ifndef	OK_STDIO
#include <stdio.h>
#define	OK_STDIO
#endif
#include "various.h"
#include "zooio.h"
#include "zoofns.h"
#include "errors.i"
#include "assert.h"
#include "debug.h"
#include "parse.h"

/*
Miscellaneous routines to support zooadd().
*/

/****************
This function is called with zoo_file positioned to the first
directory entry in an archive.  It skips past all existing files,
counts the number of deleted files, saves the latest data and time
encountered, and adds all filenames encountered to a global list. The
long filename is added if available, else the MSDOS filename is
added.  
*/

void skip_files (zoo_file, latest_date, latest_time, delcount, 
                  latest_name, latest_pos)
ZOOFILE zoo_file;
unsigned int *latest_date, *latest_time;
int *delcount;
char latest_name[];
long *latest_pos;
{
   long save_offset, next_ptr;
   struct direntry direntry;
   struct direntry *drp = &direntry;

   *latest_pos = 0L;
   do {
      /* read a directory entry */
      save_offset = zootell (zoo_file);     /* save pos'n of this dir entry */
      readdir (&direntry, zoo_file, 1);   /* read directory entry */
      if (drp->next == 0L) {                 /* END OF CHAIN */
         zooseek (zoo_file, save_offset, 0);      /* back up */
         break;                                 /* EXIT on end of chain */
      } else
         *latest_pos = save_offset;
      /* remember most recent date and time, for files not marked deleted */
      if (!drp->deleted)
         if (drp->date > *latest_date ||
            (drp->date == *latest_date && drp->time > *latest_time)) {
               *latest_date = drp->date;
               *latest_time = drp->time;
         }
      next_ptr = drp->next;            /* ptr to next dir entry */
      if (drp->deleted)
         ++(*delcount);                      /* count deleted entries */
      /* add name of file and position of direntry into global list */
      /* but only if the entry is not deleted */
      if (!drp->deleted) {
#ifdef FOLD
			/* IS THIS REALLY NEEDED?  IF SO, WHAT ABOUT drp->lfname? */
         str_lwr(drp->fname);
#endif
			/* add full pathname to global list */
			strcpy (latest_name, fullpath (drp));
         addfname (latest_name, save_offset, drp->date, drp->time,
							drp->vflag, drp->version_no);
      }
      zooseek (zoo_file, next_ptr, 0);   /* ..seek to next dir entry */
   } while (next_ptr != 0L);              /* loop terminates on null ptr */
}

/*******************/
/* kill_files() deletes all files in the supplied list of pointers to
filenames */

int kill_files (flist, pathlength)
char *flist[];                      /* list of ptrs to input fnames */
int pathlength;                     /* length of longest pathname */
{
   int status = 0;
   int fptr;
   prterror ('M', "-----\nErasing added files...\n");
   for (fptr = 0;  flist[fptr] != NULL; fptr++) {
#ifdef CHEKUDIR
				if (isuadir(flist[fptr]))
					continue;
#else /* CHEKUDIR */
# ifdef CHEKDIR
				if (isfdir(flist[fptr]))
					continue;
# endif /* CHEKDIR */
#endif /* CHEKUDIR */
      prterror ('m', "%-*s -- ", pathlength, flist[fptr]);
      if (unlink (flist[fptr]) == 0) {
         prterror ('M', "erased\n");
      } else {
         prterror ('w', "Could not erase %s.\n", flist[fptr]);
         status = 1;
      }
   }
   return (status);
}

#ifndef PORTABLE
/*******************/
void copyfields (drp, thp)
struct direntry *drp;
struct tiny_header *thp;
{
   drp->org_size = thp->org_size;
   drp->file_crc = thp->file_crc;
   drp->size_now = thp->size_now;
   drp->major_ver = thp->major_ver;
   drp->minor_ver = thp->minor_ver;
}
#endif

/*******************/
/* processes option switches for zooadd() */
void opts_add (option, zootime, quiet, suppress, move, new, pack,
          update, add_comment, z_fmt, need_dir, inargs, genson,
			 use_lzh, arch_cmnt)
char *option;
int *zootime, *quiet, *suppress, *move, *new, *pack,
   *update, *add_comment, *z_fmt, *need_dir, *inargs,
	*genson, *use_lzh, *arch_cmnt;

{
   if (*option == 'T') {
      (*zootime)++;
      option++;
      while (*option) {
         switch (*option) {
            case 'q': (*quiet)++; break;
            default:
               prterror ('f', inv_option, *option);
         }
      option++;
      }
   }
   
   while (*option) {
      switch (*option) {
         case 'a': break;  
			case 'h': (*use_lzh)++; break;         /* use lzh compression */
         case 'f': (*suppress)++; break;        /* suppress compression */
         case 'M': (*move)++; break;            /* delete files after adding them */
         case 'n': (*new)++; break;             /* add only files not in archive */
         case 'P': (*pack)++; break;            /* pack after adding */
         case 'u': (*update)++;   break;        /* add only files already in archive */
         case 'q': (*quiet)++; break;           /* be quiet */
         case 'c': (*add_comment)++; break;     /* add comment */
         case ':': *need_dir = 0; break;        /* don't store directories */
         case 'I': (*inargs)++; break;        	/* get filenames from stdin */
         case 'C': (*arch_cmnt)++; break;			/* do an archive comment */
/* #ifdef PORTABLE */ /* avoid Turbo C warning about unused param */
         case 'z': (*z_fmt)++; break;           /* look for Z format files */
/* #endif */
			case '+': 
						*genson = 1;  break;
			case '-': 
						*genson = 0;  break;
         default:
            prterror ('f', inv_option, *option);
      }
      option++;
   } /* end while */
	if (*suppress && *use_lzh)
		prterror ('f', "\"f\" and \"h\" can't both be used\n");
}

/* 
Stores long filename into direntry.lfname iff it is different from MSDOS
filename.  Also stores directory name if need_dir is true.  Moved out of 
zooadd() so zooadd() doesn't get too big for optimization.
*/
void storefname (direntry, this_path, need_dir)
struct direntry *direntry;
char *this_path;
int need_dir;
{
   struct path_st path_st;
   parse (&path_st, this_path);
   direntry->lfname[0] = '\0';
   direntry->namlen = 0;
#ifdef SPECMOD
   specfname (path_st.lfname);
   specdir (path_st.dir);
#endif
   if (strcmp(path_st.lfname,direntry->fname) != 0) {
         strcpy (direntry->lfname, path_st.lfname); /* full filename */
         direntry->namlen = strlen(direntry->lfname) + 1;
   }
   if (need_dir) {
      strcpy (direntry->dirname, path_st.dir);   /* directory name */
      direntry->dirlen = strlen(direntry->dirname) + 1;
      if (direntry->dirlen == 1) /* don't store trailing null alone */
         direntry->dirlen = 0;
   } else {
      direntry->dirname[0] = '\0';
      direntry->dirlen = 0;
   }
}

/* 
Function getsdtin() gets a pathname from standard input, cleans
it if necessary by removing any following blanks/tabs and other
junk, and returns it in a static area that is overwritten by each
call.
*/
char *getstdin()
{
	char *chptr;									/* temp pointer */
	static char tempname[PATHSIZE];
	do {
		if (fgets (tempname, PATHSIZE, stdin) == NULL)
			return (NULL);
		/* remove trailing blank, tab, newline */
		for (chptr = tempname; *chptr != '\0';  chptr++) {
			if (
	/* PURIFY means remove trailing blanks/tabs and all subsequent chars */
#ifdef PURIFY
					*chptr == '\t' || *chptr == ' ' ||
#endif
					*chptr == '\n'						/* always remove trailing \n */
				)
			{
	
				*chptr = '\0';
				break;
			}
		}
	} while (*tempname == '\0');				/* get a nonempty line */
#ifdef FOLD
	str_lwr (tempname);
#endif
	return (tempname);
}

/* 
Function newdir() adds some default information to a directory entry.
This will be a new directory entry added to an archive.
*/
void newdir (direntry)
register struct direntry *direntry;
{
#ifdef GETTZ
	long gettz();
#endif
   direntry->zoo_tag = ZOO_TAG;
   direntry->type = 2;                  /* type is now 2 */
#ifdef GETTZ
	direntry->tz = (uchar) (gettz() / (15 * 60)); /* seconds => 15-min units */
#else
   direntry->tz = NO_TZ;                /* timezone unknown */
#endif
   direntry->struc = 0;                 /* unstructured file */
   direntry->system_id = SYSID_NIX;     /* identify **IX filesystem */
	direntry->vflag = VFL_ON|VFL_LAST;	 /* latest version */
	direntry->version_no = 1;					/* begin with version 1 */
	/* 1 for namlen, 1 for dirlen, 2 for system id, 3 for attributes,
		1 for version flag and 2 for version number */
   direntry->var_dir_len = direntry->dirlen + direntry->namlen + 10;
}
