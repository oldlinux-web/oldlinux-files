/* bigram -- list bigrams for locate

   Usage: bigram < text > bigrams
   
   Use 'code' to encode a file using this output.

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

/* The name this program was run with.  */
char *program_name;

char *path;

char *oldpath;

void
main (argc, argv)
     int argc;
     char **argv;
{
  register int count, j;
  unsigned line_length;
  int path_max;

  program_name = argv[0];
  path_max = PATH_MAX;
  path = xmalloc (path_max + 2);
  oldpath = xmalloc (path_max + 2);
  path[path_max] = '\0';
  strcpy (oldpath, " ");

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

      count = prefix_length (oldpath, path);
      /* Output post-residue bigrams only. */
      for (j = count; path[j] != '\0'; j += 2)
	{
	  if (path[j + 1] == '\0')
	    break;
	  putchar (path[j]);
	  putchar (path[j + 1]);
	  putchar ('\n');
	}
      strcpy (oldpath, path);
    }
  exit (0);
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
