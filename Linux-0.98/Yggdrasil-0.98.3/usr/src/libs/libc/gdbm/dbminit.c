/* dbminit.c - Open the file for dbm operations. This looks like the
   DBM interface. */

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


#include "gdbmdefs.h"
#include "gdbmerrno.h"
#include "extern.h"

extern gdbm_error gdbm_errno;

/* Initialize dbm system.  FILE is a pointer to the file name.  In
   standard dbm, the database is found in files called FILE.pag and
   FILE.dir.  To make gdbm compatable with dbm using the dbminit call,
   the same file names are used.  Specifically, dbminit will use the file
   name FILE.pag in its call to gdbm open.  If the file (FILE.pag) has a
   size of zero bytes, a file initialization procedure is performed,
   setting up the initial structure in the file.  Any error detected will
   cause a return value of -1.  No errors cause the return value of 0.
   NOTE: file.dir will be linked to file.pag. */

int
dbminit (file)
     char *file;
{
  char* pag_file;	    /* Used to construct "file.pag". */
  char* dir_file;	    /* Used to construct "file.dir". */
  struct stat dir_stat;	    /* Stat information for "file.dir". */


  /* Prepare the correct names of "file.pag" and "file.dir". */
  pag_file = (char *) alloca (strlen (file)+5);
  dir_file = (char *) alloca (strlen (file)+5);

  strcpy (pag_file, file);
  strcat (pag_file, ".pag");
  strcpy (dir_file, file);
  strcat (dir_file, ".dir");

  if (_gdbm_file != NULL)
    gdbm_close (_gdbm_file);

  /* Try to open the file as a writer.  DBM never created a file. */
  _gdbm_file = gdbm_open (pag_file, 0, GDBM_WRITER, 0, NULL);

  /* If it was not opened, try opening it as a reader. */
  if (_gdbm_file == NULL)
    {
      _gdbm_file = gdbm_open (pag_file, 0, GDBM_READER, 0, NULL);
  
      /* Did we successfully open the file? */
      if (_gdbm_file == NULL)
	{
	  gdbm_errno = GDBM_FILE_OPEN_ERROR;
	  return -1;
	}
    }

  /* If the database is new, link "file.dir" to "file.pag". This is done
     so the time stamp on both files is the same. */
  if (stat (dir_file, &dir_stat) == 0)
    {
      if (dir_stat.st_size == 0)
	if (unlink (dir_file) != 0 || link (pag_file, dir_file) != 0)
	  {
	    gdbm_errno = GDBM_FILE_OPEN_ERROR;
	    gdbm_close (_gdbm_file);
	    return -1;
	  }
    }
  else
    {
      /* Since we can't stat it, we assume it is not there and try
         to link the dir_file to the pag_file. */
      if (link (pag_file, dir_file) != 0)
	{
	  gdbm_errno = GDBM_FILE_OPEN_ERROR;
	  gdbm_close (_gdbm_file);
	  return -1;
	}
    }
	    
  return 0;
}
