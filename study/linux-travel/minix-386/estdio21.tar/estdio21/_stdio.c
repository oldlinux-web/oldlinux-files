/*                               _ s t d i o                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE *__iop;				/* stream list pointer */
					/* bss cleared at startup */

__stdiobuf_t *__iob[POOLSIZE];		/* buffer pool */
					/* bss cleared at startup */

#ifdef		MSDOS
int _fmode = O_BINARY;
#endif
