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

#include <ansidecl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <hurd.h>

/* Duplicate FD, returning a new file descriptor open on the same file.  */
int
DEFUN(__dup, (fd), int fd)
{
  int new;
  int dealloc, dealloc_ctty;
  io_t port, ctty;
  int flags;
  int dealloc_dt;
  struct _hurd_fd_user d;

  /* Extract the ports and flags from FD.  */
  d = _hurd_fd (fd, &dealloc_dt);
  if (d.d == NULL)
    {
    badf:
      errno = EBADF;
      return -1;
    }
  flags = d.d->flags;
  ctty = _hurd_port_get (&d.d->ctty, &dealloc_ctty);
  port = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */

  if (port == MACH_PORT_NULL)
    goto badf;

  /* Get a new descriptor.  */
  new = _hurd_dalloc (port, ctty, flags);
  if (new >= 0)
    {
      /* Give the ports each a user ref for the new descriptor.  */
      __mach_port_mod_refs (__mach_task_self (), port,
			    MACH_PORT_RIGHT_SEND, 1);
      if (ctty != MACH_PORT_NULL)
	__mach_port_mod_refs (__mach_task_self (), ctty,
			      MACH_PORT_RIGHT_SEND, 1);
    }

  _hurd_port_free (&d->port, port, &dealloc);
  if (ctty != MACH_PORT_NULL)
    _hurd_port_free (&d->ctty, ctty, &dealloc_ctty);

  _hurd_fd_done (d, &dealloc_dt);

  return new;
}
