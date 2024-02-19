#include <amoeba.h>
#include "header.h"

char buffer[BUF_SIZE];

main(argc, argv)
int argc;
char *argv[];
{
  unshort getreq(), putrep();

  header hdr;
  int cnt, i, iterations = 0;

  if (argc != 2) {
	printf("Usage: server1 portname\n");
	exit(1);
  }

  strncpy( (char *) &hdr.h_port, argv[1], PORTSIZE);  /* init port */

  while (1) {

	/* Wait for a request to arrive. */
	if ((cnt = (short) getreq(&hdr, buffer, BUF_SIZE)) < 0) {
	    printf("Server's getreq failed. Error = %d.   ", cnt);
	    printf("Hit F1 to see if AMTASK running.\n");
	    exit(1);
	}
	
	/* Opcode QUIT indicates that we are done. */
	if (hdr.h_command == QUIT) {
		putrep(&hdr, buffer, 0);
		exit(0);
	}

	/* We have a request. Increment each byte. */
	for (i = 0; i < cnt; i++) buffer[i]++;

	/* Send back reply. */
	hdr.h_status = 0;
	putrep(&hdr, buffer, cnt);
  }
}
