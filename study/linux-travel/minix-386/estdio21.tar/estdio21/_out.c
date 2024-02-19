/*                                 _ o u t                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

static __stdiobuf_t __bout[BUFSIZ];

FILE __stdout = {NULL, NULL, NULL, NULL,
                 &__bout[0], sizeof(__bout)/sizeof(__bout[0]),
		 _IOSTDOUT|_IOINSERT|_IOWRITE|_IOFBF|_IOPOOLBUF,
                 STDOUT_FILENO, 0,
                 __bffil, __bwronly, __btfls, NULL};
