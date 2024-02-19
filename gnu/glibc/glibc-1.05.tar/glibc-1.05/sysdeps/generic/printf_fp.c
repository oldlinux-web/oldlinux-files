/* Floating-point printing for `printf'.
   This is an implementation of a restricted form of the `Dragon4'
   algorithm described in "How to Print Floating-Point Numbers Accurately",
   by Guy L. Steele, Jr. and Jon L. White, presented at the ACM SIGPLAN '90
   Conference on Programming Language Design and Implementation.

Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <ctype.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <localeinfo.h>

#include <printf.h>

#define	outchar(x)							      \
  do									      \
    {									      \
      register CONST int outc = (x);					      \
      if (putc (outc, s) == EOF)					      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

#if FLT_RADIX != 2

double
frexp (double f, int *e)
{
  #error "Don't know how to extract fraction and exponent from `double'."
}

#undef	ldexp
#ifdef	__GNUC__
inline
#endif
static double
ldexp (double f, int e)
{
  while (e > 0)
    {
      f *= FLT_RADIX;
      --e;
    }
  while (e < 0)
    {
      f /= FLT_RADIX;
      ++e;
    }
}

#endif


int
DEFUN(__printf_fp, (s, info, args),
      FILE *s AND CONST struct printf_info *info AND va_list *args)
{
  int done = 0;

  /* Decimal point character.  */
  CONST char *CONST decimal = _numeric_info->decimal_point;

  LONG_DOUBLE fpnum;		/* Input.  */
  int is_neg;

  LONG_DOUBLE f;		/* Fraction.  */

  int e;			/* Base-2 exponent of the input.  */
  CONST int p = DBL_MANT_DIG;	/* Internal precision.  */
  LONG_DOUBLE scale, scale10;	/* Scale factor.  */
  LONG_DOUBLE loerr, hierr;	/* Potential error in the fraction.  */
  int k;			/* Digits to the left of the decimal point.  */
  int cutoff;			/* Where to stop generating digits.  */
  LONG_DOUBLE r, r2, r10;	/* Remainder.  */
  int roundup;
  int low, high;
  char digit;

  int j;

  char type = tolower (info->spec);
  int prec = info->prec;
  int width = info->width;

  /* This algorithm has the nice property of not needing a buffer.
     However, to get the padding right for %g format, we need to know
     the length of the number before printing it.  */

#ifndef	LDBL_DIG
#define	LDBL_DIG	DBL_DIG
#endif
#ifndef	LDBL_MAX_10_EXP
#define	LDBL_MAX_10_EXP	DBL_MAX_10_EXP
#endif

  char *buf = __alloca ((prec > LDBL_DIG ? prec : LDBL_DIG) +
			LDBL_MAX_10_EXP + 3); /* Dot, e, exp. sign.  */
  register char *bp = buf;
#define	put(c)	*bp++ = (c)

  /* Fetch the argument value.  */
  if (info->is_long_double)
    fpnum = va_arg (*args, LONG_DOUBLE);
  else
    fpnum = (LONG_DOUBLE) va_arg (*args, double);

#ifdef	HANDLE_SPECIAL
  /* Allow for machine-dependent (or floating point format-dependent) code.  */
  HANDLE_SPECIAL (done, s, info, fpnum);
#endif

#ifndef	IS_NEGATIVE
#define	IS_NEGATIVE(num)	((num) < 0)
#endif

  is_neg = IS_NEGATIVE (fpnum);
  if (is_neg)
    fpnum = - fpnum;

  if (prec == -1)
    prec = 6;
  
  if (type == 'g')
    {
      if (prec == 0)
	prec = 1;

      if (fpnum != 0)
	{
	  if (fpnum < 1e-4)
	    type = 'e';
	  else
	    {
	      f = 10;
	      j = prec;
	      if (j > p)
		j = p;
	      while (--j > 0)
		{
		  f *= 10;
		  if (fpnum > f)
		    {
		      type = 'e';
		      break;
		    }
		}
	    }
	}

      /* For 'g'/'G' format, the precision specifies "significant digits",
	 not digits to come after the decimal point.  */
      --prec;
    }

  if (fpnum == 0)
    /* Special case for zero.
       The general algorithm does not work for zero.  */
    {
      put ('0');
      if (tolower (info->spec) != 'g' || info->alt)
	{
	  if (prec > 0 || info->alt)
	    put (*decimal);
	  while (prec-- > 0)
	    put ('0');
	}
      if (type == 'e')
	{
	  put (info->spec);
	  put ('+');
	  put ('0');
	  put ('0');
	}
    }
  else
    {
      /* Split the number into a fraction and base-2 exponent.  */
      f = frexp (fpnum, &e);

      /* Scale the fractional part by the highest possible number of
	 significant bits of fraction.  We want to represent the
	 fractional part as a (very) large integer.  */
      f = ldexp (f, p);

      cutoff = -prec;

      roundup = 0;

      if (e > p)
	{
	  /* The exponent is bigger than the number of fractional digits.  */
	  r = ldexp (f, e - p);
	  scale = 1;
	  /* The number is (E - P) factors of two larger than
	     the fraction can represent; this is the potential error.  */
	  loerr = ldexp (1.0, e - p);
	}
      else
	{
	  /* The number of fractional digits is greater than the exponent.
	     Scale by the difference factors of two.  */
	  r = f;
	  scale = ldexp (1.0, p - e);
	  loerr = 1.0;
	}
      hierr = loerr;

      /* Fixup.  */

      if (f == ldexp (1.0, p - 1))
	{
	  /* Account for unequal gaps.  */
	  hierr = ldexp (hierr, 1);
	  r = ldexp (r, 1);
	  scale = ldexp (scale, 1);
	}

      scale10 = ceil (scale / 10.0);
      k = 0;
      while (r < scale10)
	{
	  --k;
	  r *= 10;
	  loerr *= 10;
	  hierr *= 10;
	}
      do
	{
	  r2 = 2 * r;

	  while (r2 + hierr >= 2 * scale)
	    {
	      scale *= 10;
	      ++k;
	    }

	  /* Perform any necessary adjustment of loerr and hierr to
	     take into account the formatting requirements.  */

	  if (type == 'e')
	    cutoff += k;	/* CutOffMode == "relative".  */
	  /* Otherwise CutOffMode == "absolute".  */

	  {			/* CutOffAdjust.  */
	    int a = cutoff - k;
	    double y = scale;
	    while (a > 0)
	      {
		y *= 10;
		--a;
	      }
	    while (a < 0)
	      {
		y = ceil (y / 10);
		++a;
	      }
	    /* y == ceil (scale * pow (10.0, (double) (cutoff - k))) */
	    if (y > loerr)
	      loerr = y;
	    if (y > hierr)
	      {
		hierr = y;
		roundup = 1;
	      }
	  }			/* End CutOffAdjust.  */
	} while (r2 + hierr >= 2 * scale);

      /* End Fixup.  */

      /* First digit.  */
      --k;
      r10 = r * 10;
      digit = '0' + (unsigned int) floor (r10 / scale);
      r = fmod (r10, scale);
      loerr *= 10;
      hierr *= 10;
      
      low = 2 * r < loerr;
      if (roundup)
	high = 2 * r >= (2 * scale) - hierr;
      else
	high = 2 * r > (2 * scale) - hierr;
      
      if (low || high || k == cutoff)
	{
	  if ((high && !low) || (2 * r > scale))
	    ++digit;
	}

      if (type == 'e')
	{
	  /* Exponential notation.  */

	  int expt = k;		/* Base-10 exponent.  */
	  int expt_neg;

	  expt_neg = k < 0;
	  if (expt_neg)
	    expt = - expt;

	  /* Find the magnitude of the exponent.  */
	  j = 1;
	  do
	    j *= 10;
	  while (j <= expt);

	  /* Write the first digit.  */
	  put (digit);

	  if (low || high || k == cutoff)
	    {
	      if (prec > 0 || info->alt)
		put (*decimal);
	    }
	  else
	    {
	      put (*decimal);

	      /* First post-decimal digit.  */
	      --k;
	      r10 = r * 10;
	      digit = '0' + (unsigned int) floor (r10 / scale);
	      r = fmod (r10, scale);
	      loerr *= 10;
	      hierr *= 10;
	      
	      low = 2 * r < loerr;
	      if (roundup)
		high = 2 * r >= (2 * scale) - hierr;
	      else
		high = 2 * r > (2 * scale) - hierr;
	      
	      if (low || high || k == cutoff)
		{
		  if ((high && !low) || (2 * r > scale))
		    ++digit;
		  put (digit);
		}
	      else
		{
		  put (digit);
		  
		  /* Remaining digits.  */
		  while (1)
		    {
		      --k;
		      r10 = r * 10;
		      digit = '0' + (unsigned int) floor (r10 / scale);
		      r = fmod (r10, scale);
		      loerr *= 10;
		      hierr *= 10;
		      
		      low = 2 * r < loerr;
		      if (roundup)
			high = 2 * r >= (2 * scale) - hierr;
		      else
			high = 2 * r > (2 * scale) - hierr;
		      
		      if (low || high || k == cutoff)
			{
			  if ((high && !low) || (2 * r > scale))
			    ++digit;
			  put (digit);
			  break;
			}
		      
		      put (digit);
		    }
		}
	    }

	  if (tolower (info->spec) != 'g' || info->alt)
	    /* Pad with zeros.  */
	    while (k-- >= cutoff)
	      put ('0');

	  /* Write the exponent.  */
	  put (isupper (info->spec) ? 'E' : 'e');
	  put (expt_neg ? '-' : '+');
	  if (expt < 10)
	    /* Exponent always has at least two digits.  */
	    put ('0');
	  do
	    {
	      j /= 10;
	      put ('0' + (expt / j));
	      expt %= j;
	    }
	  while (j > 1);
	}
      else
	{
	  /* Decimal fraction notation.  */

	  if (k < 0)
	    {
	      put ('0');
	      if (prec > 0 || info->alt)
		put (*decimal);
	    }

	  /* Write leading fractional zeros.  */
	  j = 0;
	  while (--j > k)
	    put ('0');

	  if (low || high || k == cutoff)
	    put (digit);
	  else
	    while (1)
	      {
		put (digit);
		
		--k;
		digit = '0' + (unsigned int) floor ((r * 10) / scale);
		r = fmod (r * 10, scale);
		loerr *= 10;
		hierr *= 10;
		
		low = 2 * r < loerr;
		if (roundup)
		  high = 2 * r >= (2 * scale) - hierr;
		else
		  high = 2 * r > (2 * scale) - hierr;
		
		if (low || high || k == cutoff)
		  {
		    if ((high && !low) || (2 * r > scale))
		      ++digit;
		    put (digit);
		    break;
		  }
		
		if (k == -1)
		  put (*decimal);
	      }
	  while (k > 0)
	    {
	      put ('0');
	      --k;
	    }
	  if (prec > 0 || info->alt)
	    {
	      if (k == 0)
		put (*decimal);
	      while (k-- > -prec)
		put ('0');
	    }
	}
    }

#undef	put

  /* The number is all converted in BUF.
     Now write it with sign and appropriate padding.  */

  if (is_neg || info->showsign || info->space)
    --width;

  width -= bp - buf;

  if (!info->left && info->pad == ' ')
    /* Pad with spaces on the left.  */
    while (width-- > 0)
      outchar (' ');

  /* Write the sign.  */
  if (is_neg)
    outchar ('-');
  else if (info->showsign)
    outchar ('+');
  else if (info->space)
    outchar (' ');

  if (!info->left && info->pad == '0')
    /* Pad with zeros on the left.  */
    while (width-- > 0)
      outchar ('0');

  if (fwrite (buf, bp - buf, 1, s) != 1)
    return -1;
  done += bp - buf;

  if (info->left)
    /* Pad with spaces on the right.  */
    while (width-- > 0)
      outchar (' ');

  return done;
}
