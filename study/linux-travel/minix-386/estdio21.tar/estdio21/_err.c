/*                                 _ e r r                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

FILE __stderr = {NULL, NULL, NULL, NULL,
                 &__stderr.__buf, 1,
		 _IOSTDERR|_IOINSERT|_IOWRITE|_IONBF,
                 STDERR_FILENO, 0,
                 __bffil, __bwronly, __btfls, NULL};
