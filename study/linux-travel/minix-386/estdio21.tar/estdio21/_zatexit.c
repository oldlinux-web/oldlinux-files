/*                             _ z a t e x i t                             */

/* This file contains a pointer to the exit handler dispatch code.
 * It is initialise to NULL and set by atexit(). It is used by
 * exit() to call the exit handler code.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

void (*__Zatexit) F0();			/* exit handler pointer */
					/* bss cleared at startup */
