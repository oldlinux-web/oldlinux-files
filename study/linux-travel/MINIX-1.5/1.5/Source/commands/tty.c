/* tty.c - Return tty name		Author: Freeman P. Pascal IV */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(argc, argv)
int argc;
char *argv[];
{
  char *tty_name;

  tty_name = ttyname(0);
  if ((argc == 2) && (!strcmp(argv[1], "-s")))
	 /* Do nothing - shhh! we're in silent mode */ ;
  else
	puts((tty_name != (char *) NULL) ? tty_name : "Not a tty");

  if (tty_name == (char *) NULL)
	exit(1);
  else
	exit(0);
}
