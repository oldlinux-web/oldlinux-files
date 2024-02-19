/* Copyright (C) 1991 Free Software Foundation, Inc.
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

#define	STACK_SIZE	(4 * __vm_page_size) /* ? XXX */

error_t
_hurd_start_sigthread (thread_t sigthread)
{
  error_t error;
  struct i386_thread_state ts;
  size_t tssize = i386_THREAD_STATE_COUNT;
  vm_address_t stack;

  if (error = __vm_allocate (__mach_task_self (), &stack, STACK_SIZE, 1))
    return error;

  bzero (&ts, sizeof (ts));
  ts.eip = (int) _hurd_sigport_receive;
  ts.esp = stack + STACK_SIZE;

  if (error = __thread_set_state (sigthread, i386_THREAD_STATE, &ts, tssize))
    return error;

  return __thread_resume (sigthread);
}
