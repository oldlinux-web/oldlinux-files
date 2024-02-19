/*---------------------------------------------------------------------------+
 |  reg_mul.c                                                                |
 |                                                                           |
 | Multiply one REG by another and put the result in a destination REG.      |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 | The destination may be any REG, including one of the source REGs.         |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "reg_constant.h"
#include "fpu_emu.h"


/* This routine must be called with non-empty registers */
void reg_mul(REG *a, REG *b, REG *dest)
{
  if (!(a->tag | b->tag))
    {
      /* This should be the most common case */
      reg_u_mul(a, b, dest);
      dest->exp += - EXP_BIAS + 1;
      dest->sign = (a->sign ^ b->sign);
      dest->tag = TW_Valid;
      if ( dest->exp <= EXP_UNDER )
	{ arith_underflow(st0_ptr); }
      else if ( dest->exp >= EXP_OVER )
	{ arith_overflow(st0_ptr); }
      return;
    }
  else if ((a->tag <= TW_Zero) && (b->tag <= TW_Zero))
    {
      /* Must have either both arguments == zero, or
	 one valid and the other zero.
	 The result is therefore zero. */
      reg_move(&CONST_Z, dest);
    }
  else if ((a->tag <= TW_Denormal) && (b->tag <= TW_Denormal))
    {
      /* One or both arguments are de-normalized */
      /* Internal de-normalized numbers are not supported yet */
      EXCEPTION(EX_INTERNAL|0x105);
      reg_move(&CONST_Z, dest);
    }
  else
    {
      /* Must have infinities, NaNs, etc */
      if ( (a->tag == TW_NaN) || (b->tag == TW_NaN) )
	{ real_2op_NaN(a, b, dest); return; }
      else if (a->tag == TW_Infinity)
	{
	  if (b->tag == TW_Zero)
	    { arith_invalid(dest); return; }
	  else
	    {
	      reg_move(a, dest);
	      dest->sign = a->sign == b->sign ? SIGN_POS : SIGN_NEG;
	    }
	}
      else if (b->tag == TW_Infinity)
	{
	  if (a->tag == TW_Zero)
	    { arith_invalid(dest); return; }
	  else
	    {
	      reg_move(b, dest);
	      dest->sign = a->sign == b->sign ? SIGN_POS : SIGN_NEG;
	    }
	}
#ifdef PARANOID
      else
	{
	  EXCEPTION(EX_INTERNAL|0x102);
	}
#endif PARANOID
      dest->sign = (a->sign ^ b->sign);
    }
}
