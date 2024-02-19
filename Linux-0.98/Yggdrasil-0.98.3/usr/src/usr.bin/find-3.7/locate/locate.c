/* locate -- list files in databases that match a pattern

   Usage: locate pattern...
   Searches a pre-computed file list constructed nightly by cron.
   Its effect is similar to, but much faster than,
   find / -mtime +0 \( -name "*pattern1*" -o -name "*pattern2*" ... \) -print

   `locate' scans a file list for the full pathname of a file
   given only a piece of the name.  The list has been processed with
   "front-compression" and bigram coding.  Front compression reduces
   space by a factor of 4-5, bigram coding by a further 20-25%.

   The codes are:

   0-28		likeliest differential counts + offset to make nonnegative
   30		escape code for out-of-range count to follow in next word
   128-255 	bigram codes (128 most common, as determined by 'updatedb')
   32-127  	single character (printable) ASCII residue

   A novel two-tiered string search technique is employed:

   First, a metacharacter-free subpattern and partial pathname is
   matched BACKWARDS to avoid full expansion of the pathname list.
   The time savings is 40-50% over forward matching, which cannot efficiently
   handle overlapped search patterns and compressed path residue.

   Then, the actual shell glob-style regular expression (if in this form)
   is matched against the candidate pathnames using the slower standard
   shell filename matching routines.

   Author: James A. Woods (jaw@riacs.edu)
   Modified by David MacKenzie (djm@gnu.ai.mit.edu)
   Public domain. */

#include <stdio.h>
#include <fnmatch.h>
#include <getopt.h>
#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#define index strchr
#else
#include <strings.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#else
char *getenv ();
#endif

#ifdef STDC_HEADERS
#include <errno.h>
#include <stdlib.h>
#else
extern int errno;
#endif
#include <sys/types.h>
#include "pathmax.h"

#define	TRUE	1
#define	FALSE	0

#define	OFFSET	14

#define	ESCCODE	30

char *next_element ();
char *xmalloc ();
char *patprep ();
void error ();

/* Print the entries in DBFILE that match PATHPART.
   Return the number of entries printed.  */

int
locate (pathpart, dbfile)
     char *pathpart, *dbfile;
{
  register char *p, *s;
  register int c;
  char *q;
  int i, count = 0, globflag;
  FILE *fp;
  char *patend, *cutoff;
  char *path;
  int path_max;
  char bigram1[128], bigram2[128];
  int found = FALSE;
  int printed = 0;

  fp = fopen (dbfile, "r");
  if (fp == NULL)
    {
      error (0, errno, "%s", dbfile);
      return 0;
    }

  path_max = PATH_MAX;
  path = xmalloc (path_max + 2);

  for (i = 0; i < 128; i++)
    {
      bigram1[i] = getc (fp);
      bigram2[i] = getc (fp);
    }

  globflag = index (pathpart, '*') || index (pathpart, '?')
    || index (pathpart, '[');
  patend = patprep (pathpart);

  c = getc (fp);
  while (c != EOF)
    {
      count += ((c == ESCCODE) ? getw (fp) : c) - OFFSET;

      for (p = path + count; (c = getc (fp)) > ESCCODE;)
	/* Overlay old path. */
	if (c < 0200)
	  *p++ = c;
	else
	  {
	    /* Bigrams are parity-marked. */
	    *p++ = bigram1[c & 0177];
	    *p++ = bigram2[c & 0177];
	  }
      *p-- = '\0';
      cutoff = path;
      if (!found)
	cutoff += count;

      for (found = FALSE, s = p; s >= cutoff; s--)
	if (*s == *patend)
	  {
	    /* Fast first char check. */
	    for (p = patend - 1, q = s - 1; *p != '\0'; p--, q--)
	      if (*q != *p)
		break;
	    if (*p == '\0')
	      {
		/* Success on fast match. */
		found = TRUE;
		if (globflag == FALSE || fnmatch (pathpart, path, 0) == 0)
		  {
		    puts (path);
		    ++printed;
		  }
		break;
	      }
	  }
    }
  return printed;
}

/* Extract the last glob-free subpattern in NAME for fast pre-match;
   prepend '\0' for backwards match; return the end of the new pattern. */

char *
patprep (name)
     char *name;
{
  static char globfree[100];
  register char *subp = globfree;
  register char *p, *endmark;

  *subp++ = '\0';
  p = name + strlen (name) - 1;
  /* Skip trailing metacharacters (and [] ranges). */
  for (; p >= name; p--)
    if (index ("*?", *p) == 0)
      break;
  if (p < name)
    p = name;
  if (*p == ']')
    for (p--; p >= name; p--)
      if (*p == '[')
	{
	  p--;
	  break;
	}
  if (p < name)
    p = name;

  /* If pattern has only metacharacters,
     check every path (force '/' search). */
  if (p == name && index ("?*[]", *p) != 0)
    *subp++ = '/';
  else
    {
      for (endmark = p; p >= name; p--)
	if (index ("]*?", *p) != 0)
	  break;
      for (++p; p <= endmark && subp < (globfree + sizeof (globfree));)
	*subp++ = *p++;
    }
  *subp-- = '\0';

  return subp;
}

/* The name this program was run with. */
char *program_name;

void
usage ()
{
  fprintf (stderr, "Usage: %s [-d path] [--database=path] pattern...\n",
	   program_name);
  exit (1);
}

struct option longopts[] =
{
  {"database", 1, NULL, 'd'},
  {NULL, 0, NULL, 0}
};

void
main (argc, argv)
     int argc;
     char **argv;
{
  char *dbpath, *e;
  int found = 0, optc;

  program_name = argv[0];

  dbpath = getenv ("LOCATE_PATH");
  if (dbpath == NULL)
    dbpath = LOCATE_DB;

  while ((optc = getopt_long (argc, argv, "d:", longopts, (int *) 0)) != -1)
    switch (optc)
      {
      case 'd':
	dbpath = optarg;
	break;

      default:
	usage ();
      }

  if (optind == argc)
    usage ();

  for (; optind < argc; optind++)
    {
      next_element (dbpath);	/* Initialize.  */
      while (e = next_element ((char *) NULL))
	found |= locate (argv[optind], e);
    }

  exit (!found);
}
