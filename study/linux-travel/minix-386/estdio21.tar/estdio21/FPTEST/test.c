/* Here is a list of test values. Try:
 *
 *	grep ^+ xfptest.c | xfptest | more

+30 1234567890.12345678901234567890
+30 123456789012345678901234567890
+30 0.00000123456789012345678901234567890
+0 0.987654321
+0 0.123456789
+1 0.987654321
+1 0.123456789
+2 0.0987654321
+2 0.0123456789
+1 0.00000997654321
+2 0.00000997654321
+2 0.00987654321
+2 0.00123456789
+2 99.9999999999
+16 2.22044604925031e-16
+16 1.79769313486231e+308
+16 2.2250738585071e-308
*/

#include <stdio.h>

int main()

{
  int p;
  int b;
  double f;

  for (;;) {
    if (scanf("%d%lf", &p, &f) != 2)
      break;
    b = printf("F %.*lf\nF %#.*lf\n", p, f, p, f);
    printf("%d bytes\n", b);
    b = printf("E %.*le\nE %#.*le\n", p, f, p, f);
    printf("%d bytes\n", b);
    b = printf("G %.*lg\nG %#.*lg\n", p, f, p, f);
    printf("%d bytes\n", b);
    putchar('\n');
  }
  return 0;
}
