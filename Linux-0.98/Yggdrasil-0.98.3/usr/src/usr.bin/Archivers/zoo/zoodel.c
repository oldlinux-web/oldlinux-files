#ifndef LINT
/* @(#) zoodel.c 2.19 88/02/06 21:23:36 */
/*$Source: /usr/home/dhesi/zoo/RCS/zoodel.c,v $*/
/*$Id: zoodel.c,v 1.4 91/07/09 01:54:11 dhesi Exp $*/
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/zoodel.c,v $\n\
$Id: zoodel.c,v 1.4 91/07/09 01:54:11 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
/* Deletes or undeletes entries from an archive.  choice=1 requests
   deletion and choice=0 requests undeletion. */
#include "zoo.h"
#include "portable.h"
#ifndef	OK_STDIO
#include <stdio.h>
#define	OK_STDIO
#endif
#include "various.h" /* may not be needed */
#include "zooio.h"
#include "zoofns.h"
#include "errors.i"

#ifndef NOSIGNAL
#include <signal.h>
#endif

int needed PARMS((char *, struct direntry *, struct zoo_header *));
int ver_too_high PARMS((struct zoo_header *));

extern int quiet;

void zoodel (zoo_path, option, choice)
char *zoo_path;
char *option;
int choice;
{
#ifndef NOSIGNAL
   T_SIGNAL (*oldsignal)();        /* to save previous SIGINT handler */
#endif
   int delcount = 0;          /* how many entries we [un]deleted */
   char matchname[PATHSIZE];  /* will hold full pathname */
   register ZOOFILE zoo_file;
   struct zoo_header zoo_header;
   struct direntry direntry;
   unsigned int latest_date = 0;      /* so we can set time of archive later */
   unsigned int latest_time = 0;
   int pack = 0;              /* pack after deletion? */
   int file_deleted = 0;      /* any files deleted? */
   int one = 0;               /* del/undel one file only */
   int done;                  /* loop control */
	int action;						/* delete/undelete or adjust generation */
	int subopt;						/* sub option to action */
	long gencount;					/* generation count */
	int doarchive = 0;			/* whether to adjust archive gen count */
	unsigned valtoshow;			/* value to show in informative message */
	int dodel = 0;					/* selection of deleted files */
	int selected;					/* if current direntry selected */

/* values for action */
#define NO_ACTION	0	/* nothing */
#define DEL_UNDEL	1	/* delete or undelete file */
#define ADJ_LIM	2	/* adjust generation limit */
#define ADJ_GCNT	3	/* adjust generation count */
#define GEN_ON		4	/* turn on generations */
#define GEN_OFF		5	/* turn off generations */

/* values for subopt */
#define	SET		0
#define	INC		1

action = NO_ACTION;
if (*option == 'g') {
	while (*(++option)) {
		switch (*option) {
			case 'A': doarchive = 1; break;
			case 'q': quiet++; break;
			case 'l': action = ADJ_LIM; break;
			case 'c': action = ADJ_GCNT; break;
			case '=':
				subopt = SET; gencount = calc_ofs (++option);
				if (action == ADJ_GCNT && gencount == 0)
					prterror ('f', "Generation count must be nonzero.\n");
				goto opts_done;
			case '+':
				if (action == NO_ACTION) {
					if (option[1] =='\0') {
						action = GEN_ON;
						goto opts_done;
					} else
						prterror ('f', garbled);
				} else {
					subopt = INC; gencount = calc_ofs (++option);
					goto opts_done;
				}
			case '-':
				if (action == NO_ACTION) {
					if (option[1] =='\0') {
						action = GEN_OFF;
						goto opts_done;
					} else
						prterror ('f', garbled);
				} else {
					subopt = INC; gencount = - calc_ofs (++option);
					goto opts_done;
				}
			case 'd':
				dodel++; break;
			default:
				prterror ('f', garbled);
		} /* end switch */
	} /* end while */
	/* if normal exit from while loop, it means bad command string */
	prterror ('f', garbled);
	opts_done: 							/* jump here from exit in while loop above */
		if (action == NO_ACTION)
			prterror ('f', garbled);
} else {
	action = DEL_UNDEL;
	while (*(++option)) {
		switch (*option) {
			case 'P': pack++; break;            /* pack after adding */
			case 'q': quiet++; break;           /* be quiet */
			case '1': one++; break;             /* del or undel only one file */
			default:
				prterror ('f', inv_option, *option);
		}
	} /* end while */
}

   /* Open archive for read/write/binary access.  It must already exist */
   if ((zoo_file = zooopen (zoo_path, Z_RDWR)) == NOFILE) {
      prterror ('f', could_not_open, zoo_path);
   }
   
   /* read archive header */
   frd_zooh (&zoo_header, zoo_file);
   if ((zoo_header.zoo_start + zoo_header.zoo_minus) != 0L)
      prterror ('f', failed_consistency);
   if (ver_too_high (&zoo_header))
      prterror ('f', wrong_version, zoo_header.major_ver, zoo_header.minor_ver);

	if (doarchive) {									/* manipulate archive gen val */
		unsigned zoo_date, zoo_time;
#ifdef GETUTIME
		getutime (zoo_path, &zoo_date, &zoo_time);	/* save archive timestamp */
#else
		gettime (zoo_file, &zoo_date, &zoo_time);
#endif
		if (zoo_header.type == 0)
			prterror ('f', packfirst);
		if (action == ADJ_LIM)	{
			unsigned newgencount;	
			if (subopt == SET)
				newgencount = (unsigned) gencount;
			else																		/* INC */
				newgencount = (zoo_header.vdata & VFL_GEN) + (unsigned) gencount;
			newgencount &= VFL_GEN;			/* reduce to allowed bits */
			zoo_header.vdata &= (~VFL_GEN);
			zoo_header.vdata |= newgencount;
			prterror ('M', "Archive generation limit is now %u\n", newgencount);
		} else if (action == GEN_ON) {
			zoo_header.vdata |= VFL_ON;
			prterror ('M', "Archive generations on\n");
		} else if (action == GEN_OFF) {
			zoo_header.vdata &= (~VFL_ON);
			prterror ('M', "Archive generations off\n");
		} else 
			prterror ('f', garbled);
		zooseek (zoo_file, 0L, 0);		/* back to begining of file */
		fwr_zooh (&zoo_header, zoo_file);
#ifdef NIXTIME
		zooclose (zoo_file);
		setutime (zoo_path, zoo_date, zoo_time);	/* restore archive timestamp */
#else
		settime (zoo_file, zoo_date, zoo_time);
		zooclose (zoo_file);
#endif
		return;
	}

   zooseek (zoo_file, zoo_header.zoo_start, 0); /* seek to where data begins */

   done = 0;            /* loop not done yet */
   while (1) {
      long this_dir_offset;
      this_dir_offset = zootell (zoo_file);   /* save pos'n of this dir entry */
      frd_dir (&direntry, zoo_file);
      if (direntry.zoo_tag != ZOO_TAG) {
         prterror ('f', bad_directory);
      }
      if (direntry.next == 0L) {                /* END OF CHAIN */
         break;                                 /* EXIT on end of chain */
      }

		/* select directory entry if it matches criteria */
		selected = (
						  (action == DEL_UNDEL && direntry.deleted != choice)
                  ||
						  (action != DEL_UNDEL &&
                     	(dodel && direntry.deleted ||
                    			(dodel < 2 && !direntry.deleted))
						  )
					  );

		/* WARNING: convention of choice=1 for deleted entry must be same as
		in direntry definition in zoo.h */
	
		/* Test for "done" so if "one" option requested, [un]del only 1 file */
		/* But we go through the whole archive to adjust archive time */

		strcpy (matchname, fullpath (&direntry));		/* get full pathname */
		if (zoo_header.vdata & VFL_ON)
			add_version (matchname, &direntry);			/* add version suffix */

		if (!done && selected && needed(matchname, &direntry, &zoo_header)) {
			prterror ('m', "%-14s -- ", matchname);
			delcount++;
			if (action == DEL_UNDEL) {
				direntry.deleted = choice;
				if (choice)
					file_deleted++;      /* remember if any files actually deleted */
			} else {							/* ADJ_LIM or ADJ_GENCNT */
				if (direntry.vflag & VFL_ON) {		/* skip if no versions */
					if (action == ADJ_LIM) {
						unsigned newgencount;
						if (subopt == SET)
							newgencount = (unsigned) gencount;
						else 													/* INC */
							newgencount =
								(int) (direntry.vflag & VFL_GEN) + (int) gencount;
						newgencount &= VFL_GEN;
						direntry.vflag &= (~VFL_GEN);
						direntry.vflag |= newgencount;
						valtoshow = newgencount;
					} else {													/* ADJ_GCNT */
						if (subopt == SET)
							direntry.version_no = (unsigned) gencount;
						else 													/* INC */
							direntry.version_no += (int) gencount;
						direntry.version_no &= VER_MASK; /* avoid extra bits */
						valtoshow = direntry.version_no;
					}
				}
			}

			zooseek (zoo_file, this_dir_offset, 0);

#ifndef NOSIGNAL
			oldsignal = signal (SIGINT, SIG_IGN);  /* disable ^C for write */
#endif
			if (fwr_dir (&direntry, zoo_file) == -1)
				prterror ('f', "Could not write to archive\n");
#ifndef NOSIGNAL
			signal (SIGINT, oldsignal);
#endif
			if (action == DEL_UNDEL)
				prterror ('M', choice ? "deleted\n" : "undeleted\n");
			else {
				if (direntry.vflag & VFL_ON)
					prterror ('M', "adjusted to %u\n", valtoshow);
				else
					prterror ('M', "no generations\n");
			}
			if (one)
				done = 1;            /* if 1 option, done after 1 file */
		}

      /* remember most recent date and time if entry is not deleted */
      if (!direntry.deleted)
         if (direntry.date > latest_date ||
            (direntry.date == latest_date && direntry.time > latest_time)) {
               latest_date = direntry.date;
               latest_time = direntry.time;
         }
      zooseek (zoo_file, direntry.next, 0); /* ..seek to next dir entry */
   } /* endwhile */

   if (!delcount)
      printf ("Zoo:  No files matched.\n");
   else {
#ifdef NIXTIME
      zooclose (zoo_file);
      setutime (zoo_path, latest_date, latest_time);
#else
#if 0
      fflush (zoo_file);         /* superstition:  might help time stamp */
#endif
      settime (zoo_file, latest_date, latest_time);
#endif
   }

#ifndef NIXTIME
zooclose (zoo_file);
#endif

if (file_deleted && pack) {   /* pack if files were deleted and user asked */
   prterror ('M', "-----\nPacking...");
   zoopack (zoo_path, "PP");
   prterror ('M', "done\n");
}

}
