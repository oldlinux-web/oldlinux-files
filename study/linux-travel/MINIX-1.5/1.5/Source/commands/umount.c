/* umount - unmount a file system		Author: Andy Tanenbaum */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define BUFSIZE 1024

extern int errno;
char *mounttable = "/etc/mtab";
char buffer[BUFSIZE], *p = &buffer[0], *q;

main(argc, argv)
int argc;
char *argv[];
{

  if (argc != 2) usage();
  if (umount(argv[1]) < 0) {
	if (errno == EINVAL)
		std_err("Device not mounted\n");
	else
		perror("umount");
	exit(1);
  }
  std_err(argv[1]);
  std_err(" unmounted\n");
  do_mtab(argv[1]);
  exit(0);
}

do_mtab(devname)
char *devname;
{
/* Remove an entry from mtab. */
  int n, fd;
  char line[256];

  /* Read in the mount table and then overwrite the file. */
  fd = open(mounttable, O_RDWR);
  n = read(fd, buffer, BUFSIZE);
  close(fd);
  q = &buffer[n];
  fd = creat(mounttable, 0554);

  n = strlen(devname);
  while (getline(line) != 0) {
	if (strncmp(line, devname, n) == 0) continue;
	write(fd, line, strlen(line));
  }
}

int getline(ptr)
char *ptr;
{
  char c;

  while (p < q) {
	c = *p++;
	*ptr++ = c;
	if (c == '\n') {
		*ptr++ = 0;
		return(1);
	}
  }
  return(0);
}


usage()
{
  std_err("Usage: umount special\n");
  exit(1);
}
