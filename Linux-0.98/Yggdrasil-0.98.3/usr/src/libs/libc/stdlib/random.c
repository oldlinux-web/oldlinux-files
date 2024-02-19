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
#include <stdlib.h>

#undef	random
#undef	srandom
#undef	initstate
#undef	setstate

#include <gnu-stabs.h>

function_alias(random, __random, long int, (),
	       DEFUN_VOID(random))
function_alias(srandom, __srandom, void, (seed),
	       DEFUN(srandom, (seed), unsigned int seed))
function_alias(initstate, __initstate, PTR, (seed, buf, size),
	       DEFUN(initstate, (seed, buf, size),
		     unsigned int seed AND PTR buf AND size_t size))
function_alias(setstate, __setstate, PTR, (buf),
	       DEFUN(setstate, (buf), PTR buf))

