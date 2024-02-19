/* tput -- shell-level interface to terminfo, emulated by termcap.
   Copyright (C) 1989 Free Software Foundation, Inc.

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

/* Usage: tput [-T termtype] [+terminal=termtype] capability [parameter...]

   Options:
   -T termtype
   +terminal=termtype	Override $TERM.

   Link with -ltermcap.
   Requires the GNU termcap library.
   Also requires the bsearch library function.

   David MacKenzie <djm@ai.mit.edu> */

#include <stdio.h>
#include <signal.h>
#ifdef linux
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#else
#include <sgtty.h>
char *bsearch ();
char *getenv ();
#endif
#include <termcap.h>
#include <getopt.h>
#include "tput.h"

#ifndef SIGTYPE
#define SIGTYPE void
#endif

/* Exit codes. */
#define CAP_PRESENT 0		/* Normal operation. */
#define BOOLEAN_FALSE 1		/* Boolean capability not present. */
#define USAGE_ERROR 2		/* Invalid arguments given. */
#define UNKNOWN_TERM 3		/* $TERM not found in termcap file. */
#define MISSING_CAP 4		/* Termcap entry lacks this capability. */
#define ERROR_EXIT 5		/* Real error or signal. */


int info_compare ();
SIGTYPE signal_handler ();
int tcputchar ();
struct conversion *find_info ();
void put_bool ();
void put_num ();
void put_str ();
void restore_translations ();
void translations_off ();
void setup_termcap ();
void usage ();

/* The name this program was run with, for error messages. */
char *program_name;

struct option long_options[] =
{
  {"terminal", 1, NULL, 0},
  {NULL, 0, NULL, 0}
};

void
main (argc, argv)
     int argc;
     char **argv;
{
  struct conversion *conv;	/* Conversion table entry. */
  char *term;			/* Terminal type. */
  int c;			/* Option character. */

  program_name = argv[0];
  term = getenv ("TERM");

  while ((c = getopt_long (argc, argv, "T:", long_options, (int *) 0)) != EOF)
    {
      switch (c)
	{
	case 0:
	  break;

	case 'T':
	  term = optarg;
	  break;

	default:
	  usage ();
	}
    }

  if (optind == argc)
    usage ();

  setup_termcap (term);

  conv = find_info (argv[optind]);
  if (conv == NULL)
    {
      fprintf (stderr, "%s: Unknown terminfo capability `%s'\n",
	       program_name, argv[optind]);
      exit (MISSING_CAP);
    }

  ++optind;

  if (conv->type & NUM)
    put_num (conv);
  else if (conv->type & BOOL)
    put_bool (conv);
  else
    put_str (conv, argv, argc);

  exit (CAP_PRESENT);
}

/* Return a pointer to the conversion table entry for terminfo
   capability INFONAME.
   Return NULL if INFONAME is not a valid terminfo capability. */

struct conversion *
find_info (infoname)
     char *infoname;
{
  struct conversion conv;

  conv.info = infoname;
  return (struct conversion *)
    bsearch ((char *) &conv, (char *) conversions, CAPABILITIES,
	     sizeof (struct conversion), info_compare);
}

/* Terminfo capability name comparison function for bsearch. */

int
info_compare (conv1, conv2)
     struct conversion *conv1, *conv2;
{
  return strcmp (conv1->info, conv2->info);
}

void
put_num (conv)
     struct conversion *conv;
{
  printf ("%d\n", tgetnum (conv->cap));
}

void
put_bool (conv)
     struct conversion *conv;
{
  if (!tgetflag (conv->cap))
    exit (BOOLEAN_FALSE);
}

void
put_str (conv, argv, argc)
     struct conversion *conv;
     char **argv;
     int argc;
{
  char *string_value;		/* String capability. */
  int lines_affected;		/* Number of lines affected by capability. */

  string_value = tgetstr (conv->cap, (char **) NULL);
  if (string_value == NULL)
    exit (MISSING_CAP);

  if (!strcmp (conv->cap, "cm"))
    {
      BC = tgetstr ("le", (char **) NULL);
      UP = tgetstr ("up", (char **) NULL);

      /* The order of command-line arguments is `vertical horizontal'.
	 If horizontal is not given, it defaults to 0. */
      switch (argc - optind)
	{
	case 0:
	  break;
	case 1:
	  string_value = tgoto (string_value, 0, atoi (argv[optind]));
	  break;
	default:
	  string_value = tgoto (string_value,
			      atoi (argv[optind + 1]), atoi (argv[optind]));
	  break;
	}
    }
  else
    /* Although the terminfo `sgr' capability can take 9 parameters,
       the GNU tparam function only accepts up to 4.
       I don't know whether tparam could interpret an `sgr'
       capability reasonably even if it could accept that many
       parameters.  For now, we'll live with the 4-parameter limit. */
    switch (argc - optind)
      {
      case 0:
	break;
      case 1:
	string_value = tparam (string_value, (char *) NULL, 0,
			       atoi (argv[optind]));
	break;
      case 2:
	string_value = tparam (string_value, (char *) NULL, 0,
			       atoi (argv[optind]),
			       atoi (argv[optind + 1]));
	break;
      case 3:
	string_value = tparam (string_value, (char *) NULL, 0,
			       atoi (argv[optind]),
			       atoi (argv[optind + 1]),
			       atoi (argv[optind + 2]));
	break;
      default:
	string_value = tparam (string_value, (char *) NULL, 0,
			       atoi (argv[optind]),
			       atoi (argv[optind + 1]),
			       atoi (argv[optind + 2]),
			       atoi (argv[optind + 3]));
	break;
      }

  /* Since we don't know where the cursor is, we have to be
     pessimistic for capabilities that need padding proportional to
     the number of lines affected, and tell them that the whole
     screen is affected.  */
  if (conv->type & PAD)
    lines_affected = tgetnum ("li");
  else
    lines_affected = 1;

  if (lines_affected < 1)
    lines_affected = 1;

  translations_off ();
  tputs (string_value, lines_affected, tcputchar);
  fflush (stdout);
  restore_translations ();
}

/* Output function for tputs.  */

int
tcputchar (c)
     char c;
{
  putchar (c & 0x7f);
  return c;
}

/* Read in the needed termcap strings for terminal type TERM. */

void
setup_termcap (term)
     char *term;
{
  char *tc_pc;			/* "pc" termcap string. */

  if (term == NULL)
    {
      fprintf (stderr, "%s: No value for $TERM and no -T specified\n",
	       program_name);
      exit (UNKNOWN_TERM);
    }
  switch (tgetent ((char *) NULL, term))
    {
    case 0:
      fprintf (stderr, "%s: Unknown terminal type `%s'\n", program_name, term);
      exit (UNKNOWN_TERM);
    case -1:
      fprintf (stderr, "%s: No termcap database\n", program_name);
      exit (UNKNOWN_TERM);
    }

  tc_pc = tgetstr ("pc", (char **) NULL);
  PC = tc_pc ? *tc_pc : 0;
}

#ifdef _POSIX_SOURCE
struct termios old_modes, new_modes;
#else
struct sgttyb old_modes, new_modes;
#endif

/* Turn off expansion of tabs into spaces, saving the old
   terminal settings first.
   Also set OSPEED.  */

void
translations_off ()
{
  if (isatty (1))
    {
#ifdef _POSIX_SOURCE
      tcgetattr(1, &old_modes);
#else
      gtty (1, &old_modes);
#endif

      if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, signal_handler);
      if (signal (SIGHUP, SIG_IGN) != SIG_IGN)
	signal (SIGHUP, signal_handler);
      if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
	signal (SIGQUIT, signal_handler);
      signal (SIGTERM, signal_handler);

      new_modes = old_modes;
#ifdef _POSIX_SOURCE
      new_modes.c_oflag &= ~XTABS;
      tcsetattr(1, 0, &new_modes);
      ospeed = cfgetospeed(&old_modes);
#else
      new_modes.sg_flags &= ~XTABS;
      stty (1, &new_modes);
      ospeed = old_modes.sg_ospeed;
#endif
    }
  else
    ospeed = 0;
}

/* Restore the old terminal settings.  */

void
restore_translations ()
{
#ifdef _POSIX_SOURCE
  tcgetattr(1, &old_modes);
#else
  stty (1, &old_modes);
#endif
}

SIGTYPE
signal_handler ()
{
  restore_translations ();
  exit (ERROR_EXIT);
}

void
usage ()
{
  fprintf (stderr, "\
Usage: %s [-T termtype] [+terminal=termtype] capability [parameter...]\n",
	   program_name);
  exit (USAGE_ERROR);
}
