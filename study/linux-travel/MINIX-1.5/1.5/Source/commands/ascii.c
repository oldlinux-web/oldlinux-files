/* ascii - list lines with/without ASCII characters */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 4*1024

static char buf[BUFSIZE + 1];	/* input buffer - +1 for sentinel */
static char carry[BUFSIZE];	/* buffer for partial line carryover */

int main(argc, argv)
int argc;
char **argv;
{
  int nflag = 0;		/* 1 if invoked with -n */
  int ascii_line;		/* set to 1 if line is all ASCII */
  int ascii_file = 1;		/* set to 0 if file is not all ASCII */
  int count;			/* count of characters in buf */
  char *start;			/* points to beginning of line */
  register char *end;		/* points to end of line */
  char *sentinel;		/* points past last character in buffer */
  int carry_count;		/* size of carry over */

  --argc;
  ++argv;
  if (argc > 0 && strcmp(*argv, "-n") == 0) {
	nflag = 1;
	--argc;
	++argv;
  }
  switch (argc) {
      case 0:
	break;
      case 1:
	close(0);
	if (open(*argv, O_RDONLY) != 0) {
		std_err("ascii: cannot open ");
		std_err(*argv);
		std_err("\n");
		exit(1);
	}
	break;
      default:
	std_err("Usage: ascii [-n] file\n");
	exit(1);
  }

  if ((count = read(0, buf, BUFSIZE)) <= 0) exit(0);
  *(sentinel = &buf[count]) = '\n';
  start = buf;
  ascii_line = 1;
  carry_count = 0;
  while (1) {
	for (end = start; *end != '\n'; ++end)
		if ((*end & 0x80) != 0) {
			ascii_line = 0;
			ascii_file = 0;
			end = (char *) memchr(end, '\n', BUFSIZE);
			break;
		}
	if (end != sentinel) {
		++end;
		if (ascii_line != nflag) {
			if (carry_count != 0)
				fwrite(carry, carry_count, 1, stdout);
			fwrite(start, (int)(end - start), 1, stdout);
		}
		carry_count = 0;
		start = end;
		ascii_line = 1;
	} else {
		if (carry_count != 0) {
			std_err("ascii: line too long\n");
			exit(1);
		}
		if (ascii_line != nflag) {
			carry_count = end - start;
			memcpy(carry, start, carry_count);
		}
		if ((count = read(0, buf, BUFSIZE)) <= 0) break;
		*(sentinel = &buf[count]) = '\n';
		start = buf;
	}
  }
  if (ascii_line != nflag && carry_count != 0)
	fwrite(carry, carry_count, 1, stdout);

  exit(ascii_file == 0);
}
