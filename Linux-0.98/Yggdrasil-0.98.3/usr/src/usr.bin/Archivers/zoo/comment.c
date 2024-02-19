#ifndef LINT
static char sccsid[]="@(#) comment.c 2.14 88/01/24 12:42:13";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/

#include "options.h"
#include "portable.h"
/* comment() */
/* Updates comments */

/* buffer size for any one comment line */
#define  COMMENT_LINE_SIZE 76

#define  MAX_COMMENT_SIZE  32767
#include "zooio.h"
#include "various.h"

#ifndef NOSIGNAL
#include <signal.h>
#endif

#include "zoo.h"
#include "zoofns.h"
#include "errors.i"

void show_comment PARMS ((struct direntry *, ZOOFILE, int, char *));
void get_comment PARMS ((struct direntry *, ZOOFILE, char *));
int needed PARMS ((char *, struct direntry *, struct zoo_header *));

void comment(zoo_path, option)
char *zoo_path, *option;
{
#ifndef NOSIGNAL  
T_SIGNAL (*oldsignal)();
#endif
ZOOFILE zoo_file;                         /* stream for open archive */
long next_ptr;                            /* pointers to within archive */
long this_dir_offset;                     /* pointers to within archive */
struct direntry direntry;                 /* directory entry */
struct zoo_header zoo_header;
int matched = 0;                          /* any files matched? */
unsigned int zoo_date, zoo_time;          /* for restoring archive timestamp */
char whichname[PATHSIZE];                 /* which name to use */
#ifdef ZOOCOMMENT
int acmt = 0;										/* if changing archive comment */
#endif

/* on entry option points to first letter */
option++;											/* skip 'c' */
#ifdef ZOOCOMMENT
while (*option != '\0') {
	if (*option == 'A') {
		acmt++;										/* changing archive comment */
		option++;
	} else
   	prterror ('f', inv_option, *option);
}
#else
if (*option != '\0')
	prterror ('f', inv_option, *option);
#endif /* ZOOCOMMENT */

if ((zoo_file = zooopen (zoo_path, Z_RDWR)) == NOFILE)
   prterror ('f', could_not_open, zoo_path);

/* save archive timestamp */
#ifdef GETUTIME
getutime (zoo_path, &zoo_date, &zoo_time);
#else
gettime (zoo_file, &zoo_date, &zoo_time);
#endif

/* read header and rewrite with updated version numbers, but ask user to pack
archive first if archive comment is to be added and header type is 0 */
#ifdef ZOOCOMMENT
if (acmt)
	rwheader (&zoo_header, zoo_file, 0);
else
	rwheader (&zoo_header, zoo_file, 1);
#else
rwheader (&zoo_header, zoo_file, 1);
#endif

#ifdef ZOOCOMMENT
/* if archive comment being added, handle it and return */
if (acmt) {
	void do_acmt PARMS ((struct zoo_header *, ZOOFILE, char *));
	do_acmt (&zoo_header, zoo_file, zoo_path);
#ifdef NIXTIME
	zooclose (zoo_file);
	setutime (zoo_path, zoo_date, zoo_time);	/* restore timestamp */
#else
	settime (zoo_file, zoo_date, zoo_time);	/* restore timestamp */
	zooclose (zoo_file);
#endif
	return;
}
#endif /* ZOOCOMMENT */

/* Loop through and add comments for matching files */
while (1) {
   this_dir_offset = zootell (zoo_file);  /* save pos'n of this dir entry */
   readdir (&direntry, zoo_file, 1);      /* read directory entry */
   next_ptr = direntry.next;              /* ptr to next dir entry */

   /* exit on end of directory chain or end of file */
   if (next_ptr == 0L || feof(stdin))
      break;

	strcpy (whichname, fullpath (&direntry));		/* full pathname */
	add_version (whichname, &direntry);				/* add version suffix */
   /* add comments for matching non-deleted files */
   if (!direntry.deleted && needed (whichname, &direntry, &zoo_header)) {
      matched++;
      show_comment (&direntry, zoo_file, 1, whichname);
      get_comment (&direntry, zoo_file, whichname);
      zooseek (zoo_file, this_dir_offset, 0);
#ifndef NOSIGNAL
      oldsignal = signal (SIGINT, SIG_IGN);
#endif
      fwr_dir (&direntry, zoo_file);
#ifndef NOSIGNAL
      signal (SIGINT, oldsignal);
#endif
   }
   zooseek (zoo_file, next_ptr, 0);   /* ..seek to next dir entry */
} /* end while */

#ifdef NIXTIME
zooclose (zoo_file);
setutime (zoo_path, zoo_date, zoo_time);	/* restore timestamp */
#else
settime (zoo_file, zoo_date, zoo_time);	/* restore timestamp */
zooclose (zoo_file);
#endif

if (!matched)
   printf ("Zoo:  %s", no_match);
} /* comment */

/* show_comment() */
/* shows comment on screen.  If show=1, says "Current comment is..." */

void show_comment (direntry, zoo_file, show, name)
struct direntry *direntry;
ZOOFILE zoo_file;
int show;
char *name;       /* name of file for which comment is being added */
{
   if (direntry->cmt_size != 0) {
      unsigned int i;
      char ch;
      int newline = 1;
      zooseek (zoo_file, direntry->comment, 0);   
      if (show)
         printf ("Current comment for %s is:\n", name);
      for (i = 0; i < direntry->cmt_size; i++) {/* show it */
         ch = zgetc (zoo_file) & 0x7f;          /* 7 bits only */
         if (newline)
            printf (" |");    /* indent and mark comment lines thus */
         zputchar (ch);
         if (ch == '\n')
            newline = 1;
         else
            newline = 0;
      }
      if (!newline)              /* always terminate with newline */
         zputchar ('\n');
   }
} /* show_comment() */


/* get_comment() */
/* Shows user old comment and updates it */

/* INPUT:
   direntry points to current directory entry.
   zoo_file is archive file.
   this_path is full pathname of file being updated/added.

   OUTPUT:
   Comment is added to file and supplied directory entry is updated
   with comment size and seek position but directory entry is
   not written to file.  Exceptions:  If RETURN is hit as first line,
   previous comment is left unchanged.  If /END is hit, previous
   comment is superseded, even if new comment is null.
*/

char cmt_prompt[]="[Enter %scomment for %s then type /END]\n";

void get_comment (direntry, zoo_file, this_path)  /* update comment */
register struct direntry *direntry;
ZOOFILE zoo_file;
char *this_path;
{
   unsigned int line_count = 0;        /* count of new comment lines */

   zooseek (zoo_file, 0L, 2);            /* ready to append new comment */
#if 0
   fprintf (stderr, "[Enter comment for %s then type /END]\n", this_path);
#else
   fprintf (stderr, cmt_prompt, "", this_path);
#endif
   while (1) {
      char cmt_line[COMMENT_LINE_SIZE];
      int cmt_size;
      if (fgets (cmt_line, sizeof(cmt_line), stdin) == NULL)
         break;
      line_count++;
      if (line_count == 1) {                 /* first line typed */
         if (!strcmp (cmt_line, "\n"))   /* exit if first line blank */
            break;
         direntry->comment = zootell (zoo_file);
         direntry->cmt_size = 0;
      }
      if (!str_icmp (cmt_line, "/end\n"))
         break;
      cmt_size = strlen (cmt_line);
      if (MAX_COMMENT_SIZE - direntry->cmt_size > cmt_size) {
         direntry->cmt_size += (unsigned int) cmt_size;
         if (zoowrite (zoo_file, cmt_line, cmt_size) < cmt_size)
            prterror ('f', disk_full);
      }
   } /* end while */
} /* get_comment() */

#ifdef ZOOCOMMENT
/*
do_acmt() updates archive comment by showing it to user and 
requesting a new one.  Typed input terminates as with file comment,
i.e., empty initial line leaves comment unchanged, case-insensitive
"/end" terminates input comment.
*/
void do_acmt (zoo_header, zoo_file, zoo_path)
struct zoo_header *zoo_header;
ZOOFILE zoo_file;
char *zoo_path;
{
   unsigned int line_count = 0;        /* count of new comment lines */
	void show_acmt PARMS ((struct zoo_header *, ZOOFILE, int));

	show_acmt (zoo_header, zoo_file, 1);	/* show current archive comment */
   zooseek (zoo_file, 0L, 2);            /* ready to append new comment */
#if 0
   fprintf (stderr, "[Enter archive comment for %s then type /END]\n", 
							zoo_path);
#else
   fprintf (stderr, cmt_prompt, "archive ", zoo_path);
#endif

   while (1) {
      char cmt_line[COMMENT_LINE_SIZE];
      int cmt_size;
      if (fgets (cmt_line, sizeof(cmt_line), stdin) == NULL)
         break;
      line_count++;
      if (line_count == 1) {                 /* first line typed */
         if (!strcmp (cmt_line, "\n"))   /* exit if first line blank */
            break;
         zoo_header->acmt_pos = zootell (zoo_file);
         zoo_header->acmt_len = 0;
      }
      if (!str_icmp (cmt_line, "/end\n"))
         break;
      cmt_size = strlen (cmt_line);
      if (MAX_COMMENT_SIZE - zoo_header->acmt_len > cmt_size) {
         zoo_header->acmt_len += (unsigned int) cmt_size;
         if (zoowrite (zoo_file, cmt_line, cmt_size) < cmt_size)
            prterror ('f', disk_full);
      }
   } /* end while */
	zooseek (zoo_file, 0L, 0);					/* seek back to beginning */
	fwr_zooh (zoo_header, zoo_file);			/* write update zoo_header */
} /* do_acmt() */
#endif /* ZOOCOMMENT */

/* Prints archive comment.  If show==1, says "Current archive comment is:" */
void show_acmt (zoo_header, zoo_file, show)
struct zoo_header *zoo_header;
ZOOFILE zoo_file;
int show;
{
   if (zoo_header->zoo_start != FIXED_OFFSET && zoo_header->acmt_len > 0) {
      unsigned int i;
      char ch;
      int newline = 1;
      zooseek (zoo_file, zoo_header->acmt_pos, 0);   
		if (show)
      	printf ("Current archive comment is:\n");
      for (i = 0; i < zoo_header->acmt_len; i++) {/* show it */
         ch = zgetc (zoo_file) & 0x7f;          /* 7 bits only */
         if (newline)
            printf (">> ");		/* indent and mark comment lines thus */
         zputchar (ch);
         if (ch == '\n')
            newline = 1;
         else
            newline = 0;
      }
      if (!newline)              /* always terminate with newline */
         zputchar ('\n');
   }
} /* show_acmt() */
