#ifndef _LIMITS_H
#define _LIMITS_H

#include <linux/limits.h>

#ifndef RAND_MAX
/* The largest number rand will return (same as INT_MAX).  */
#define RAND_MAX		2147483647
#endif

/* Those valueas are minimum values */

/* The length of the arguments for one of the exec functions in bytes,
 * including environment data.
 */
#define _POSIX_ARG_MAX		4096
/* The number of simultaneous processes per real user ID. */
#define _POSIX_CHILD_MAX	6
/* The value of a files link count. */
#define _POSIX_LINK_MAX		8
/* The number of bytes in a terminal canonical input line. */
#define _POSIX_MAX_CANON	255
/* The number of bytes for which will be available in a terminal
 * input queue.
 */
#define _POSIX_MAX_INPUT	255
/* The number of bytes in a file name. */
#define _POSIX_NAME_MAX		14
/* The number of simultaneous supplementary group IDs per process. */
#define _POSIX_NGROUPS_MAX	0
/* The number of files that one process can have open at one time. */
#define _POSIX_OPEN_MAX		16
/* The number of bytes in a pathname. */
#define _POSIX_PATH_MAX		255
/* pipes writes of at least 512 bytes can be atomic */
#define _POSIX_PIPE_BUF		512
/* The number of bytes that can be written atomically when writing to
 * a pipe.
 */
#define _POSIX_SSIZE_MAX	32767
/* The maximum number of bytes supported for the name of a time zone
 * (not of the TZ variable).
 */
#define _POSIX_TZNAME_MAX	3
/* The number of streams that on process can have open at one time. */
#define _POSIX_STREAM_MAX	8

/*
 * The following limit is also available using the sysconf() function.
 * Use of sysconf() is advised over use of the constant value defined
 * here, since it should pose fewer portability and
 * forward-compatability problems.
 */
#define STREAM_MAX		OPEN_MAX
#define SSIZE_MAX		2147483647

/* don't even think about changing it without checking tzfile.h
 * in source code dir ./time first.
 */
#define TZNAME_MAX		50

#define UCHAR_MAX 255
#define CHAR_MAX UCHAR_MAX

#endif /* _LIMITS_H */
