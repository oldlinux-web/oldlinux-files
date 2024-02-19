/* who.c - simple utility to display the active users */

#include <stdio.h>
#include <fcntl.h>
#include "utmp.h"
#include "pathnames.h"

main()
{
	struct utmp *ut;
	
	utmpname(_PATH_UTMP);
	setutent();
	
	printf("User     TTY Login-time\n");
	while(ut = getutent()) {
		if(ut->ut_type == USER_PROCESS)
			printf("%-8s  %-2s %s", ut->ut_user, ut->ut_id,
				ctime(&ut->ut_time));
	}
	endutent();
}
	