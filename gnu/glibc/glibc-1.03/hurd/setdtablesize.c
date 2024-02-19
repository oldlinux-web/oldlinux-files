/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <hurd.h>

/* Change the size of the descriptor table.
   You cannot shrink the table if any of the descriptors
   which would be removed are being used.  */

int
setdtablesize (size_t size)
{
  int i;
  struct _hurd_fd *table;

  __mutex_lock (&_hurd_dtable_lock);

  /* Check that no descriptors which are in use are going to disappear.  */
  for (i = size; i < _hurd_dtable.size; ++i)
    {
      struct _hurd_fd *const d = &_hurd_dtable.d[i];

      __spin_lock (&d->port.lock);
      if (d->port.port != MACH_PORT_NULL)
	{
	  __spin_unlock (&d->port.lock);
	  __mutex_unlock (&_hurd_dtable_lock);
	  errno = EBUSY;
	  return -1;
	}
    }

  /* Resize the table.  */

  if (_hurd_dtable_user_dealloc == NULL)
    /* Noone is using the table.  We can relocate it.  */
    table = realloc (_hurd_dtable.d, size * sizeof (*table));
  else
    /* Someone else is using the table.
       We must make a new copy, and let them free the old one.  */
    table = malloc (size * sizeof (*table));

  if (table == NULL)
    {
      __mutex_unlock (&_hurd_dtable_lock);
      return -1;
    }

  if (_hurd_dtable_user_dealloc != NULL)
    {
      *_hurd_dtable_user_dealloc = 1;
      _hurd_dtable_user_dealloc = NULL;

      memcpy (table, _hurd_dtable.d, _hurd_dtable.size * sizeof (table[0]));
    }

  /* If the table grew, initialize the new slots.  */
  for (i = _hurd_dtable.size; i < size; ++i)
    {
      table[i].flags = 0;
      _hurd_port_init (&table[i].port, MACH_PORT_NULL);
      _hurd_port_init (&table[i].ctty, MACH_PORT_NULL);
    }

  _hurd_dtable.size = size;
  _hurd_dtable.d = table;

  __mutex_unlock (&_hurd_dtable_lock);

  return 0;
}
