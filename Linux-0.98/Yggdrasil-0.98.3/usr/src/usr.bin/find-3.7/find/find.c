/* find -- search for files in a directory hierarchy
   Copyright (C) 1987, 1990, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* GNU find was written by Eric Decker (cire@cisco.com),
   with enhancements by David MacKenzie (djm@gnu.ai.mit.edu),
   Jay Plett (jay@silence.princeton.nj.us),
   and Tim Wood (axolotl!tim@toad.com). */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#define index strchr
#define rindex strrchr
#else
#include <strings.h>
#endif
#include "defs.h"
#include "modetype.h"

#ifndef S_IFLNK
#define lstat stat
#endif

int lstat ();
int stat ();

#define apply_predicate(pathname, stat_buf_ptr, node)	\
  (*(node)->pred_func)((pathname), (stat_buf_ptr), (node))

boolean parse_open ();
boolean parse_close ();
char *savedir ();
void error ();
void scan_directory ();

/* Name this program was run with. */
char *program_name;

/* All predicates for each path to process. */
struct predicate *predicates;

/* The last predicate allocated. */
struct predicate *last_pred;

/* The root of the evaluation tree. */
struct predicate *eval_tree;

/* If true, process directory before contents.  True unless -depth given. */
boolean do_dir_first;

/* If >=0, don't descend more than this many levels of subdirectories. */
int maxdepth;

/* If >=0, don't process files above this level. */
int mindepth;

/* Current depth; 0 means current path is a command line arg. */
int curdepth;

/* Seconds between 00:00 1/1/70 and either one day before now
   (the default), or the start of today (if -daystart is given). */
time_t cur_day_start;

/* If true, cur_day_start has been adjusted to the start of the day. */
boolean full_days;

/* If true, do not assume that files in directories with nlink == 2
   are non-directories. */
boolean no_leaf_check;

/* If true, don't cross filesystem boundaries. */
boolean stay_on_filesystem;

/* If true, don't descend past current directory.
   Can be set by -prune, -maxdepth, and -xdev. */
boolean stop_at_current_level;

/* If true, we have called stat on the current path. */
boolean have_stat;

/* Status value to return to system. */
int exit_status;

/* Length of current path. */
int path_length;

/* Pointer to the function used to stat files. */
int (*xstat) ();

#ifdef DEBUG_STAT
int
debug_stat (file, bufp)
     char *file;
     struct stat *bufp;
{
  fprintf (stderr, "debug_stat (%s)\n", file);
  return lstat (file, bufp);
}
#endif /* DEBUG_STAT */

void
main (argc, argv)
     int argc;
     char *argv[];
{
  int i;
  PFB parse_function;		/* Pointer to who is to do the parsing. */
  struct predicate *cur_pred;
  char *predicate_name;		/* Name of predicate being parsed. */

  program_name = argv[0];

  predicates = NULL;
  last_pred = NULL;
  do_dir_first = true;
  maxdepth = mindepth = -1;
  cur_day_start = time ((time_t *) 0) - DAYSECS;
  full_days = false;
  no_leaf_check = false;
  stay_on_filesystem = false;
  exit_status = 0;
#ifdef DEBUG_STAT
  xstat = debug_stat;
#else /* !DEBUG_STAT */
  xstat = lstat;
#endif /* !DEBUG_STAT */

#ifdef DEBUG
  printf ("cur_day_start = %s", ctime (&cur_day_start));
#endif /* DEBUG */

  /* Find where in ARGV the predicates begin. */
  for (i = 1; i < argc && index ("-!(),", argv[i][0]) == NULL; i++)
    /* Do nothing. */ ;

  /* Enclose the expression in `( ... )' so a default -print will
     apply to the whole expression. */
  parse_open (argv, &argc);
  /* Build the input order list. */
  while (i < argc)
    {
      if (index ("-!(),", argv[i][0]) == NULL)
	usage ("paths must precede expression");
      predicate_name = argv[i];
      parse_function = find_parser (predicate_name);
      if (parse_function == NULL)
	error (1, 0, "invalid predicate `%s'", predicate_name);
      i++;
      if (!(*parse_function) (argv, &i))
	{
	  if (argv[i] == NULL)
	    error (1, 0, "missing argument to `%s'", predicate_name);
	  else
	    error (1, 0, "invalid argument to `%s'", predicate_name);
	}
    }
  if (predicates->pred_next == NULL)
    {
      /* No predicates that do something other than set a global variable
	 were given; remove the unneeded initial `(' and add `-print'. */
      cur_pred = predicates;
      predicates = last_pred = predicates->pred_next;
      free ((char *) cur_pred);
      parse_print (argv, &argc);
    }
  else if (!no_side_effects (predicates->pred_next))
    {
      /* One or more predicates that produce output were given;
	 remove the unneeded initial `('. */
      cur_pred = predicates;
      predicates = predicates->pred_next;
      free ((char *) cur_pred);
    }
  else
    {
      /* `( user-supplied-expression ) -print'. */
      parse_close (argv, &argc);
      parse_print (argv, &argc);
    }

#ifdef	DEBUG
  printf ("Predicate List:\n");
  print_list (predicates);
#endif /* DEBUG */

  /* Done parsing the predicates.  Build the evaluation tree. */
  cur_pred = predicates;
  eval_tree = get_expr (&cur_pred, NO_PREC);
#ifdef	DEBUG
  printf ("Eval Tree:\n");
  print_tree (eval_tree, 0);
#endif /* DEBUG */

  /* Rearrange the eval tree in optimal-predicate order. */
  opt_expr (&eval_tree);

  /* Determine the point, if any, at which to stat the file. */
  mark_stat (eval_tree);

#ifdef DEBUG
  printf ("Optimized Eval Tree:\n");
  print_tree (eval_tree, 0);
#endif /* DEBUG */

  /* If no paths given, default to ".". */
  for (i = 1; i < argc && index ("-!(),", argv[i][0]) == NULL; i++)
    {
      curdepth = 0;
      path_length = strlen (argv[i]);
      process_path (argv[i], false);
    }
  if (i == 1)
    {
      curdepth = 0;
      path_length = 1;
      process_path (".", false);
    }

  exit (exit_status);
}

/* Recursively descend path PATHNAME, applying the predicates.
   LEAF is nonzero if PATHNAME is in a directory that has no
   unexamined subdirectories, and therefore it is not a directory.
   This allows us to avoid stat as long as possible for leaf files.

   Return nonzero iff PATHNAME is a directory. */

int
process_path (pathname, leaf)
     char *pathname;
     boolean leaf;
{
  struct stat stat_buf;
  int pathlen;			/* Length of PATHNAME. */
  static dev_t root_dev;	/* Device ID of current argument pathname. */

  pathlen = strlen (pathname);

  /* Assume non-directory initially. */
  stat_buf.st_mode = 0;

  if (leaf)
    have_stat = false;
  else
    {
      if ((*xstat) (pathname, &stat_buf) != 0)
	{
	  fflush (stdout);
	  error (0, errno, "%s", pathname);
	  exit_status = 1;
	  return 0;
	}
      have_stat = true;
    }

  if (!S_ISDIR (stat_buf.st_mode))
    {
      if (curdepth >= mindepth)
	apply_predicate (pathname, &stat_buf, eval_tree);
      return 0;
    }

  stop_at_current_level = maxdepth >= 0 && curdepth >= maxdepth;

  if (stay_on_filesystem)
    {
      if (curdepth == 0)
	root_dev = stat_buf.st_dev;
      else if (stat_buf.st_dev != root_dev)
	stop_at_current_level = true;
    }

  if (do_dir_first && curdepth >= mindepth)
    apply_predicate (pathname, &stat_buf, eval_tree);

  if (stop_at_current_level == false)
    /* Scan directory on disk. */
    scan_directory (pathname, pathlen, &stat_buf);

  if (do_dir_first == false && curdepth >= mindepth)
    apply_predicate (pathname, &stat_buf, eval_tree);

  return 1;
}

/* Scan directory PATHNAME and recurse through process_path for each entry.
   PATHLEN is the length of PATHNAME.
   STATP is the results of *xstat on it. */

void
scan_directory (pathname, pathlen, statp)
     char *pathname;
     int pathlen;
     struct stat *statp;
{
  char *name_space;		/* Names of files in PATHNAME. */
  int subdirs_left;		/* Number of unexamined subdirs in PATHNAME. */

  subdirs_left = statp->st_nlink - 2; /* Account for name and ".". */

  errno = 0;
  /* On some systems (VAX 4.3BSD+NFS), NFS mount points have st_size < 0.  */
  name_space = savedir (pathname, statp->st_size > 0 ? statp->st_size : 512);
  if (name_space == NULL)
    {
      if (errno)
	{
	  fflush (stdout);
	  error (0, errno, "%s", pathname);
	  exit_status = 1;
	}
      else
	{
	  fflush (stdout);
	  error (1, 0, "virtual memory exhausted");
	}
    }
  else
    {
      register char *namep;	/* Current point in `name_space'. */
      char *cur_path;		/* Full path of each file to process. */
      unsigned cur_path_size;	/* Bytes allocated for `cur_path'. */
      register unsigned file_len; /* Length of each path to process. */
      register unsigned pathname_len; /* PATHLEN plus trailing '/'. */

      if (pathname[pathlen - 1] == '/')
	pathname_len = pathlen + 1; /* For '\0'; already have '/'. */
      else
	pathname_len = pathlen + 2; /* For '/' and '\0'. */
      cur_path_size = 0;
      cur_path = NULL;

      for (namep = name_space; *namep; namep += file_len - pathname_len + 1)
	{
	  /* Append this directory entry's name to the path being searched. */
	  file_len = pathname_len + strlen (namep);
	  if (file_len > cur_path_size)
	    {
	      while (file_len > cur_path_size)
		cur_path_size += 1024;
	      if (cur_path)
		free (cur_path);
	      cur_path = xmalloc (cur_path_size);
	      strcpy (cur_path, pathname);
	      cur_path[pathname_len - 2] = '/';
	    }
	  strcpy (cur_path + pathname_len - 1, namep);

	  curdepth++;
	  if (!no_leaf_check)
	    /* Normal case optimization.
	       On normal Unix filesystems, a directory that has no
	       subdirectories has two links: its name, and ".".  Any
	       additional links are to the ".." entries of its
	       subdirectories.  Once we have processed as many
	       subdirectories as there are additional links, we know
	       that the rest of the entries are non-directories --
	       in other words, leaf files. */
	    subdirs_left -= process_path (cur_path, subdirs_left == 0);
	  else
	    /* There might be weird (NFS?) filesystems mounted,
	       which don't have Unix-like directory link counts. */
	    process_path (cur_path, false);
	  curdepth--;
	}
      if (cur_path)
	free (cur_path);
      free (name_space);
    }
}

/* Return true if there are no side effects in any of the predicates in
   predicate list PRED, false if there are any. */

boolean
no_side_effects (pred)
     struct predicate *pred;
{
  while (pred != NULL)
    {
      if (pred->side_effects)
	return (false);
      pred = pred->pred_next;
    }
  return (true);
}
