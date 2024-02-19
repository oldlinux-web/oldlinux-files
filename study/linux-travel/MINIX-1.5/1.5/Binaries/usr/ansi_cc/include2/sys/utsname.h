/* The <sys/utsname.h> header gives the system name. */

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#define _UTSLEN	(8 + 1)
#define _SNLEN	(14 + 1)

struct utsname {
  char sysname[_UTSLEN];
  char nodename[_UTSLEN];
  char release[_UTSLEN];
  char version[_UTSLEN];
  char machine[_UTSLEN];
  char idnumber[_SNLEN];
};

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( int uname, (struct utsname *_name)				);

#endif /* _SYS_UTSNAME_H */
