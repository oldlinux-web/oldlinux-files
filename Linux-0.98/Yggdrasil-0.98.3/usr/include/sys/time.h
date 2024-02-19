#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <traditional.h>
#include <linux/time.h>

#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */
#define	DST_GB		7	/* Great Britain and Eire */
#define	DST_RUM		8	/* Rumania */
#define	DST_TUR		9	/* Turkey */
#define	DST_AUSTALT	10	/* Australian style with shift in 1986 */

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 * 
 * Note: timercmp may work for for >= or <= now. H.J.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
	(((tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec) \
	|| (tvp)->tv_sec cmp (uvp)->tv_sec)
#define	timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	gettimeofday _ARGS ((struct timeval * __tp,
			struct timezone * __tz));
extern int	settimeofday _ARGS ((const struct timeval *__tv,
			const struct timezone *__tz));
extern int	select _ARGS ((int __width, fd_set * __readfds,
			fd_set * __writefds, fd_set * __exceptfds,
			struct timeval * __timeout));

extern int	getitimer _ARGS ((int __which,
			struct itimerval *__value));
extern int	setitimer _ARGS ((int __which,
			const struct itimerval *__value,
			struct itimerval *__ovalue));

#ifdef __cplusplus
}
#endif

#endif /*_SYS_TIME_H*/
