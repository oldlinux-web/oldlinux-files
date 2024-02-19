/* rev - reverse an ASCII line	  Authors: Paul Polderman & Michiel Huisjes */

#include <sys/types.h>
#include <fcntl.h>
#include <blocksize.h>

#ifndef EOF
#define	EOF	((char) -1)
#endif


int fd;				/* File descriptor from file being read */

main(argc, argv)
int argc;
char *argv[];
{
  register unsigned short i;

  if (argc == 1) {		/* If no arguments given, use stdin as input */
	fd = 0;
	rev();
	exit(0);
  }
  for (i = 1; i < argc; i++) {	/* Reverse each line in arguments */
	if ((fd = open(argv[i], O_RDONLY)) < 0) {
		std_err("Cannot open ");
		std_err(argv[i]);
		std_err("\n");
		continue;
	}
	rev();
	close(fd);
  }
  exit(0);
}




rev()
{
  char output[BLOCK_SIZE];	/* Contains a reversed line */
  register unsigned short i;	/* Index in output array */

  do {
	i = BLOCK_SIZE - 1;
	while ((output[i] = nextchar()) != '\n' && output[i] != EOF) i--;
	write(1, &output[i + 1], BLOCK_SIZE - 1 - i); /* write reversed line */
	if (output[i] == '\n')	/* and write a '\n' */
		write(1, "\n", 1);
  } while (output[i] != EOF);
}




char buf[BLOCK_SIZE];
nextchar()
{				/* Does a sort of buffered I/O */
  static int n = 0;		/* Read count */
  static int i;			/* Index in input buffer to next character */

  if (--n <= 0) {		/* We've had this block. Read in next block */
	n = read(fd, buf, BLOCK_SIZE);
	i = 0;			/* Reset index in array */
  }
  return((n <= 0) ? EOF : buf[i++]);	/* Return -1 on EOF */
}
