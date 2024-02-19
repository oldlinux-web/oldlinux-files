/* utmp.h for Linux, by poe@daimi.aau.dk */

#ifndef UTMP_H
#define UTMP_H

#include <traditional.h>
#include <sys/types.h>
#include <time.h>

#define UTMP_FILE	"/etc/utmp"
#define WTMP_FILE	"/etc/wtmp"

#define UTMP_FILENAME	UTMP_FILE
#define WTMP_FILENAME	WTMP_FILE

/* defines for the ut_type field */
#define UT_UNKNOWN	0

#define RUN_LVL		1
#define BOOT_TIME	2
#define NEW_TIME	3
#define OLD_TIME	4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8


struct utmp {
	short	ut_type;	/* type of login */
	pid_t	ut_pid;		/* pid of login-process */
	char	ut_line[12];	/* devicename of tty -"/dev/", null-term */
	char	ut_id[2];	/* abbrev. ttyname, as 01, s1 etc. */
	time_t	ut_time;	/* logintime */
	char	ut_user[8];	/* username, not null-term */
	char	ut_host[16];	/* hostname for remote login... */
	long	ut_addr;	/* IP addr of remote host */
};

#define ut_name ut_user

#ifdef __cplusplus
extern "C" {
#endif

extern void		setutent _ARGS ((void));
extern void		utmpname _ARGS ((const char *));
extern struct utmp	*getutent _ARGS ((void));
extern struct utmp	*getutid _ARGS ((struct utmp *));
extern struct utmp 	*getutline _ARGS ((struct utmp *));
extern void		pututline _ARGS ((struct utmp *));
extern struct utmp	*_getutline _ARGS ((struct utmp *));
extern void		endutent _ARGS ((void));

#ifdef __cplusplus
}
#endif

#endif	
