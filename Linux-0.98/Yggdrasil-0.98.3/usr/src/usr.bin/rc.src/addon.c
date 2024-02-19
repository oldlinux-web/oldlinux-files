/*
   This file contains the implementations of any locally defined
   builtins.
*/

#ifdef	DWS

/*
   This is what DaviD Sanderson (dws@cs.wisc.edu) uses.
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "rc.h"		/* for boolean TRUE, FALSE */
#include "status.h"	/* for set() */
#include "addon.h"
#include "utils.h"

#include "addon/access.c"
#include "addon/test.c"

#endif
