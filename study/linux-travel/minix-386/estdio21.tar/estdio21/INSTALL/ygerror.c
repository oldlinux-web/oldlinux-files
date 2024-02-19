#include <errno.h>
#include <stdio.h>

extern char *sys_errlist[];
extern int sys_nerr;

struct err {
  int errno;
  char *name;
} errlist[512];
int errs = 0;

int main()

{
  int err;
  int i, j;
  char *p;
  char name[512];
  void *malloc();
  int strlen();
  char *strcpy();

  while (scanf("%s%d", name, &err) == 2) {
    errlist[errs].errno = err;
    errlist[errs].name  = (char *) malloc((unsigned) strlen(name)+1);
    (void) strcpy(errlist[errs].name, name);
    errs++;
  }

  printf("/* EUNKNOWN          0 */ \"Unknown error\"");
  for (err = 0, i = 1; i < sys_nerr; i++) {
    for (j = err; j < errs; j++) {
      if (errlist[j].errno == i) {
	err = j+1;
	break;
      }
    }
    if (j == errs)
      p = "ENONYMOUS";
    else
      p = errlist[j].name;
    printf(",\n");
    printf("/* %-15.15s %3d */ \"%s\"", p, i, sys_errlist[i]);
  }
  printf("\n");
}
