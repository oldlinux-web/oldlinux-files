/* last.c - poe@daimi.aau.dk, simple util. for perusing wtmp */

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include "utmp.h"
#include "pathnames.h"
#include <time.h>
#include <sys/time.h>

main(int argc, char *argv[])
{
	int fd,pos;
	struct utmp ut;
		
	fd = open(_PATH_WTMP, O_RDONLY);
	if(fd < 0) {
		perror("last: Can't open wtmp file");
		exit(1);
	}
	
	(void)lseek(fd, (off_t)sizeof(struct utmp), SEEK_END);
	
	printf("Tty User     Time\n");
	while((pos = lseek(fd, (off_t)-2*sizeof(struct utmp), SEEK_CUR)) >= 0) {
		(void)read(fd, (char *)&ut, sizeof(struct utmp));
		printf("%2s  %-8s %s", ut.ut_id, ut.ut_user,
			ctime(&ut.ut_time));
	}
	close(fd);
}

