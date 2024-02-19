/* write.c - poe@daimi.aau.dk */
/* also source for wall(1), make a link wall -> write */

#include <sys/types.h>
#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include "pathnames.h"

#define BSIZ 1600

char message[BSIZ];
char *prog;

usage()
{
	fprintf(stderr, "Usage: write user ...\n");
	exit(0);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	char *p;
	
	prog = argv[0];
	if(p = strrchr(argv[0], '/')) prog = p + 1;

	if(!strcmp("wall", prog)) {
		get_message();
		
		wall();
	} else {
		if(argc < 2) usage();

		get_message();
	
		for(i = 1; i < argc; i++)
			write_user(argv[i]);
	}
}

get_message()
{
	char *buf, c;
	int siz = 0;
	
	siz = fread(message, 1, BSIZ, stdin);
	message[siz] = '\0';
}

wall()
{
	/* write to all users. */
	struct utmp *ut;
		
	utmpname(_PATH_UTMP);
	setutent();
	
	while(ut = getutent()) {
		if(ut->ut_type == USER_PROCESS)
			write_line(ut);
	}
	endutent();
}

write_user(char *user)
{
	struct utmp *ut;

	utmpname(_PATH_UTMP);
	setutent();
	
	while(ut = getutent()) {
		if(ut->ut_type == USER_PROCESS 
		   && !strncmp(user, ut->ut_user, sizeof(ut->ut_user)))
			write_line(ut);
	}
	endutent();
}

write_line(struct utmp *ut)
{
	FILE *f;
	int minutes;
	char term[40] = {'/','d','e','v','/',0};
	
	(void) strcat(term, ut->ut_line);
		
	if(f = fopen(term, "w")) {
		fprintf(f, "\r\nMessage from %s:\r\n", getlogin());
		fwrite(message, 1, strlen(message), f);
		fclose(f);
	} else {
		fprintf(stderr, "Couldn't write %-8s at %s\n",
			ut->ut_user, ut->ut_line);
	}
}
