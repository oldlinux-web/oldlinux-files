/*---------------------------------------------------------------------------+
 |  fpu_trig.c                                                               |
 |                                                                           |
 | Implementation of the FPU "transcendental" functions.                     |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"
#include "status_w.h"
#include "control_w.h"
#include "reg_constant.h"	



static int trig_arg(REG *X)
{
  REG tmp, quot;
  int rv;
  long long q;
  int old_cw = control_word;

  control_word &= ~CW_RC;
  control_word |= RC_CHOP;
  
  reg_move(X, &quot);
  reg_div(&quot, &CONST_PI2, &quot);

  reg_move(&quot, &tmp);
  round_to_int(&tmp);
  if ( tmp.sigh & 0x80000000 )
    return -1;              /* |Arg| is >= 2^63 */
  tmp.exp = EXP_BIAS + 63;
  q = *(long long *)&(tmp.sigl);
  normalize(&tmp);

  reg_sub(&quot, &tmp, X);
  rv = q & 7;
  
  control_word = old_cw;
  return rv;;
}


/* Convert a long to register */
void convert_l2reg(long *arg, REG *dest)
{
  long num = *arg;
  
  if (num == 0)
    { reg_move(&CONST_Z, dest); return; }

  if (num > 0)
    dest->sign = SIGN_POS;
  else
    { num = -num; dest->sign = SIGN_NEG; }
  
  dest->sigh = num;
  dest->sigl = 0;
  dest->exp = EXP_BIAS + 31;
  dest->tag = TW_Valid;
  normalize(dest);
}


static void single_arg_error(void)
{
  switch ( st0_tag )
    {
    case TW_NaN:
      if ( !(st0_ptr->sigh & 0x40000000) )   /* Signaling ? */
	{
	  EXCEPTION(EX_Invalid);
	  /* Convert to a QNaN */
	  st0_ptr->sigh |= 0x40000000;
	}
    case TW_Empty:
      stack_underflow();
#ifdef PARANOID
    default:
      EXCEPTION(EX_INTERNAL|0x0112);
#endif PARANOID
    }
}


/*---------------------------------------------------------------------------*/

static void f2xm1()
{
  switch ( st0_tag )
    {
    case TW_Valid:
      {
	struct reg rv, tmp;
	
	if ( st0_ptr->sign == SIGN_POS )
	  {
	    /* poly_2xm1(x) requires 0 < x < 1. */
	    if ( poly_2xm1(st0_ptr, &rv) )
	      return;
	    reg_mul(&rv, st0_ptr, st0_ptr);
	    return;
	  }
	else
	  {
/* **** Should change poly_2xm1() to at least handle numbers near 0 */
	    /* poly_2xm1(x) doesn't handle negative numbers. */
	    /* So we compute (poly_2xm1(x+1)-1)/2, for -1 < x < 0 */
	    reg_add(st0_ptr, &CONST_1, &tmp);
	    poly_2xm1(&tmp, &rv);
	    reg_mul(&rv, &tmp, &tmp);
	    reg_sub(&tmp, &CONST_1, st0_ptr);
	    st0_ptr->exp--;
	  }
	if ( st0_ptr->exp <= EXP_UNDER )
	  arith_underflow(st0_ptr);
	return;
      }
    case TW_Zero:
      return;
    case TW_Infinity:
      if ( st0_ptr->sign == SIGN_NEG )
	{
	  /* -infinity gives -1 (p16-10) */
	  reg_move(&CONST_1, st0_ptr);
	  st0_ptr->sign = SIGN_NEG;
	}
      return;
    default:
      single_arg_error();
    }
}

static void fptan()
{
  REG *st_new_ptr;
  int q;
  char arg_sign = st0_ptr->sign;

  if ( STACK_OVERFLOW )
    { stack_overflow(); return; }

  switch ( st0_tag )
    {
    case TW_Valid:
      st0_ptr->sign = SIGN_POS;
      if ( (q = trig_arg(st0_ptr)) != -1 )
	{
	  if (q & 1)
	    reg_sub(&CONST_1, st0_ptr, st0_ptr);
	  
	  poly_tan(st0_ptr, st0_ptr);

	  st0_ptr->sign = (q & 1) ^ arg_sign;
	  
	  if ( st0_ptr->exp <= EXP_UNDER )
	    arith_underflow(st0_ptr);
	  
	  push();
	  reg_move(&CONST_1, st0_ptr);
	  setcc(0);
	}
      else
	{
	  /* Operand is out of range */
	  setcc(SW_C2);
	  st0_ptr->sign = arg_sign;         /* restore st(0) */
	  return;
	}
      break;
    case TW_Infinity:
      arith_invalid(st0_ptr);
      setcc(0);
      return;
    case TW_Zero:
      push();
      reg_move(&CONST_1, st0_ptr);
      setcc(0);
      break;
    default:
      single_arg_error();
      break;
    }
}


static void fxtract()
{
  REG *st_new_ptr;
  register REG *st1_ptr = st0_ptr;  /* anticipate */

  if ( STACK_OVERFLOW )
    {  stack_overflow(); return; }

  if ( !(st0_tag ^ TW_Valid) )
    {
      long e;
	  
      push();
      reg_move(st1_ptr, st0_ptr);
      st0_ptr->exp = EXP_BIAS;
      e = st1_ptr->exp - EXP_BIAS;
      convert_l2reg(&e, st1_ptr);
      return;
    }
  else if ( st0_tag == TW_Zero )
    {
      char sign = st0_ptr->sign;
      divide_by_zero(SIGN_NEG, st0_ptr);
      push();
      reg_move(&CONST_Z, st0_ptr);
      st0_ptr->sign = sign;
      return;
    }
  else if ( st0_tag == TW_Infinity )
    {
      char sign = st0_ptr->sign;
      st0_ptr->sign = SIGN_POS;
      push();
      reg_move(&CONST_INF, st0_ptr);
      st0_ptr->sign = sign;
      return;
    }
  else if ( st0_tag == TW_NaN )
    {
      if ( !(st0_ptr->sigh & 0x40000000) )   /* Signaling ? */
	{
	  EXCEPTION(EX_Invalid);
	  /* Convert to a QNaN */
	  st0_ptr->sigh |= 0x40000000;
	}
      push();
      reg_move(st1_ptr, st0_ptr);
      return;
    }
  else if ( st0_tag == TW_Empty )
    {
      /* Is this the correct behaviour? */
      if ( control_word & EX_Invalid )
	{
	  stack_underflow();
	  push();
	  stack_underflow();
	}
      else
	EXCEPTION(EX_StackUnder);
    }
#ifdef PARANOID
  else
    EXCEPTION(EX_INTERNAL | 0x119);
#endif PARANOID
}


static void fdecstp()
{
  top--;  /* st0_ptr will be fixed in math_emulate() before the next instr */
}

static void fincstp()
{
  top++;  /* st0_ptr will be fixed in math_emulate() before the next instr */
}


static void fsqrt_()
{
  if ( !(st0_tag ^ TW_Valid) )
    {
      int expon;
      
      if (st0_ptr->sign == SIGN_NEG)
	{
	  arith_invalid(st0_ptr);
	  return;
	}

      expon = st0_ptr->exp - EXP_BIAS;
      st0_ptr->exp = EXP_BIAS + (expon & 1);  /* make st(0) in  [1.0 .. 4.0) */
      
      wm_sqrt(st0_ptr);	/* Do the computation */
      
      st0_ptr->exp += expon >> 1;
      st0_ptr->tag = TW_Valid;
      st0_ptr->sign = SIGN_POS;
    }
  else if ( st0_tag == TW_Zero )
    return;
  else if ( st0_tag == TW_Infinity )
    {
      if ( st0_ptr->sign == SIGN_NEG )
	arith_invalid(st0_ptr);
      return;
    }
  else
    single_arg_error();
}


static void frndint_()
{
  if ( !(st0_tag ^ TW_Valid) )
    {
      if (st0_ptr->exp > EXP_BIAS+63)
	return;

      round_to_int(st0_ptr);  /* Fortunately, this can't overflow to 2^64 */
      st0_ptr->exp = EXP_BIAS + 63;
      normalize(st0_ptr);
      return;
    }
  else if ( (st0_tag == TW_Zero) || (st0_tag == TW_Infinity) )
    return;
  else
    single_arg_error();
}


static void fsin()
{
  if ( st0_tag == TW_Valid )
    {
      int q;
      char arg_sign = st0_ptr->sign;
      st0_ptr->sign = SIGN_POS;
      if ( (q = trig_arg(st0_ptr)) != -1 )
	{
	  REG rv;
	  
	  if (q & 1)
	    reg_sub(&CONST_1, st0_ptr, st0_ptr);
	  
	  poly_sine(st0_ptr, &rv);
	  
	  setcc(0);
	  if (q & 2)
	    rv.sign ^= SIGN_POS ^ SIGN_NEG;
	  rv.sign ^= arg_sign;
	  reg_move(&rv, st0_ptr);

	  if ( st0_ptr->exp <= EXP_UNDER )
	    arith_underflow(st0_ptr);

	  return;
	}
      else
	{
	  /* Operand is out of range */
	  setcc(SW_C2);
	  st0_ptr->sign = arg_sign;         /* restore st(0) */
	  EXCEPTION(EX_Invalid);
	  return;
	}
    }
  else if ( st0_tag == TW_Zero )
    {
      setcc(0);
      return;
    }
  else if ( st0_tag == TW_Infinity )
    {
      arith_invalid(st0_ptr);
      setcc(0);
      return;
    }
  else
    single_arg_error();
}


static int f_cos(REG *arg)
{
  if ( arg->tag == TW_Valid )
    {
      int q;
      char arg_sign = arg->sign;
      arg->sign = SIGN_POS;
      if ( (q = trig_arg(arg)) != -1 )
	{
	  REG rv;
	  
	  if ( !(q & 1) )
	    reg_sub(&CONST_1, arg, arg);
	  
	  poly_sine(arg, &rv);
	  
	  setcc(0);
	  if ((q+1) & 2)
	    rv.sign ^= SIGN_POS ^ SIGN_NEG;
	  reg_move(&rv, arg);
	  
	  return 0;
	}
      else
	{
	  /* Operand is out of range */
	  setcc(SW_C2);
	  arg->sign = arg_sign;         /* restore st(0) */
	  EXCEPTION(EX_Invalid);
	  return 1;
	}
    }
  else if ( arg->tag == TW_Zero )
    {
      reg_move(&CONST_1, arg);
      setcc(0);
      return 0;
    }
  else if ( st0_tag == TW_Infinity )
    {
      arith_invalid(st0_ptr);
      setcc(0);
      return 1;
    }
  else
    {
      single_arg_error();  /* requires arg == &st(0) */
      return 1;
    }
}


static void fcos()
{
  f_cos(st0_ptr);
}


static void fsincos()
{
  REG *st_new_ptr;
  REG arg;
	  
  if ( STACK_OVERFLOW )
    { stack_overflow(); return; }

  reg_move(st0_ptr,&arg);
  if ( !f_cos(&arg) )
    {
      fsin();
      push();
      reg_move(&arg,st0_ptr);
    }

}


/*---------------------------------------------------------------------------*/
/* The following all require two arguments: st(0) and st(1) */

/* remainder of st(0) / st(1) */
/* Assumes that st(0) and st(1) are both TW_Valid */
static void fprem_kernel(int round)
{
  REG *st1_ptr = &st(1);
  char st1_tag = st1_ptr->tag;

  if ( !((st0_tag ^ TW_Valid) | (st1_tag ^ TW_Valid)) )
    {
      REG tmp;
      int old_cw = control_word;
      int expdif = st0_ptr->exp - (st1_ptr)->exp;
      
      control_word &= ~CW_RC;
      control_word |= round;
      
      if (expdif < 64)
	{
	  /* This should be the most common case */
	  long long q;
	  int c = 0;
	  reg_div(st0_ptr, st1_ptr, &tmp);
	  
	  round_to_int(&tmp);  /* Fortunately, this can't overflow to 2^64 */
	  tmp.exp = EXP_BIAS + 63;
	  q = *(long long *)&(tmp.sigl);
	  normalize(&tmp);
	  
	  reg_mul(st1_ptr, &tmp, &tmp);
	  reg_sub(st0_ptr, &tmp, st0_ptr);
	  
	  if (q&4) c |= SW_C3;
	  if (q&2) c |= SW_C1;
	  if (q&1) c |= SW_C0;
	  
	  setcc(c);
	}
      else
	{
	  /* There is a large exponent difference ( >= 64 ) */
	  int N_exp;
	  
	  reg_div(st0_ptr, st1_ptr, &tmp);
	  /* N is 'a number between 32 and 63' (p26-113) */
	  N_exp = (tmp.exp & 31) + 32;
	  tmp.exp = EXP_BIAS + N_exp;
	  
	  round_to_int(&tmp);  /* Fortunately, this can't overflow to 2^64 */
	  tmp.exp = EXP_BIAS + 63;
	  normalize(&tmp);
	  
	  tmp.exp = EXP_BIAS + expdif - N_exp;
	  
	  reg_mul(st1_ptr, &tmp, &tmp);
	  reg_sub(st0_ptr, &tmp, st0_ptr);
	  
	  setcc(SW_C2);
	}
      control_word = old_cw;

      if ( st0_ptr->exp <= EXP_UNDER )
	arith_underflow(st0_ptr);
      return;
    }
  else if ( (st0_tag == TW_Empty) | (st1_tag == TW_Empty) )
    { stack_underflow(); return; }
  else if ( st0_tag == TW_Zero )
    {
      if ( (st1_tag == TW_Valid) || (st1_tag == TW_Infinity) )
	{ setcc(0); return; }
      if ( st1_tag == TW_Zero )
	{ arith_invalid(st0_ptr); return; }
    }

  if ( (st0_tag == TW_NaN) | (st1_tag == TW_NaN) )
    { real_2op_NaN(st0_ptr, st1_ptr, st0_ptr); return; }
  else if ( st0_tag == TW_Infinity )
    { arith_invalid(st0_ptr); return; }
#ifdef PARANOID
  else
    EXCEPTION(EX_INTERNAL | 0x118);
#endif PARANOID

}


/* ST(1) <- ST(1) * log ST;  pop ST */
static void fyl2x()
{
  REG *st1_ptr = &st(1);
  char st1_tag = st1_ptr->tag;

  if ( !((st0_tag ^ TW_Valid) | (st1_tag ^ TW_Valid)) )
    {
      if ( st0_ptr->sign == SIGN_POS )
	{
	  poly_l2(st0_ptr, st0_ptr);
	  
	  reg_mul(st0_ptr, st1_ptr, st1_ptr);
	  pop();
	  if ( st0_ptr->exp <= EXP_UNDER )
	    arith_underflow(st0_ptr);
	  else if ( st0_ptr->exp >= EXP_OVER )
	    arith_overflow(st0_ptr);
	}
      else
	{
	  /* negative	*/
	  pop();
	  arith_invalid(st0_ptr);
	}
      return;
    }

  if ( (st0_tag == TW_Empty) || (st1_tag == TW_Empty) )
    { stack_underflow(); return; }

  if ( (st0_tag == TW_NaN) || (st1_tag == TW_NaN) )
    {
      real_2op_NaN(st0_ptr, st1_ptr, st1_ptr);
      pop();
      return;
    }

  if ( (st0_tag <= TW_Zero) && (st1_tag <= TW_Zero) )
    {
      /* one of the args is zero, the other valid, or both zero */
      if ( st0_tag == TW_Zero )
	{
	  pop();
	  if ( st0_ptr->tag == TW_Zero )
	    arith_invalid(st0_ptr);
	  else
	    divide_by_zero(st1_ptr->sign ^ SIGN_NEG, st0_ptr);
	  return;
	}
      if ( st1_ptr->sign == SIGN_POS )
	{
	  /* Zero is the valid answer */
	  char sign = st0_ptr->sign;
	  if ( st0_ptr->exp < EXP_BIAS ) sign ^= SIGN_NEG;
	  pop();
	  reg_move(&CONST_Z, st0_ptr);
	  st0_ptr->sign = sign;
	  return;
	}
      pop();
      arith_invalid(st0_ptr);
      return;
    }

  /* One or both arg must be an infinity */
  if ( st0_tag == TW_Infinity )
    {
      if ( (st0_ptr->sign == SIGN_NEG) || (st1_tag == TW_Zero) )
	{ pop(); arith_invalid(st0_ptr); return; }
      else
	{
	  char sign = st1_ptr->sign;
	  pop();
	  reg_move(&CONST_INF, st0_ptr);
	  st0_ptr->sign = sign;
	  return;
	}
    }

  /* st(1) must be infinity here */
  if ( (st0_tag == TW_Valid) && (st0_ptr->sign == SIGN_POS) )
    {
      if ( st0_ptr->exp >= EXP_BIAS )
	{
	  if ( (st0_ptr->exp == EXP_BIAS) &&
	      (st0_ptr->sigh == 0x80000000) &&
	      (st0_ptr->sigl == 0) )
	    {
	      pop();
	      arith_invalid(st0_ptr);
	      return;
	    }
	  pop();
	  return;
	}
      else
	{
	  pop();
	  st0_ptr->sign ^= SIGN_NEG;
	  return;
	}
    }
  /* st(0) must be zero or negative */
  pop();
  arith_invalid(st0_ptr);
  return;
}


static void fpatan()
{
  REG *st1_ptr = &st(1);
  char st1_tag = st1_ptr->tag;

  if ( !((st0_tag ^ TW_Valid) | (st1_tag ^ TW_Valid)) )
    {
      struct reg sum;
      int quadrant = st1_ptr->sign | ((st0_ptr->sign)<<1);
      st1_ptr->sign = st0_ptr->sign = SIGN_POS;
      if (compare(st1_ptr) == COMP_A_LT_B)
	{
	  quadrant |= 4;
	  reg_div(st0_ptr, st1_ptr, &sum);
	}
      else
	reg_div(st1_ptr, st0_ptr, &sum);
      
      poly_atan(&sum);
      
      if (quadrant & 4)
	{
	  reg_sub(&CONST_PI2, &sum, &sum);
	}
      if (quadrant & 2)
	{
	  reg_sub(&CONST_PI, &sum, &sum);
	}
      if (quadrant & 1)
	sum.sign ^= SIGN_POS^SIGN_NEG;
      
      reg_move(&sum, st1_ptr);
      pop();
      if ( st0_ptr->exp <= EXP_UNDER )
	arith_underflow(st0_ptr);
      return;
    }

  if ( (st0_tag == TW_Empty) || (st1_tag == TW_Empty) )
    { stack_underflow(); return; }

  if ( (st0_tag == TW_NaN) || (st1_tag == TW_NaN) )
    {
      real_2op_NaN(st0_ptr, st1_ptr, st1_ptr);
      pop();
      return;
    }

  if ( (st0_tag == TW_Infinity) || (st1_tag == TW_Infinity) )
    {
      char sign = st1_ptr->sign;
      if ( st0_tag == TW_Infinity )
	{
	  if ( st1_tag == TW_Infinity )
	    {
	      if ( st0_ptr->sign == SIGN_POS )
		{ reg_move(&CONST_PI4, st1_ptr); }
	      else
		reg_add(&CONST_PI4, &CONST_PI2, st1_ptr);
	    }
	  else
	    {
	      if ( st0_ptr->sign == SIGN_POS )
		{ reg_move(&CONST_Z, st1_ptr); }
	      else
		reg_move(&CONST_PI, st1_ptr);
	    }
	}
      else
	{
	  reg_move(&CONST_PI2, st1_ptr);
	}
      st1_ptr->sign = sign;
      pop();
      return;
    }

 if ( st1_tag == TW_Zero )
    {
      char sign = st1_ptr->sign;
      /* st(0) must be valid or zero */
      if ( st0_ptr->sign == SIGN_POS )
	{ reg_move(&CONST_Z, st1_ptr); }
      else
	reg_move(&CONST_PI, st1_ptr);
      st1_tag = sign;
      pop();
      return;
    }
  else if ( st0_tag == TW_Zero )
    {
      char sign = st1_ptr->sign;
      /* st(1) must be TW_Valid here */
      reg_move(&CONST_PI2, st1_ptr);
      st1_tag = sign;
      pop();
      return;
    }
#ifdef PARANOID
  EXCEPTION(EX_INTERNAL | 0x220);
#endif PARANOID
}


static void fprem()
{
  fprem_kernel(RC_CHOP);
}


static void fprem1()
{
  fprem_kernel(RC_RND);
}


static void fyl2xp1()
{
  REG *st1_ptr = &st(1);
  char st1_tag = st1_ptr->tag;

  if ( !((st0_tag ^ TW_Valid) | (st1_tag ^ TW_Valid)) )
    {
      if ( poly_l2p1(st0_ptr, st0_ptr) )
	{
	  arith_invalid(st1_ptr); pop(); return;
	}
      
      reg_mul(st0_ptr, st1_ptr, st1_ptr);
      pop();
      return;
    }
  else if ( (st0_tag == TW_Empty) | (st1_tag == TW_Empty) )
    stack_underflow();
  else if ( st0_tag == TW_Zero )
    {
      if ( st1_tag <= TW_Zero )
	{
	  st1_ptr->sign ^= st0_ptr->sign;
	  reg_move(st0_ptr, st1_ptr);
	}
      else if ( st1_tag == TW_Infinity )
	{
	  arith_invalid(st1_ptr);
	}
      else if ( st1_tag == TW_NaN )
	{
	  if ( !(st1_ptr->sigh & 0x40000000) )
	    EXCEPTION(EX_Invalid);            /* signaling NaN */
	  st1_ptr->sigh |= 0x40000000;     /* QNaN */
	}
#ifdef PARANOID
      else
	{
	  EXCEPTION(EX_INTERNAL | 0x116);
	}
#endif PARANOID
      pop();
      return;
    }
  else if ( st0_tag == TW_NaN )
    {
      real_2op_NaN(st0_ptr, st1_ptr, st1_ptr);
      pop();
      return;
    }
  else if ( st0_tag == TW_Infinity )
    {
      if ( st1_tag == TW_NaN )
	real_2op_NaN(st0_ptr, st1_ptr, st1_ptr);
      else
	arith_invalid(st1_ptr);
      pop();
      return;
    }
#ifdef PARANOID
  else
    EXCEPTION(EX_INTERNAL | 0x117);
#endif PARANOID
}


static void fscale()
{
  REG *st1_ptr = &st(1);
  char st1_tag = st1_ptr->tag;

  if ( !((st0_tag ^ TW_Valid) | (st1_tag ^ TW_Valid)) )
    {
      long scale;
      REG tmp;

      /* 2^31 is far too large, 2^-31 is far too small */
      if ( st1_ptr->exp > EXP_BIAS + 30 )
	{
	  char sign;
	  EXCEPTION(EX_Overflow);
	  sign = st0_ptr->sign;
	  reg_move(&CONST_INF, st0_ptr);
	  st0_ptr->sign = sign;
	  return;
	}
      else if ( st1_ptr->exp < EXP_BIAS - 30 )
	{
	  EXCEPTION(EX_Underflow);
	  reg_move(&CONST_Z, st0_ptr);
	  return;
	}

      reg_move(st1_ptr, &tmp);
      round_to_int(&tmp);               /* This can never overflow here */
      scale = st1_ptr->sign ? -tmp.sigl : tmp.sigl;
      scale += st0_ptr->exp;
      st0_ptr->exp = scale;

      if ( scale <= EXP_UNDER )
	arith_underflow(st0_ptr);
      else if ( scale >= EXP_OVER )
	arith_overflow(st0_ptr);

      return;
    }
  else if ( st0_tag == TW_Valid )
    {
      if ( st1_tag == TW_Zero )
	{ return; }
      if ( st1_tag == TW_Infinity )
	{
	  char sign = st1_ptr->sign;
	  if ( sign == SIGN_POS )
	    { reg_move(&CONST_INF, st0_ptr); }
	  else
	      reg_move(&CONST_Z, st0_ptr);
	  st0_ptr->sign = sign;
	  return;
	}
      if ( st1_tag == TW_NaN )
	{ real_2op_NaN(st0_ptr, st1_ptr, st0_ptr); return; }
    }
  else if ( st0_tag == TW_Zero )
    {
      if ( st1_tag <= TW_Zero ) { return; }
      else if ( st1_tag == TW_Infinity )
	{
	  if ( st1_ptr->sign == SIGN_NEG )
	    return;
	  else
	    { arith_invalid(st0_ptr); return; }
	}
      else if ( st1_tag == TW_NaN )
	{ real_2op_NaN(st0_ptr, st1_ptr, st0_ptr); return; }
    }
  else if ( st0_tag == TW_Infinity )
    {
      if ( ((st1_tag == TW_Infinity) && (st1_ptr->sign == SIGN_POS))
	  || (st1_tag <= TW_Zero) )
	return;
      else if ( st1_tag == TW_Infinity )
	{ arith_invalid(st0_ptr); return; }
      else if ( st1_tag == TW_NaN )
	{ real_2op_NaN(st0_ptr, st1_ptr, st0_ptr); return; }
    }
  else if ( st0_tag == TW_NaN )
    {
      if ( st1_tag != TW_Empty )
	{ real_2op_NaN(st0_ptr, st1_ptr, st0_ptr); return; }
    }

#ifdef PARANOID
  if ( !((st0_tag == TW_Empty) || (st1_tag == TW_Empty)) )
    {
      EXCEPTION(EX_INTERNAL | 0x115);
      return;
    }
#endif

  /* At least one of st(0), st(1) must be empty */
  stack_underflow();

}


/*---------------------------------------------------------------------------*/

static FUNC trig_table_a[] = {
  f2xm1, fyl2x, fptan, fpatan, fxtract, fprem1, fdecstp, fincstp
};

void trig_a()
{
  (trig_table_a[FPU_rm])();
}


static FUNC trig_table_b[] =
  {
    fprem, fyl2xp1, fsqrt_, fsincos, frndint_, fscale, fsin, fcos
  };

void trig_b()
{
  (trig_table_b[FPU_rm])();
}
