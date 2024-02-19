/*
 *			Calculate powers of ten
 *
 * This code will compute the required bit string for accurate power
 * of ten tables.
 */

#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_BIT	16
#define LOBYTE(x)	((x) & 0x0000ffffL)
#define HIBYTE(x)	LOBYTE(((x) >> WORD_BIT))
#define BIT(x,y)	((x) & (1 << y))
#define BIGWORD(h,l)	((((long)LOBYTE(h)) << WORD_BIT) + LOBYTE(l))

typedef unsigned int WORD;

typedef struct {
  int words;
  WORD *word;
} NUMBER;

#ifdef	__STDC__
  typedef long double longdouble;
#else
  typedef double longdouble;
#endif

void *malloc();
void *realloc();
void free();
int atoi();
double atof();

int ldbl_mant_bit;
int guards = 0;
int verbose = 0;

/* Determine number of bits in representation */

void initbits()

{
  longdouble m, f, g;

  for (ldbl_mant_bit = 0, m = 0.0, f = 1.0; ; f *= 2.0) {
    g = m + f;
    if (g - f != m)
      break;
    m = g;
    ldbl_mant_bit++;
  }
}

/* Memory allocation with error trap */

void *xmalloc(n)

unsigned n;

{
  void *p;

  if ((p = malloc(n)) == (void *) NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  return p;
}

/* Memory reallocation with error trap */

void *xrealloc(p, n)

void *p;
unsigned n;

{
  if ((p = realloc(p, n)) == (void *) NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  return p;
}

/* Allocate a new number */

NUMBER *newnumber()

{
  NUMBER *p;

  p = (NUMBER *) xmalloc(sizeof(*p));
  p->words = 0;
  return p;
}

/* Set number to a value */

void setnumber(p, v)

NUMBER *p;
int v;

{
  if (p->words != 0)
    free(p->word);

  p->word    = (WORD *) xmalloc(sizeof(*p->word));
  p->word[0] = v;
  p->words   = 1;
}

/* Set number to be fixed point with specified bits */

void setfixed(p, b)

NUMBER *p;
int b;

{
  int i;

  p->words = 1 + (b + WORD_BIT - 1) / WORD_BIT;
  p->word  = (WORD *) xrealloc(p->word, sizeof(*p->word) * p->words);
  for (i = 1; i < p->words; i++)
    p->word[i] = 0;
}

/* Multiply one number by another */

void multiply(p, q)

NUMBER *p, *q;

{
  NUMBER r;
  int i, j;
  long carry;
  long sum;

  r.words = p->words + q->words;
  r.word  = (WORD *) xmalloc(sizeof(*r.word) * r.words);

  for (i = 0; i < r.words; i++)
    r.word[i] = 0;

  for (i = 0; i < p->words; i++) {
    carry = 0;
    for (j = 0; j < q->words; j++) {
      sum = r.word[i+j] + carry + (long) p->word[i] * q->word[j];
      r.word[i+j] = (WORD) LOBYTE(sum);
      carry = HIBYTE(sum);
    }
    r.word[i+j] = (WORD) carry;
  }

  if (carry == 0)
    r.words--;

  assert(r.word[r.words-1] != 0);

  free(p->word);
  *p = r;
}

/* Divide fixed point number by a small integer */

void fixdivint(p, n)

NUMBER *p;
unsigned int n;

{
  int i;
  long rem;

  for (rem = 0, i = 0; i < p->words; i++) {
    rem = BIGWORD(rem, p->word[i]);
    p->word[i] = rem / n;
    rem %= n;
  }
}

/* Generate scale factor */

longdouble toscale(off)

int off;

{
  longdouble m;
 
  m = 1.0;
  if (off > 0) {
    while (off-- > 0)
      m *= 2.0;
  }
  else {
    while (off++ < 0)
      m /= 2.0;
  }
  return m;
}

/* Convert fixed point to double */

void fixtodouble(p, unit, f, g)

NUMBER *p;
longdouble unit;
longdouble *f, *g;

{
  int w, ww;
  int b, bb;
  int i;
  int lastbit;
  longdouble scale;
  longdouble highscale;
  longdouble lowscale;
  longdouble m;

  b = WORD_BIT-1;
  w = 1;

  scale = 1.0;
  highscale = 1.0;
  lowscale = 1.0;

  *f = 0.0;
  *g = 0.0;

  for (; BIT(p->word[w], b) == 0; scale *= 2.0) {
    if (b != 0)
      b--;
    else if (w == p->words-1)
      return;
    else {
      w++;
      b = WORD_BIT-1;
    }
  }

  if (guards) {
    for (m = 0.0, i = 0; i < ldbl_mant_bit/2; i++) {
      m *= 2.0;
      highscale *= 2.0;
      if (BIT(p->word[w], b) != 0)
        m += unit;
      if (b != 0)
        b--;
      else if (w == p->words-1) {
	m /= highscale;
        *f = m / scale;
        return;
      }
      else {
        w++;
        b = WORD_BIT-1;
      }
    }

    m /= highscale;
    *f = m / scale;

    for (; BIT(p->word[w], b) == 0; lowscale *= 2.0) {
      if (b != 0)
        b--;
      else if (w == p->words-1)
        return;
      else {
        w++;
        b = WORD_BIT-1;
      }
    }
  }

  for (m = 0.0, i = 0; i < ldbl_mant_bit; i++) {
    m *= 2.0;
    lowscale *= 2.0;
    if ((lastbit = BIT(p->word[w], b)) != 0)
      m += unit;
    if (b != 0)
      b--;
    else if (w == p->words-1) {
      m /= highscale;
      m /= lowscale;
      *g = m / scale;
      return;
    }
    else {
      w++;
      b = WORD_BIT-1;
    }
  }

  if (BIT(p->word[w], b) != 0) {
    for (ww = w, bb = b; ; ) {
      if (bb != 0)
	bb--;
      else if (ww != p->words-1) {
	ww++;
	bb = WORD_BIT-1;
      }
      else {
	if (lastbit != 0)
	  m += unit;
	break;
      }
      if (BIT(p->word[ww], bb) != 0) {
	m += unit;
	break;
      }
    }
  }

  m /= highscale;
  m /= lowscale;
  *g = m / scale;
}

/* Convert integer to double */

void todouble(p, unit, f, g)

NUMBER *p;
longdouble unit;
longdouble *f, *g;

{
  int w, ww;
  int b, bb;
  int lastbit;
  int i;
  longdouble m;

  w = p->words-1;
  b = WORD_BIT-1;

  *f = 0.0;
  *g = 0.0;

  for (; BIT(p->word[w], b) == 0; b--)
    ;

  if (guards) {
    for (m = 0.0, i = 0; i < ldbl_mant_bit/2; i++) {
      m *= 2.0;
      if (BIT(p->word[w], b) != 0)
        m += unit;
      if (b != 0)
        b--;
      else if (w == 0) {
        *f = m;
        return;
      }
      else {
        w--;
        b = WORD_BIT-1;
      }
    }

    for (i = w*WORD_BIT + b + 1; i--; )
      m *= 2.0;

    *f = m;

    for (; BIT(p->word[w], b) == 0; ) {
      if (b != 0)
        b--;
      else if (w == 0)
        return;
      else {
        w--;
        b = WORD_BIT-1;
      }
    }
  }

  for (m = 0.0, i = 0; i < ldbl_mant_bit; i++) {
    m *= 2.0;
    if ((lastbit = BIT(p->word[w], b)) != 0)
      m += unit;
    if (b != 0)
      b--;
    else if (w == 0) {
      *g = m;
      return;
    }
    else {
      w--;
      b = WORD_BIT-1;
    }
  }

  if (BIT(p->word[w], b) != 0) {
    for (ww = w, bb = b; ; ) {
      if (bb != 0)
	bb--;
      else if (ww != 0) {
	ww--;
	bb = WORD_BIT-1;
      }
      else {
	if (lastbit != 0)
	  m += unit;
	break;
      }
      if (BIT(p->word[ww], bb) != 0) {
	m += unit;
	break;
      }
    }
  }

  for (i = w*WORD_BIT + b + 1; i--; )
    m *= 2.0;

  *g = m;
}

/* Dump the floating point number as long words */

void dump(n, f, g)

int n;
longdouble f, g;

{
  char b[128];
  longdouble m;
  longdouble a;
  double aa;
  int *p;
  int i;
  int iwidth;
  int xwidth;

  iwidth = sizeof(int) < 4 ? 7 : 11;
  xwidth = sizeof(int) * 2;

  printf("  ");
  if (guards) {
    p = (int *) &f;
    for (i = sizeof(longdouble)/sizeof(int); i--; p++)
      printf("%*d,", iwidth, *p);
  }
  p = (int *) &g;
  for (i = sizeof(longdouble)/sizeof(int); i--; p++)
    printf("%*d,", iwidth, *p);
  printf(" /* 1e%03d */\n", n);

  printf("/* ");
  if (guards) {
  p = (int *) &f;
    for (i = sizeof(longdouble)/sizeof(int); i--; p++)
      printf("0x%*x, ", xwidth, *p);
  }
  p = (int *) &g;
  for (i = sizeof(longdouble)/sizeof(int); i--; p++)
    printf("0x%*x, ", xwidth, *p);
  printf("*/\n");

  if (verbose) {
    m = f + g;
    p = (int *) &m;
    printf("/* ");
    for (i = sizeof(longdouble)/sizeof(int); i--; p++)
      printf("0x%*x, ", xwidth, *p);
    printf(" ypow10  */\n");

    sprintf(b, "1e%d", n);

    a = atof(b);
    p = (int *) &a;
    printf("/* ");
    for (i = sizeof(longdouble)/sizeof(int); i--; p++)
      printf("0x%*x, ", xwidth, *p);
    printf(" atof()  */");
    if (a != m)
      printf(" /*!*/");
    printf("\n");

    sscanf(b, "%lf", &aa);
    a = aa;
    p = (int *) &a;
    printf("/* ");
    for (i = sizeof(longdouble)/sizeof(int); i--; p++)
      printf("0x%*x, ", xwidth, *p);
    printf(" scanf() */");
    if (a != m)
      printf(" /*!*/");
    printf("\n");
  }
}

int main(argc, argv)

int argc;
char **argv;

{
  NUMBER *ten;
  NUMBER *x;
  longdouble unit;
  longdouble f, g;
  longdouble (*powtab)[2];
  int j, k;
  int n, low, high, step;
  int offset;
  int arg;

  if (argc != 5 && argc != 6) {
    fprintf(stderr, "Usage: ypower [vg] low high step offset\n");
    exit(1);
  }

  if (argc == 5)
    arg = 1;
  else {
    arg = 2;
    for (j = 0; argv[1][j]; j++) {
      switch (argv[1][j]) {
      case 'v': verbose = 1; break;
      case 'g': guards  = 1; break;
      }
    }
  }

  low    = atoi(argv[arg + 0]);
  high   = atoi(argv[arg + 1]);
  step   = atoi(argv[arg + 2]);
  offset = atoi(argv[arg + 3]);
  unit   = toscale(offset);

  if (low < 0 && (high >= 0 || step >= 0)) {
    fprintf(stderr, "Inconsistent negative powers\n");
    exit(1);
  }

  if (low >= 0 && (high < 0 || step < 0)) {
    fprintf(stderr, "Inconsistent positive powers\n");
    exit(1);
  }

  initbits();

  printf("/* %d bit table generated by ypow10 */\n\n", ldbl_mant_bit);

  if (low >= 0) {
    ten = newnumber();
    x   = newnumber();
    setnumber(ten, 10);
    setnumber(x,   1);
    for (j = 0, n = low; n <= high; n += step) {
      for (; j < n; j++)
	multiply(x, ten);
      todouble(x, unit, &f, &g);
      dump(n, f, g);
    }
  }
  else {
    x = newnumber();
    setnumber(x, 1);
    setfixed(x, -high*8);
    powtab = (longdouble (*)[2]) xmalloc((low-high-step)/(-step)*sizeof(*powtab));
    for (k = j = 0, n = low; n >= high; n += step, k++) {
      for (; j > n; j--)
	fixdivint(x, 10);
      fixtodouble(x, unit, &powtab[k][0], &powtab[k][1]);
    }
    for (j = k; --j >= 0; ) {
      n -= step;
      dump(n, powtab[j][0], powtab[j][1]);
    }
  }

  return 0;
}
