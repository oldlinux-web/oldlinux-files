/*
 * Print Working Directory (pwd)
 */

#include <sys/types.h>
#include <limits.h>
#include <unistd.h>

char dir[PATH_MAX+1];

main()
{
  char *p;

  p = getcwd(dir, PATH_MAX);
  if (p == (char *) NULL) {
	std_err("pwd: cannot search some directory on the path\n");
	exit(1);
  }
  write(1, p, strlen(p));
  write(1, "\n", 1);
  exit(0);
}
