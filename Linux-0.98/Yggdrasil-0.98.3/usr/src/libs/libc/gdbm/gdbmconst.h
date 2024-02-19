/* gdbmconst.h - The constants defined for use in gdbm. */

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
       e-mail:  phil@cs.wwu.edu
      us-mail:  Philip A. Nelson
                Computer Science Department
                Western Washington University
                Bellingham, WA 98226
        phone:  (206) 676-3035
       
*************************************************************************/

/* Start with the constant definitions.  */
#define  TRUE    1
#define  FALSE   0

/* Parameters to gdbm_open. */
#define  GDBM_READER  0		/* READERS only. */
#define  GDBM_WRITER  1		/* READERS and WRITERS.  Can not create. */
#define  GDBM_WRCREAT 2		/* If not found, create the db. */
#define  GDBM_NEWDB   3		/* ALWAYS create a new db.  (WRITER) */

/* Parameters to gdbm_store for simple insertion or replacement in the
   case a key to store is already in the database. */
#define  GDBM_INSERT  0		/* Do not overwrite data in the database. */
#define  GDBM_REPLACE 1		/* Replace the old value with the new value. */


/* In freeing blocks, we will ignore any blocks smaller (and equal) to
   IGNORE_SIZE number of bytes. */
#define IGNORE_SIZE 4

/* The number of key bytes kept in a hash bucket. */
#define SMALL    4

/* The number of bucket_avail entries in a hash bucket. */
#define BUCKET_AVAIL 6

/* The size of the bucket cache. */
#ifndef CACHE_SIZE
#define CACHE_SIZE  100
#endif
