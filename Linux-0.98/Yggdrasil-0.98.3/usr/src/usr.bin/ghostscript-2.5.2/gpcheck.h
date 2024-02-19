/* Copyright (C) 1992 Aladdin Enterprises.  All rights reserved.
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

/* gpcheck.h */
/* Interrupt check interface for Ghostscript */

/*
 * On some platforms, Ghostscript must check periodically for user-
 * initiated actions.  (Eventually, this may be extended to all platforms,
 * to handle multi-tasking within Ghostscript.)  Routines that run for
 * a long time must periodically call gp_check_interrupts(), and if it
 * returns true, must clean up whatever they are doing and return an
 * e_interrupted (or gs_error_interrupted) exceptional condition.
 *
 * On platforms that require an interrupt check, the makefile defines
 * a symbol CHECK_INTERRUPTS.  Currently this is only the Microsoft
 * Windows platform.
 */

#ifdef CHECK_INTERRUPTS
extern int gp_check_interrupts(P0());
#else
#  define gp_check_interrupts() 0
#endif
