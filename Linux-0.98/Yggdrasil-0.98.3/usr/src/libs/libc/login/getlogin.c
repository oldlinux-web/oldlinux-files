/* getlogin.c - poe@daimi.aau.dk */

#include <sys/types.h>
#include <fcntl.h>
#include <utmp.h>
#include <unistd.h>
#include <memory.h>
#include "pathnames.h"

static char name[9];

char *ttyname(int);

/* return logname of the user of "this" tty */
char *getlogin(void)
{
	char *ttyn;
	struct utmp ut;
	int fd;
	
	if(!(ttyn = ttyname(0)))
		return (char *)0;

	/* we cannot use the getutline() function, 'cause that
	   would prohibit interleavig of getlogin() and getut...()
	   functions, eg. in write(1) */
	
	if((fd = open(_PATH_UTMP, O_RDONLY)) >= 0) {
		while(read(fd, (char *)&ut, sizeof(ut)) == sizeof(ut)) {
			if(!strncmp(ttyn + 5, ut.ut_line, sizeof(ut.ut_line))) {
				strncpy(name, ut.ut_user, sizeof(ut.ut_user));
				close(fd);
				return name;
			}
		}
		close(fd);
	}
	return (char *)0;
}

#ifdef DEBUGGING
main()
{
	puts(getlogin());
}
#endif
