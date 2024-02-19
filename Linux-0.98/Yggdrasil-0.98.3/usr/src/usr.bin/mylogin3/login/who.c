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
	
	printf("Tty User     Login-time\n");
	while(ut = getutent()) {
		printf("%2s  %-8s %s", ut->ut_id, ut->ut_user,
			ctime(&ut->ut_time));
	}
	endutent();
}
	