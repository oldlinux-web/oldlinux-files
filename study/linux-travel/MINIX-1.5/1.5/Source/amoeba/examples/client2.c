/* This file is the client of a test program for measuring RPC speed. */

#include <amoeba.h>
#include <minix/callnr.h>
#include "header.h"

#define TRIALS 1000

char buffer[MAX_TRANS];
int bytes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 
		8192, 16384, 30000, 0};

header hdr1, hdr2;

main(argc, argv)
int argc;
char *argv[];
{
  int i, cnt;

  if (argc != 2) {
	printf("Usage: client2 portname\n");
	exit(1);
  }

  /* Copy the filename into the start of the buffer. */
  strncpy(&hdr1.h_port, argv[1], PORTSIZE);
  hdr1.h_command = READ;

  printf("Buf size        Delay       Throughput      (Each test repeated %d times)\n", TRIALS);
  printf("            (msec/trans)    (bytes/sec)\n");
  printf("--------    ------------    -----------\n");

  i = 0;
  while (bytes[i] != 0) {
	run_test(bytes[i], TRIALS);
	i++;
  }
  hdr1.h_command = QUIT;
  trans(&hdr1, buffer, 0, &hdr2, buffer, 0);
  exit(0);
}


run_test(count, trials)
int count, trials;
{
/* Run a performance test. */

  int i, n;
  long start, finish, traffic, sec, msec, delay, throughput;
  
  time(&start);				/* record starting time */
  for (i = 0; i < trials; i++) {
	hdr1.h_command = READ;
	hdr1.h_size = (unshort) count;
	n = trans(&hdr1, buffer, 0, &hdr2, buffer, MAX_TRANS);
  	if (n < 0) {
		printf("Transaction failed. Error = %d.   ", n);
		printf("Hit F1 to see if AMTASK running.\n");
		exit(1);
	}
  }
  time(&finish);
  sec = finish - start;			/* time for this trial in seconds */
  msec = 1000L * sec;
  traffic = (long) trials * (long) count;
  delay = msec/trials;			/* msec per transaction */
  if (sec != 0L)
	throughput = traffic/sec;
  else
	throughput = 0L;
  printf("  %5d         %4D          %7D\n", count, delay, throughput);
}


