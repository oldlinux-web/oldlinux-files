/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* gdebug.h */
/* Debugging machinery for Ghostscript */

/* Debugging options array */
extern char gs_debug[128];
/* If debugging, direct all error output to gs_debug_out. */
#ifdef DEBUG
extern FILE *gs_debug_out;
#undef dstderr
#define dstderr gs_debug_out
#undef estderr
#define estderr gs_debug_out
#endif

/* Insert code conditionally if debugging. */
#ifdef DEBUG
#  define do_debug(x) x
#else
#  define do_debug(x)
#endif

/* Debugging printout macros. */
#ifdef DEBUG
#  define debug_c(c)\
    (c>='a' && c<='z' ? gs_debug[c] | gs_debug[c^32] : gs_debug[c])
#  define if_debug0(c,s)\
    if (debug_c(c)) dprintf(s)
#  define if_debug1(c,s,a1)\
    if (debug_c(c)) dprintf1(s,a1)
#  define if_debug2(c,s,a1,a2)\
    if (debug_c(c)) dprintf2(s,a1,a2)
#  define if_debug3(c,s,a1,a2,a3)\
    if (debug_c(c)) dprintf3(s,a1,a2,a3)
#  define if_debug4(c,s,a1,a2,a3,a4)\
    if (debug_c(c)) dprintf4(s,a1,a2,a3,a4)
#  define if_debug5(c,s,a1,a2,a3,a4,a5)\
    if (debug_c(c)) dprintf5(s,a1,a2,a3,a4,a5)
#  define if_debug6(c,s,a1,a2,a3,a4,a5,a6)\
    if (debug_c(c)) dprintf6(s,a1,a2,a3,a4,a5,a6)
#  define if_debug7(c,s,a1,a2,a3,a4,a5,a6,a7)\
    if (debug_c(c)) dprintf7(s,a1,a2,a3,a4,a5,a6,a7)
#  define if_debug8(c,s,a1,a2,a3,a4,a5,a6,a7,a8)\
    if (debug_c(c)) dprintf8(s,a1,a2,a3,a4,a5,a6,a7,a8)
#  define if_debug9(c,s,a1,a2,a3,a4,a5,a6,a7,a8,a9)\
    if (debug_c(c)) dprintf9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
#  define if_debug0(c,s) 0
#  define if_debug1(c,s,a1) 0
#  define if_debug2(c,s,a1,a2) 0
#  define if_debug3(c,s,a1,a2,a3) 0
#  define if_debug4(c,s,a1,a2,a3,a4) 0
#  define if_debug5(c,s,a1,a2,a3,a4,a5) 0
#  define if_debug6(c,s,a1,a2,a3,a4,a5,a6) 0
#  define if_debug7(c,s,a1,a2,a3,a4,a5,a6,a7) 0
#  define if_debug8(c,s,a1,a2,a3,a4,a5,a6,a7,a8) 0
#  define if_debug9(c,s,a1,a2,a3,a4,a5,a6,a7,a8,a9) 0
#endif
