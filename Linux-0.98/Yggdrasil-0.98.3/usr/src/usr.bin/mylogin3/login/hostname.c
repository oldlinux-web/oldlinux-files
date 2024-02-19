/* hostname.c - poe@daimi.aau.dk */

#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>


main(int argc, char *argv[])
{
	char hn[MAXHOSTNAMELEN + 1];
	
	if(argc >= 2) {
		if(geteuid() || getuid()) {
			puts("You must be root to change the hostname");
			exit(1);
		}
		if(strlen(argv[1]) > MAXHOSTNAMELEN) {
			puts("That name is too long.");
			exit(1);
		}
		sethostname(argv[1], strlen(argv[1]));
	} else {
		gethostname(hn, MAXHOSTNAMELEN);
		puts(hn);
	}
}
