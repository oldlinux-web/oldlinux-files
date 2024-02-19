#ifndef LINT
/* @(#) machine.c 2.3 88/01/02 01:21:44 */
static char sccsid[]="@(#) machine.c 2.3 88/01/02 01:21:44";
#endif /* LINT */

/*
The contents of this file are hereby released to the public domain.

                                    -- Rahul Dhesi  1986/12/31
*/

/* This file is in two parts. */

#include "options.h"
#include "zooio.h"
#include "zoo.h"
#include "zoofns.h"
#include "various.h"

/***********************************************************************/
/* PART 1.  FOR UNBUFFERED I/O ONLY.  DO NOT CHANGE.                   */
/***********************************************************************/

#ifdef UNBUF_IO
int write PARMS ((int, VOIDPTR, unsigned));

/*
blockwrite() is like write() except that it ignores all
output to file descriptor -2, which stands for the null file.
*/
int blockwrite (fd, buf, count)
int fd;
#ifdef VOIDPTR
VOIDPTR buf;
#else
char *buf;
#endif /* VOIDPTR */
unsigned count;
{
	if (fd == -2)
		return (count);
	else
		return (write (fd, buf, count));
}
#endif

/***********************************************************************/
/* PART 2.  FOR EACH SPECIFIC SYSTEM, INCLUDE A C FILE HERE.           */
/***********************************************************************/

#ifdef SYS_V
#include "sysv.c"
#endif

#ifdef GENERIC
#include "generic.c"
#endif

#ifdef BSD4_3
#include "bsd.c"
#endif

#ifdef DLC
#include "generic.c"
#endif

#ifdef VMS
#include "vms.c"
#endif

#ifdef MSC
#include "ERROR -- NOT SUPPORTED"
#endif

#ifdef TURBOC
#ifdef PORTABLE
#include "generic.c"
#else
#include "turboc.c"
#endif
#endif
