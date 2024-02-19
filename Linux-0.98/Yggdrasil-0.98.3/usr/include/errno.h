#ifndef _ERRNO_H
#define _ERRNO_H

#include <traditional.h>
#include <linux/errno.h>

#ifdef __STDC__
extern const char* const sys_errlist[];
#else
extern char* sys_errlist[];
#endif
extern int	sys_nerr;
extern int	errno;

#ifdef __cplusplus
extern "C" {
#endif

extern void	perror _ARGS ((const char*));
extern char*	strerr _ARGS ((int));

#ifdef __cplusplus
}
#endif

#endif
