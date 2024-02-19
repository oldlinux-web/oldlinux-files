#include <stdio.h>

int main()

{
  double n;
  int i;

  for (n = 1.0, i = 0; i < 300; i++, n *= 10.0)
    printf("%.16g\n", n);

  for (n = 1.0, i = 0; i < 300; i++, n /= 10.0)
    printf("%.16g\n", n);

  return 0;
}
