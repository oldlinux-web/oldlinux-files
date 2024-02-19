/*----------------------------------------------------------------------*/
/*		Directory access routine for LHarc UNIX			*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*	Emulate opendir(),readdir(),closedir() function for LHarc	*/
/*									*/
/*  V0.00  Original				1988.05.31  Y.Tagawa	*/
/*  V0.03  Release #3 for LHarc UNIX		1988.07.02  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*----------------------------------------------------------------------*/


#include <sys/types.h>

/* Where is O_RDONLY ? (^_^) */
#include <sys/file.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif

#define direct old_direct
#include <sys/dir.h>
#undef direct

#ifndef DIRSIZ
/* Warning :  Are you sure? (normally defined in <sys/dir.h> */
#define DIRSIZ	14
#endif

#include "lhdir.h"


DIR *
opendir (name)
     char *name;
{
  register DIR *dirp;
  register int fd;

  if ((fd = open (name, O_RDONLY)) >= 0)
    {
      if ((dirp = (DIR*)malloc (sizeof (DIR))) != (DIR*)0)
	{
	  dirp->dd_fd = fd;
	  dirp->dd_loc = 0;
	  dirp->dd_size = 0;
	  return dirp;
	}

      close (fd);
    }

  return (DIR*)0;
}

struct direct *
readdir (dirp)
     register DIR *dirp;
{
  static struct direct lhdir;
  register struct old_direct *dp;

  do {
    if (dirp->dd_loc >= dirp->dd_size)
      {
	dirp->dd_loc = 0;
	if ((dirp->dd_size = read (dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ)) <= 0)
	  return (struct direct *)0;
      }

    dp = (struct old_direct *)(dirp->dd_buf + dirp->dd_loc);

    if (dirp->dd_loc + sizeof (struct old_direct) > dirp->dd_size)
      return (struct direct *)0;

    dirp->dd_loc += sizeof (struct old_direct);

  } while (dp->d_ino == 0) ;

  /* construct new format */
  lhdir.d_ino = dp->d_ino;
  strncpy (lhdir.d_name, dp->d_name, DIRSIZ);
  lhdir.d_name[DIRSIZ] = '\0';
  lhdir.d_namlen = strlen (lhdir.d_name);

  return &lhdir;
}

closedir (dirp)
     DIR *dirp;
{
  close (dirp->dd_fd);
  free (dirp);
}

