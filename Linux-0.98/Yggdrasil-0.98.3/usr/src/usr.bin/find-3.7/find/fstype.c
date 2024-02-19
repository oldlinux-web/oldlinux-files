/* fstype.c -- determine type of filesystems that files are on
   Copyright (C) 1990, 1991 Free Software Foundation, Inc.

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

/* Written by David MacKenzie (djm@gnu.ai.mit.edu). */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "modetype.h"
#include <errno.h>
#ifndef STDC_HEADERS
extern int errno;
#endif

#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#else
#include <strings.h>
#endif

char *strdup ();
char *strstr ();

int xatoi ();
char *xmalloc ();
char *xstrdup ();
void error ();

#ifdef FSTYPE_MNTENT		/* 4.3BSD, SunOS, HP-UX, Dynix, Irix.  */
#include <mntent.h>
#if !defined(MOUNTED)
#  if defined(MNT_MNTTAB)	/* HP-UX.  */
#    define MOUNTED MNT_MNTTAB
#  endif
#  if defined(MNTTABNAME)	/* Dynix.  */
#    define MOUNTED MNTTABNAME
#  endif
#endif
#endif

#ifdef FSTYPE_GETMNT		/* Ultrix.  */
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/fs_types.h>
#endif

#ifdef FSTYPE_USG_STATFS	/* SVR3.  */
#include <sys/statfs.h>
#include <sys/fstyp.h>
#endif

#ifdef FSTYPE_STATVFS		/* SVR4.  */
#include <sys/statvfs.h>
#include <sys/fstyp.h>
#endif

#ifdef FSTYPE_STATFS		/* 4.4BSD.  */
#include <sys/mount.h>

char *dirname ();

static char *
fstype_to_string (t)
     short t;
{
  switch (t)
    {
    case MOUNT_UFS:
      return "ufs";
    case MOUNT_NFS:
      return "nfs";
    case MOUNT_PC:
      return "pc";
#ifdef MOUNT_MFS
    case MOUNT_MFS:
      return "mfs";
#endif
#ifdef MOUNT_LO
    case MOUNT_LO:
      return "lo";
#endif
#ifdef MOUNT_TFS
    case MOUNT_TFS:
      return "tfs";
#endif
#ifdef MOUNT_TMP
    case MOUNT_TMP:
      return "tmp";
#endif
    default:
      return "?";
    }
}
#endif

#ifdef FSTYPE_AIX_STATFS	/* AIX.  */
#include <sys/vmount.h>
#include <sys/statfs.h>

#define FSTYPE_STATFS		/* Otherwise like 4.4BSD.  */
#define f_type f_vfstype

char *dirname ();

static char *
fstype_to_string (t)
     short t;
{
  switch (t)
    {
    case MNT_AIX:
#if 0				/* NFS filesystems are actually MNT_AIX. */
      return "aix";
#endif
    case MNT_NFS:
      return "nfs";
    case MNT_JFS:
      return "jfs";
    case MNT_CDROM:
      return "cdrom";
    default:
      return "?";
    }
}
#endif

/* Return the type of filesystem that the file PATH, described by STATP,
   is on in a newly allocated string.
   Return "unknown" if its filesystem type is unknown.  */

char *
filesystem_type (path, statp)
     char *path;
     struct stat *statp;
{
  char *type = NULL;

#ifdef FSTYPE_MNTENT		/* 4.3BSD, SunOS, HP-UX, Dynix, Irix.  */
  char *table = MOUNTED;
  FILE *mfp;
  struct mntent *mnt;

  mfp = setmntent (table, "r");
  if (mfp == NULL)
    {
      fflush (stdout);
      error (1, errno, "%s", table);
    }

  /* Find the entry with the same device number as STATP, and return
     that entry's fstype. */
  while (type == NULL && (mnt = getmntent (mfp)))
    {
      char *devopt;
      dev_t dev;
      struct stat disk_stats;

#ifdef MNTTYPE_IGNORE
      if (!strcmp (mnt->mnt_type, MNTTYPE_IGNORE))
	continue;
#endif

      /* Newer systems like SunOS 4.1 keep the dev number in the mtab,
	 in the options string.	 For older systems, we need to stat the
	 directory that the filesystem is mounted on to get it. */
      devopt = strstr (mnt->mnt_opts, "dev=");
      if (devopt)
	{
	  if (devopt[4] == '0' && (devopt[5] == 'x' || devopt[5] == 'X'))
	    dev = xatoi (devopt + 6);
	  else
	    dev = xatoi (devopt + 4);
	}
      else
	{
	  if (stat (mnt->mnt_dir, &disk_stats) == -1)
	    {
	      fflush (stdout);
	      error (1, errno, "error in %s: %s", table, mnt->mnt_dir);
	    }
	  dev = disk_stats.st_dev;
	}

      if (dev == statp->st_dev)
	type = mnt->mnt_type;
    }

  if (endmntent (mfp) == 0)
    {
      fflush (stdout);
      error (0, errno, "%s", table);
    }
#endif

#ifdef FSTYPE_GETMNT		/* Ultrix.  */
  int offset = 0;
  struct fs_data fsd;

  while (type == NULL
	 && getmnt (&offset, &fsd, sizeof (fsd), NOSTAT_MANY, 0) > 0)
    {
      if (fsd.fd_req.dev == statp->st_dev)
	type = gt_names[fsd.fd_req.fstype];
    }
#endif

#ifdef FSTYPE_USG_STATFS	/* SVR3.  */
  struct statfs fss;
  char typebuf[FSTYPSZ];

  if (statfs (path, &fss, sizeof (struct statfs), 0) == -1)
    {
      /* Don't die if a file was just removed. */
      if (errno != ENOENT)
	{
	  fflush (stdout);
	  error (1, errno, "%s", path);
	}
    }
  else if (!sysfs (GETFSTYP, fss.f_fstyp, typebuf))
    type = typebuf;
#endif

#ifdef FSTYPE_STATVFS		/* SVR4.  */
  struct statvfs fss;

  if (statvfs (path, &fss) == -1)
    {
      /* Don't die if a file was just removed. */
      if (errno != ENOENT)
	{
	  fflush (stdout);
	  error (1, errno, "%s", path);
	}
    }
  else
    type = fss.f_basetype;
#endif

#ifdef FSTYPE_STATFS		/* 4.4BSD.  */
  struct statfs fss;
  char *p;

  if (S_ISLNK (statp->st_mode))
    p = dirname (path);
  else
    p = path;

  if (statfs (p, &fss) == -1)
    {
      /* Don't die if symlink to nonexisting file, or a file that was
	 just removed. */
      if (errno != ENOENT)
	{
	  fflush (stdout);
	  error (1, errno, "%s", path);
	}
    }
  else
    type = fstype_to_string (fss.f_type);
  if (p != path)
    free (p);
#endif

  return xstrdup (type ? type : "unknown");
}

#ifdef FSTYPE_MNTENT		/* 4.3BSD etc.  */
/* Return the value of the hexadecimal number represented by CP.
   No prefix (like '0x') or suffix (like 'h') is expected to be
   part of CP. */

int
xatoi (cp)
     char *cp;
{
  int val;
  
  val = 0;
  while (*cp)
    {
      if (*cp >= 'a' && *cp <= 'f')
	val = val * 16 + *cp - 'a' + 10;
      else if (*cp >= 'A' && *cp <= 'F')
	val = val * 16 + *cp - 'A' + 10;
      else if (*cp >= '0' && *cp <= '9')
	val = val * 16 + *cp - '0';
      else
	break;
      cp++;
    }
  return val;
}
#endif
