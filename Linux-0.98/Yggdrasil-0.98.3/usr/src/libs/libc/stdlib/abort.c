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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/* Cause an abnormal program termination with core-dump.  */
volatile void
DEFUN_VOID(abort)
{
  sig_atomic_t aborting = 0;
  sigset_t sigs;

  if (sigemptyset(&sigs) == 0 &&
      sigaddset(&sigs, SIGABRT) == 0)
    (void) sigprocmask(SIG_UNBLOCK, &sigs, (sigset_t *) NULL);

  if (!aborting)
    {
      aborting = 1;
#if defined(HAVE_GNU_LD) && !defined(linux)
      {
	extern unsigned long int __libc_atexit[];
	register unsigned long int i;
	for (i = 1; i <= __libc_atexit[0]; ++i)
	  (*(void EXFUN((*), (NOARGS))) __libc_atexit[i])();
      }
#else
      {
	extern void EXFUN(_cleanup, (NOARGS));
	_cleanup();
      }
#endif
    }

  while (1)
    if (raise(SIGABRT))
      /* If we can't signal ourselves, exit.  */
      _exit(127);
  /* If we signal ourselves and are still alive,
     or can't exit, loop forever.  */
}
