/*
 * Resource control/accounting header file for linux
 */

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <traditional.h>
#include <sys/time.h>
#include <linux/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	getrlimit _ARGS ((int __resource,
			struct rlimit *__rlp));
extern int	setrlimit _ARGS ((int __resource,
			const struct rlimit *__rlp));
extern int	getrusage _ARGS ((int __who, struct rusage *__rusage));

extern int      getpriority _ARGS((int __which, int __who));
extern int      setpriority _ARGS((int __which, int __who,
			int __prio));

#ifdef __cplusplus
}
#endif

#endif /* _SYS_RESOURCE_H */
