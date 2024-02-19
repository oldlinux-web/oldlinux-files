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
#include <stddef.h>
#include <setjmp.h>
#include <signal.h>

#ifdef linux
extern sigset_t ___sgetmask();
#endif

/* Store the calling environment in ENV, also saving the
   signal mask if SAVEMASK is nonzero.  Return 0.  */
int
DEFUN(sigsetjmp, (env, savemask), sigjmp_buf env AND int savemask)
{
  if (savemask)
#ifdef linux
    env[0].__sigmask = ___sgetmask(), env[0].__savemask = 1;
#else
    env[0].__savemask = sigprocmask(SIG_BLOCK, (sigset_t *) NULL,
				    &env[0].__sigmask) == 0;
#endif
  else
    env[0].__savemask = 0;

#define REGS \
  REG (bx);\
  REG (si);\
  REG (di)

  /* Save the general registers.  */
#define REG(xx) asm volatile ("movl %%e" #xx ",%0" : \
	"=m" ((long int) ((env[0].__jmpbuf)[0].__##xx)) : )
  REGS;

#ifdef SAVE_387_CONTROL_WORD
  asm volatile ("fnstcw %0" : "=m" ((env[0].__jmpbuf)[0].__cw) : );
#endif

  /* Save the return PC.  */
  (env[0].__jmpbuf)[0].__pc = (PTR) ((PTR *) &env)[-1];

  /* Save caller's FP, not our own.  */
  (env[0].__jmpbuf)[0].__bp = (PTR) ((PTR *) &env)[-2];

  /* Save caller's SP, not our own.  */
  (env[0].__jmpbuf)[0].__sp = (PTR) &env;

  return 0;
}
