#ifndef _TIMES_H
#define _TIMES_H

#include <traditional.h>
#include <sys/types.h>
#include <linux/times.h>

#ifdef __cplusplus
extern "C" {
#endif

extern clock_t times _ARGS ((struct tms * __tp));

#ifdef __cplusplus
}
#endif

#endif
