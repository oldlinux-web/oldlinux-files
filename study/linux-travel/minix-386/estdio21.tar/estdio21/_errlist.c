/*                             _ e r r l i s t                             */

/* This encodes a table of system error messages for perror.
 * The error messages themselves are stored as a sequence
 * of strings in errlist.h. This code is conditionally
 * compiled since we prefer to use the one in the system itself.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

#ifdef		ERRLIST
int __errlist = 1;			/* installation flag */

char *sys_errlist[] = {
#include "errlist.h"
};

int sys_nerr = sizeof(sys_errlist) / sizeof(sys_errlist[0]);
#endif
