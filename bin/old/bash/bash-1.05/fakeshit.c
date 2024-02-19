#include <signal.h>
#include <stdio.h>


/* fake gethostname for MINIX */

#include <stdio.h>

char *gethostname(char *hostname, int len)
{
   FILE *f;

   if ((f = fopen("/etc/uucpname", "r")) == (FILE *)NULL) {
	fprintf(stderr, "gethostname: can't open /etc/uucpname\n");
	exit(-1);
   }
   fgets(hostname, len, f);
   hostname[strlen(hostname) - 1] = '\0'; /* remove newline */
   fclose(f);
}

/* really fake ulimit for MINIX -- personally I think ulimit is a bogus idea */

long ulimit(int cmd, long newlimit)
{
	return 0L;
}

/* fake NDELAY read for MINIX -- will be replaced by fcntl() someday */

int crappy_read(int fd, char *ch)
{
  int pid;
  int i;
  int what_happened;
  char mych;

  switch (pid = fork()) {
	case -1:
		perror("fork");
		exit(-1);
	break;

	case 0: /* child */
		read(fd, &mych, 1);
		exit(mych);
	break;

	default: /* parent */
		for (i = 0; i < 25; i++) /* give child some time */
			;
		kill(pid, SIGKILL); /* OK time's up */
		if (wait(&what_happened) < 0)
			perror("wait");
	break;
  }
  if (what_happened & 0x00ff != 0) {
	/* no input seen */
	*ch = '\0';
	return 0;
  } else {
	/* input seen */
	*ch = (what_happened & 0xff00) >> 8;
	return 1;
  }
}
