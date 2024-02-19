/*
 * man.c
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
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <unistd.h>
#include "config.h"
#include "gripes.h"
#include "version.h"

#ifdef STD_HEADERS
#include <stdlib.h>
#else
extern char *malloc ();
extern char *getenv ();
extern void free ();
extern int system ();
extern int strcmp ();
extern int strncmp ();
extern int exit ();
extern int fflush ();
extern int fprintf ();
extern FILE *fopen ();
extern int fclose ();
#ifdef CHARSPRINTF
extern char *sprintf ();
#else
extern int sprintf ();
#endif
#endif

extern char *strdup ();

extern char **glob_vector ();
/*extern int access ();*/
extern int unlink ();
extern int system ();
extern int stat ();

char *prognam;
static char *zcat;
static char *pager;
static char *manp;
static char *manpathlist[MAXDIRS];
static char *section;
static int apropos;
static int whatis;
static int findall;

#ifdef ALT_SYSTEMS
static int alt_system;
static char *alt_system_name;
#endif

#ifdef HAS_TROFF
static int troff;
#endif

int debug;

int
main (argc, argv)
     int argc;
     char **argv;
{
  int status = 0;
  char *nextarg;
  char *tmp;
  extern int optind;
  extern char *mkprogname ();
  char *is_section ();
  void man_getopt ();
  void do_apropos ();
  void do_whatis ();
  int man ();

  prognam = mkprogname (argv[0]);

  man_getopt (argc, argv);

  if (optind == argc)
    gripe_no_name (NULL);

  if (optind == argc - 1)
    {
      tmp = is_section (argv[optind]);

      if (tmp != NULL)
	gripe_no_name (tmp);
    }

  while (optind < argc)
    {
      nextarg = argv[optind++];

      /*
       * See if this argument is a valid section name.  If not,
       * is_section returns NULL.
       */
      tmp = is_section (nextarg);

      if (tmp != NULL)
	{
	  section = tmp;

	  if (debug)
	    fprintf (stderr, "\nsection: %s\n", section);

	  continue;
	}

      if (apropos)
	do_apropos (nextarg);
      else if (whatis)
	do_whatis (nextarg);
      else
	{
	  status = man (nextarg);

	  if (status == 0)
	    gripe_not_found (nextarg, section);
	}
    }
  return status;
}

/*
 * Get options from the command line and user environment.
 */
void
man_getopt (argc, argv)
     register int argc;
     register char **argv;
{
  register int c;
  register char *p;
  register char *end;
  register char **mp;
  extern char *optarg;
  extern int getopt ();
  extern void downcase ();
  extern char *manpath ();
  void usage ();

#ifdef HAS_TROFF
#ifdef ALT_SYSTEMS
  while ((c = getopt (argc, argv, "M:P:S:adfhkt?")) != EOF)
#else
  while ((c = getopt (argc, argv, "M:P:adfhkt?")) != EOF)
#endif
#else
#ifdef ALT_SYSTEMS
  while ((c = getopt (argc, argv, "M:P:S:adfhk?")) != EOF)
#else
  while ((c = getopt (argc, argv, "M:P:adfhk?")) != EOF)
#endif
#endif
    {
      switch (c)
	{
	case 'M':
	  manp = strdup (optarg);
	  break;
	case 'P':
	  pager = strdup (optarg);
	  break;
#ifdef ALT_SYSTEMS
	case 'S':
	  alt_system++;
	  alt_system_name = strdup (optarg);
	  break;
#endif
	case 'a':
	  findall++;
	  break;
	case 'd':
	  debug++;
	  break;
	case 'f':
#ifdef HAS_TROFF
	  if (troff)
	    gripe_incompatible ("-f and -t");
#endif
	  if (apropos)
	    gripe_incompatible ("-f and -k");
	  whatis++;
	  break;
	case 'k':
#ifdef HAS_TROFF
	  if (troff)
	    gripe_incompatible ("-t and -k");
#endif
	  if (whatis)
	    gripe_incompatible ("-f and -k");
	  apropos++;
	  break;
#ifdef HAS_TROFF
	case 't':
	  if (apropos)
	    gripe_incompatible ("-t and -k");
	  if (whatis)
	    gripe_incompatible ("-t and -f");
	  troff++;
	  break;
#endif
	case 'h':
	case '?':
	default:
	  usage();
	  break;
	}
    }

  zcat = strdup (ZCAT);
  if (pager == NULL || *pager == NULL)
    if ((pager = getenv ("PAGER")) == NULL)
      pager = strdup (PAGER);

  if (debug)
    fprintf (stderr, "\nusing %s as pager\n", pager);

  if (manp == NULL)
    {
      if ((manp = manpath (0)) == NULL)
	gripe_manpath ();

      if (debug)
	fprintf (stderr,
		 "\nsearch path for pages determined by manpath is\n%s\n\n",
		 manp);
    }

#ifdef ALT_SYSTEMS
  if (alt_system_name == NULL || *alt_system_name == NULL)
    if ((alt_system_name = getenv ("SYSTEM")) == NULL)
      alt_system_name = strdup (alt_system_name);

  downcase (alt_system_name);
#endif

  /*
   * Expand the manpath into a list for easier handling.
   */
  mp = manpathlist;
  for (p = manp ; ; p = end+1)
    {
      if ((end = strchr (p, ':')) != NULL)
	*end = '\0';

      if (debug)
	fprintf (stderr, "adding %s to manpathlist\n", p);

#ifdef ALT_SYSTEMS
      if (alt_system)
	{
	  char buf[BUFSIZ];

	  strcpy (buf, p);
	  strcat (buf, "/");
	  strcat (buf, alt_system_name);

	  *mp++ = strdup (buf);
	}
#else
      *mp++ = strdup (p);
#endif

      if (end == NULL)
	break;
    }
  *mp = NULL;

}

void
usage ()
{
  static char usage_string[1024] = "%s, version %s\n\n";

#ifdef HAS_TROFF
#ifdef ALT_SYSTEMS
  static char s1[] =  "usage: %s [-afhkt] [section] [-M path] [-P pager] [-S system] name ...\n\n";
#else
  static char s1[] =  "usage: %s [-afhkt] [section] [-M path] [-P pager] name ...\n\n";
#endif
#else
#ifdef ALT_SYSTEMS
  static char s1[] =  "usage: %s [-afhk] [section] [-M path] [-P pager] [-S system] name ...\n\n";
#else
  static char s1[] =  "usage: %s [-afhk] [section] [-M path] [-P pager] name ...\n\n";
#endif
#endif

static char s2[] = "  a : find all matching entries\n\
  d : print gobs of debugging information\n\
  f : same as whatis(1)\n\
  h : print this help message\n\
  k : same as apropos(1)\n";

#ifdef HAS_TROFF
  static char s3[] = "  t : use troff to format pages for printing\n";
#endif

  static char s4[] = "\n  M path   : set search path for manual pages to `path'\n\
  P pager  : use program `pager' to display pages\n";

#ifdef ALT_SYSTEMS
  static char s5[] = "  S system : search for alternate system's man pages\n";
#endif

  strcat (usage_string, s1);
  strcat (usage_string, s2);

#ifdef HAS_TROFF
  strcat (usage_string, s3);
#endif

  strcat (usage_string, s4);

#ifdef ALT_SYSTEMS
  strcat (usage_string, s5);
#endif

  fprintf (stderr, usage_string, prognam, version, prognam);
  exit(1);
}

/*
 * Check to see if the argument is a valid section number.  If the
 * first character of name is a numeral, or the name matches one of
 * the sections listed in config.h, we'll assume that it's a section.
 * The list of sections in config.h simply allows us to specify oddly
 * named directories like .../man3f.  Yuk. 
 */
char *
is_section (name)
     register char *name;
{
  register char **vs;

  for (vs = valid_sections; *vs != NULL; vs++)
    if ((strcmp (*vs, name) == NULL) || (isdigit (name[0])))
      return strdup (name);

  return NULL;
}

/*
 * Handle the apropos option.  Cheat by using another program.
 */
void
do_apropos (name)
     register char *name;
{
  int status;
  register int len;
  register char *command;

  len = strlen (APROPOS) + strlen (name) + 2;

  if ((command = malloc(len)) == NULL)
    gripe_alloc (len, "command");

  sprintf (command, "%s %s", APROPOS, name);

  status = 0;
  if (debug)
    fprintf (stderr, "\ntrying command: %s\n", command);
  else
    status = system (command);

  if (status == 127)
    gripe_system_command (status);

  free (command);
}

/*
 * Handle the whatis option.  Cheat by using another program.
 */
void
do_whatis (name)
     register char *name;
{
  int status;
  register int len;
  register char *command;

  len = strlen (WHATIS) + strlen (name) + 2;

  if ((command = malloc(len)) == NULL)
    gripe_alloc (len, "command");

  sprintf (command, "%s %s", WHATIS, name);

  status = 0;
  if (debug)
    fprintf (stderr, "\ntrying command: %s\n", command);
  else
    status = system (command);

  if (status == 127)
    gripe_system_command (status);

  free (command);
}

/*
 * Search for manual pages.
 *
 * If preformatted manual pages are supported, look for the formatted
 * file first, then the man page source file.  If they both exist and
 * the man page source file is newer, or only the source file exists,
 * try to reformat it and write the results in the cat directory.  If
 * it is not possible to write the cat file, simply format and display
 * the man file.
 *
 * If preformatted pages are not supported, or the troff option is
 * being used, only look for the man page source file.
 *
 */
int
man (name)
     char *name;
{
  register int found;
  register int glob;
  register char **mp;
  register char **sp;
  int try_section ();

  found = 0;

  fflush (stdout);
  if (section != NULL)
    {
      for (mp = manpathlist; *mp != NULL; mp++)
	{
	  if (debug)
	    fprintf (stderr, "\nsearching in %s\n", *mp);

	  glob = 0;

	  found += try_section (*mp, section, name, glob);

	  if (found && !findall)   /* i.e. only do this section... */
	    return found;
	}
    }
  else
    {
      for (sp = valid_sections; *sp != NULL; sp++)
	{
	  for (mp = manpathlist; *mp != NULL; mp++)
	    {
	      if (debug)
		fprintf (stderr, "\nsearching in %s\n", *mp);

	      glob = 1;

	      found += try_section (*mp, *sp, name, glob);

	      if (found && !findall)   /* i.e. only do this section... */
		return found;

	    }
	}
    }
  return found;
}

/*
 * See if the preformatted man page or the source exists in the given
 * section.
 */
int
try_section (path, section, name, glob)
     register char *path;
     register char *section;
     register char *name;
     register int glob;
{
  register int found = 0;
  register int to_cat;
  register int cat;
  register char **names;
  register char **np;
  char **glob_for_file ();
  char **make_name ();
  char *convert_name ();
  char *ultimate_source ();
  int display_cat_file ();
  int format_and_display ();

  if (debug)
    {
      if (glob)
	fprintf (stderr, "trying section %s with globbing\n", section);
      else
	fprintf (stderr, "trying section %s without globbing\n", section);
    }

  /*
   * Look for man page source files.
   */
  cat = 0;
  if (glob)
    names = glob_for_file (path, section, name, cat);
  else
    names = make_name (path, section, name, cat);

  if ((int) names == -1 || *names == NULL)
    /*
     * No files match.  If we're not using troff and we're supporting
     * preformatted pages, see if there's one around that we can
     * display. 
     */
    {
#ifdef HAS_TROFF
      if (cat_support && !troff)
#else
      if (cat_support)
#endif
	{
	  cat = 1;
	  if (glob)
	    names = glob_for_file (path, section, name, cat);
	  else
	    names = make_name (path, section, name, cat);

	  if ((int) names != -1 && *names != NULL)
	    {
	      for (np = names; *np != NULL; np++)
		found+= display_cat_file (*np);
	    }
	}
    }
  else
    {
      for (np = names; *np != NULL; np++)
	{
	  register char *cat_file = NULL;
	  register char *man_file;

	  man_file = ultimate_source (*np, path);

#ifdef HAS_TROFF
	  if (cat_support && !troff)
#else
	  if (cat_support)
#endif
	    {
	      to_cat = 1;

	      cat_file = convert_name (man_file, to_cat);
	      if (debug)
		fprintf (stderr, "will try to write %s if needed\n", cat_file);
	    }

	  found += format_and_display (path, man_file, cat_file);
	}
    }

  return found;
}

/*
 * Change a name of the form ...man/man1/name.1 to ...man/cat1/name.1
 * or a name of the form ...man/cat1/name.1 to ...man/man1/name.1
 */
char *
convert_name (name, to_cat)
     register char *name;
     register int to_cat;
{
  register char *to_name;
  register char *t1;
  register char *t2 = NULL;

  to_name = strdup (name);

  t1 = strrchr (to_name, '/');
  if (t1 != NULL)
    {
      *t1 = NULL;
      t2 = strrchr (to_name, '/');
      *t1 = '/';
    }

  if (t2 == NULL)
    gripe_converting_name (name, to_cat);

  if (to_cat)
    {
      *(++t2) = 'c';
      *(t2+2) = 't';
    }
  else
    {
      *(++t2) = 'm';
      *(t2+2) = 'n';
    }

  return to_name;
}


/*
 * Try to find the man page corresponding to the given name.  The
 * reason we do this with globbing is because some systems have man
 * page directories named man3 which contain files with names like
 * XtPopup.3Xt.  Rather than requiring that this program know about
 * all those possible names, we simply try to match things like
 * .../man[sect]/name[sect]*.  This is *much* easier.
 *
 * Note that globbing is only done when the section is unspecified.
 */
char **
glob_for_file (path, section, name, cat)
     register char *path;
     register char *section;
     register char *name;
     register int cat;
{
  char pathname[BUFSIZ];
  char **glob_filename ();
  char **gf;

  if (cat)
    sprintf (pathname, "%s/cat%s/%s.%s*", path, section, name, section);
  else
    sprintf (pathname, "%s/man%s/%s.%s*", path, section, name, section);

  if (debug)
    fprintf (stderr, "globbing %s\n", pathname);

  gf = glob_filename (pathname);

  if (((int) gf == -1 || *gf == NULL) && isdigit (*section))
    {
      if (cat)
	sprintf (pathname, "%s/cat%s/%s.%c*", path, section, name, *section);
      else
	sprintf (pathname, "%s/man%s/%s.%c*", path, section, name, *section);

      gf = glob_filename (pathname);
    }
  return gf;
}

/*
 * Return an un-globbed name in the same form as if we were doing
 * globbing. 
 */
char **
make_name (path, section, name, cat)
     register char *path;
     register char *section;
     register char *name;
     register int cat;
{
  register int i = 0;
  static char *names[3];
  char buf[BUFSIZ];

  if (cat)
    sprintf (buf, "%s/cat%s/%s.%s", path, section, name, section);
  else
    sprintf (buf, "%s/man%s/%s.%s", path, section, name, section);

  if (access (buf, R_OK) == 0)
    names[i++] = strdup (buf);
  else {
	if (cat) {
    	sprintf (buf, "%s/cat%s/%s.%s.Z", path, section, name, section);
		if (access (buf, R_OK) == 0)
			names[i++] = strdup (buf);
	}
  }

  /*
   * If we're given a section that looks like `3f', we may want to try
   * file names like .../man3/foo.3f as well.  This seems a bit
   * kludgey to me, but what the hey...
   */
  if (section[1] != '\0')
    {
      if (cat)
	sprintf (buf, "%s/cat%c/%s.%s", path, section[0], name, section);
      else
	sprintf (buf, "%s/man%c/%s.%s", path, section[0], name, section);

      if (access (buf, R_OK) == 0)
	names[i++] = strdup (buf);
    }

  names[i] = NULL;

  return &names[0];
}

/*
 * Simply display the preformmated page.
 */
int
display_cat_file (file)
     register char *file;
{
  int status;
  register int found;
  char command[BUFSIZ];

  found = 0;

  if (access (file, R_OK) == NULL)
    {
	  if (file[strlen(file)-1] == 'Z') {
      	sprintf ( command, "%s %s | %s", zcat, file, pager );
	  }
	  else {
      	sprintf (command, "%s %s", pager, file);
	  }

      status = 0;
      if (debug)
	fprintf (stderr, "\ntrying command: %s\n", command);
      else
	status = system (command);

      if (status == 127)
	gripe_system_command (status);
      else
	found++;
    }
  return found;
}

/*
 * Try to find the ultimate source file.  If the first line of the
 * current file is not of the form
 *
 *      .so man3/printf.3s
 *
 * the input file name is returned.
 */
char *
ultimate_source (name, path)
     char *name;
     char *path;
{
  FILE *fp;
  char buf[BUFSIZ];
  char ult[BUFSIZ];
  char *beg;
  char *end;

  strcpy (ult, name);
  strcpy (buf, name);

 next:

  if ((fp = fopen (ult, "r")) == NULL)
    return buf;

  if (fgets (buf, BUFSIZ, fp) == NULL)
    return ult;

  if (strlen (buf) < 5)
    return ult;

  beg = buf;
  if (*beg++ == '.' && *beg++ == 's' && *beg++ == 'o')
    {
      while ((*beg == ' ' || *beg == '\t') && *beg != '\0')
	beg++;

      end = beg;
      while (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\0')
	end++;

      *end = '\0';

      strcpy (ult, path);
      strcat (ult, "/");
      strcat (ult, beg);

      strcpy (buf, ult);

      goto next;
    }

  if (debug)
    fprintf (stderr, "found ultimate source file %s\n", ult);

  return ult;
}

/*
 * Try to format the man page source and save it, then display it.  If
 * that's not possible, try to format the man page source and display
 * it directly.
 *
 * Note that in the commands below, the cd is necessary because some
 * man pages are one liners like my version of sprintf.3s:
 *
 *      .so man3/printf.3s
 */
int
format_and_display (path, man_file, cat_file)
     register char *path;
     register char *man_file;
     register char *cat_file;
{
  int status;
  int mode;
  register int found;
  FILE *fp;
  char command[BUFSIZ];
  int is_newer ();

  found = 0;

  if (access (man_file, R_OK) != 0)
    return found;
  
#ifdef HAS_TROFF
  if (troff || !cat_support)
    {
      if (troff)
	sprintf (command, "(cd %s ; %s %s)", path, troff_command, man_file);
      else
	sprintf (command, "(cd %s ; %s %s | %s)", path, nroff_command,
		 man_file, pager); 
#else
  if (!cat_support)
    {
      sprintf (command, "(cd %s ; %s %s | %s)", path, nroff_command,
	       man_file, pager); 
#endif
      status = 0;
      if (debug)
	fprintf (stderr, "\ntrying command: %s\n", command);
      else
	status = system (command);

      if (status == 127)
	gripe_system_command (status);
      else
	found++;
    }
  else
    {
      if ((status = is_newer (man_file, cat_file)) == 1 || status == -2)
	{
	  if ((fp = fopen (cat_file, "w")) != NULL)
	    {
	      fclose (fp);
	      unlink (cat_file);

	      fprintf (stderr, "Formatting page, please wait...\n");

	      sprintf (command, "(cd %s ; %s %s > %s)", path,
		       nroff_command, man_file, cat_file);

	      signal (SIGINT, SIG_IGN);

	      status = 0;
	      if (debug)
		fprintf (stderr, "\ntrying command: %s\n", command);
	      else
		status = system (command);

	      if (status == 127)
		gripe_system_command (status);
	      else
		found++;
	      
	      mode = CATMODE;
	      chmod (cat_file, mode);

	      if (debug)
		fprintf (stderr, "mode of %s is now %o\n", cat_file, mode);

	      signal (SIGINT, SIG_DFL);

	      found = display_cat_file (cat_file);
	    }
	  else
	    {
	      sprintf (command, "(cd %s ; %s %s | %s)", path, nroff_command,
		       man_file, pager); 

	      status = 0;
	      if (debug)
		fprintf (stderr, "\ntrying command: %s\n", command);
	      else
		status = system (command);

	      if (status == 127)
		gripe_system_command (status);
	      else
		found++;
	    }
	}
      else if (access (cat_file, R_OK) == 0)
	{
	  found = display_cat_file (cat_file);
	}
    }
  return found;
}

/*
 * Is file a newer than file b?
 *
 * case:
 *
 *   a newer than b         returns    1
 *   a older than b         returns    0
 *   stat on a fails        returns   -1
 *   stat on b fails        returns   -2
 *   stat on a and b fails  returns   -3
 */
int
is_newer (fa, fb)
  register char *fa;
  register char *fb;
{
  struct stat fa_sb;
  struct stat fb_sb;
  register int fa_stat;
  register int fb_stat;
  register int status = 0;

  fa_stat = stat (fa, &fa_sb);
  if (fa_stat != 0)
    status = 1;

  fb_stat = stat (fb, &fb_sb);
  if (fb_stat != 0)
    status |= 2;

  if (status != 0)
    return -status;

  if (fa_sb.st_mtime > fb_sb.st_mtime)
    {
      status = 1;

      if (debug)
	fprintf (stderr, "%s is newer than %s\n", fa, fb);
    }
  else
    {
      status = 0;

      if (debug)
	fprintf (stderr, "%s is older than %s\n", fa, fb);
    }
  return status;
}
