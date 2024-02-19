/*                                _ z o u t                                */

/* This file contains a pointer to stdout. It is initialised to
 * NULL and set by _bwronly(). It is used by _brd(), fread() and
 * _fgetlx() to flush stdout on reads.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *__Zout;				/* stdout pointer */
					/* bss cleared at startup */
