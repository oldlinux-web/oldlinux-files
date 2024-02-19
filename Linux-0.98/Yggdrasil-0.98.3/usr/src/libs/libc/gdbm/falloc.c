/* falloc.c - The file space management routines for dbm. */

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


/* The forward definitions for this file.  See the functions for
   the definition of the function. */

static avail_elem get_elem ();
static avail_elem get_block ();
static push_avail_block ();
static pop_avail_block ();
static adjust_bucket_avail ();

/* Allocate space in the file DBF for a block NUM_BYTES in length.  Return
   the file address of the start of the block.  

   Each hash bucket has a fixed size avail table.  We first check this
   avail table to satisfy the request for space.  In most cases we can
   and this causes changes to be only in the current hash bucket.
   Allocation is done on a first fit basis from the entries.  If a
   request can not be satisfied from the current hash bucket, then it is
   satisfied from the file header avail block.  If nothing is there that
   has enough space, another block at the end of the file is allocated
   and the unused portion is returned to the avail block.  This routine
   "guarantees" that an allocation does not cross a block boundary unless
   the size is larger than a single block.  The avail structure is
   changed by this routine if a change is needed.  If an error occurs,
   the value of 0 will be returned.  */

long
_gdbm_alloc (dbf, num_bytes)
     gdbm_file_info *dbf;
     int num_bytes;
{
  long  file_adr;		/* The address of the block. */
  avail_elem av_el;		/* For temporary use. */

  /* The current bucket is the first place to look for space. */
  av_el = get_elem (num_bytes, dbf->bucket->bucket_avail,
		    &dbf->bucket->av_count);

  /* If we did not find some space, we have more work to do. */
  if (av_el.av_size == 0)
    {
     /* Is the header avail block empty and there is something on the stack. */
      if (dbf->header->avail.count == 0 && dbf->header->avail.next_block != 0)
	pop_avail_block (dbf);
      
      /* Get another full block from end of file. */
      av_el = get_block (num_bytes, dbf);

      dbf->header_changed = TRUE;
    }

  /* We now have the place from which we will allocate the new space. */
  file_adr = av_el.av_adr;

  /* Put the unused space back in the avail block. */
  av_el.av_adr += num_bytes;
  av_el.av_size -= num_bytes;
  _gdbm_free (dbf, av_el.av_adr, av_el.av_size);

  /* Return the address. */
  return file_adr;
  
}



/* Free space of size NUM_BYTES in the file DBF at file address FILE_ADR.  Make
   it avaliable for reuse through _gdbm_alloc.  This routine changes the
   avail structure.  The value TRUE is returned if there were errors.  If no
   errors occured, the value FALSE is returned. */

_gdbm_free (dbf, file_adr, num_bytes)
     gdbm_file_info *dbf;
     long file_adr;
     int num_bytes;
{
  avail_elem temp;

  /* Is it too small to worry about? */
  if (num_bytes <= IGNORE_SIZE)
    return;

  /* Initialize the avail element. */
  temp.av_size = num_bytes;
  temp.av_adr = file_adr;

  /* Is the freed space large or small? */
  if (num_bytes >= dbf->header->block_size)
    {
      if (dbf->header->avail.count == dbf->header->avail.size)
	{
	  push_avail_block (dbf);
	}
      _gdbm_put_av_elem (temp, dbf->header->avail.av_table,
			 &dbf->header->avail.count);
      dbf->header_changed = TRUE;
    }
  else
    {
      /* Try to put into the current bucket. */
      if (dbf->bucket->av_count < BUCKET_AVAIL)
	_gdbm_put_av_elem (temp, dbf->bucket->bucket_avail,
			   &dbf->bucket->av_count);
      else
	{
	  if (dbf->header->avail.count == dbf->header->avail.size)
	    {
	      push_avail_block (dbf);
	    }
	  _gdbm_put_av_elem (temp, dbf->header->avail.av_table,
			     &dbf->header->avail.count);
	  dbf->header_changed = TRUE;
	}
    }

  if (dbf->header_changed)
    adjust_bucket_avail (dbf);

  /* All work is done. */
  return;
}



/* The following are all utility routines needed by the previous two. */


/* Gets the avail block at the top of the stack and loads it into the
   active avail block.  It does a "free" for itself! */

static
pop_avail_block (dbf)
     gdbm_file_info *dbf;
{
  int  num_bytes;		/* For use with the read system call. */
  long file_pos;		/* For use with the lseek system call. */
  avail_elem temp;

  /* Set up variables. */
  temp.av_adr = dbf->header->avail.next_block;
  temp.av_size = ( ( (dbf->header->avail.size * sizeof (avail_elem)) >> 1)
		  + sizeof (avail_block));

  /* Read the block. */
  file_pos = lseek (dbf->desc, temp.av_adr, L_SET);
  if (file_pos != temp.av_adr)  _gdbm_fatal (dbf, "lseek error");
  num_bytes = read (dbf->desc, &dbf->header->avail, temp.av_size);
  if (num_bytes != temp.av_size) _gdbm_fatal (dbf, "read error");

  /* We changed the header. */
  dbf->header_changed = TRUE;

  /* Free the previous avail block. */
  _gdbm_put_av_elem (temp, dbf->header->avail.av_table,
		     &dbf->header->avail.count);
}


/* Splits the header avail block and pushes half onto the avail stack. */

static
push_avail_block (dbf)
     gdbm_file_info *dbf;
{
  int  num_bytes;
  int  av_size;
  int  av_adr;
  int  index;
  long file_pos;
  avail_block *temp;
  avail_elem  new_loc;
 

  /* Caclulate the size of the split block. */
  av_size = ( (dbf->header->avail.size * sizeof (avail_elem)) >> 1)
            + sizeof (avail_block);

  /* Get address in file for new av_size bytes. */
  new_loc = get_elem (av_size, dbf->header->avail.av_table,
		      &dbf->header->avail.count);
  if (new_loc.av_size == 0)
    new_loc = get_block (av_size, dbf);
  av_adr = new_loc.av_adr;


  /* Split the header block. */
  temp = (avail_block *) alloca (av_size);
  /* Set the size to be correct AFTER the pop_avail_block. */
  temp->size = dbf->header->avail.size;
  temp->count = 0;
  temp->next_block = dbf->header->avail.next_block;
  dbf->header->avail.next_block = av_adr;
  for (index = 1; index < dbf->header->avail.count; index++)
    if ( (index & 0x1) == 1)	/* Index is odd. */
      temp->av_table[temp->count++] = dbf->header->avail.av_table[index];
    else
      dbf->header->avail.av_table[index>>1]
	= dbf->header->avail.av_table[index];

  /* Update the header avail count to previous size divided by 2. */
  dbf->header->avail.count >>= 1;

  /* Free the unneeded space. */
  new_loc.av_adr += av_size;
  new_loc.av_size -= av_size;
  _gdbm_free (dbf, new_loc.av_adr, new_loc.av_size);

  /* Update the disk. */
  file_pos = lseek (dbf->desc, av_adr, L_SET);
  if (file_pos != av_adr) _gdbm_fatal (dbf, "lseek error");
  num_bytes = write (dbf->desc, temp, av_size);
  if (num_bytes != av_size) _gdbm_fatal (dbf, "write error");

}



/* Get_elem returns an element in the AV_TABLE block which is
   larger than SIZE.  AV_COUNT is the number of elements in the
   AV_TABLE.  If an item is found, it extracts it from the AV_TABLE
   and moves the other elements up to fill the space. If no block is 
   found larger than SIZE, get_elem returns a size of zero.  This
   routine does no I/O. */

static avail_elem
get_elem (size, av_table, av_count)
     int size;
     avail_elem av_table[];
     int *av_count;
{
  int index;			/* For searching through the avail block. */
  avail_elem val;		/* The default return value. */

  /* Initialize default return value. */
  val.av_adr = 0;
  val.av_size = 0;

  /* Search for element.  List is sorted by size. */
  index = 0;
  while (index < *av_count && av_table[index].av_size < size)
    {
      index++;
    }

  /* Did we find one of the right size? */
  if (index >= *av_count)
    return val;

  /* Ok, save that element and move all others up one. */
  val = av_table[index];
  *av_count -= 1;
  while (index < *av_count)
    {
      av_table[index] = av_table[index+1];
      index++;
    }

  return val;
}


/* This routine inserts a single NEW_EL into the AV_TABLE block in
   sorted order. This routine does no I/O. */

_gdbm_put_av_elem (new_el, av_table, av_count)
     avail_elem new_el;
     avail_elem av_table[];
     int *av_count;
{
  int index;			/* For searching through the avail block. */
  int index1;

  /* Is it too small to deal with? */
  if (new_el.av_size <= IGNORE_SIZE)
    return FALSE; 

  /* Search for place to put element.  List is sorted by size. */
  index = 0;
  while (index < *av_count && av_table[index].av_size < new_el.av_size)
    {
      index++;
    }

  /* Move all others up one. */
  index1 = *av_count-1;
  while (index1 >= index)
    {
      av_table[index1+1] = av_table[index1];
      index1--;
    }
  
  /* Add the new element. */
  av_table[index] = new_el;

  /* Increment the number of elements. */
  *av_count += 1;  

  return TRUE;
}



/* Get_block "allocates" new file space and the end of the file.  This is
   done in integral block sizes.  (This helps insure that data smaller than
   one block size is in a single block.)  Enough blocks are allocated to
   make sure the number of bytes allocated in the blocks is larger than SIZE.
   DBF contains the file header that needs updating.  This routine does
   no I/O.  */

static avail_elem
get_block (size, dbf)
     int size;
     gdbm_file_info *dbf;
{
  avail_elem val;

  /* Need at least one block. */
  val.av_adr  = dbf->header->next_block;
  val.av_size = dbf->header->block_size;

  /* Get enough blocks to fit the need. */
  while (val.av_size < size)
    val.av_size += dbf->header->block_size;

  /* Update the header and return. */
  dbf->header->next_block += val.av_size;

  /* We changed the header. */
  dbf->header_changed = TRUE;

  return val;
  
}


/*  When the header already needs writing, we can make sure the current
    bucket has its avail block as close to 1/2 full as possible. */
static
adjust_bucket_avail (dbf)
     gdbm_file_info *dbf;
{
  int third = BUCKET_AVAIL / 3;
  avail_elem av_el;

  /* Can we add more entries to the bucket? */
  if (dbf->bucket->av_count < third)
    {
      if (dbf->header->avail.count > 0)
	{
	  dbf->header->avail.count -= 1;
	  av_el = dbf->header->avail.av_table[dbf->header->avail.count];
	  _gdbm_put_av_elem (av_el, dbf->bucket->bucket_avail,
			     &dbf->bucket->av_count);
	  dbf->bucket_changed = TRUE;
	}
      return;
    }

  /* Is there too much in the bucket? */
  while (dbf->bucket->av_count > BUCKET_AVAIL-third
	 && dbf->header->avail.count < dbf->header->avail.size)
    {
      av_el = get_elem (0, dbf->bucket->bucket_avail, &dbf->bucket->av_count);
      _gdbm_put_av_elem (av_el, dbf->header->avail.av_table,
			 &dbf->header->avail.count);
      dbf->bucket_changed = TRUE;
    }
}


