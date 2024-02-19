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

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <hurd.h>

extern const struct
  {
    int set;
    struct _hurd_dtable *dtable;
  } _hurd_dtable_set;

/* Clone the calling process, but without copying the whole address space.
   The the calling process is suspended until the the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
int
DEFUN_VOID(__vfork)
{
  error_t err;
  process_t newproc;

  struct _hurd_sigstate *ss = _hurd_thread_sigstate (__mach_thread_self ());
  __mutex_lock (&_hurd_lock);

  __proc_dostop (_hurd_proc, __mach_thread_self ());

  if (err = __proc_register (_hurd_proc, __mach_task_self (),
			     _hurd_sigport, &newproc))
    goto lose;

  if (_hurd_dtable_set.set)
    {
      struct _hurd_dtable *const dt = _hurd_dtable_set.dtable;
      __mutex_lock (&dt->lock);
      ss->vfork_saved.dtable = *dt;
      dt->d = malloc (dt->size * sizeof (dt->d[0]));
      if (dt->d == NULL)
	{
	  dt->d = ss->vfork_saved.dtable.d;
	  __mutex_unlock (&dt->lock);
	  goto lose;
	}
      for (i = 0; i < dt->size; ++i)
	{
	  dt->d[i] = ss->vfork_saved.dtable.d[i];
	  if (dt->d[i].server != MACH_PORT_NULL)
	    __mach_port_mod_refs (__mach_task_self (),
				  dt->d[i].server,
				  MACH_PORT_RIGHT_SEND, 1);
	}
      __mutex_unlock (&dt->lock);
    }

  ss->vforked = 1;
  ss->vfork_saved.proc = _hurd_proc;
  _hurd_proc = newproc;
  ss->vfork_saved.auth = _hurd_auth;
  __mach_port_mod_refs (__mach_task_self (), ss->vfork_saved.auth,
			MACH_PORT_RIGHT_SEND, 1);
  ss->vfork_saved.ccdir = _hurd_ccdir;
  __mach_port_mod_refs (__mach_task_self (), ss->vfork_saved.ccdir,
			MACH_PORT_RIGHT_SEND, 1);
  ss->vfork_saved.cwdir = _hurd_cwdir;
  __mach_port_mod_refs (__mach_task_self (), ss->vfork_saved.cwdir,
			MACH_PORT_RIGHT_SEND, 1);
  ss->vfork_saved.crdir = _hurd_crdir;
  __mach_port_mod_refs (__mach_task_self (), ss->vfork_saved.crdir,
			MACH_PORT_RIGHT_SEND, 1);
  ss->vfork_saved.umask = _hurd_umask;
  ss->vfork_saved.ctty_fstype = _hurd_ctty_fstype;
  ss->vfork_saved.ctty_fsid = _hurd_ctty_fsid;
  ss->vfork_saved.ctty_fileid = _hurd_ctty_fileid;

  __proc_getpids (_hurd_proc, &_hurd_pid, &_hurd_ppid, &_hurd_orphaned);

  if (setjmp (ss->vfork_saved.continuation))
    {
      /* The vfork child has exec'd or exitted.
	 SS->lock is held.  */
      pid_t child = _hurd_pid;

      /* Restore the saved state of the parent fork.  */
      mach_port_t osigport, otask;
      __mutex_lock (&_hurd_lock);
      ss->vforked = 0;
      _hurd_proc = ss->vfork_saved.proc;
      _hurd_auth = ss->vfork_saved.auth;
      _hurd_id_valid = 0;
      if (_hurd_rid_auth != MACH_PORT_NULL)
	{
	  __mach_port_deallocate (__mach_task_self (), _hurd_rid_auth);
	  _hurd_rid_auth = MACH_PORT_NULL;
	}
      _hurd_ccdir = ss->vfork_saved.ccdir;
      _hurd_cwdir = ss->vfork_saved.cwdir;
      _hurd_crdir = ss->vfork_saved.crdir;
      _hurd_umask = ss->vfork_saved.umask;
      __mutex_lock (&_hurd_dtable.lock);
      _hurd_ctty_fstype = ss->vfork_saved.ctty_fstype;
      _hurd_ctty_fsid = ss->vfork_saved.ctty_fsid;
      _hurd_ctty_fileid = ss->vfork_saved.ctty_fileid;
      if (_hurd_dtable_set.set)
	{
	  __mutex_lock (&_hurd_dtable_set.dtable->lock);
	  *_hurd_dtable_set.dtable = *ss->vfork_saved.dtable;
	  __mutex_unlock (&_hurd_dtable_set.dtable->lock);
	}
      __proc_setports (_hurd_proc, __mach_task_self (), sigport,
		       &osigport, &otask);
      if (osigport != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), osigport);
      if (otask != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), otask);
      __proc_getpids (_hurd_proc, &_hurd_pid, &_hurd_ppid, &_hurd_orphaned);

      err = 0;

    lose:
      __mutex_unlock (&_hurd_lock);
      __mutex_unlock (&ss->lock);

      /* Resume all threads.  */
      {
	const thread_t me = __mach_thread_self ();
	thread_t *threads;
	size_t nthreads;
	for (__task_threads (__mach_task_self (), &threads, &nthreads);
	     nthreads-- > 0;
	     __mach_port_deallocate (__mach_task_self (), *threads++))
	  if (*threads != me && *threads != _hurd_sigport_thread)
	    __thread_resume (*threads);
      }

      if (err)
	return __hurd_fail (err);

      return child;
    }

  /* Return to the vfork child.  */

  __mutex_unlock (&ss->lock);

  __thread_resume (_hurd_sigport_thread);

  return 0;
}
