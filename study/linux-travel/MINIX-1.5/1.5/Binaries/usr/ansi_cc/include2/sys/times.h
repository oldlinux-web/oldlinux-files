/* The <times.h> header is for time times() system call. */

#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#ifndef _CLOCK_T
#define _CLOCK_T
typedef	long clock_t;		/* time in ticks since process started */
#endif

struct tms {
  clock_t tms_utime;
  clock_t tms_stime;
  clock_t tms_cutime;
  clock_t tms_cstime;
};


/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( clock_t times, (struct tms *_buffer)			);

#endif /* _SYS_TIMES_H */
