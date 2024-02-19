#ifndef _UTIME_H
#define _UTIME_H

#include <traditional.h>
#include <sys/types.h>	/* I know - shouldn't do this, but .. */
#include <linux/utime.h>

#ifdef __cplusplus
extern  "C" {
#endif

extern int utime _ARGS ((const char *__filename,
		struct utimbuf *__times));

#ifdef __cplusplus
}
#endif

#endif
