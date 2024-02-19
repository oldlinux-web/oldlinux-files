#include <stdio.h>
#include <soft.h>

extern double Sqrt();

main ()
{
  int power;
  double f, x1, x2, y, t1, t2;

  for (y = 0; y < 100; y += 0.3435) {
  t1 = sqrt (y);
  t2 = Sqrt (y);
  fprintf (stderr, "%g: %30.20g\t%30.20g\n", y, t2, t1);
  }
}
