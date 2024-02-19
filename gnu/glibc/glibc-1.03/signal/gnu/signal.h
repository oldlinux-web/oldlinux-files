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


/* Get signal number constants and `_NSIG'.  Do this outside _GNU_SIGNAL_H
   because this file could be included more than once, sometimes with
   _SIGNAL_H defined and sometimes without.  */
#include <signum.h>


#ifndef	_GNU_SIGNAL_H

#define	_GNU_SIGNAL_H	1

typedef int __sig_atomic_t;

/* Return a mask that includes SIG only.  */
#define	__sigmask(sig)	(1 << ((sig) - 1))

/* A `sigset_t' has a bit for each signal.  */
typedef unsigned long int __sigset_t;

#define	__sigemptyset(set)	((*(set) = 0L), 0)
#define	__sigfillset(set)	((*(set) = -1L), 0)
#define	__sigaddset(set, sig)	((*(set) |= __sigmask (sig)), 0)
#define	__sigdelset(set, sig)	((*(set) &= ~__sigmask (sig)), 0)
#define	__sigismember(set, sig)	((*(set) & __sigmask (sig)) ? 1 : 0)


/* Type of a signal handler.  */
typedef void EXFUN((*__sighandler_t), (int));

/* Structure describing the action to be taken when a signal arrives.  */
struct __sigaction
  {
    /* Signal handler.  */
    __sighandler_t sa_handler;
    
    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* Special flags.  */
    int sa_flags;
  };

/* Bits in `sa_flags'.  */
#define	__SA_ONSTACK	1	/* Take signal on signal stack.  */
#define	__SA_RESTART	2	/* Don't restart syscall on signal return.  */
#define	__SA_NOCLDSTOP	4	/* Don't send SIGCHLD when children stop.  */


#endif	/* gnu/signal.h  */
