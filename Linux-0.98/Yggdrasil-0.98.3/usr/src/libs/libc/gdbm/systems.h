/* systems.h - Most of the system dependant code and defines are here. */

/*  This file is part of GDBM, the GNU data base manager, by Philip A. Nelson.
    Copyright (C) 1990, 1991  Free Software Foundation, Inc.

    GDBM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    GDBM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GDBM; see the file COPYING.  If not, write to
    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

    You may contact the author by:
       e-mail:  phil@wwu.edu
      us-mail:  Philip A. Nelson
                Computer Science Department
                Western Washington University
                Bellingham, WA 98226
        phone:  (206) 676-3035
       
*************************************************************************/



/*         System V changes and defines.          */
/**************************************************/

#ifdef linux

/* !!NOTE: linux doesn't do file locking, so we fake the retval! */

#include <stdlib.h>
#include <unistd.h>

/* Some files need fcntl.h for locking. */
#include <fcntl.h>
#define UNLOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_UNLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  fcntl (dbf->desc, F_SETLK, &flock);	\
	}
#ifdef NO_FILELOCK
#define READLOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_RDLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	  lock_val = 0; 			\
	}
#else
#define READLOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_RDLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	}
#endif
#ifdef NO_FILELOCK
#define WRITELOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_WRLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	  lock_val = 0; 			\
	}
#else
#define WRITELOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_WRLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	}
#endif

#if 0
#include <blocksize.h>
#define STATBLKSIZE BLOCK_SIZE

#include <string.h>
#define _HAVE_STRING

#define bcmp(d1, d2, n)	memcmp(d1, d2, n)
#define bcopy(d1, d2, n) memcpy(d2, d1, n)
#endif

/* linux does not have fsync. */
#define fsync(f) sync(); sync()

#undef SYSV

#endif /* linux */

#ifdef SYSV

/* File seeking needs L_SET defined .*/
#include <unistd.h>
#define L_SET SEEK_SET

/* Some files need fcntl.h for locking. */
#include <fcntl.h>
#define UNLOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_UNLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  fcntl (dbf->desc, F_SETLK, &flock);	\
	}
#define READLOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_RDLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	}
#define WRITELOCK_FILE(dbf) \
	{					\
	  struct flock flock;			\
	  flock.l_type = F_WRLCK;		\
	  flock.l_whence = 0;			\
	  flock.l_start = flock.l_len = 0L;	\
	  lock_val = fcntl (dbf->desc, F_SETLK, &flock);	\
	}

/* Send bcmp to the right place. */
#include <memory.h>
#define bcmp(d1, d2, n)	memcmp(d1, d2, n)
#define bcopy(d1, d2, n) memcpy(d2, d1, n)

/* Sys V does not have fsync. */
#define fsync(f) sync(); sync()

/* Stat does not have a st_blksize field. */
#define STATBLKSIZE 512

/* Does not have rename(). */
#define NEED_RENAME

/* Get string definitions. */
#include <string.h>
#define _HAVE_STRING
#endif

/* Sys V does not have a truncate call, how primitive. */
#define TRUNCATE(dbf) close( open (dbf->name, O_RDWR|O_TRUNC, mode));


/*      End of System V changes and defines.      */
/**************************************************/



/* Alloca is builtin in gcc.  Use the builtin alloca if compiled with gcc. */
#ifdef __GNUC__
#define BUILTIN_ALLOCA
#endif

/* Also, if this is a sun spark, use the builtin alloca. */
#ifdef sun
#ifdef sparc
#define BUILTIN_ALLOCA
#endif
#endif

#ifndef linux
/* Define the proper alloca procedure. */
#ifdef BUILTIN_ALLOCA
#define alloca(x) __builtin_alloca(x)
#else
extern char *alloca();
#endif
#endif

#ifndef linux
/* Malloc definition. */
extern char *malloc();
#endif

/* String definitions. */
#ifndef _HAVE_STRING
#include <strings.h>
#endif

/* The BSD defines are the default defines.  If something is not
   defined above in the above conditional code, it will be set
   in the following code to the BSD code.  */

/* Default block size.  Some systems do not have blocksize in their
   stat record. This code uses the BSD blocksize from stat. */

#ifndef STATBLKSIZE
#define STATBLKSIZE file_stat.st_blksize
#endif


/* Locking is done differently on different systems.  Here is the BSD
   locking routines.  */

#ifndef UNLOCK_FILE
#define UNLOCK_FILE(dbf) flock (dbf->desc, LOCK_UN)
#define READLOCK_FILE(dbf) lock_val = flock (dbf->desc, LOCK_SH + LOCK_NB)
#define WRITELOCK_FILE(dbf) lock_val = flock (dbf->desc, LOCK_EX + LOCK_NB)
#endif

#ifndef TRUNCATE
#define TRUNCATE(dbf) ftruncate (dbf->desc, 0)
#endif
