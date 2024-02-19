/* printroot - print root device on stdout	Author: Bruce Evans */

/* This program figures out what the root device is by doing a stat on it, and
 * then searching /dev until it finds an entry with the same device number.
 * A typical use (probably the only use) is in /etc/rc for initializing
 * /etc/mtab, as follows:
 *
 *	/usr/bin/printroot >/etc/mtab
 *
 *  9 Dec     1989 - clean up for 1.5 - full prototypes (BDE)
 * 15 October 1989 - avoid ACK cc bugs (BDE):
 *		   - sizeof "foo" is 2 (from wrong type char *) instead of 4
 *		   - char foo[10] = "bar"; allocates 4 bytes instead of 10
 *  1 October 1989 - Minor changes by Andy Tanenbaum
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char DEV_PATH[] = "/dev/";	/* #define would step on sizeof bug */
static char MESSAGE[] =	" is root device\n";	/* ditto */
#define UNKNOWN_DEV	"/dev/unknown"
#define ROOT		"/"

#ifdef __STDC__
static void done(char *name, int status);
#else
static void done();
#endif

int main(argc, argv)
int argc;
char **argv;
{
  struct direct dir;
  int fd;
  struct stat filestat, rootstat;
  static char namebuf[sizeof DEV_PATH + NAME_MAX];

  if (stat(ROOT, &rootstat) == 0 && (fd = open(DEV_PATH, O_RDONLY)) >= 0) {
	while (read(fd, (char *) &dir, sizeof dir) == sizeof dir) {
		if (dir.d_ino == 0) continue;
		strcpy(namebuf, DEV_PATH);

		/* If next does not null-terminate, last in buf does it. */
		strncat(namebuf, dir.d_name, NAME_MAX);
		if (stat(namebuf, &filestat) != 0) continue;
		if ((filestat.st_mode & S_IFMT) != S_IFBLK) continue;
		if (filestat.st_rdev != rootstat.st_dev) continue;
		done(namebuf, 0);
	}
  }
  done(UNKNOWN_DEV, 1);
  return 0;			/* not reached */
}

static void done(name, status)
char *name;
int status;
{
  write(1, name, strlen(name));
  write(1, MESSAGE, sizeof MESSAGE - 1);
  exit(status);
}
