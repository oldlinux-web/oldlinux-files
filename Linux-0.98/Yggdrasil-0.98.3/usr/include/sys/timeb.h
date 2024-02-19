#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H

#include <traditional.h>
#include <sys/types.h>

struct timeb {
	time_t   time;
	unsigned short millitm;
	short    timezone;
	short    dstflag;
};


#ifdef __cplusplus
extern  "C" {
#endif

extern int      ftime _ARGS ((struct timeb *__tp));

#ifdef __cplusplus
}
#endif

#endif /* _SYS_TIMEB_H */
