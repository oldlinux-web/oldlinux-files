#ifndef _TIME_H
#define _TIME_H

#include <features.h>
#include <traditional.h>

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long clock_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define CLOCKS_PER_SEC	100
#define CLK_TCK		100	/* That must be the same as HZ ???? */

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

#define	__isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

extern char *tzname[2];
extern int daylight;
extern long int timezone;

#ifdef __cplusplus
extern  "C" {
#endif

extern int	stime _ARGS ((time_t* __tptr));

extern clock_t	clock _ARGS ((void));
extern time_t	time _ARGS ((time_t * __tp));
extern double	difftime _ARGS ((time_t __time2, time_t __time1));
extern time_t	mktime _ARGS ((struct tm * __tp));

extern char *	asctime _ARGS ((const struct tm * __tp));
extern char *	ctime _ARGS ((const time_t * __tp));
extern size_t	strftime _ARGS ((char * __s, size_t __smax,
			const char * __fmt, const struct tm * __tp));
extern void	tzset _ARGS ((void));

extern struct tm*	gmtime _ARGS ((const time_t *__tp));
extern struct tm*	localtime _ARGS ((const time_t * __tp));

#ifdef __cplusplus
}
#endif

#endif
