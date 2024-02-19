int main()

{
  int flt_radix;
  double fullmantissa, e, m, f, g;

/* Decide on the radix. The way this is done is to generate e, the
 * smallest power of two for which 1 + e == 1. Once, e is discovered
 * determine the least m > e. The radix is then m - e.
 */
  for (e = 1.0; 1 + e != e; e *= 2.0)
    ;
  for (m = 1.0; e + m == e; m += 1.0)
    ;
  g = e + m;
  flt_radix = g - e;

/* Generate a mantissa full of flt_radix-1 digits.
 */
  for (fullmantissa = 0, f = flt_radix-1; ; ) {
    g = fullmantissa + f;
    if (g + 1 == g) break;
    fullmantissa = g;
    f *= flt_radix;
  }
  fullmantissa /= f/(flt_radix-1);

/* See if it is possible to divide it down into a denormalised number.
 */
  for (;;) {
    g = fullmantissa / flt_radix;
    if (g * flt_radix != fullmantissa)
      break;
    fullmantissa = g;
  }

  exit(g/flt_radix == 0.0);
}
