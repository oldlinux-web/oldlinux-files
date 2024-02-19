#include <signal.h>

char console[] = "/dev/console";

run(uid, gid, argv)
char **argv;
{

	for (;;)
		switch (fork()) {
		default:
			return;
		case 0:
			if (setgid(gid) < 0) perror("can't set gid");
			if (setuid(uid) < 0) perror("can't set uid");
/*
			execvp(*argv, argv);
*/
			execv(*argv, argv);
			perror("master: exec'ing");
			prints("can't execute %s\n", *argv);
/*
			kill(getppid(), SIGTERM);
*/
			/* If the exec failed, don't try it again immediately.
			 * Give the kernel a chance to do something else.
			 */
			sleep (5);
			_exit(1);
		case -1:
			sleep(10);
		}
}

main(argc, argv)
char **argv;
{
  register n, uid, gid;

  /* Minix can't do this
	setpgrp(getpid(), getpid());
  */
	if (argc < 4) {
		prints("Usage: master # uid gid command args ...\n");
		return(1);
	}
	n = atoi(argv[1]);
	if (n < 1 || n > 20) {
		prints("Bad count.\n");
		return(1);
	}
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

/* Put the program into the background. */
	switch (fork()) {
	case 0:
		break;
	case -1:
		perror(argv[0]);
		return 1;
	default:
		return 0;
	}
	uid = atoi(argv[2]);
	gid = atoi(argv[3]);

/* Start n copies of the program. */
	do
		run(uid, gid, &argv[4]);
	while (--n);

/* Replace each one that dies. */
	while (wait((int *) 0) > 0)
		run(uid, gid, &argv[4]);
	return(0);
}
