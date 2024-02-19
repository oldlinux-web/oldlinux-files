#include <stdio.h>
#include <math.h>
#include <float.h>

extern double Strtod (char *, char **);
extern double strtod (char *, char **);

void
main ()
{
  double d, d1;
  char *ptr;

  d1 =atof("1.7976931348623157e+308F");
  d = Strtod ("1.7976931348623157e+308F", &ptr);
  printf ("1.7976931348623157e+308F %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
  d1 = atof("1.7976931348623157e+1408L");
  d = Strtod ("1.7976931348623157e+1408L", &ptr);
  printf ("1.7976931348623157e+1408L %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
  d1 = atof("-1.7976931348623157e1408F");
  d = Strtod ("-1.7976931348623157e1408F", &ptr);
  printf ("-1.7976931348623157e1408F %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
  d = atof("1.7976931348623157e-1408F");
  d = Strtod ("1.7976931348623157e-1408F", &ptr);
  printf ("1.7976931348623157e-1408F %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
  d1 = atof("1.7976931348623157e-1408F");
  d = Strtod ("1.7976931348623157e-1408F", &ptr);
  printf ("1.7976931348623157e-1408F %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
  d1 = atof(NULL);
  d = Strtod (NULL, &ptr);
  printf ("NULL %30.26g, %30.26g\n", d, d1);
  printf ("%c\n", *ptr);
}
