/*                                _ z e r r                                */

/* This file contains a pointer to stderr. It is initialised to
 * NULL and set by _bwronly(). It is used by perror() to flush
 * stderr.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *__Zerr;				/* stderr pointer */
					/* bss cleared at startup */
