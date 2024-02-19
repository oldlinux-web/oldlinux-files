/*                                  _ i n                                  */

#include "stdiolib.h"

/*LINTLIBRARY*/

static __stdiobuf_t __bin[BUFSIZ];

FILE __stdin  = {NULL, NULL, NULL, NULL,
                 &__bin[0], sizeof(__bin)/sizeof(__bin[0]),
		 _IOSTDIN|_IOINSERT|_IOREAD|_IOFBF|_IOPOOLBUF,
                 STDIN_FILENO, 0,
                 __brdonly, __bffls, __btfls, NULL};
