#include <amoeba.h>
#include "header.h"

char buffer[MAX_TRANS];

main(argc, argv)
int argc;
char *argv[];
{

  header hdr;
  int count;

  if (argc != 2) {
	printf("Usage: server2 portname\n");
	exit(1);
  }

  strncpy( (char *) &hdr.h_port, argv[1], PORTSIZE);  /* init port */

  while (1) {
	/* Wait for a request to arrive. */
	count = (short) getreq(&hdr, buffer, MAX_TRANS);
	if (count < 0) {
	    printf("Server's getreq failed. Error = %d.   ", count);
	    printf("Hit F1 to see if AMTASK running.\n");
	    exit(1);
	}
	
	/* Opcode QUIT indicates that we are done. */
	if (hdr.h_command == QUIT) {
		putrep(&hdr, buffer, 0);
		exit(0);
	}

	/* We have a request. Reply. */
	count = hdr.h_size;
	putrep(&hdr, buffer, count);
  }
}
