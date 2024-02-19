/*
 * manpath.c
 *
 * Copyright (c) 1991, John W. Eaton.
 *
 * You may distribute under the terms of the GNU General Public
 * License as specified in the README file that comes with the man 1.0
 * distribution.  
 *
 * John W. Eaton
 * jwe@che.utexas.edu
 * Department of Chemical Engineering
 * The University of Texas at Austin
 * Austin, Texas  78712
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "config.h"
#include "manpath.h"
#include "gripes.h"

#ifdef STD_HEADERS
#include <stdlib.h>
#else
extern int fprintf ();
extern int strcmp ();
extern int strncmp ();
extern char *getenv();
extern char *malloc();
extern void free ();
extern int exit ();
#endif

extern char *strdup ();

#ifndef MAIN
extern int debug;
#endif

#ifdef MAIN

#ifndef __ANSI__
extern char *strcpy ();
extern int fflush ();
#endif

char *prognam;
int debug;

/*
 * Examine user's PATH and print a reasonable MANPATH.
 */
int
main(argc, argv)
     int argc;
     char **argv;
{
  int c;
  int quiet;
  char *mp;
  extern int getopt ();
  extern char *mkprogname ();
  void usage ();
  char *manpath ();

  quiet = 1;

  prognam = mkprogname (argv[0]);

  while ((c = getopt (argc, argv, "dhq?")) != EOF)
    {
      switch (c)
	{
	case 'd':
	  debug++;
	  break;
	case 'q':
	  quiet = 0;
	  break;
	case '?':
	case 'h':
	default:
	  usage();
          break;
	}
    }

  mp = manpath (quiet);

  fprintf (stdout, "%s\n", mp);
  fflush (stdout);

  return 0;
}

void
usage ()
{
  fprintf (stderr, "usage: %s [-q]\n", prognam);
  exit (1);
}
#endif /* MAIN */

/*
 * If the environment variable MANPATH is set, return it.
 * If the environment variable PATH is set and has a nonzero length,
 * try to determine the corresponding manpath, otherwise, return the
 * default manpath.
 *
 * The manpath.config file is used to map system wide /bin directories
 * to top level man page directories.
 *
 * For directories which are in the user's path but not in the
 * manpath.config file, see if there is a subdirectory `man' or `MAN'.
 * If so, add that directory to the path.  Example:  user has
 * $HOME/bin in his path and the directory $HOME/bin/man exists -- the
 * directory $HOME/bin/man will be added to the manpath.
 */
char *
manpath (perrs)
     register int perrs;
{
  register int len;
  register char *manpathlist;
  register char *path;
  int  get_dirlist ();
  char *def_path ();
  char *get_manpath ();

  if (get_dirlist ())
      gripe_reading_mp_config ();

  if ((manpathlist = getenv ("MANPATH")) != NULL)
    /*
     * This must be it.
     */
    {
      if (perrs)
	fprintf (stderr, "(Warning: MANPATH environment variable set)\n");
      return manpathlist;
    }
  else if ((path = getenv ("PATH")) == NULL)
    /*
     * Things aren't going to work well, but hey...
     */
    {
      if (perrs)
	fprintf (stderr, "Warning: path not set\n");
      return def_path (perrs);
    }
  else
    {
      if ((len = strlen (path)) == 0)
	/*
	 * Things aren't going to work well here either...
	 */
	{
	  if (perrs)
	    fprintf (stderr, "Warning: path set but has zero length\n");
	  return def_path (perrs);
	}
      return get_manpath (perrs, path);
    }
}

/*
 * Get the list of bin directories and the corresponding man
 * directories from the manpath.config file.
 *
 * This is ugly.
 */
int
get_dirlist ()
{
  int i;
  char *bp;
  char *p;
  char buf[BUFSIZ];
  DIRLIST *dlp = list;
  FILE *config;

  if ((config = fopen (config_file, "r")) == NULL)
    gripe_getting_mp_config (config_file);

  while ((bp = fgets (buf, BUFSIZ, config)) != NULL)
    {
      while (*bp && (*bp == ' ' || *bp == '\t'))
	bp++;

      if (*bp == '#' || *bp == '\n')
	continue;

      if (!strncmp ("MANBIN", bp, 6))
	continue;

      if (!strncmp ("MANDATORY_MANPATH", bp, 17))
	{
	  if ((p = strchr (bp, ' ')) == NULL)
	    if ((p = strchr (bp, '\t')) == NULL)
	      return -1;

	  bp = p;

	  dlp->mandatory = 1;

	  while (*bp && *bp != '\n' && (*bp == ' ' || *bp == '\t'))
	    bp++;

	  i = 0;
	  while (*bp && *bp != '\n' && *bp != ' ' && *bp != '\t')
	    dlp->mandir[i++] = *bp++;
	  dlp->mandir[i] = '\0';

	  if (debug)
	    fprintf (stderr, "found mandatory man directory %s\n",
		     dlp->mandir);
	}
      else if (!strncmp ("MANPATH_MAP", bp, 11))
	{
	  if ((p = strchr (bp, ' ')) == NULL)
	    if ((p = strchr (bp, '\t')) == NULL)
	      return -1;

	  bp = p;

	  dlp->mandatory = 0;

	  while (*bp && *bp != '\n' && (*bp == ' ' || *bp == '\t'))
	    bp++;

	  i = 0;
	  while (*bp && *bp != '\n' && *bp != ' ' && *bp != '\t')
	    dlp->bin[i++] = *bp++;
	  dlp->bin[i] = '\0';

	  while (*bp && *bp != '\n' && (*bp == ' ' || *bp == '\t'))
	    bp++;

	  i = 0;
	  while (*bp && *bp != '\n' && *bp != ' ' && *bp != '\t')
	    dlp->mandir[i++] = *bp++;
	  dlp->mandir[i] = '\0';

	  if (debug)
	    fprintf (stderr, "found manpath map %s --> %s\n",
		     dlp->bin, dlp->mandir);
	}
      else
	{
	  gripe_reading_mp_config ();
	}
      dlp++;
    }

  dlp->bin[0] = NULL;
  dlp->mandir[0] = NULL;
  dlp->mandatory = 0;

  return 0;
}

/*
 * Construct the default manpath.  This picks up mandatory manpaths
 * only.
 */
char *
def_path (perrs)
     int perrs;
{
  register int len;
  register char *manpathlist;
  register DIRLIST *dlp;

  len = 0;
  dlp = list;
  while (dlp->mandatory != 0)
    {
      len += strlen (dlp->mandir) + 1;
      dlp++;
    }

  if ((manpathlist = malloc (len)) == NULL)
    {
      fprintf (stderr, "Fatal: can't malloc %d bytes for manpathlist\n", len);
      exit (1);
    }
  *manpathlist = '\0';

  dlp = list;
  while (dlp->mandatory != 0)
    {
      strcat (manpathlist, dlp->mandir);
      strcat (manpathlist, ":");
      dlp++;
    }

  manpathlist[len-1] = '\0';

  return manpathlist;
}

/*
 * For each directory in the user's path, see if it is one of the
 * directories listed in the manpath.config file.  If so, and it is
 * not already in the manpath, add it.  If the directory is not listed
 * in the manpath.config file, see if there is a subdirectory `man' or
 * `MAN'.  If so, and it is not already in the manpath, add it.
 * Example:  user has $HOME/bin in his path and the directory
 * $HOME/bin/man exists -- the directory $HOME/bin/man will be added
 * to the manpath.
 */
char *
get_manpath (perrs, path)
     register int perrs;
     register char *path;
{
  register int len;
  register char *tmppath;
  register char *t;
  register char *p;
  register char **lp;
  register char *end;
  register char *manpathlist;
  register DIRLIST *dlp;
  void add_dir_to_list ();
  char *has_subdirs ();

  tmppath = strdup (path);

  for (p = tmppath; ; p = end+1)
    {
      if (end = strchr(p, ':'))
	*end = '\0';

      if (debug)
	fprintf (stderr, "\npath directory %s ", p);

      for (dlp = list; dlp->mandir[0] != NULL; dlp++)
	if (dlp->bin[0] != NULL && !strcmp (p, dlp->bin))
	  {
	    /*
	     * The directory we're working on is in the config file.
	     * If we haven't added it to the list yet, do.
	     */
	    if (debug)
	      fprintf (stderr, "is in the config file\n");

	    add_dir_to_list (tmplist, dlp->mandir);
	    goto found;
	  }
      /*
       * The directory we're working on isn't in the config file.  See
       * if it has man or MAN subdirectories.  If so, and it hasn't
       * been added to the list, do.
       */
      if (debug)
	fprintf (stderr, "is not in the config file\n");

      t = has_subdirs (p);
      if (t != NULL)
	{
	  if (debug)
	    fprintf (stderr, "but it does have a man or MAN subdirectory\n");

	  add_dir_to_list (tmplist, t);
	  free (t);
	}
      else
	{
	  if (debug)
	    fprintf (stderr, "and doesn't have man or MAN subdirectories\n");
	}

    found:

      if (!end)
	break;
    }

  if (debug)
    fprintf (stderr, "\nadding mandatory man directories\n\n");

  dlp = list;
  while (dlp->mandatory != 0)
    {
      add_dir_to_list (tmplist, dlp->mandir);
      dlp++;
    }

  len = 0;
  lp = tmplist;
  while (*lp != NULL)
    {
      len += strlen (*lp) + 1;
      lp++;
    }

  if ((manpathlist = malloc (len)) == NULL)
    {
      fprintf (stderr, "Fatal: can't malloc %d bytes for manpathlist\n", len);
      exit (1);
    }
  *manpathlist = '\0';

  lp = tmplist;
  while (*lp != NULL)
    {
      strcat (manpathlist, *lp);
      strcat (manpathlist, ":");
      lp++;
    }

  manpathlist[len-1] = '\0';

  return manpathlist;
}

/*
 * Add a directory to the manpath list if it isn't already there.
 */
void
add_dir_to_list (lp, dir)
     char **lp;
     char *dir;
{
  extern char *strdup ();

  while (*lp != NULL)
    {
      if (!strcmp (*lp, dir))
	{
	  if (debug)
	    fprintf (stderr, "%s is already in the manpath\n", dir);
	  return;
	}
      lp++;
    }
  /*
   * Not found -- add it.
   */
  if (debug)
    fprintf (stderr, "adding %s to manpath\n", dir);

  *lp = strdup (dir);
}

/*
 * Check to see if the current directory has man or MAN
 * subdirectories. 
 */
char *
has_subdirs (p)
     register char *p;
{
  int len;
  register char *t;
  register struct stat sb;
  extern int stat ();

  len = strlen (p);

  t = malloc ((unsigned) len + 5);

  if (t == 0)
    gripe_alloc (len, "p\n");

  strcpy (t, p);
  strcat (t, "/man");

  if (stat (t, &sb) == 0 && (sb.st_mode & S_IFDIR) == S_IFDIR)
    return t;

  strcpy (t, p);
  strcat (t, "/MAN");
  
  if (stat (t, &sb) == 0 && (sb.st_mode & S_IFDIR) == S_IFDIR)
    return t;

  return NULL;
}
