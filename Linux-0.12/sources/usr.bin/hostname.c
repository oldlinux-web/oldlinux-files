/*
 * Set or display hostname.  Jeff Comstock - Bloomington, MN USA 1992
 * Usage: hostname [name]
 * Only root may change the hostname.
*/
#include <stdio.h>
#include <unistd.h>

main(int argc, char **argv) {
struct utsname uts;

	if ( argc == 2 ) {
		if ( sethostname(argv[1],strlen(argv[1]))) {
			perror("sethostname");
			exit(1);
		}
	}
	else {
		if (uname(&uts)) {
			perror("uname");
			exit(1);
		}
		else 
			puts(uts.nodename);
	}
	return(0);
}
