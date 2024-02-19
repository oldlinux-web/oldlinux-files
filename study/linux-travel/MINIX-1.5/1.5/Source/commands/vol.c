/* vol - break stdin into volumes	Author: Andy Tanenbaum */

/* This program reads standard input and writes it onto diskettes, pausing
 * at the start of each one.  It's main use is for saving files that are
 * larger than a single diskette.  Vol just writes its standard input onto
 * a diskette, and prompts for a new one when it is full.  This mechanism
 * is transparent to the process producing vol's standard input. For example,
 *	tar c - . | vol 360 /dev/fd0
 * puts the tar output as as many diskettes as needed.  To read them back in,
 * use
 *	vol -u 360 /dev/fd0 | tar x -
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <blocksize.h>
#include <signal.h>
#include <errno.h>

extern int errno;
extern char *itoa();

char buffer[BLOCK_SIZE];

main(argc, argv)
int argc;
char *argv[];
{
  int volume = 1, size, reading, fd, tty;
  char *p, *name;
  struct stat stb;

  signal(SIGPIPE, SIG_IGN);

  /* Fetch and verify the arguments. */
  if (argc != 3 && argc != 4)
	message("Usage: vol [-u] size block-special\n", "");
  p = argv[1];
  reading = (*p == '-' && *(p + 1) == 'u' ? 1 : 0);
  size = atoi(argv[reading + 1]);
  name = argv[reading + 2];
  tty = open("/dev/tty", O_RDONLY);

  if (size <= 0) message("vol: bad volume size\n", argv[reading + 1]);
  if (stat(name, &stb) < 0) message("vol: cannot stat %s\n", name);
  if ((stb.st_mode & S_IFMT) != S_IFBLK)
	message("vol: %s is not a block special file\n", name);
  if (tty < 0) message("vol: cannot open /dev/tty\n", "");

  while (1) {
	/* Open the special file. */
	fd = open(name, 1 - reading);
	if (fd < 0) message("vol: cannot open %s\n", name);

	std_err("Please insert volume ");
	num(volume);
	std_err(" and hit return\n");
	read(tty, buffer, BLOCK_SIZE);
	volume++;

	/* Read or write the requisite number of blocks. */
	if (reading)
		diskio(size, fd, 1, name, "stdout");	/* vol -u | tar xf - */
	else
		diskio(size, 0, fd, "stdin", name);	/* tar cf - | vol */

	close(fd);
  }
}

diskio(size, fd1, fd2, errstr1, errstr2)
int size, fd1, fd2;
char *errstr1, *errstr2;
{
/* Read 'size' blocks from 'fd1' and write them on 'fd2'.  Watch out for
 * the fact that reads on pipes can return less than the desired data.
 */

  int n, m, count;
  long needed;

  needed = (long) BLOCK_SIZE *(long) size;	/* # bytes to read */
  while (needed > 0L) {
	count = (needed > (long) BLOCK_SIZE ? BLOCK_SIZE : (int) needed);
	n = read(fd1, buffer, count);
	if (n == 0) exit(0);
	if (n < 0) message("Error encountered while reading %s\n", errstr1);
	m = write(fd2, buffer, n);
	if (m < 0 && errno == SIGPIPE) exit(0);
	if (m > 0 && m != n) message("Output error on %s\n", errstr2);
	if (m < 0) message("Error encountered while writing %s\n", errstr2);
	needed -= n;
  }
}



message(s1, s2)
char *s1, *s2;
{
  printf(s1, s2);
  exit(1);
}

num(n)
int n;
{
  char out[3];

  out[0] = ' ';
  out[1] = '0';
  out[2] = 0;
  if (n < 10) {
	out[1] += n;
  } else {
	out[1] += (n % 10);
	out[0] = '0' + (n / 10);
  }
  std_err(out);
}
