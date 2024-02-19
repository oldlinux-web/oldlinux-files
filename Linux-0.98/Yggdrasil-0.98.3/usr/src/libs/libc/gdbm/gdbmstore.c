/* gdbmstore.c - Add a new key/data pair to the database. */

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

extern gdbm_error gdbm_errno;	/* gdbm's error variable. */


/* Add a new element to the database.  CONTENT is keyed by KEY.  The
   file on disk is updated to reflect the structure of the new database
   before returning from this procedure.  The FLAGS define the action to
   take when the KEY is already in the database.  The value GDBM_REPLACE
   asks that the old data be replaced by the new CONTENT.  The value
   GDBM_INSERT asks that an error be returned and no action taken.  A
   return value of 0 means no errors.  A return value of -1 means that
   the item was not stored in the data base because the caller was not an
   official writer. A return value of 0 means that the item was not stored
   because the argument FLAGS was GDBM_INSERT and the KEY was already in
   the database. */

int
gdbm_store (dbf, key, content, flags)
     gdbm_file_info *dbf;
     datum key;
     datum content;
     int flags;
{
  long new_hash_val;		/* The new hash value. */
  int  elem_loc;		/* The location in hash bucket. */
  long file_adr;		/* The address of new space in the file.  */
  long file_pos;		/* The position after a lseek. */
  int  num_bytes;		/* Used for error detection. */
  long free_adr;		/* For keeping track of a freed section. */
  int  free_size;

  char *write_data;		/* To write both key and data in 1 call. */
  char *src;			/* Used to prepare write_data. */
  char *dst;			/* Used to prepare write_data. */
  int   cnt;			/* Counter for loops to fill write_data. */
  int   new_size;		/* Used in allocating space. */
  char *temp;			/* Used in _gdbm_findkey call. */


  /* First check to make sure this guy is a writer. */
  if (dbf->read_write != GDBM_WRITER)
    {
      gdbm_errno = GDBM_READER_CANT_STORE;
      return -1;
    }

  /* Check for illegal data values.  A NULL dptr field is illegal because
     NULL dptr returned by a lookup procedure indicates an error. */
  if ((key.dptr == NULL) || (content.dptr == NULL))
    {
      gdbm_errno = GDBM_ILLEGAL_DATA;
      return -1;
    }

  /* Look for the key in the file.
     A side effect loads the correct bucket and calculates the hash value. */
  elem_loc = _gdbm_findkey (dbf, key, &temp, &new_hash_val);


  /* Did we find the item? */
  if (elem_loc != -1)
    {
      if (flags == GDBM_REPLACE)
	{
	  /* Just replace the data. */
	  free_adr = dbf->bucket->h_table[elem_loc].data_pointer;
	  free_size = dbf->bucket->h_table[elem_loc].key_size
	              + dbf->bucket->h_table[elem_loc].data_size;
	  _gdbm_free (dbf, free_adr, free_size);
	}
      else
	{
	  gdbm_errno = GDBM_CANNOT_REPLACE;
	  return 1;
	}
    }


  /* Get the file address for the new space.
     (Current bucket's free space is first place to look.) */
  new_size = key.dsize+content.dsize;
  file_adr = _gdbm_alloc (dbf, new_size);

  /* If this is a new entry in the bucket, we need to do special things. */
  if (elem_loc == -1)
    {
      if (dbf->bucket->count == dbf->header->bucket_elems)
	{
	  /* Split the current bucket. */
	  _gdbm_split_bucket (dbf, new_hash_val);
	}
      
      /* Find space to insert into bucket and set elem_loc to that place. */
      elem_loc = new_hash_val % dbf->header->bucket_elems;
      while (dbf->bucket->h_table[elem_loc].hash_value != -1)
	{  elem_loc = (elem_loc + 1) % dbf->header->bucket_elems; }

      /* We now have another element in the bucket.  Add the new information.*/
      dbf->bucket->count += 1;
      dbf->bucket->h_table[elem_loc].hash_value = new_hash_val;
      bcopy (key.dptr, dbf->bucket->h_table[elem_loc].key_start,
	     (SMALL < key.dsize ? SMALL : key.dsize));
    }


  /* Update current bucket data pointer and sizes. */
  dbf->bucket->h_table[elem_loc].data_pointer = file_adr;
  dbf->bucket->h_table[elem_loc].key_size = key.dsize;
  dbf->bucket->h_table[elem_loc].data_size = content.dsize;

  /* Prepare write_data. The key is written first.  */
  write_data = (char *) alloca (key.dsize+content.dsize);
  dst = write_data;
  src = key.dptr;
  for (cnt=0; cnt < key.dsize; cnt++)
    * (dst++) = * (src++);
  src = content.dptr;
  for (cnt=0; cnt < content.dsize; cnt++)
    * (dst++) = * (src++);

  /* Write the data to the file. */
  file_pos = lseek (dbf->desc, file_adr, L_SET);
  if (file_pos != file_adr) _gdbm_fatal (dbf, "lseek error");
  num_bytes = write (dbf->desc, write_data, key.dsize+content.dsize);
  if (num_bytes != key.dsize+content.dsize) _gdbm_fatal (dbf, "write error");

  /* Current bucket has changed. */
  dbf->cache_entry->ca_changed = TRUE;
  dbf->bucket_changed = TRUE;

  /* Write everything that is needed to the disk. */
  _gdbm_end_update (dbf);
  return 0;
  
}
