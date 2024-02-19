/*                               _ f _ t v c                               */

/* This code implements the %f, %g and %e formats for input conversion
 * of floating point numbers. It is called from _vfscanf().
 */

extern int __mpow10;

#include "stdiolib.h"
#include <math.h>

/*LINTLIBRARY*/

#define NEXTCH()	(nbytes++, ch = getc(fp))

#if '0'+1 != '1' || '1'+1 != '2' || '2'+1 != '3' || '3'+1 != '4' || \
    '4'+1 != '5' || '5'+1 != '6' || '6'+1 != '7' || '7'+1 != '8' || \
    '8'+1 != '9'
  << Violation of collating sequence assumption >>
#endif

char __xfptvc = 0;			/* linkage for library */

/****************************************************************/
/* Assume that division or multiplication by FLT_RADIX is exact */
/****************************************************************/

int __tvc F5(FILE *, fp, int, width, VA_LIST *, argp, int, fptype, char *, ok)

{
  int ch;				/* look-ahead character */
  int nbytes;				/* bytes scanned */
  int fracpart;				/* fractional part of mantissa */
  longguard mant;			/* mantissa */
  longdouble Mantissa;			/* result */
  int exponent;				/* exponent */
  int digits;				/* significant digits */
  char haveexp;				/* exponent exists */
  char scannedzeros;			/* scanned leading zeros */
  char negative;			/* negative */
  char expnegative;			/* exponent negative */
  long imantissa;			/* mantissa collection */
  int imantdigs;			/* digits collected */
  int gdigits;				/* guard digits */
  int normexp;				/* exponent for normalised number */
  int expoffset;			/* offset for exponent */
  longguard pow10;			/* guarded power of ten */
  char *dbp;				/* point to digit insertion */
  char *edbp;				/* end of digit buffer */
  char dbuf[3*LDBL_DIG/2];		/* digit buffer */
  int i;				/* index */
  static longdouble flt_radix = FLT_RADIX;/* exponent radix */
#ifdef	LMR
  static int ldbl_max_radix[sizeof(longdouble)/sizeof(int)] = {
#include "lmr.h"
  };
#else
  static longdouble ldbl_max_radix[1];
#endif

  ch           = getc(fp);
  nbytes       = 0;
  fracpart     = 0;
  negative     = 0;
  expnegative  = 0;
  exponent     = 0;
  haveexp      = 0;
  scannedzeros = 0;
  *ok          = 0;
  dbp          = &dbuf[0];
  edbp         = &dbuf[sizeof(dbuf)/sizeof(dbuf[0])];

#ifndef	LMR
  ldbl_max_radix[0] = LDBL_MAX / FLT_RADIX;
#endif

/* Leading sign bit */
  if (width != 0) {
    if (ch == '-') {
      negative++;
      NEXTCH();
      width--;
    } else if (ch == '+') {
      NEXTCH();
      width--;
    }
  }

/* Skip leading zeros in integer part */
  while (width != 0 && ch == '0') {
    NEXTCH();
    width--;
    scannedzeros = 1;
  }

/* Integer part */
  while (width != 0 && ch >= '0' && ch <= '9') {
    if (dbp < edbp)
      *dbp++ = ch;
    else
      fracpart--;
    NEXTCH();
    width--;
  }

/* Fractional part */
  if (width != 0 && ch == '.') {
    NEXTCH();
    width--;

/* Skip leading zeros in fractional part */
    if (dbp == &dbuf[0]) {
      while (width != 0 && ch == '0') {
        NEXTCH();
        width--;
	fracpart++;
        scannedzeros = 1;
      }
    }

/* Fractional part proper */
    while (width != 0 && ch >= '0' && ch <= '9') {
      if (dbp < edbp) {
	*dbp++ = ch;
	fracpart++;
      }
      NEXTCH();
      width--;
    }
  }

/* Discard trailing zeros */
  for (digits = dbp - &dbuf[0]; digits > 0 && *--dbp == '0'; digits--)
    fracpart--;

/* Validate mantissa */
  if (scannedzeros == 0 && digits == 0)
    goto Failed;

/* Convert digits */
  imantissa     = 0;
  imantdigs     = 0;
  gdigits       = 0;
  mant.number   = 0.0;
  mant.guard    = 0.0;
  mant.exponent = 0;
  for (dbp = &dbuf[0], i = 0; i < digits; imantdigs++, i++, dbp++) {
    if (i != LDBL_DIG/2 && imantdigs < __Mipow10)
      imantissa = imantissa * 10 + (dbp[0] - '0');
    else {
      if (i == LDBL_DIG/2)
	mant.number = imantissa;
      else {
	mant.guard = mant.guard * __fpow10[imantdigs] + imantissa;
	gdigits   += imantdigs;
      }
      imantissa = dbp[0] - '0';
      imantdigs = 0;
    }
  }
  if (imantdigs != 0) {
    if (i <= LDBL_DIG/2)
      mant.number = imantissa;
    else {
      mant.guard = mant.guard * __fpow10[imantdigs] + imantissa;
      gdigits   += imantdigs;
    }
  }
  if (gdigits != 0)
    mant.guard /= __fpow10[gdigits];

/* In order to ensure that LDBL_MIN is able to be scanned properly
 * it is necessary the normalised decimal mantissa need only a single
 * guarded multiplication to convert it into the required number.
 */
   ASSERT(LDBL_DIG/2 - 1 + LDBL_MIN_10_EXP >= __mpow10);

/* Exponent */
  if (width != 0 && (ch == 'e' || ch == 'E')) {
    NEXTCH();
    width--;
    if (width != 0) {
      if (ch == '-') {
	expnegative = 1;
	NEXTCH();
	width--;
      } else if (ch == '+') {
	NEXTCH();
	width--;
      }
    }

    expoffset = digits - fracpart - 1;
    if (expnegative)
      expoffset = -expoffset;

    while (width != 0 && ch >= '0' && ch <= '9') {
      haveexp  = 1;
      if (exponent + expoffset <= LDBL_MAX_10_EXP)
        exponent = exponent * 10 + (ch - '0');
      NEXTCH();
      width--;
    }
    if (haveexp == 0)
      goto Failed;
    if (expnegative)
      exponent = -exponent;
  }

/* Merge mantissa and exponent */
  exponent += gdigits - fracpart;

  if (exponent < 0) {

    while (exponent < __mpow10) {
      __gpow10(__mpow10, &pow10);
      __ggmul(&mant, &mant, &pow10);
      __gguard(&mant);
      exponent -= __mpow10;
    }

    __gpow10(exponent, &pow10);
    Mantissa = __gmul(&mant, &pow10);
  }

  else {

    normexp = exponent + (digits - gdigits) - 1;

/* Magnitude of number is too great */
    if (normexp > LDBL_MAX_10_EXP)
      Mantissa = HUGE_VAL;

/* Exponent is in range or on limit */
    else {
      while (exponent > LDBL_MAX_10_EXP) {
	__gpow10(LDBL_MAX_10_EXP, &pow10);
	__ggmul(&mant, &mant, &pow10);
	__gguard(&mant);
	exponent -= LDBL_MAX_10_EXP;
      }

/* Number is well within range */
      if (normexp < LDBL_MAX_10_EXP) {
	__gpow10(exponent, &pow10);
	Mantissa = __gmul(&mant, &pow10);
      }

/* Number may overflow and is close to the limit of the representation. */
      else {
	__gpow10(gdigits - digits + 1, &pow10);
	__ggmul(&mant, &mant, &pow10);
	__gguard(&mant);
	__gpow10(LDBL_MAX_10_EXP, &pow10);
	Mantissa  = __gmul(&mant, &pow10);
        Mantissa /= flt_radix;
        if (Mantissa <= * (longdouble *) &ldbl_max_radix[0])
	  Mantissa *= flt_radix;
        else
	  Mantissa = HUGE_VAL;
      }
    }
  }

/* Merge sign */
  if (negative)
    Mantissa = -Mantissa;

/* Generate result */
  *ok = 1;
  switch (fptype) {
  case 1: * VA_ARG(*argp, longdouble *) = Mantissa; break;
  case 2: * VA_ARG(*argp, double *)     = Mantissa; break;
  case 3: * VA_ARG(*argp, float *)      = Mantissa; break;
  }

Failed:
  ungetc(ch, fp);
  return nbytes;
}
