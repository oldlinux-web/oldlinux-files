/* gdbmerrno.h - The enumeration type describing all the dbm errors. */

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

#if 1

/* gdbm sends back the following error codes in the variable gdbm_errno. */
typedef enum {	GDBM_NO_ERROR,
		GDBM_MALLOC_ERROR,
		GDBM_BLOCK_SIZE_ERROR,
		GDBM_FILE_OPEN_ERROR,
		GDBM_FILE_WRITE_ERROR,
		GDBM_FILE_SEEK_ERROR,
		GDBM_FILE_READ_ERROR,
		GDBM_BAD_MAGIC_NUMBER,
		GDBM_EMPTY_DATABASE,
		GDBM_CANT_BE_READER,
	        GDBM_CANT_BE_WRITER,
		GDBM_READER_CANT_DELETE,
		GDBM_READER_CANT_STORE,
		GDBM_READER_CANT_REORGANIZE,
		GDBM_UNKNOWN_UPDATE,
		GDBM_ITEM_NOT_FOUND,
		GDBM_REORGANIZE_FAILED,
		GDBM_CANNOT_REPLACE,
		GDBM_ILLEGAL_DATA}
	gdbm_error;
#endif /* _GDBM_H */
