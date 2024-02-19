/*                               _ z r l b f                               */

/* This file contains a pointer to __rlbf(). It is initialised to
 * NULL and set by setvbuf(). It is used by _brd() to adjust line
 * buffered input streams.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

__stdiosize_t (*__Zrlbf) P((FILE *, __stdiosize_t));	/* rlbf() pointer */
					/* bss cleared at startup */
