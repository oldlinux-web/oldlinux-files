/* tm.h for 486 running system V.4.  */

#include "i386v4.h"

/* By default, optimize code for the 486.  */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT	(1|2)
