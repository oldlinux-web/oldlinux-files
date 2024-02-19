/* This program tests the basic RPC to see if it works. */

#include <amoeba.h>
#include "header.h"

#define MAX_TRIALS 1000

char buffer1[BUF_SIZE], buffer2[BUF_SIZE];

main(argc, argv)
int argc;
char *argv[];
{

  header hdr1, hdr2;
  int cnt, i, base = 0, size, errors, blocks, iterations = 0;
  long bad_trans = 0;

  if (argc != 2) {
	printf("Usage: client1 portname\n");
	exit(1);
  }

  /* Copy the filename into the start of the buffer. */
  strncpy(&hdr1.h_port, argv[1], PORTSIZE);
  hdr1.h_command = WORK;

  printf("Number of tests performed = %5d ", 0);

  while (iterations < MAX_TRIALS) {
 	/* Initialize the buffer. */
	for (i = 0; i < BUF_SIZE; i++) buffer1[i] = base + i;

	size = trans(&hdr1, buffer1, BUF_SIZE, &hdr2, buffer2, BUF_SIZE);
	if (size < 0) {
	    printf("\nTransaction failed. Error = %d.   ", size);
	    printf("Hit F1 to see if AMTASK running.\n");
		exit(1);
	}

	/* Check reply. */
	errors = 0;
	for (i = 0; i < BUF_SIZE; i++) 
		if ( (buffer2[i]&0377) != ( (buffer1[i]+1)&0377)) errors++;
	if (errors > 0) bad_trans++;
	blocks++;
	base++;
	iterations++;
	if (iterations % 10 == 0) printf("\b\b\b\b\b\b%5d ", iterations);
  }

  /* Test done. Send null trans to tell server. Report on errors. */
  hdr1.h_command = QUIT;
  trans(&hdr1, buffer1, 0, &hdr2, buffer2, BUF_SIZE);
  printf("\b.   Number of errors = %d.\n", bad_trans);
}
