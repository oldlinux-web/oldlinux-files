#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#include <traditional.h>
#include <sys/types.h>
#include <sys/param.h>

#ifdef OLD_LINUX
#define	LENGTH	8
#else
#define	LENGTH	64
#endif

struct utsname {
	char sysname[LENGTH + 1];
	char nodename[MAXHOSTNAMELEN+1];
	char release[LENGTH + 1];
	char version[LENGTH + 1];
	char machine[LENGTH + 1];
};

#undef	LENGTH

#ifdef __cplusplus
extern "C" {
#endif

extern int uname _ARGS ((struct utsname * __utsbuf));

#ifdef __cplusplus
}
#endif

#endif
