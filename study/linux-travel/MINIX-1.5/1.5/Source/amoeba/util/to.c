#include <stdio.h>
#include "amoeba.h"

#define BUFSIZE		20480

main(argc, argv)
char **argv;
{
	header hdr;
	char *buf, *malloc();
	char *cmd = argv[0];
	unshort cnt, bufsize = BUFSIZE;
	unshort trans();

	if (argc > 1 && strncmp(argv[1], "-t", 2) == 0) {
		timeout(atoi(&argv[1][2]), 0);
		argc--;
		argv++;
	}
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage: %s port\n", cmd);
		exit(-1);
	}
	if (argc == 3)
		bufsize = atoi(argv[2]);
	if ((buf = malloc(bufsize)) == NULL) {
		fprintf(stderr, "%s: out of memory\n", cmd);
		exit(-1);
	}
	strncpy(&hdr.h_port, argv[1], PORTSIZE);
	for (;;) {
		cnt = read(0, buf, bufsize);
		if ((short) cnt < 0) {
			perror("read");
			exit(-3);
		}
		if (trans(&hdr, buf, cnt, &hdr, NILBUF, 0) != 0) {
			fprintf(stderr, "%s: trans failed\n", cmd);
			exit(-4);
		}
		if (hdr.h_status) {
			fprintf(stderr, "%s: write failed in from\n", cmd);
			exit(-5);
		}
		if (cnt == 0)
			exit(0);
	}
}


