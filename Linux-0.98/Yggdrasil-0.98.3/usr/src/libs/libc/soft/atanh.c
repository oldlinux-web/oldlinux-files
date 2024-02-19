#include <soft.h>

double
asinh (double x)
{
  return log (x + sqrt (x * x + 1.0));
}

double
acosh (double x)
{
  if(x <= 1.0) {
    if (x < 1.0) {
      errno = EDOM;
      perror ("acosh");
    }
    return(0.0);
  }
  return log (x + sqrt (x * x - 1.0));
}

double
atanh (double x)
{
  if (x >= 1.0 || x <= -1.0 ) {
    errno = EDOM;
    perror ("atanh");
    if (x > 1.0 || x < -1.0 ) return(0.0);
    if (x == 1.0) return(HUGE_VAL);
    else return(-HUGE_VAL);
  }

  return 0.5 * log ((1.0 + x) / (1.0 - x));
}
