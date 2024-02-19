/* shar - make a shell archive		Author: Michiel Husijes */

#include <sys/types.h>
#include <fcntl.h>
#include <blocksize.h>
#include <stdio.h>

#define IO_SIZE		(10 * BLOCK_SIZE)

char input[IO_SIZE];
char output[IO_SIZE];
int ind = 0;

main(argc, argv)
int argc;
register char *argv[];
{
  register int i;
  int fd;

  for (i = 1; i < argc; i++) {
	if ((fd = open(argv[i], O_RDONLY)) < 0) {
		write(2, "Cannot open ", 12);
		write(2, argv[i], strlen(argv[i]));
		write(2, ".\n", 2);
	} else {
		print("echo x - ");
		print(argv[i]);
		print("\nsed '/^X/s///' > ");
		print(argv[i]);
		print(" << '/'\n");
		cat(fd);
	}
  }
  if (ind) write(1, output, ind);
  exit(0);
}

cat(fd)
int fd;
{
  static char *current, *last;
  register int r = 0;
  register char *cur_pos = current;

  putchar('X');
  for (;;) {
	if (cur_pos == last) {
		if ((r = read(fd, input, IO_SIZE)) <= 0) break;
		last = &input[r];
		cur_pos = input;
	}
	putchar(*cur_pos);
	if (*cur_pos++ == '\n' && cur_pos != last) putchar('X');
  }
  print("/\n");
  (void) close(fd);
  current = cur_pos;
}

print(str)
register char *str;
{
  while (*str) putchar(*str++);
}
