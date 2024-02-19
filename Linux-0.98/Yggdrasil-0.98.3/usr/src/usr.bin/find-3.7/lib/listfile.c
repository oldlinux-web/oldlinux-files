/* listfile.c -- display a long listing of a file
   Copyright (C) 1985, 1988, 1991 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#if defined(S_IFLNK) && !defined(S_ISLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif

/* Extract or fake data from a `struct stat'.
   ST_NBLOCKS: Number of 512-byte blocks in the file
   (including indirect blocks). */
#ifdef _POSIX_SOURCE
#define ST_NBLOCKS(statp) (((statp)->st_size + 512 - 1) / 512)
#else /* !_POSIX_SOURCE */
#ifndef HAVE_ST_BLOCKS
#define ST_NBLOCKS(statp) (st_blocks ((statp)->st_size))
#else /* HAVE_ST_BLOCKS */
#if defined(hpux) || defined(__hpux__)
/* HP-UX, perhaps uniquely, counts st_blocks in 1024-byte units.
   This loses when mixing HP-UX and 4BSD filesystems, though. */
#define ST_NBLOCKS(statp) ((statp)->st_blocks * 2)
#else /* !hpux */
#define ST_NBLOCKS(statp) ((statp)->st_blocks)
#endif /* hpux */
#endif /* HAVE_ST_BLOCKS */
#endif /* _POSIX_SOURCE */

/* Convert B 512-byte blocks to kilobytes if K is nonzero,
   otherwise return it unchanged. */
#define convert_blocks(b, k) ((k) ? ((b) + 1) / 2 : (b))

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <pwd.h>
#include <grp.h>
#ifndef _POSIX_VERSION
struct passwd *getpwuid ();
struct group *getgrgid ();
#endif

#include <time.h>

#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#else
#include <strings.h>
#endif

/* Since major is a function on SVR4, we can't use `ifndef major'.  */
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#define HAVE_MAJOR
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#define HAVE_MAJOR
#endif
#ifdef major			/* Might be defined in sys/types.h.  */
#define HAVE_MAJOR
#endif

#ifndef HAVE_MAJOR
#define major(dev)  (((dev) >> 8) & 0xff)
#define minor(dev)  ((dev) & 0xff)
#endif
#undef HAVE_MAJOR

#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
char *getenv ();
extern int errno;
#endif

#if defined(S_ISLNK)
int readlink ();
#endif

char *xmalloc ();
void error ();
void mode_string ();

char *get_link_name ();
char *getgroup ();
char *getuser ();
void print_name_with_quoting ();

void
list_file (name, statp)
     char *name;
     struct stat *statp;
{
  static int kilobytes = -1;	/* -1 = uninitialized, 0 = 512, 1 = 1024. */
  char modebuf[20];
  char timebuf[40];
  time_t current_time = time ((time_t *) 0);

  if (kilobytes == -1)
    kilobytes = getenv ("POSIXLY_CORRECT") == 0;

  mode_string (statp->st_mode, modebuf);
  modebuf[10] = '\0';

  strcpy (timebuf, ctime (&statp->st_mtime));
  if (current_time > statp->st_mtime + 6L * 30L * 24L * 60L * 60L /* Old. */
      || current_time < statp->st_mtime - 60L * 60L) /* In the future. */
    {
      /* The file is fairly old or in the future.
	 POSIX says the cutoff is 6 months old;
	 approximate this by 6*30 days.
	 Allow a 1 hour slop factor for what is considered "the future",
	 to allow for NFS server/client clock disagreement.
	 Show the year instead of the time of day.  */
      strcpy (timebuf + 11, timebuf + 19);
    }
  timebuf[16] = 0;

  printf ("%6u ", statp->st_ino);

  printf ("%4u ", convert_blocks (ST_NBLOCKS (statp), kilobytes));

  /* The space between the mode and the number of links is the POSIX
     "optional alternate access method flag". */
  printf ("%s %3u ", modebuf, statp->st_nlink);

  printf ("%-8.8s ", getuser (statp->st_uid));

  printf ("%-8.8s ", getgroup (statp->st_gid));

  if (S_ISCHR (statp->st_mode) || S_ISBLK (statp->st_mode))
#ifdef HAVE_ST_RDEV
    printf ("%3u, %3u ", major (statp->st_rdev), minor (statp->st_rdev));
#else
    printf ("         ");
#endif
  else
    printf ("%8lu ", statp->st_size);

  printf ("%s ", timebuf + 4);

  print_name_with_quoting (name);

#ifdef S_IFLNK
  if (S_ISLNK (statp->st_mode))
    {
      char *linkname = get_link_name (name, statp);

      if (linkname)
	{
	  fputs (" -> ", stdout);
	  print_name_with_quoting (linkname);
	  free (linkname);
	}
    }
#endif
  putchar ('\n');
}

void
print_name_with_quoting (p)
     register char *p;
{
  register unsigned char c;

  while (c = *p++)
    {
      switch (c)
	{
	case '\\':
	  printf ("\\\\");
	  break;

	case '\n':
	  printf ("\\n");
	  break;

	case '\b':
	  printf ("\\b");
	  break;

	case '\r':
	  printf ("\\r");
	  break;

	case '\t':
	  printf ("\\t");
	  break;

	case '\f':
	  printf ("\\f");
	  break;

	case ' ':
	  printf ("\\ ");
	  break;

	case '"':
	  printf ("\\\"");
	  break;

	default:
	  if (c > 040 && c < 0177)
	    putchar (c);
	  else
	    printf ("\\%03o", (unsigned int) c);
	}
    }
}

#ifdef S_IFLNK
char *
get_link_name (filename, statp)
     char *filename;
     struct stat *statp;
{
  register char *linkbuf;
  register int bufsiz = statp->st_size;

  linkbuf = (char *) xmalloc (bufsiz + 1);
  linkbuf[bufsiz] = 0;
  if (readlink (filename, linkbuf, bufsiz) < 0)
    {
      error (0, errno, "%s", filename);
      free (linkbuf);
      return 0;
    }
  return linkbuf;
}
#endif
