#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <traditional.h>
#include <sys/types.h>
#include <sys/resource.h>

#define _LOW(v)		( (v) & 0377)
#define _HIGH(v)	( ((v) >> 8) & 0377)

#define WNOHANG		1
#define WUNTRACED	2

#define WIFEXITED(s)	(!((s)&0xFF))
#define WIFSTOPPED(s)	(((s)&0xFF)==0x7F)
#define WEXITSTATUS(s)	(((s)>>8)&0xFF)
#define WTERMSIG(s)	((s)&0x7F)
#define WCOREDUMP(s)	((s)&0x80)
#define WSTOPSIG(s)	(((s)>>8)&0xFF)
#define WIFSIGNALED(s)	(((unsigned int)(s)-1 & 0xFFFF) < 0xFF)

/*
 * Tokens for special values of the "pid" parameter to wait4.
 */
#define WAIT_ANY        (-1)    /* any process */
#define WAIT_MYPGRP     0       /* any process in my process group */

#ifdef __cplusplus
extern "C" {
#endif

extern pid_t	wait _ARGS ((int *__wait_stat));
extern pid_t	waitpid _ARGS ((pid_t __pid, int *__wait_stat,
			int __options));

/* The last parameter is not implemented. Good in 0.97 pl 3 or
   above. */
extern pid_t	wait3 _ARGS ((int *__wait_stat, int __options,
			struct rusage *__rup));
extern pid_t	wait4 _ARGS ((pid_t __pid, int *__wait_stat, int __options,
			 struct rusage *__rup));

#ifdef __cplusplus
}
#endif

#endif
