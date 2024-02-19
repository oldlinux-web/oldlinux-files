/*                             _ z w r a p u p                             */

/* __wrapup points at the stdio wrap up function which is called from
 * exit(). This is initialised to NULL. When _bwr() is called, it
 * initialises __wrapup to point to __ioflush().
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

void (*__Zwrapup) F0();			/* clean up pointer */
					/* bss cleared at startup */
