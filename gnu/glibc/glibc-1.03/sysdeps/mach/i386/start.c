/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* The first piece of initialized data.  */
int __data_start = 0;

volatile int errno;

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif
char **__environ;

extern void EXFUN(__mach_init, (NOARGS));
extern void EXFUN(__libc_init, (int argc, char **argv, char **envp));
extern int EXFUN(main, (int argc, char **argv, char **envp));

extern void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
extern void (*_cthread_exit_routine) (int status);

void
_start (void)
{
  int *entry_sp;
  int argc;
  char **argv, **p;

  asm ("leal 4(%%ebp), %0" : "=r" (entry_sp));

  argc = *entry_sp;
  argv = (char **) (entry_sp + 1);
  p = argv;
  while (*p++ != NULL)
    ;
  if (p >= (char **) argv[0])
    --p;
  __environ = p;

  __mach_init ();

  __libc_init (argc, argv, __environ);

  if (_cthread_init_routine != NULL)
    asm volatile ("movl %0, %%esp" : /* No outputs.  */ :
		  "g" ((*_cthread_init_routine) ()));
  (_cthread_exit_routine != NULL ? *_cthread_exit_routine : exit)
    (main (argc, argv, __environ));

  /* Should never get here.  */
  _exit (-1);
}
