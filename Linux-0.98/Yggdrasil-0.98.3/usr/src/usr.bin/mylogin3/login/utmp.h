/* utmp.h for Linux, by poe@daimi.aau.dk */

#ifndef UTMP_H
#include <sys/types.h>
#include <time.h>

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
	char	ut_line[12];	/* full devicename of tty, null-term */
	char	ut_id[2];	/* abbrev. ttyname, as 01, s1 etc. */
	time_t	ut_time;	/* logintime */
	char	ut_user[8];	/* username, not null-term */
	char	ut_host[16];	/* hostname for remote login... */
	long	ut_addr;	/* IP addr of remote host */
};

void		setutent(void);
void		utmpname(const char *);
struct utmp	*getutent(void);
struct utmp	*getutid(struct utmp *);
struct utmp 	*getutline(struct utmp *);
void		pututline(struct utmp *);
struct utmp	*_getutline(struct utmp *);
void		endutent(void);

#endif	