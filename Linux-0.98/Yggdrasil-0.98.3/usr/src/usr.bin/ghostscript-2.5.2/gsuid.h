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

/* gsuid.h */
/* Unique id definitions for Ghostscript */

/* A unique id (uid) may be either a UniqueID or an XUID. */
/* (XUIDs are a Level 2 feature.) */
typedef struct gs_uid_s {
	union {
		long id;	/* UniqueID, size == 0 */
		long *xvalues;	/* XUID, size != 0 */
	} u;
	ushort size;
} gs_uid;

/* A UniqueID of -1 is an indication that there is no uid. */
#define uid_is_valid(puid)\
  !((puid)->size == 0 && (puid)->u.id == -1)
#define uid_set_invalid(puid)\
  ((puid)->size = 0, (puid)->u.id = -1)
	  
/* Initialize a uid. */
#define uid_set_UniqueID(puid, idv)\
  ((puid)->size = 0, (puid)->u.id = idv)
#define uid_set_XUID(puid, pvalues, siz)\
  ((puid)->size = siz, (puid)->u.xvalues = pvalues)

/* Compare two uids for equality. */
#define uid_equal(puid1, puid2)\
  ((puid1)->size == (puid2)->size &&\
   ((puid1)->size ?\
    !memcmp((const char *)(puid1)->u.xvalues,\
	    (const char *)(puid2)->u.xvalues, (puid1)->size) :\
    (puid1)->u.id == (puid2)->u.id))

/* Free the XUID array of a uid if necessary. */
#define uid_free(puid, proc_free, cname)\
  if ( (puid)->size )\
    (proc_free)((char *)(puid)->u.xvalues, (puid)->size, sizeof(long), cname)
