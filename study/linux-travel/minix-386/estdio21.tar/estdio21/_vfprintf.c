/*                            _ v f p r i n t f                            */

/* This function performs all the formatted printing. Floating point
 * requests are forwarded to lower level floating point conversion
 * functions. The function must not drag in all of the stdio library
 * so that sprintf() can be used by programs which don't necessarily
 * want the rest of stdio.
 */

#include "stdiolib.h"
#include <ctype.h>

/*LINTLIBRARY*/

/* Number to string conversion
 *
 * Separate routines are provided to convert numbers from binary to
 * string representations for each base. Furthermore, long to int
 * front end functions are provided in order to improve performance
 * of long conversions.
 */
 
/* These definitions provide the masks necessary to convert longs
 * where longs are wider than the natural word size. The %ld format
 * requires the largest power of ten that will fit into an integer.
 * For an integer of N bits, highest power of ten that will fit in
 * N.log_10 2.
 */

#if LONG_MAX != INT_MAX
# define UINTBITS	(CHAR_BIT * sizeof(unsigned int))
# define HEXSHIFT	(UINTBITS/4*4)
# define HEXMASK	((unsigned int) ((1L << HEXSHIFT) - 1))
# define HEXDIGITS	(UINTBITS/4)
# define OCTSHIFT	(UINTBITS/3*3)
# define OCTMASK	((unsigned int) ((1L << OCTSHIFT) - 1))
# define OCTDIGITS	(UINTBITS/3)
# define DECDIGITS	(UINTBITS * 301 / 1000)
#endif

/* Radix descriptor table entry */

typedef struct radix {
  char *(*utoa) P((unsigned int, char *, char *));
					/* int to string function */
#if LONG_MAX != INT_MAX
  char *(*ultoa) P((unsigned long, char *, struct radix *));
					/* long to string function */
  int lshift;				/* bit shift */
  unsigned int lmask;			/* bit mask */
  int ldigits;				/* digits per section */
#endif
  char *conv;				/* conversion table */
} RADIX;

/* Function prototypes for unsigned and unsigned long convertors */

typedef char * (*utof) P((unsigned int, char *, char *));
typedef char * (*ultof) P((unsigned int, char *, RADIX *));

/* unsigned int to string convertors */

static char *__utod F3(register unsigned int, n,
                       register char *, p,
                       char *, conv)

{
  unsigned int q;

  do {
    q    = n / 10;
    *--p = (n - q*10) + '0';
  } while ((n = q) != 0);
  return p;
}

static char *__utoo F3(register unsigned int, n,
                       register char *, p,
                       char *, conv)

{
  do {
    *--p = (n & 0x7) + '0';
  } while ((n >>= 3) != 0);
  return p;
}

static char *__utox F3(register unsigned int, n,
                       register char *, p,
                       char *, conv)

{
  do {
    *--p = conv[n & 0xf];
  } while ((n >>= 4) != 0);
  return p;
}

/* unsigned long to string convertors */

#if LONG_MAX != INT_MAX
static char *__ultod F3(unsigned long, n, char *, p, RADIX *, r)

{
  unsigned long q;
  unsigned int d;
  char *t;
  int i;
  
  ASSERT(DECDIGITS <= __Mipow10);

  for (;;) {
    q = n / __ipow10[DECDIGITS];
    d = n - q * __ipow10[DECDIGITS];
    t = (*r->utoa)(d, p, NULL);
    if ((n = q) == 0)
      break;
    for (i = DECDIGITS - (p - t); i--; )
      *--t = '0';
    p = t;
  }
  return t;
}

static char *__ultob F3(unsigned long, n, char *, p, RADIX *, r)

{
  int i;
  char *t;

  for (;;) {
    t = (*r->utoa)((unsigned int) n & r->lmask, p, r->conv);
    if ((n >>= r->lshift) == 0)
      break;
    for (i = r->ldigits - (p - t); i--; )
      *--t = '0';
    p = t;
  }
  return t;
}
#endif

/* Digit to character conversion table */

static char __lconv[] = "0123456789abcdef";
static char __uconv[] = "0123456789ABCDEF";

/* Entries in radix descriptor table */

#define N_RADIX		(sizeof(__radix)/sizeof(__radix[0]))
#define R_DECIMAL	0
#define R_HEXL		1
#define R_HEXU		2
#define R_OCTAL		3

/* Radix descriptor table */

#if LONG_MAX != INT_MAX
#define LONG(ultoa,lshift,lmask,ldigits) (ultoa),(lshift),(lmask),(ldigits),
#else
#define LONG(ultoa,lshift,lmask,ldigits)
#endif

static RADIX __radix[] = {
  {__utod, LONG(__ultod, 0, 0, 0)        NULL},			/* %d %u */
  {__utox, LONG(__ultob, HEXSHIFT, HEXMASK, HEXDIGITS) __lconv},/* %x */
  {__utox, LONG(__ultob, HEXSHIFT, HEXMASK, HEXDIGITS) __uconv},/* %X */
  {__utoo, LONG(__ultob, OCTSHIFT, OCTMASK, OCTDIGITS) NULL},	/* %o */
};

/* Formatted component vector print
 *
 * This function prints component vectors of the output string.
 */
 
static void __printv F3(register FILE *, fp, int, vc, register FV *, vp)

{
  __stdiobuf_t *wp;			/* output buffer pointer */
  __stdiobuf_t *p;			/* buffer pointer */
  int pad;				/* padding only */
  __stdiosize_t wb;			/* bytes to write */
  __stdiosize_t wf;			/* free bytes in buffer */
  __stdiosize_t len;			/* length of string */

  for (len = 0; ;) {

/* Gauge the amount of free space in the stream buffer */
    if (! TESTFLAG(fp, _IONBF))
      wf = UNUSEDINWRITEBUFFER(fp);
    else {
      fp->__wptr = fp->__wend = fp->__base = &fp->__buf;
      wf = 0;
    }
    wp = GETWRITEPTR(fp);

    for (;;) {
      if ((wb = len) != 0 && wf != 0) {

	if (wb > wf)
	  wb = wf;

	wf  -= wb;
	len -= wb;

	if (pad != 0)
	  MEMSET((char *) wp, *p, (size_t) wb);
	else {
	  MEMCPY((char *) wp, (char *) p, (size_t) wb);
	  p += wb;
	}

	wp += wb;
      }

/* Determine the nature and size of the next string to write */
      if (len == 0) {
	if (vc == 0) {
	  SETWRITEPTR(fp, wp);
	  return;
	}

	pad = vp->att & FV_F_PADDING;
	len = vp->len;
	p   = (__stdiobuf_t *) vp->arg;
	vc--;
	vp++;
	continue;
      }

/* Flush a filled stream buffer */
      if (TESTFLAG(fp, _IONBF)) {
	fp->__base = p;
	if (pad != 0 && wb > PWRITEAHEAD)
	  wb = PWRITEAHEAD;
	len -= wb;
	wp   = p + wb;
      }
      SETWRITEPTR(fp, wp);
      (void) FFLUSH(fp);
      break;
    }
  }
}

/* Filler strings */

#if PWRITEAHEAD != 4
  << Print buffer write ahead assumed to be 4 >>
#else
char __zfill[PWRITEAHEAD] = {			/* zero fill */
  '0', '0', '0', '0' };
static char __bfill[PWRITEAHEAD] = {		/* blank fill */
  ' ', ' ', ' ', ' ' };
#endif

/* Format indicator */

#if	0 < '%'
# define FMTCH(x)	( (x) <= '%' )
#else
# define FMTCH(x)	( (x) >= '%' )
#endif

int __vfprintf F3(FILE *, fp, CONST char *, fmt, VA_LIST, args)

{
  register __stdiobuf_t *wp;		/* direct write pointer */
  register __stdiobuf_t *sp;		/* format string pointer */
  register __stdiobuf_t *we;		/* end of output buffer */
  __stdiobuf_t *pb;			/* line buffered scan point */
  __stdiobuf_t *sb;			/* base of this format */

  int c;				/* conversion temporary */

  char *p, *q;				/* pointers into workspace */

  int flag;				/* flags */

  FV leftfill;				/* left fill */
  
  __stdiosize_t bytes;			/* bytes output */
  __stdiosize_t width;			/* field width */
  __stdiosize_t precision;		/* precision */
  __stdiosize_t length;			/* raw length of output */

  char sign;				/* conversion is signed */
  char negative;			/* number is negative */
  long vl;				/* conversion temporary */
  RADIX *radix;				/* radix descriptor */
  int vi;				/* conversion temporary */

  int fvc;				/* vector count */
  int fvx;				/* vector index */
  FV *fvp;				/* vector scanner */
  FV fv[FV_ARGS];			/* formatted vectors */

  char buf[PBUFFERSIZE];		/* workspace */

  if (CHECKWRITE(fp))
    return 0;

/* Initialise once for buffered streams */
  fv[FV_FMTPREFIX].att = FV_F_VECTOR;
  fv[FV_FMTPREFIX].len = 0;

/* Buffer start for line buffered streams */
  pb = GETWRITEPTR(fp);
  
  bytes = 0;

  for (sp = (__stdiobuf_t *) fmt; ;) {

/* Scan for format specifier */
    sb = sp;
    if (TESTFLAG(fp, _IONBF)) {
      fv[FV_FMTPREFIX].arg = (char *) sb;
      if ((sp = strchr((char *) sb, '%')) != NULL)
	fv[FV_FMTPREFIX].len = sp - sb;
      else {
	length = strlen((char *) sb);
	bytes += length;
	sb = sp = "";
	fv[FV_FMTPREFIX].len = length;
	__printv(fp, 1, &fv[0]);
      }
    }
    else {
      for (;;) {

	wp = GETWRITEPTR(fp);
	we = GETWRITELIMIT(fp);

	if (BUFFERSIZE(fp) >= PWRITEAHEAD) {
	  we -= PWRITEAHEAD;
	  while (wp < we) {
	    if (FMTCH(wp[0] = sp[0]) && (wp[0] == 0 || wp[0] == '%'))
	      goto Format0;
	    if (FMTCH(wp[1] = sp[1]) && (wp[1] == 0 || wp[1] == '%'))
	      goto Format1;
	    if (FMTCH(wp[2] = sp[2]) && (wp[2] == 0 || wp[2] == '%'))
	      goto Format2;
	    if (FMTCH(wp[3] = sp[3]) && (wp[3] == 0 || wp[3] == '%'))
	      goto Format3;
	    sp += PWRITEAHEAD;
	    wp += PWRITEAHEAD;
	  }
	  we += PWRITEAHEAD;
	}
	for (; wp < we; wp++, sp++) {
	  if (FMTCH(wp[0] = sp[0]) && (wp[0] == 0 || wp[0] == '%'))
	    goto Format0;
	}
	SETWRITEPTR(fp, wp);
	FFLUSH(fp);
      }

Format3: wp += 3; sp += 3; goto Format0;
Format2: wp += 2; sp += 2; goto Format0;
Format1: wp += 1; sp += 1; goto Format0;
Format0:

      SETWRITEPTR(fp, wp);
    }

/* Format or end of format found */
    bytes += sp - sb;

    if (*sp == 0)
      break;
    sp++;

/* Initialise conversion variables */
    p = q = &buf[sizeof(buf)-1];

    flag         = 0;
    leftfill.arg = __bfill;
    width        = 0;

/* Switch through all the format options */
Parse_again:
    switch (c = *sp++) {

      case '-': flag |= F_LEFTJUSTIFY; goto Parse_again;
      case '+': flag |= F_SHOWSIGN;    goto Parse_again;
      case ' ': flag |= F_BLANKPREFIX; goto Parse_again;
      case '#': flag |= F_ALTERNATE;   goto Parse_again;

/* Check for field width and precision */
      case '*':
	if ((c = VA_ARG(args, int)) < 0) {
	  flag |= F_LEFTJUSTIFY;
	  c = -c;
	}
	width = c;
	c = *sp++;
	goto Precision;

/* Zero fill and field width */
      case '0':
	leftfill.arg = __zfill;
	c = *sp++;

      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
	for (; c >= '0' && c <= '9'; c = *sp++) {
	  width *= 10;
	  width += (c - '0');
	}

      case '.':
Precision:
	if (c == '.') {
	  c = *sp++;
	  if (c == '*') {
	    if ((c = VA_ARG(args, int)) >= 0) {
	      flag |= F_PRECISION;
	      precision = c;
	    }
	    sp++;
	  }
	  else if (c >= '0' && c <= '9') {
	    flag |= F_PRECISION;
	    precision = 0;
	    for (; c >= '0' && c <= '9'; c = *sp++) {
	      precision *= 10;
	      precision += (c - '0');
	    }
	  }
	}
	sp--;
	goto Parse_again;

/* Short indication */
      case 'h':
	flag = (flag & ~(F_SHORT | F_LONG | F_LONGDOUBLE)) | F_SHORT;
	goto Parse_again;

/* Long indication */
      case 'l':
	flag = (flag & ~(F_SHORT | F_LONG | F_LONGDOUBLE)) | F_LONG;
	goto Parse_again;

/* Long double indication */
      case 'L':
	flag = (flag & ~(F_SHORT | F_LONG | F_LONGDOUBLE)) | F_LONGDOUBLE;
	goto Parse_again;

/* Number of bytes output */
      case 'n':
	*(VA_ARG(args, int *)) = bytes;
	continue;

/* Pointer */
      case 'p':
	flag = (flag & ~(F_SHORT | F_LONG | F_LONGDOUBLE)) |
	       F_LONG * (sizeof(void *) == sizeof(long));
	sign  = 0;
	radix = &__radix[R_HEXL];
	goto oxud;

/* Integer conversions */
      case 'X':
	flag |= F_0X;
	sign  = 0;
	radix = &__radix[R_HEXU];
	goto oxud;

      case 'x':
	sign  = 0;
	radix = &__radix[R_HEXL];
	goto oxud;

      case 'u':
	sign  = 0;
	radix = &__radix[R_DECIMAL];
	goto oxud;

      case 'o':
	sign  = 0;
	radix = &__radix[R_OCTAL];
	goto oxud;

      case 'i':
      case 'd':
        sign  = 1;
	radix = &__radix[R_DECIMAL];

oxud:
	if ((flag & F_LONG) != 0) {
	  if (sign) vl = VA_ARG(args, long);
	  else      vl = VA_ARG(args, unsigned long);
	  if ((negative = sign && vl < 0) != 0)
	    vl = -vl;
#if LONG_MAX != INT_MAX
	  q = (*radix->ultoa)((unsigned long) vl, q, radix);
#else
	  q = (*radix->utoa)((unsigned int) vl, q, radix->conv);
#endif
	}
	else {
	  if ((flag & F_SHORT) != 0)
	    if (sign) vi = (int) VA_ARG(args, short);
	    else      vi = (int) VA_ARG(args, unsigned short);
	  else
	    if (sign) vi = (int) VA_ARG(args, int);
	    else      vi = (int) VA_ARG(args, unsigned);
	  if ((negative = (sign && vi < 0)) != 0)
	    vi = -vi;
	  q = (*radix->utoa)((unsigned int) vi, q, radix->conv);
	}

	if ((flag & F_PRECISION) != 0 && precision > width) {
	  width        = precision;
	  leftfill.arg = __zfill;
	}

        fv[FV_INTINT].att  = FV_F_VECTOR;
        fv[FV_INTINT].len  = p - q;
        fv[FV_INTINT].arg  = q;
        fv[FV_INTSIGN].att = FV_F_VECTOR;
        fv[FV_INTSIGN].len = 0;
        fvc = FV_INTARGS;

	if (negative) {
	  fv[FV_INTSIGN].len++;
	  *--q = '-';
	}
	else if (sign) {
	  if ((flag & F_SHOWSIGN) != 0) {
	    fv[FV_INTSIGN].len++;
	    *--q = '+';
	  }
	  else if ((flag & F_BLANKPREFIX) != 0) {
	    fv[FV_INTSIGN].len++;
	    *--q = ' ';
	  }
	}
	if ((flag & F_ALTERNATE) != 0) {
	  if (radix == &__radix[R_OCTAL]) {
	    fv[FV_INTSIGN].len++;
	    *--q = '0';
	  }
	  else if (radix == &__radix[R_HEXL] || radix == &__radix[R_HEXU]) {
	    fv[FV_INTSIGN].len += 2;
	    *--q = (flag & F_0X) != 0 ? 'X' : 'x';
            *--q = '0';
	  }
	}
        fv[FV_INTSIGN].arg = q;
        length             = p - q;
	break;

/* Floating point formats */
      case 'f':
	flag |= F_FPFORMATF;
	goto Fconvert;

      case 'E':
	flag |= F_FPCAPITAL;
      case 'e':
	flag |= F_FPFORMATE;
	goto Fconvert;

      case 'G':
	flag |= F_FPCAPITAL;
      case 'g':
	flag |= F_FPFORMATG;
Fconvert:
	if ((flag & F_PRECISION) == 0) precision = 6;
	fvc = FV_FMTARGS
	    + __cvt(&length, &fv[FV_FMTARGS], buf, &args, precision, flag);
	ASSERT(fvc <= FV_ARGS);
	break;

/* Single character format */
      case 'c':
	*--q = VA_ARG(args, int);
	goto Simple_vector;

/* String format */
      case 's':
	if ((q = VA_ARG(args, char *)) == NULL)
	  q = "(null)";
	if ((flag & F_PRECISION) == 0)
	  p = q + strlen(q);
	else {
	  if ((p = (char *) MEMCHR(q, 0, (size_t) precision)) == NULL)
	    p = q + precision;
	}
	goto Simple_vector;

/* Default just print it */
      default:
	*--q = sp[-1];

/* Construct a simple vector */
Simple_vector:
	length = p - q;
	fv[FV_INTINT].att  = FV_F_VECTOR;
	fv[FV_INTINT].len  = length;
	fv[FV_INTINT].arg  = q;
	fv[FV_INTSIGN].att = FV_F_VECTOR;
	fv[FV_INTSIGN].len = 0;
        fvc = FV_INTARGS;
	break;
    }

/* Start vector index */
    fvx    = 0;
    fvp    = &fv[0];
    bytes += length;

/* Subtract to find padding required --- no padding */
    if (width <= length) {
      width = 0;

      fv[FV_FMTARGS+1].len += fv[FV_FMTARGS].len;
      fv[FV_FMTARGS+1].arg -= fv[FV_FMTARGS].len;
      fv[FV_FMTARGS].len    = 0;
    }

/* Some padding required (may be right or left) */
    else {
      width -= length;

/* Required right justification (padding on left) */
      if ((flag & F_LEFTJUSTIFY) == 0) {

/* Print format string first */
#if	FV_FMTARGS != 1
	<< FV_FMTARGS assumed to be 1 >>
#else
	if (fvp->len > 0)
	  __printv(fp, FV_FMTARGS, fvp);
#endif
	fvp += FV_FMTARGS;
	fvx += FV_FMTARGS;

/* Check for negative and zero fill */
	if (leftfill.arg == __zfill && fv[FV_FMTARGS].len != 0) {
	  __printv(fp, 1, &fv[FV_FMTARGS]);
	  fvx++;
	  fvp++;
	}

/* Now output the rest of the padding */
	bytes       += width;
	leftfill.att = FV_F_PADDING;
	leftfill.len = width;
	width        = 0;
	__printv(fp, 1, &leftfill);
      }
    }

/* Output the string proper */
    __printv(fp, fvc-fvx, fvp);

/* Check for required right padding */
    if (width != 0) {
      bytes       += width;
      leftfill.att = FV_F_PADDING;
      leftfill.len = width;
      leftfill.arg = __bfill;
      __printv(fp, 1, &leftfill);
    }
  }

/* Flush line buffered streams */
  if (TESTFLAG(fp, _IOLBF)) {
    length = bytes;
    if (bytes + pb != GETWRITEPTR(fp)) {
      pb   = fp->__base;
      length = BYTESINWRITEBUFFER(fp);
    }
    pb = (__stdiobuf_t *) MEMCHR(pb, '\n', (size_t) length);
    if (pb != NULL)
      (void) FFLUSH(fp);
  }

  return ferror(fp) ? EOF : bytes;
}
