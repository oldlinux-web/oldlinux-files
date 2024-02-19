#include <stdio.h>

double atof();

void dump(m)

double m;

{
  int i;
  long *p;

  p = (long *) &m;
  for (i = sizeof(double)/sizeof(long); i--; p++)
    printf("%08lx ", *p);
}

int main()

{
  int ok;
  double a;
  double b;
  int bytes;
  char buf[1024];
  FILE *tmp;

  tmp = tmpfile();

  while (gets(buf) != (char *) NULL) {
    fprintf(tmp, "%s %s\n", buf, buf);
  }

  rewind(tmp);

  for (;;) {
    ok = fscanf(tmp, "%lf%n", &a, &bytes);
    if (ok == EOF)
      break;
    printf("%d bytes read %sok\n", bytes, ok ? "" : "not ");
    if (! ok)
      break;
    fgets(buf, sizeof(buf), tmp);
    b = atof(buf);
    printf("%.15lg ", a);
    dump(a); printf("\n");
    printf("%.15lg ", b);
    dump(b); printf(" Difference %g\n", a-b);
  }
  return 0;
}
