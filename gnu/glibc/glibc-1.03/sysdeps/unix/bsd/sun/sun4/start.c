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
#include <stdlib.h>
#include <unistd.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* The first piece of initialized data.  */
int __data_start = 0;

VOLATILE int errno;

#ifndef	HAVE_GNU_LD
#undef	environ
#define	__environ	environ
#endif

char **__environ;

extern void EXFUN(__libc_init, (int argc, char **argv, char **envp));
extern int EXFUN(main, (int argc, char **argv, char **envp));

register long int sp asm("%sp"), fp asm("%fp");

void
DEFUN_VOID(_start)
{
  int argc;
  char **argv, **envp;

  /* Unwind the frame built when we entered the function.  */
  asm("restore");

  fp = 0;
  /* The argument info starts after one register
     window (64 bytes) past the SP.  */
  argc = ((int *) sp)[16];
  argv = (char **) &((int *) sp)[17];
  envp = &argv[argc + 1];
  __environ = envp;
  /* Allocate 24 bytes of stack space for the register save area.  */
  sp -= 24;
  __libc_init (argc, argv, envp);
  exit (main (argc, argv, envp));
}
