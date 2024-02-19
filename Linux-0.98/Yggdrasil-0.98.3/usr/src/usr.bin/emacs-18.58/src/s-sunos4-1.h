#include "s-sunos4-0.h"

/* 4.1.1 makes these system calls interruptable.  */

#define read sys_read
#define write sys_write
#define open sys_open
#define close sys_close

#define INTERRUPTABLE_OPEN
#define INTERRUPTABLE_CLOSE
#define INTERRUPTABLE_IO


#define HAVE_TZSET

/* Cause the compilation of oldxmenu to use the right -I option.  */
#define OLDXMENU_OPTIONS CFLAGS=C_SWITCH_SYSTEM

/* The following is needed to work with the "Open Windows"
   version of X windows.  But it should do no harm if you don't have that.  */
#if 1  /* The -I and -L options should be harmless otherwise.  */
#undef LD_SWITCH_SYSTEM
#define LD_SWITCH_SYSTEM -e __start -Bstatic -L/usr/openwin/lib

#define C_SWITCH_SYSTEM -I/usr/openwin/include
#endif 

/* Enable a fix in malloc.  */
#define SUNOS_LOCALTIME_BUG
