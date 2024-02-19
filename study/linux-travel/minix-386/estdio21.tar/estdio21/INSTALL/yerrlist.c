#include <ctype.h>
#include <stdio.h>

struct err {
  int errno;
  char *name;
  char *desc;
} errlist[] = {
{ 0, "EUNKNOWN", "Unknown error" }
#include "yerrlist.h"
};

int nerr = sizeof(errlist)/sizeof(errlist[0]);

int cmp(x, y) struct err *x, *y; { return x->errno - y->errno; }

int main()

{
  int i;
  int bound;
  char yn[2];
  void exit();
  void qsort();

  qsort(&errlist[0], nerr, sizeof(errlist[0]), cmp);

  for (i = 0; i < nerr; i++) {
    if (islower(errlist[i].desc[0]))
      errlist[i].desc[0] = toupper(errlist[i].desc[0]);
  }

  bound = errlist[nerr-1].errno;
  for (i = 0; i < nerr; i++) {
    if (errlist[i].errno != i) {
      fprintf(stderr, "There is a discontinuity in error numbers following:\n\
%-15.15s %3d %s\n", errlist[i-1].name, errlist[i-1].errno, errlist[i-1].desc);
      yn[0] = '?';
      while (yn[0] == '?') {
	fprintf(stderr,
	  "\nIs it acceptable to only include errors up to this point? [yn?] ");
	fflush(stderr);
	yn[0] = 'n';
	scanf("%1s", &yn[0]);
	if (yn[0] == '?') {
	  fputs("Including errors up to this point will generate a small\n",
		stderr);
	  fputs("table of error messages. However, this table may be\n",
		stderr);
	  fputs("incomplete. You will have to determine whether your\n",
		stderr);
	  fputs("operating system will generate error numbers beyond this\n",
		stderr);
	  fputs("bound.\n", stderr);
	  fputs("\n", stderr);
	  fputs("Answering `y' is suitable for Minix.\n", stderr);
	}
      }
      if (yn[0] == 'y') {
	bound = errlist[i-1].errno;
	break;
      }
      else {
	bound = -1;
	while (bound < 0) {
	  fprintf(stderr,
		  "Enter an acceptable bound on the error numbers: [%%d] ");
	  fflush(stderr);
	  bound = -1;
	  scanf("%d", &bound);
	}
	break;
      }
    }
  }

  for (i = 0; i < nerr; i++) {
    if (errlist[i].errno > bound)
      break;
    if (i != 0)
      printf(",\n");
    printf("/* %-15.15s %3d */ \"%s\"", errlist[i].name,
				        errlist[i].errno,
				        errlist[i].desc);
  }
  printf("\n");
  exit(0);
  return 0;
}
