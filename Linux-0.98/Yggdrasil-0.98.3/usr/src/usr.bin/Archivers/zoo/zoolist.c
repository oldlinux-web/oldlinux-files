#ifndef LINT
/* derived from: zoolist.c 2.27 88/08/15 11:03:16 */
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/zoolist.c,v $\n\
$Id: zoolist.c,v 1.4 91/07/09 01:54:16 dhesi Exp $";
#endif /* LINT */

/*
If TRACE_LIST is defined, any list command may be followed
by 'D' to show verbose information about each directory
entry in the archive.   Do not define both TRACE_LIST and
TRACE_IO else a symbol conflict will occur and in any case
duplicate information will be dumped.
*/

/* #define TRACE_LIST */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
#include "portable.h"
#include "zoomem.h"  /* to get ZOOCOUNT */

/* Lists files in archive */
#include "zoo.h"
#include "errors.i"
#include "zooio.h"
#include "various.h"
#include "zoofns.h"

#ifdef TRACE_LIST
void show_dir PARMS ((struct direntry *direntry));
static int trace_list = 0;
#endif /* TRACE_LIST */

static char tot_fmt[] = "%8lu %3u%% %8lu  %4d file";
static char tot_line[] =
   /* "------------  --------  ---  --------  --------- --------\n"; */
   "--------  --- --------  --------- --------\n";

static char dbl_percent[] = "Archive %s:  %s";

extern int quiet;				/* assumed initialized to zero */

void show_comment PARMS((struct direntry *, ZOOFILE, int, char *));
int ver_too_high PARMS((struct zoo_header *));
int needed PARMS((char *, struct direntry *, struct zoo_header *));
void printtz PARMS((int));

void zoolist (argv, option, argc)
char **argv, *option;
int argc;
{
char whichname[PATHSIZE];  /* which name to use */
char *this_zoo;            /* currently matched archive name */
register ZOOFILE zoo_file;
char *flist[ZOOCOUNT];       /* list of ptrs to input archive names */
int fptr;                  /* will point to within list of archive names */

struct direntry direntry;
struct zoo_header zoo_header;
int size_factor;
unsigned long tot_org_siz = 0L, tot_siz_now = 0L;
int   tot_sf;
int file_count = 0;
int del_count = 0;                  /* number of deleted entries */
int bad_pack;                 /* 1 if packing method is unknown */
static char *month_list="000JanFebMarAprMayJunJulAugSepOctNovDec";
static char dashes[] = "------------\n";
int year, month, day, hours, min, sec;
int list_deleted = 0;         /* list deleted files too */
int fast = 0;                 /* fast list */
long fiz_ofs = 0;             /* offset where to start */
long dat_ofs = 0;             /* ... data offset of file data */
int verb_list = 0;            /* if verbose listing needed */
int show_name = 0;            /* if archive name to be included in listing */
int show_crc = 0;					/* if crc should be listed */
int zoocount = 1;             /* number of archives to list */
int biglist = 0;              /* multiarchive listing */
int one_col = 0;					/* one column listing requested */
int showdir = 0;					/* show directory name in fast listing */
int longest;                  /* length of longest archive name */
int talking;						/* opposite of quiet */
int column = 0;               /* for column printing */
int first_ever = 1;				/* first time ever -- very special case */
int neednl = 0;					/* whether to print a newline */
int need_acmt = 0;				/* show archive comment */
int show_gen = 0;					/* show generation count */
int genson = 1;					/* enable/disable generations */
#ifdef FATTR
int show_mode = 0;				/* show file protection */
#endif
int first_dir = 1;				/* if first direntry -- to adjust dat_ofs */

while (*option) {
   switch (*option) {
      case 'a': show_name++; break;
#ifdef TRACE_LIST
		case 'D': trace_list++; break;
#endif /* TRACE_LIST */
      case 'd': list_deleted++; break;
      case 'f': fast++; break;
		case 'g': show_gen++; break;
		case '/': showdir++; break;
		case 'A':
		case 'v': need_acmt++; break;
      case 'V': need_acmt++; /* fall through */
      case 'c': verb_list++; break;
		case 'C': show_crc++; break;
      case 'l': break;
      case 'L': biglist++; zoocount = argc; break;
#ifdef FATTR
		case 'm': show_mode++; break;
#endif
		case '1': one_col++; break;
		case '+': genson = 1; break;
		case '-': genson = 0; break;
		/* following code same as in zooext.c */
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
		case 'q': quiet++; break;
      default:
         prterror ('w', option_ignored, *option);
   }
   option++;
}

no_more:  /* come from exit from while loop above */

if (fast && show_name) {      /* don't allow 'a' with 'f' */
   show_name = 0;
   prterror ('w', option_ignored, 'a');
}

talking = !quiet;					/* for convenience */

#ifdef WILDCARD
   /* For each archive name supplied, if it is not a char range and
      does not contain a dot, append "*.zoo". */
   {
      int i;
      for (i = 0; i < argc;  i++) {
         if (strchr (nameptr (argv[i]), EXT_CH) == NULL && 
                           !match_half (nameptr (argv[0]), "?-?"))
            argv[i] = newcat (argv[i], "*.zoo");
      }
   }
#endif

makelist (zoocount, argv, flist,        ZOOCOUNT-2,   (char *) NULL,".","..", &longest);
/*        ^argc     ^argv ^list_pointer ^max_no_files   ^exclude */

for (fptr = 0;  (this_zoo = flist[fptr]) != NULL; fptr++) {
   int ercount;                  /* count of errors */
   int entrycount;               /* count of directory entries */
   int expl_deleted;             /* explain what D means */
   int expl_comment;             /* explain what comment means */
   int expl_ver;                 /* Explain what V means */
   int expl_star;                /* Explain what * means */
   int first_time;               /* first time through loop for an archive */

   ercount = entrycount = del_count =
      expl_deleted = expl_comment = expl_ver = expl_star = 0;

	if (talking)
		column = 0;						/* if quiet, names will run together */

   first_time = 1;

#ifndef WILDCARD
   /* Add default extension if none supplied */
   if (strchr (nameptr (this_zoo), EXT_CH) == NULL)
      this_zoo = newcat (this_zoo, EXT_DFLT);
#endif

   zoo_file = zooopen (this_zoo, Z_READ);

   if (zoo_file == NOFILE) {
      prterror ('e', could_not_open, this_zoo);
      continue;
   } else if (!show_name && talking)
      printf ("\nArchive %s:\n", this_zoo);
   
if (fiz_ofs != 0L) {                /* if offset specified, start there */
	prterror ('m', start_ofs, fiz_ofs, dat_ofs);
   zooseek (zoo_file, fiz_ofs, 0);
} else {
   if (frd_zooh (&zoo_header, zoo_file) == -1 ||
                                             zoo_header.zoo_tag != ZOO_TAG) {
      prterror ('e', dbl_percent, this_zoo, invalid_header);
      goto loop_end;
   }
#if 0
	if (talking && (!show_name || verb_list || need_acmt))
#else
	if (need_acmt && talking)
#endif
	{
		void show_acmt PARMS ((struct zoo_header *, ZOOFILE, int));
		show_acmt (&zoo_header, zoo_file, 0);		/* show archive comment */
	}

   /* Seek to the beginning of the first directory entry */
   if (zooseek (zoo_file, zoo_header.zoo_start, 0) != 0) {
      ercount++;
      prterror ('e', dbl_percent, this_zoo, bad_directory);
      goto loop_end;
   }
   if (!show_name && ver_too_high (&zoo_header)) {
      ercount++;
      if (ercount < 2)
         prterror ('M', wrong_version, 
										zoo_header.major_ver, zoo_header.minor_ver);
   }
} /* end if (fiz_ofs !- 0L) */

   /* Now we print information about each file in the archive */
   
   if (!show_name) { /* initialize for each file only if not disk catalog */
      tot_org_siz = 0L;  
      tot_siz_now = 0L;
      file_count = 0;
      del_count = 0;
   }

   while (1) {
      if (readdir (&direntry, zoo_file, 0) == -1) {
         prterror ('F', dbl_percent, this_zoo, bad_directory);
         goto givesummary;
      }
      if (direntry.zoo_tag != ZOO_TAG) {
         long currpos, zoolength;
         prterror ('F', dbl_percent, this_zoo, invalid_header);
         if ((currpos = zootell (zoo_file)) != -1L)
            if (zooseek (zoo_file, 0L, 2) == 0)
               if ((zoolength = zootell (zoo_file)) != -1L)
                  printf (cant_process, zoolength - currpos);              
         goto givesummary;
      }
   
      if (direntry.next == 0L)      /* EXIT on end of chain */
         break;                                 
      else
         entrycount++;              /* Number of directory entries */
		/* first direntry read, change dat_ofs from abs. pos. to rel. offset */
		if (first_dir && dat_ofs != 0) {
			dat_ofs -= direntry.offset;
			first_dir = 0;
		}
		direntry.next += dat_ofs;				/* allow for user-specified offset */
		if (direntry.comment != 0L)
			direntry.comment += dat_ofs;		/* so show_comment finds it */
   
      if (direntry.deleted)
         ++del_count;

#ifdef TRACE_LIST
		if (trace_list)
			show_dir (&direntry);
#endif /* TRACE_LIST */
      
		/* Into `whichname' put the filename to display. Use long filename if 
		it exists, else use short filename.  */
			strcpy (whichname, fullpath (&direntry));
			if (zoo_header.vdata & VFL_ON)
				add_version (whichname, &direntry);	/* add version suffix */
#ifdef DEBUG
      printf("matching against [%s] and [%s]\n", 
               nameptr(whichname), whichname);
#endif

      if ( ( (list_deleted && direntry.deleted) ||
               (list_deleted < 2 && !direntry.deleted)
           ) 
              && (biglist || needed(whichname, &direntry, &zoo_header))) {
			/* if generations forced off, then strip added version field */
			if (!genson) {	/* HORRENDOUSLY INEFFICIENT AND REPETITIOUS */
				char *ver_pos;
				ver_pos = findlast (whichname, VER_DISPLAY);
				if (ver_pos != NULL)
					*ver_pos = '\0';
			}
   
         file_count++;
   
         if (direntry.packing_method > MAX_PACK) {
            bad_pack = 1;
            expl_ver = 1;
         }  else
            bad_pack = 0;
      
         size_factor = cfactor (direntry.org_size, direntry.size_now);
   
         year  =  ((unsigned int) direntry.date >> 9) & 0x7f;
         month =  ((unsigned int) direntry.date >> 5) & 0x0f;
         day   =  direntry.date        & 0x1f;
   
         hours =  ((unsigned int) direntry.time >> 11)& 0x1f;
         min   =  ((unsigned int) direntry.time >> 5) & 0x3f;
         sec   =  ((unsigned int) direntry.time & 0x1f) * 2;
   
			/* Alignment in columns is a horrendously complex undertaking. */

         if (fast) {
				int space_left;
				int namelen;
				int next_col;
#if 0
            if ( (quiet && !first_ever || !first_time) && one_col)
               fputchar ('\n');
				first_ever = 0;
#endif
				/* If we are showing directories, whichname already contains the
				full pathname string.  Else we only use the filename as follows:
				long filename if possible, else short filename */
				if (!showdir) {
				   strcpy (whichname, 
				      (direntry.namlen != 0) ? direntry.lfname : direntry.fname);
					if (genson && zoo_header.vdata & VFL_ON)
						add_version (whichname, &direntry);	/* add version suffix */
				}
				namelen = strlen (whichname);

#define MARGIN			78
#define COL_WIDTH		16
#if 1
				/* if not enough space left, move to next line */
				if (!one_col && column != 0) {
					space_left = MARGIN - column;
					if (namelen > space_left) {
						neednl = 1;
						column = 0;
					}
				}
#endif
            if ( (quiet && !first_ever || !first_time) && (neednl || one_col))
					printf ("\n");
				first_ever = 0;
				neednl = 0;

            printf("%s", whichname);
				fflush (stdout);
				/* move to next column stop */
				column += namelen;
				next_col = ((column + (COL_WIDTH - 1)) / COL_WIDTH) * COL_WIDTH;
				if (next_col - column < 2)		/* need at least 2 spaces */
					next_col += COL_WIDTH;
				if (next_col > MARGIN) {
					neednl = 1;
					column = 0;
				} else {
					if (!one_col)
						printf ("%*s", (next_col - column), " ");
					column = next_col;
				}
   
         } else {
            if (talking && first_time && !show_name) {/*print archive header */
               printf ("Length    CF  Size Now  Date      Time\n");
               printf (tot_line);
            }
            printf ("%8lu %3u%% %8lu  %2d %-.3s %02d %02d:%02d:%02d",  
                     direntry.org_size, 
                     size_factor, direntry.size_now, 
                     day, &month_list[month*3], 
                     (day && month) ?  (year+80) % 100 : 0,
                     hours, min, sec);
               tot_org_siz += direntry.org_size;
               tot_siz_now += direntry.size_now;
#ifdef GETTZ
				printtz ((int) direntry.tz);	/* show timezone */
#else
				printf (" ");
#endif

				if (show_crc)
					printf ("%04x ", direntry.file_crc);
				if (show_gen) {
					if (direntry.vflag & VFL_ON)
						printf ("%2dg ", direntry.vflag & VFL_GEN);
					else
						printf ("--g ");
				}
   
            if (direntry.cmt_size) {
               expl_comment++;
               printf ("C");
            } else
               printf (" ");
   
            if (direntry.deleted) {
               expl_deleted++;
               printf ("D");
            }  else
               printf (" ");
            if (list_deleted)
               printf (" ");
            if (show_name)
               printf ("%-*s ", longest, this_zoo);

#ifdef FATTR
				if (show_mode) {
					if (direntry.fattr == 0)
						printf ("--- ");
					else if ((direntry.fattr >> 22) == 1)
						printf ("%03o ", direntry.fattr & 0x1ff);
					else
						printf ("??? ");
				}
#endif /* FATTR */

				/* new code to get around a common compiler bug */
				printf ("%s", whichname);
				if (direntry.dir_crc != 0) {
					expl_star++;
					printf ("*");
				}

            if (bad_pack)
               printf (" (V%d.%d)", direntry.major_ver, direntry.minor_ver);
            printf ("\n");
         }
         first_time = 0;
   
         /* if verbose listing requested show any comment.  f overrrides v */
         if (verb_list && !fast)
            show_comment (&direntry, zoo_file, 0, (char *) NULL);
      } /* end if (lots of conditions) */
   
		/* ..seek to next dir entry */
      zooseek (zoo_file, direntry.next, 0);
   } /* end while */
   
   givesummary:
   
	if (fast && talking) {
	   if (file_count) {
	   	if (del_count || (show_gen && zoo_header.type > 0))
	      	printf ("\n-----\n");
			else
	      	fputchar ('\n');
		}
		if (del_count)
			printf ("%d deleted.\n", del_count);
		if (show_gen && zoo_header.type > 0) {
			printf ("Generation limit %u",
						zoo_header.vdata & VFL_GEN);
			if ((zoo_header.vdata & VFL_ON) == 0)
				printf (" (off).\n");
			else
				printf (".\n");
		}
	} /* end if (fast && talking) */

   if (talking && !show_name) {
      if (!fast && file_count) {
         tot_sf = cfactor (tot_org_siz, tot_siz_now);
         printf (tot_line);
      
         printf (tot_fmt, tot_org_siz, tot_sf, tot_siz_now, file_count);
			if (file_count > 1)
				printf ("s\n");
			else
				printf ("\n");
         
         if (del_count || expl_ver || expl_deleted || expl_comment ||
					expl_star || (show_gen && (zoo_header.type > 0)))
            printf (dashes);
      }
   
      if (!fast) {
         if (del_count) {
            if (expl_deleted)
               printf ("D: deleted file.\n");
            else {
               if (del_count == 1)
                  printf ("There is 1 deleted file.\n");
               else
                  printf ("There are %d deleted files.\n", del_count);
            }
         }
      }
      if (expl_comment && !fast && !verb_list) 
         printf ("C: file has attached comment.\n");
      if (expl_ver && !fast)
         printf ("V: minimum version of Zoo needed to extract this file.\n");
      if (expl_star && !fast)
         printf ("*: directory entry may be corrupted.\n");
      if (!file_count)
         printf ("Zoo:  %s", no_match);
      
      if (!entrycount && !fiz_ofs)
         printf ("(The archive is empty.)\n");
		if (show_gen && (zoo_header.type > 0) && !fast) {
			printf ("Archive generation limit is %u",
						zoo_header.vdata & VFL_GEN);
			if ((zoo_header.vdata & VFL_ON) == 0)
				printf (" (generations off).\n");
			else
				printf (".\n");
		}
   } /* end if (talking && !show_name) */
loop_end:            /* jump here on badly structured archive */
   zooclose (zoo_file);
} /* end for */

if (talking && show_name) {
   if (file_count) {
      tot_sf = cfactor (tot_org_siz, tot_siz_now);
      printf (tot_line);
      printf (tot_fmt, tot_org_siz, tot_sf, tot_siz_now, file_count);
		if (file_count > 1)
			printf ("s\n");
		else
			printf ("\n");
   } 
} else if (fast && quiet)
	fputchar ('\n');
	

if (!file_count)
   zooexit (1);            /* Consider it an error if there were no files */
} /* zoolist() */

#ifdef GETTZ
void printtz (file_tz)
int file_tz;
{
	long gettz();
	int diff_tz;				/* timezone difference */
	if (file_tz == NO_TZ) 	/* if no timezone stored ..*/
		printf ("   ");			/* .. just pad with blanks */
	else {
		diff_tz = (file_tz / 4) - (int) (gettz() / 3600);
		if (diff_tz == 0)
			printf ("   ");					/* print nothing if same */
		else if (diff_tz > 0)			/* else print signed difference */
			printf ("+%1d ", diff_tz);
		else
			printf ("-%1d ", -diff_tz);
	}
}
#endif

/*
FOLLOWING CODE IS FOR DEBUGGING ONLY.  IT IS COMPILED IN ONLY
IF THE SYMBOL TRACE_LIST IS DEFINED
*/

#ifdef TRACE_LIST
/* code copied from portable.c near end */
/* dump contents of directory entry */
void show_dir (direntry)
struct direntry *direntry;
{
   printf ("Directory entry for file [%s][%s]:\n",
            direntry->fname, direntry->lfname);
   printf ("tag = [%8lx] type = [%d] PM = [%d] Next = [%8lx] Offset = [%8lx]\n",
            direntry->zoo_tag, (int) direntry->type, 
            (int) direntry->packing_method, direntry->next, 
            direntry->offset);
   printf ("Orig size = [%ld] Size now = [%ld] dmaj_v.dmin_v = [%d.%d]\n",
         direntry->org_size, direntry->size_now,
         (int) direntry->major_ver, (int) direntry->minor_ver);
   printf ("Struc = [%d] DEL = [%d] comment_offset = [%8lx] cmt_size = [%d]\n",
         (int) direntry->struc, (int) direntry->deleted, direntry->comment,
         direntry->cmt_size);
   printf ("var_dir_len = [%d] TZ = [%d] dir_crc = [%4x]\n",
            direntry->var_dir_len, (int) direntry->tz, direntry->dir_crc);
   printf ("system_id = [%d]  dirlen = [%d]  namlen = [%d] fattr=[%24lx]\n", 
		direntry->system_id, direntry->dirlen, direntry->namlen, direntry->fattr);
	printf ("vflag = [%4x] version_no = [%4x]\n",
				direntry->vflag, direntry->version_no);
   if (direntry->dirlen > 0)
      printf ("dirname = [%s]\n", direntry->dirname);
   printf ("---------\n");
}
#endif   /* TRACE_IO */
