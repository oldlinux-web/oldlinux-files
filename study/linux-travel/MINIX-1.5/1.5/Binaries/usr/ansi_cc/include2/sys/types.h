/* The <sys/types.h> header contains important data type definitions.
 * It is considered good programming practice to use these definitions, 
 * instead of the underlying base type.  By convention, all type names end 
 * with _t.
 */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/* The type size_t holds the result of the size_of operator.  At first glance,
 * it seems obvious that it should be an unsigned int, but this is not always 
 * the case. For example, MINIX-ST (68000) has 32-bit pointers and 16-bit
 * integers. When one asks for the size of a 70K struct or array, the result 
 * requires 17 bits to express, so size_t must be a long type.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	  /* type returned by sizeof */
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		/* time in sec since 1 Jan 1970 0000 GMT */
#endif

typedef unsigned short	dev_t;	  /* holds (major|minor) device pair */
typedef unsigned char	gid_t;	  /* group id */
typedef unsigned short	ino_t;	  /* i-node number */
typedef unsigned short	mode_t;	  /* mode number within an i-node */
typedef unsigned char	nlink_t;  /* number-of-links field within an i-node */
typedef long		off_t;	  /* offsets within a file */
typedef int		pid_t;	  /* type for pids (must be signed) */
typedef unsigned short	uid_t;	  /* user id */

#endif /* _SYS_TYPES_H */
