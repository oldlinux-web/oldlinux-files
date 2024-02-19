/* kill - send a signal to a process	Author: Adri Koppes  */

#include <sys/types.h>
#include <signal.h>

extern char *itoa();

main(argc, argv)
int argc;
char **argv;
{
  int proc, signal = SIGTERM;

  if (argc < 2) usage();
  if (argc > 1 && *argv[1] == '-') {
	signal = atoi(&argv[1][1]);
	if (!signal) usage();
	argv++;
	argc--;
  }
  while (--argc) {
	argv++;
	proc = atoi(*argv);
	if (!proc && strcmp(*argv, "0")) usage();
	if (kill(proc, signal)) {
		prints("Kill: %s no such process\n", itoa(proc));
		exit(1);
	}
  }
  exit(0);
}

usage()
{
  prints("Usage: kill [-sig] pid\n");
  exit(1);
}
