/* derived from: zoomem.h 2.1 87/12/25 12:26:18 */
/* $Path$ */
/* $Id: zoomem.h,v 1.3 91/07/09 01:43:06 dhesi Exp $ */

/*
(C) Copyright 1991 Rahul Dhesi -- All rights reserved

Defines parameters used for memory allocation.
*/

/* ZOOCOUNT is the number of archive names that may be matched by the
archive filespec specified for a list.

MAXADD is the number of filenames that may be added to an archive
at one go.  The total number of files that an archive may contain
is not determined by MAXADD but is determined by available memory.
*/

#ifdef   SMALL_MEM
#define  ZOOCOUNT   (30)
#define  MAXADD     (100)
#endif

#ifdef   MED_MEM
#define  ZOOCOUNT   (50)
#define  MAXADD     (200)
#endif

#ifdef   BIG_MEM
#define  ZOOCOUNT   (400)
#define  MAXADD     (4000)
#endif

/* Customizable sizes */
#ifdef   SPEC_MEM
#define  ZOOCOUNT    (100)
#define  MAXADD      (400)
#endif

extern char *out_buf_adr;              /* global I/O buffer */

/*************************************************************/
/* DO NOT CHANGE THE REST OF THIS FILE.                      */
/*************************************************************/

/*
The main I/O buffer (called in_buf_adr in zoo.c) is reused
in several places.
*/

#define  IN_BUF_SIZE       8192
#define  OUT_BUF_SIZE      8192

/* MEM_BLOCK_SIZE must be no less than (2 * DICSIZ + MAXMATCH)
(see ar.h and lzh.h for values).  The buffer of this size will
also hold an input buffer of IN_BUF_SIZE and an output buffer
of OUT_BUF_SIZE.  FUDGE is a fudge factor, to keep some spare and
avoid off-by-one errors. */

#define FUDGE		8
#define  MEM_BLOCK_SIZE    (8192 + 8192 + 256 + 8)

