/*                               _ f _ c v t                               */

/* This code implements the %f, %g and %e formats for output conversion
 * of floating point numbers. It is called from _vfprintf().
 */

#include "stdiolib.h"
#include <math.h>

/*LINTLIBRARY*/

#if '0'+1 != '1' || '1'+1 != '2' || '2'+1 != '3' || '3'+1 != '4' || \
    '4'+1 != '5' || '5'+1 != '6' || '6'+1 != '7' || '7'+1 != '8' || \
    '8'+1 != '9'
  << Violation of collating sequence assumption >>
#endif

#define LOG_10_2	(0.301029996)	/* log_10(2) */

char __xfpcvt = 0;			/* linkage for library */

/* Break into sign, mantissa and exponent
 *
 * The mantissa and exponent are extracted (base 10). This requires
 * some conversion from the internal floating point representation.
 */

static void __signmanexp F4(longdouble, x, int *, s, longdouble *, m, int *, e)

{
  int exp;				/* exponent */
  int expoffset;			/* exponent offset */
  unsigned int uxp;			/* absolute value of exponent */
  int exp10;				/* exponent base 10 */
  longdouble fexp10;			/* floating exponent base 10 */
  longdouble mantissa;			/* extracted mantissa */

/* Take absolute value */
  *s = 0;
  if (x < 0.0) {
    *s = 1;
    x  = -x;
  }

/* Break into mantissa and exponent */
  (void) LFREXP(x, &exp);

/* Scale exponent 1.0 <= man < 2.0 */
  if (x != 0)
    exp--;

/* Generate magnitude for scaling */
  expoffset = 0;
  uxp       = exp >= 0 ? exp : -exp;
  fexp10    = LOG_10_2 * uxp;
  exp10     = (int) fexp10;
#ifndef	TRUNCATE
  if (exp10 > fexp10)
    exp10--;
#endif
  if (exp < 0)
    exp10 = -exp10;

/* Divide or multiply by scale to obtain mantissa */
  for (;;) {

/* Check for denormalised operands */
    if (exp10 < LDBL_MIN_10_EXP) {
#ifdef DENORMAL
      x          *= __pow10(LDBL_MIN_10_EXP - exp10);
      expoffset  -= LDBL_MIN_10_EXP - exp10;
      exp10       = LDBL_MIN_10_EXP;
#else
      exp10    = 0;
      mantissa = 0.0;
      break;
#endif
    }

    mantissa = x / __pow10(exp10);

    if (exp10 >= 0) {
      if (mantissa >= 10.0) {
	exp10++;
	if (mantissa > 10.0)
	  continue;
	mantissa = 1.0;
      }
    }
    else {
      if (mantissa < 1.0) {
	exp10--;
	continue;
      }
    }
    break;
  }
  *e = exp10 + expoffset;
  *m = mantissa;
}

/* Convert mantissa into string
 *
 * The mantissa m == 0 || 1 <= m < 10, is converted to a string with 
 * fracdigits fractional digits and intdigits integer digits. If
 * idp != 0, a decimal point will be inserted fracdigits from the end.
 * The converted string will be rounded. The function will return the
 * number of digits in the integer part (in case this has increased due
 * to rounding). If intdigits == 0, rounding will still occur, only the
 * fractional digits will be returned in the buffer. The function will
 * subtract the number of trailing zeros from *trailing. The returned
 * string is *not* null terminated.
 */

static int __mantostr F6(char *, p, longdouble, m,
			 int, intdigits, int, fracdigits,
			 int *, trailing, int, idp)

{
  int i;				/* index */
  int j;				/* index */
  int d;				/* current digit */
  long digs;				/* current digits */
  long power;				/* power of 10 to extract top digit */
  longdouble fpower;			/* power of 10 to extract digits */
  longdouble fdigs;			/* digits in floating point */
  longguard gm;				/* guarded mantissa */
  int zeros;				/* trailing zeros */
  int dp;				/* decimal point adjustment */
  char *q, *r;				/* buffer indices */

/* The following code requires fpower * m->number not to lose accuracy.
 * This means that __Mipow10 must be at most HALFMANTDIGITS.
 */
  ASSERT(__Mipow10 <= HALFMANTDIGITS);

/* Convert the required number of digits */
  ASSERT(intdigits + fracdigits != 0);

  for (dp = 1, zeros = 0, q = p, i = intdigits+fracdigits; i != 0; dp = 0) {
    j = __Mipow10;
    if (j > i)
      j = i;
    i -= j;

    fpower = __fpow10[j-dp];
    if (dp == 0)
      m *= fpower;
    else {
      gm.number   = m;
      gm.guard    = 0.0;
      gm.exponent = 0;
      __gguard(&gm);
      gm.number *= fpower;
      gm.guard  *= fpower;
      m          = __gnumber(&gm);
    }

    m = LMODF(m, &fdigs);
    digs     = fdigs;

    while (j--) {
      power = __ipow10[j];
      d = digs / power;
      if (d == 0)
	zeros++;
      else
	zeros = 0;
      *q++  = '0' + d;
      digs -= d * power;
    }
  }

/* Round converted digit stream */
  if (m >= 0.5) {
    for (zeros = 0, r = q; ++(*--r) > '9'; ) {
      *r = '0';
      zeros++;
      if (r == p) {
	if (intdigits++ == 0) {
	  *trailing -= zeros;
	  return 1;
	}
	*r   = '1';
	*q++ = '0';
	break;
      }
    }
  }

/* Compute number of trailing zeros */
  if (zeros > fracdigits)
    zeros = fracdigits;
  *trailing -= zeros;

/* Insert decimal point */
  if (idp != 0) {
    for (r = q, i = fracdigits; i--; )
      *q-- = *--r;
    *r = '.';
  }

  return intdigits;
}

/* Convert floating point to string
 *
 * A vector of components is constructed in fv[]. The length of the
 * vector is returned by the function. The length of the string made
 * up of the vectors is returned in *length. The workspace used in
 * constructing the significant digits is passed in buf. fflag encodes
 * the format as follows:
 *
 *		format
 *		  %f		F_FPFORMATF
 *		  %e		F_FPFORMATE
 *		  %g		F_FPFORMATG
 *		  %E		F_FPFORMATE	F_FPCAPITAL
 *		  %G		F_FPFORMATG	F_FPCAPITAL
 */

int __cvt F6(__stdiosize_t *, length, FV *, fv,
             char *, buf, VA_LIST *, argp, int, precision, int, fflag)

{
  int sign;				/* sign of number */
  int intpart;				/* integer */
  int fracpart;				/* fraction */
  int exppart;				/* exponent */
  int intfill;				/* filler for integer (pre dp) */
  int fracfill;				/* filler for fraction */
  int pointfill;			/* filler after dp */
  int exponent;				/* exponent */
  int *zeros;				/* chopping of trailing zeros */
  int digits;				/* significant digits */
  int lsd;				/* least significant digit */
  int maxfltdigits;			/* digits to generate */
  char *bp;				/* buffer pointer */
  int fvx;				/* format vector index */
  longdouble x;				/* number to convert */
  longdouble mantissa;			/* mantissa */
  int rubbish;				/* dumping ground */

  fvx = 0;
  bp  = buf;
  x   = (fflag & F_LONGDOUBLE) ? VA_ARG(*argp, longdouble)
			       : VA_ARG(*argp, double);
  __signmanexp(x, &sign, &mantissa, &exponent);

/* Constrain digit generation */
  maxfltdigits = MAXFLTDIGITS;
  if (exponent < LDBL_MIN_10_EXP-1)
    if ((maxfltdigits -= LDBL_MIN_10_EXP - 1 - exponent) <= 0)
      maxfltdigits = 1;

/* Force sign if required */
  if (sign)
    *bp++    = '-';
  else if (fflag & F_SHOWSIGN)
    *bp++    = '+';
  fv[0].att = FV_F_VECTOR;
  fv[0].len = bp - buf;
  fv[0].arg = buf;
  fvx++;

/* Trailing zero check */
  zeros    = (fflag & (F_FPFORMATG | F_ALTERNATE)) == F_FPFORMATG
	     ? &fracpart : &rubbish;
  fracpart = precision;

/* No fill */
  intfill   = 0;
  pointfill = 0;
  fracfill  = 0;

/* Check for %f or %g not forcing %e format */
  if ((fflag & F_FPFORMATF) ||
     ((fflag & F_FPFORMATG) && (exponent >= -4 && exponent < fracpart))) {

/* Integer part starts here */
    if (exponent >= 0) {

/* %g format keeps the number of significant digits constant */
      if (fflag & F_FPFORMATG) {
	digits    = fracpart;
	fracpart -= exponent + 1;
      }

/* Integer part */
      intpart = exponent + 1;

/* Restrain digit generation */
      if (intpart + fracpart > maxfltdigits) {
	if (intpart > maxfltdigits) {
	  intfill  = intpart - maxfltdigits;
	  intpart  = maxfltdigits;
	  fracfill = fracpart;
	  fracpart = 0;
	}
	else {
	  fracfill = intpart + fracpart - maxfltdigits;
	  fracpart = maxfltdigits - intpart;
	}
      }

/* Convert mantissa to digit string */
      intpart = __mantostr(bp, mantissa, intpart, fracpart, zeros, 1);

/* Rounding increased number of significant digits */
      if ((fflag & F_FPFORMATG) && intpart > digits) {
	exponent++;
	bp[0]     = '1';
	bp[1]     = '.';
	fracpart  = 0;
	fracfill  = (fflag & F_ALTERNATE) != 0 ? precision-1 : 0;
	goto ForceG;
      }
    }

    else {
      bp[0]   = '0';
      bp[1]   = '.';
      intpart = 1;

      if (fracpart >= -exponent) {

/* Leading zeros in notation */
	pointfill = -exponent-1;
	fracpart -= pointfill;
	if (fracpart > maxfltdigits) {
	  fracfill = fracpart - maxfltdigits;
	  fracpart = maxfltdigits;
	}

/* Convert significant digits in mantissa */
	if (__mantostr(&bp[2], mantissa, 0, fracpart, zeros, 0)) {

/* Rounded up --- %f || fracpart == 0 */
	  if (pointfill == 0)
	    bp[0] = '1';
	  else {
	    bp[2]     = '1';
	    fracfill += fracpart;
	    fracpart  = 1;
	    pointfill--;
	  }
	}
      }

/* fracpart < -exponent --- no significant digits in fraction window */
      else {
	lsd = mantissa;
#ifndef	TRUNCATE
	if (lsd > mantissa)
	  lsd--;
#endif

	if (exponent == -1) {

/* fracpart == 0 and exponent == -1 */
	  if (lsd >= 5)
	    bp[0] = '1';
	}

	else {
    
	  lsd = fracpart == -exponent-1 && lsd >= 5;
	  if (fracpart == 0 ||
	      (fflag & (F_FPFORMATG | F_ALTERNATE)) == F_FPFORMATG && lsd == 0)
	  
/* fracpart == 0 and exponent <= -2 or %g and rounded to zero */
	    fracpart = 0;

/* Rounding to zero with fracpart != 0 && exponent <= -2 */
	  else {
	    pointfill = fracpart-1;
	    bp[2]     = "01"[lsd];
	    fracpart  = 1;
	  }
	}
      }
    }

/* Integer part */
    fv[fvx].att = FV_F_VECTOR;
    fv[fvx].len = intpart;
    fv[fvx].arg = bp;
    bp         += intpart;
    if (intfill != 0) {
      fv[++fvx].att = FV_F_PADDING;
      fv[fvx].len   = intfill;
      fv[fvx].arg   = __zfill;
      fv[++fvx].att = FV_F_VECTOR;
      fv[fvx].len   = 0;
      fv[fvx].arg   = bp;
    }

/* Decimal point, leading zeros and leading fractional part */
    if (fracpart != 0              ||
	(fflag & F_ALTERNATE) != 0 ||
	(fflag & F_FPFORMATF) != 0 && fracfill != 0) {
      fv[fvx].len++;
      bp++;
    }
    if (pointfill != 0) {
      fv[++fvx].att = FV_F_PADDING;
      fv[fvx].len   = pointfill;
      fv[fvx].arg   = __zfill;
      fv[++fvx].att = FV_F_VECTOR;
      fv[fvx].len   = 0;
      fv[fvx].arg   = bp;
    }
    fv[fvx].len += fracpart;
    bp          += fracpart;

/* Trailing zeros */
    if (fracfill == 0 || (fflag & (F_FPFORMATG | F_ALTERNATE)) == F_FPFORMATG)
      fracfill = 0;
    else {
      fv[++fvx].att = FV_F_PADDING;
      fv[fvx].len   = fracfill;
      fv[fvx].arg   = __zfill;
    }

    *length = (bp - buf) + fracfill + pointfill + intfill;

    return fvx+1;
  }

/* Must be %e or %g forcing %e */
  if ((fflag & F_FPFORMATG) && precision > 0)
    fracpart--;
  if (fracpart > maxfltdigits - 1) {
    fracfill = fracpart - maxfltdigits + 1;
    fracpart = maxfltdigits - 1;
  }

  if (__mantostr(bp, mantissa, 1, fracpart, zeros, 1) == 2) {
    exponent++;
    bp[1] = '.';
    bp[2] = '0';
  }

/* Count mantissa digits and decimal point */
ForceG:
  fv[fvx].att = FV_F_VECTOR;
  fv[fvx].len = 1 + fracpart;
  if (fracpart != 0              ||
      (fflag & F_ALTERNATE) != 0 ||
      (fflag & F_FPFORMATG) == 0 && precision > 1)
    fv[fvx].len++;
  fv[fvx].arg = bp;
  bp         += fv[fvx].len;

/* Convert exponent */
  if (fracfill != 0 && (fflag & F_FPFORMATE)) {
    fv[++fvx].att = FV_F_PADDING;
    fv[fvx].len   = fracfill;
    fv[fvx].arg   = __zfill;
    fv[++fvx].att = FV_F_VECTOR;
    fv[fvx].len   = 0;
    fv[fvx].arg   = bp;
  }

  bp[0] = (fflag & F_FPCAPITAL) ? 'E' : 'e';

  if (exponent >= 0)
    bp[1] = '+';
  else {
    bp[1] = '-';
    exponent = -exponent;
  }

  exppart      = __utoa(&bp[2], (unsigned int) exponent, 2) - bp;
  fv[fvx].len += exppart;
  *length      = (bp - buf) + exppart + fracfill;

  return fvx+1;
}
