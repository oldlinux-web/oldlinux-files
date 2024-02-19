/* code -- code filenames for locate

   Compress a sorted list.
   Works with 'find' to encode a filename database.

   Usage:

   bigram < list > bigrams
   process-bigrams > common_bigrams
   code common_bigrams < list > squeezed_list

   Uses 'front compression' (see ";login:", March 1983, p. 8).
   Output format is, per line, an offset differential count byte
   followed by a partially bigram-encoded ASCII residue.
   
   The codes are:
   
   0-28		likeliest differential counts + offset to make nonnegative
   30		escape code for out-of-range count to follow in next word
   128-255 	bigram codes (128 most common, as determined by 'updatedb')
   32-127  	single character (printable) ASCII residue

   Author: James A. Woods (jaw@riacs.edu)
   Modified by David MacKenzie (djm@ai.mit.edu)
   Public domain. */

#include <stdio.h>
#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#include <sys/types.h>
#include "pathmax.h"

char *xmalloc ();
int prefix_length ();
int strindex ();

/* Switch code. */
#define	RESET	30

/* The name this program was run with.  */
char *program_name;

char *path;

char *oldpath;

char bigrams[257] = {0};

void
main (argc, argv)
     int argc;
     char *argv[];
{
  int count, oldcount, diffcount;
  int j, code;
  char bigram[3];
  FILE *fp;
  unsigned line_length;
  int path_max;

  program_name = argv[0];
  oldcount = 0;
  bigram[2] = '\0';

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s common_bigrams < list > coded_list\n",
	       argv[0]);
      exit (2);
    }

  fp = fopen (argv[1], "r");
  if (fp == NULL)
    {
      fprintf (stderr, "%s: ", argv[0]);
      perror (argv[1]);
      exit (1);
    }

  path_max = PATH_MAX;
  path = xmalloc (path_max + 2);
  oldpath = xmalloc (path_max + 2);
  path[path_max] = '\0';
  strcpy (oldpath, " ");

  fgets (bigrams, 257, fp);
  fwrite (bigrams, 1, 256, stdout);

  while (fgets (path, path_max, stdin) != NULL)
    {
      line_length = strlen (path);
      if (line_length == 0)
	fprintf (stderr, "%s: null line in input\n", argv[0]);
      else if (path[line_length - 1] != '\n')
	fprintf (stderr, "%s: long line in input; truncating to `%s'\n",
		 argv[0], path);
      else
	path[line_length - 1] = '\0'; /* Remove newline. */

      /* Squelch unprintable chars so as not to botch decoding. */
      for (j = 0; path[j] != '\0'; j++)
	{
	  path[j] &= 0177;
	  if (path[j] < 040 || path[j] == 0177)
	    path[j] = '?';
	}
      count = prefix_length (oldpath, path);
      diffcount = count - oldcount;
      if (diffcount < -14 || diffcount > 14)
	{
	  putc (RESET, stdout);
	  putw (diffcount + 14, stdout);
	}
      else
	putc (diffcount + 14, stdout);

      for (j = count; path[j] != '\0'; j += 2)
	{
	  if (path[j + 1] == '\0')
	    {
	      putchar (path[j]);
	      break;
	    }
	  bigram[0] = path[j];
	  bigram[1] = path[j + 1];
	  /* Linear search for specific bigram in string table. */
	  code = strindex (bigrams, bigram);
	  if (code % 2 == 0)
	    putchar ((code / 2) | 0200);
	  else
	    fputs (bigram, stdout);
	}
      strcpy (oldpath, path);
      oldcount = count;
    }
  exit (0);
}

/* Return location of PATTERN in STRING or -1. */

int
strindex (string, pattern)
     char *string, *pattern;
{
  register char *s, *p, *q;

  for (s = string; *s != '\0'; s++)
    if (*s == *pattern)
      {
	/* Fast first char check. */
	for (p = pattern + 1, q = s + 1; *p != '\0'; p++, q++)
	  if (*q != *p)
	    break;
	if (*p == '\0')
	  return q - strlen (pattern) - string;
      }
  return -1;
}

/* Return length of longest common prefix of strings S1 and S2. */

int
prefix_length (s1, s2)
     char *s1, *s2;
{
  register char *start;

  for (start = s1; *s1 == *s2; s1++, s2++)
    if (*s1 == '\0')
      break;
  return s1 - start;
}
