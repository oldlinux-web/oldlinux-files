/* mount - mount a file system		Author: Andy Tanenbaum */

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 1024

extern int errno;
char *mounttable = "/etc/mtab";
char buffer[BUFSIZE];

main(argc, argv)
int argc;
char *argv[];
{
  int ro, fd;

  if (argc == 1) list();
  if (argc < 3 || argc > 4) usage();
  if (argc == 4 && *argv[3] != '-' && *(argv[3] + 1) != 'r') usage();
  ro = (argc == 4 ? 1 : 0);
  if (mount(argv[1], argv[2], ro) < 0) {
	if (errno == EINVAL) {
		std_err("mount: ");
		std_err(argv[1]);
		std_err(" is not a valid file system.\n");
	} else {
		perror("mount");
	}
	exit(1);
  }
  std_err(argv[1]);
  std_err(" mounted\n");
  if ((fd = open(mounttable, O_RDWR)) < 0) exit(1);
  lseek(fd, 0L, SEEK_END);	/* seek to EOF */
  write(fd, argv[1], strlen(argv[1]));
  write(fd, " is mounted on ", 15);
  write(fd, argv[2], strlen(argv[2]));
  write(fd, "\n", 1);
  exit(0);
}


list()
{
  int fd, n;

  fd = open(mounttable, O_RDONLY);
  if (fd < 0) {
	std_err("mount: cannot open ");
	std_err(mounttable);
	std_err("\n");
	exit(1);
  }
  n = read(fd, buffer, BUFSIZE);
  write(1, buffer, n);
  exit(0);
}


usage()
{
  std_err("Usage: mount special name [-r]\n");
  exit(1);
}
