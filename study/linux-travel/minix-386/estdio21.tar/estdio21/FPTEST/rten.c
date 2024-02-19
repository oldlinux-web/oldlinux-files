#include <stdio.h>

int main(argc, argv)

int argc;
char *argv;

{
  int exp;
  char buf[1024];
  double num;
  int i;
  double atof();

  if (argc > 1) {
    for (i = 0; i < 20; i++) {
      for (exp = -300; exp <= 300; exp++) {
        sprintf(buf, "1e%d", exp);
        sscanf(buf, "%lf", &num);
      }
    }
  }
  else {
    for (i = 0; i < 20; i++) {
      for (exp = -300; exp <= 300; exp++) {
        sprintf(buf, "1e%d", exp);
        num = atof(buf);
      }
    }
  }
  return 0;
}
