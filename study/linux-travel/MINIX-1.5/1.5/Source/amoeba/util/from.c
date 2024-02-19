#include <stdio.h>
#include <signal.h>
#include "amoeba.h"

#define BUFSIZE		20480

char *	cmd;


catchalarm()
{
	fprintf(stderr, "%s: timeout\n", cmd);
	exit(1);
}


main(argc, argv)
int	argc;
char **	argv;
{
	unshort		getreq();
	unshort		putrep();
	char *		malloc();
	int		catchalarm();

	char *		buf;
	header		hdr;
	unshort		cnt;
	unshort		bufsize = BUFSIZE;
	unsigned	tim = 15 * 60;		/* #secs before timeout */

	cmd = *argv;
	if (argc > 1 && strncmp(argv[1], "-t", 2) == 0)
	{
		tim = atoi(&argv[1][2]);
		argc--;
		argv++;
	}
	if (tim)
		signal(SIGALRM, catchalarm);
	if (argc != 2 && argc != 3)	/* if (!(argc & 2))?? */
	{
		fprintf(stderr, "Usage: %s [-ttime] [bufsize] port\n", cmd);
		exit(-1);
	}
	if (argc == 3)
		bufsize = atoi(argv[2]);
	if ((buf = malloc(bufsize * sizeof (char))) == NULL)
	{
		fprintf(stderr, "%s: out of memory\n", cmd);
		exit(-1);
	}
	strncpy(&hdr.h_port, argv[1], PORTSIZE);
	for (;;)
	{
		if (tim)
			alarm(tim);
		cnt = getreq(&hdr, buf, bufsize);
		if (tim)
			alarm(0);
		if ((short)cnt < 0)
		{
			fprintf(stderr, "%s: getreq failed\n", cmd);
			exit(-3);
		}
		hdr.h_status = 0;
		if (cnt != 0)
			if (write(1, buf, cnt) < 0)
			{
				perror(cmd);
				hdr.h_status = -1;
			}
		putrep(&hdr, NILBUF, 0);
		if (hdr.h_status)
			exit(-4);
		if (cnt == 0)
			exit(0);
	}
}
