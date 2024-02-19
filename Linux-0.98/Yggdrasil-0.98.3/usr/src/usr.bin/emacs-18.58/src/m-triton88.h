/* m- file for Motorola System V/88 machines
   Copyright (C) 1985 Free Software Foundation, Inc.

This file is not a part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */

#include "m-delta88.h"

#ifndef TRITON88
#define TRITON88
#endif /* TRITON88 */

/* I don't think Trinto88 needs this. */

#ifdef BROKEN_FIONREAD
#undef BROKEN_FIONREAD
#endif /* BROKEN_FIONREAD */

/* Need this to prevent Ctrl-Z from suspending Emacs before suspend-emacs
   have been called. */

#define HAVE_TCATTR

/* libc defines the following functions. */

#define HAVE_RENAME
#define HAVE_CLOSEDIR
#define HAVE_DUP2
#define HAVE_SETSID

/* Have to this because of the brain damaged malloc in libc on Triton88
   machines. */

#define C_SWITCH_SYSTEM -Dmalloc=_malloc -Dfree=_free  -Drealloc=_realloc -Dcalloc=_calloc

/* There is no termio.h.  */

#define NO_TERMIO
