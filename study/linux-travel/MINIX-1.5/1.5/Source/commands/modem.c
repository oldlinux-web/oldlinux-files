/* modem - Put modem into DIALIN or DIALOUT mode.	Author: F. van Kempen */

/* Exit:	1	OK, suspended/restarted GETTY
 *		0	Error, GETTY would not listen
 *		-1	Error in UTMP file
 *		-2	TTY not found
 *		-3	Process not GETTY
 *		-4	Process busy
 *		-5	Unknown process type
 *
 * Version:	1.3 	12/30/89
 *
 * Author:	F. van Kempen, MicroWalt Corporation
 */

#include <sys/types.h>
#include <fcntl.h>
#include <sgtty.h>
#include <signal.h>
#include <string.h>
#include <utmp.h>

static char *Version = "@(#) MODEM 1.3 (12/30/89)";
int debug = 0;			/* generate some debugging output */
int rings = -999;		/* number of rings for dial-in */


extern char *itoa();


/* Tell modem to go into DIAL-IN mode.
 * We do this by resetting it and by setting the
 * S0-register to a positive value.
 */
int dialin(tty)
char *tty;
{
  struct sgttyb old, new;
  char buff[32];
  int fd;

  /* Create file name of modem. */
  strcpy(buff, "/dev/");
  strcat(buff, tty);

  /* open the device */
  fd = open(buff, O_RDWR);
  if (fd < 0) return(0);

  /* Get old terminal status. */
  ioctl(fd, TIOCGETP, &old);

  /* Set temporary parameters. This equals 'stty sane 1200'. */
  ioctl(fd, TIOCGETP, &new);
  new.sg_ispeed = B1200;
  new.sg_ospeed = B1200;
  new.sg_flags = (BITS8 | CRMOD | XTABS);
  ioctl(fd, TIOCSETP, &new);

  /* Say hello to the modem. */
  write(fd, "+++", 3);
  sleep(2);

  /* Reset the modem. */
  write(fd, "AT Z\n", 5);  
  sleep(1);

  /* Create the ANSWER string. */
  strcpy(buff, "AT S0=");
  strcat(buff, itoa(rings));
  strcat(buff, "\n");
  write(fd, buff, strlen(buff));

  /* Restore the old terminal parameters. */
  ioctl(fd, TIOCSETP, &old);

  close(fd);
  return(1);
}


/* Tell modem to go into DIAL-OUT mode.
 * We do this by resetting it and by setting the
 * S0-register to a zero (zero rings!).
 */
int dialout(tty)
char *tty;
{
  struct sgttyb old, new;
  char buff[32];
  int fd;

  /* create file name of modem */
  strcpy(buff, "/dev/");
  strcat(buff, tty);

  /* open the device */
  fd = open(buff, O_RDWR);
  if (fd < 0) return(0);

  /* Get old terminal status. */
  ioctl(fd, TIOCGETP, &old);

  /* Set temporary parameters. This equals 'stty sane 1200'. */
  ioctl(fd, TIOCGETP, &new);
  new.sg_ispeed = B1200;
  new.sg_ospeed = B1200;
  new.sg_flags = (BITS8 | CRMOD | XTABS);
  ioctl(fd, TIOCSETP, &new);

  /* Say hello to the modem. */
  write(fd, "+++", 3);
  sleep(2);

  /* Reset the modem. */
  write(fd, "AT Z\n", 5);  
  sleep(1);

  /* Send the NO-ANSWER string. */
  write(fd, "AT S0=0\n", 8);

  /* Restore the old terminal parameters. */
  ioctl(fd, TIOCSETP, &old);

  close(fd);
  return(1);
}


void usage()
{
   write(2, "Usage: modem [-d] [-g] <-i rings | -o> line\n", 44);
  exit(-1);
}


main(argc, argv)
int argc;
char *argv[];
{
  struct utmp entry;
  char *tty, *s;
  int fd, st;

  st = 0;
  while (st < argc) {
	s = argv[++st];
	if (s && *s == '-') switch (*++s) {
		    case 'd':	/* DEBUG mode */
			debug = 1;
			break;
		    case 'g':	/* GETTY mode */
			rings = -1;
			break;
		    case 'i':	/* DIAL-IN mode: suspend GETTY and
				 * call modem */
			s++;
			if (*s != '\0')
				rings = atoi(s);
			else
				rings = atoi(argv[++st]);
			break;
		    case 'o':	/* DIAL-OUT mode: restart GETTY and
				 * call modem */
			rings = 0;
			break;
		    default:
			usage();
	} else
		break;
  }

  if (rings == -999) usage();	/* badly formed parameters! */

  if (st >= argc) usage();	/* we need the TTY name as well! */
  tty = argv[st];		/* get the terminal name */

  /* Read the UTMP file to find out the PID and STATUS of the GETTY. */
  if ((fd = open(UTMP, O_RDONLY)) < 0) {
	write(2, "modem: cannot open UTMP !\n", 26);
	exit(-1);
  }
  while (read(fd, &entry, sizeof(struct utmp)) == sizeof(struct utmp)) {
	if (!strcmp(entry.ut_line, tty)) {
		close(fd);
		tty = entry.ut_line;
		fd = -1;
		break;
	}
  }
  close(fd);

  /* Process the terminal entry if we got one. */
  if (fd == -1) switch (entry.ut_type) {
	    case INIT_PROCESS:		/* this is not a tty-process! */
		if (debug)
			write(2, "modem: no TTY-servicing process!\n", 33);
		exit(-3);
	    case LOGIN_PROCESS:		/* getty waiting for a call */
		break;
	    case USER_PROCESS:		/* login or user-shell */
		if (debug) write(2, "modem: line is busy.\n", 21);
		exit(-4);
		break;
	    default:
		if (debug) write(2, "modem: unknown UTMP entry\n", 26);
		exit(-5);
  } else
	exit(-2);

  /* Now perform the desired action (DIALIN or DIALOUT). */
  switch (rings) {
      case -1:				/* -1 means "plain GETTY" */
	kill(entry.ut_pid, SIGBUS);	/* send IGNORE signal */
	st = 0;
	break;
      case 0:
	kill(entry.ut_pid, SIGEMT);	/* send SUSPEND signal */
	st = dialout(tty);		/* put MODEM into DIALOUT */
	break;
      default:
	kill(entry.ut_pid, SIGIOT);	/* send RESTART signal */
	st = dialin(tty);		/* put MODEM in DIALIN */
  }
  exit(st);
}
