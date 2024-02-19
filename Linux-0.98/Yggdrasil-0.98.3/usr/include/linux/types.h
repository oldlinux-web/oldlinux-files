#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long clock_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef int ptrdiff_t;
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

typedef int pid_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef unsigned short dev_t;
#ifdef OLD_LINUX
typedef unsigned short ino_t;
#else
typedef unsigned long ino_t;
#endif
typedef unsigned short mode_t;
typedef unsigned short umode_t;
typedef unsigned short nlink_t;
typedef int daddr_t;
typedef long off_t;

/* bsd */
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

/* sysv */
typedef unsigned char unchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef char *caddr_t;

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned long tcflag_t;

typedef unsigned long fd_set;

struct ustat {
	daddr_t f_tfree;
	ino_t f_tinode;
	char f_fname[6];
	char f_fpack[6];
};

#endif
