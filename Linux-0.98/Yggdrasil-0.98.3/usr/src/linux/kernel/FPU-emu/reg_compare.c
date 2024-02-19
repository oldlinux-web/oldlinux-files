/*---------------------------------------------------------------------------+
 |  reg_compare.c                                                            |
 |                                                                           |
 | Compare two floating point registers                                      |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 | compare() is the core REG comparison function                             |
 +---------------------------------------------------------------------------*/

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"
#include "status_w.h"


int compare(REG *b)
{
  int diff;

  if ( st0_ptr->tag | b->tag )
    {
      if ( st0_ptr->tag == TW_Zero )
	{
	  if ( b->tag == TW_Zero ) return COMP_A_EQ_B;
	  if ( b->tag == TW_Valid )
	    {
	      return (b->sign == SIGN_POS) ? COMP_A_LT_B : COMP_A_GT_B ;
	    }
	}
      else if ( b->tag == TW_Zero )
	{
	  if ( st0_ptr->tag == TW_Valid )
	    {
	      return (st0_ptr->sign == SIGN_POS) ? COMP_A_GT_B : COMP_A_LT_B ;
	    }
	}

      if ( st0_ptr->tag == TW_Infinity )
	{
	  if ( (b->tag == TW_Valid) || (b->tag == TW_Zero) )
	    {
	      return (st0_ptr->sign == SIGN_POS) ? COMP_A_GT_B : COMP_A_LT_B;
	    }
	  else if ( b->tag == TW_Infinity )
	    {
	      /* The 80486 book says that infinities can be equal! */
	      return (st0_ptr->sign == b->sign) ? COMP_A_EQ_B :
		((st0_ptr->sign == SIGN_POS) ? COMP_A_GT_B : COMP_A_LT_B);
	    }
	  /* Fall through to the NaN code */
	}
      else if ( b->tag == TW_Infinity )
	{
	  if ( (st0_ptr->tag == TW_Valid) || (st0_ptr->tag == TW_Zero) )
	    {
	      return (b->sign == SIGN_POS) ? COMP_A_LT_B : COMP_A_GT_B;
	    }
	  /* Fall through to the NaN code */
	}

      /* The only possibility now should be that one of the arguments
	 is a NaN */
      if ( (st0_ptr->tag == TW_NaN) || (b->tag == TW_NaN) )
	{
	  if ( ((st0_ptr->tag == TW_NaN) && !(st0_ptr->sigh & 0x40000000))
	      || ((b->tag == TW_NaN) && !(b->sigh & 0x40000000)) )
	    /* At least one arg is a signaling NaN */
	    return COMP_NOCOMP | COMP_SNAN | COMP_NAN;
	  else
	    /* Neither is a signaling NaN */
	    return COMP_NOCOMP | COMP_NAN;
	}
      
      EXCEPTION(EX_Invalid);
    }
  
#ifdef PARANOID
  if (!(st0_ptr->sigh & 0x80000000)) EXCEPTION(EX_Invalid);
  if (!(b->sigh & 0x80000000)) EXCEPTION(EX_Invalid);
#endif PARANOID
  
  if (st0_ptr->sign != b->sign)
    return (st0_ptr->sign == SIGN_POS) ? COMP_A_GT_B : COMP_A_LT_B;
  
  diff = st0_ptr->exp - b->exp;
  if ( diff == 0 )
    {
      diff = st0_ptr->sigh - b->sigh;
      if ( diff == 0 )
	diff = st0_ptr->sigl - b->sigl;
    }

  if ( diff > 0 )
    return (st0_ptr->sign == SIGN_POS) ? COMP_A_GT_B : COMP_A_LT_B ;
  if ( diff < 0 )
    return (st0_ptr->sign == SIGN_POS) ? COMP_A_LT_B : COMP_A_GT_B ;
  return COMP_A_EQ_B;

}


void compare_st_data(void)
{
  int f;
  int c = compare(&FPU_loaded_data);

  if (c & COMP_NAN)
    {
      EXCEPTION(EX_Invalid);
      f = SW_C3 | SW_C2 | SW_C0;
    }
  else
    switch (c)
      {
      case COMP_A_LT_B:
	f = SW_C0;
	break;
      case COMP_A_EQ_B:
	f = SW_C3;
	break;
      case COMP_A_GT_B:
	f = 0;
	break;
      case COMP_NOCOMP:
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#ifdef PARANOID
      default:
	EXCEPTION(EX_INTERNAL|0x121);
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#endif PARANOID
      }
  setcc(f);
}


static void compare_st_st(int nr)
{
  int c = compare(&st(nr));
  int f;
  if (c & COMP_NAN)
    {
      EXCEPTION(EX_Invalid);
      f = SW_C3 | SW_C2 | SW_C0;
    }
  else
    switch (c)
      {
      case COMP_A_LT_B:
	f = SW_C0;
	break;
      case COMP_A_EQ_B:
	f = SW_C3;
	break;
      case COMP_A_GT_B:
	f = 0;
	break;
      case COMP_NOCOMP:
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#ifdef PARANOID
      default:
	EXCEPTION(EX_INTERNAL|0x122);
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#endif PARANOID
      }
  setcc(f);
}


static void compare_u_st_st(int nr)
{
  int f;
  int c = compare(&st(nr));
  if (c & COMP_NAN)
    {
      if (c & COMP_SNAN)       /* This is the only difference between
				  un-ordered and ordinary comparisons */
	EXCEPTION(EX_Invalid);
      f = SW_C3 | SW_C2 | SW_C0;
    }
  else
    switch (c)
      {
      case COMP_A_LT_B:
	f = SW_C0;
	break;
      case COMP_A_EQ_B:
	f = SW_C3;
	break;
      case COMP_A_GT_B:
	f = 0;
	break;
      case COMP_NOCOMP:
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#ifdef PARANOID
      default:
	EXCEPTION(EX_INTERNAL|0x123);
	f = SW_C3 | SW_C2 | SW_C0;
	break;
#endif PARANOID
      }
  setcc(f);
}

/*---------------------------------------------------------------------------*/

void fcom_st()
{
  /* fcom st(i) */
  compare_st_st(FPU_rm);
}


void fcompst()
{
  /* fcomp st(i) */
  compare_st_st(FPU_rm);
  pop();
}


void fcompp()
{
  /* fcompp */
  if (FPU_rm != 1)
    return Un_impl();
  compare_st_st(1);
  pop();
  pop();
}


void fucom_()
{
  /* fucom st(i) */
  compare_u_st_st(FPU_rm);
}


void fucomp()
{
  /* fucomp st(i) */
  compare_u_st_st(FPU_rm);
  pop();
}


void fucompp()
{
  /* fucompp */
  if (FPU_rm == 1)
    {
      compare_u_st_st(1);
      pop();
      pop();
    }
  else
    Un_impl();
}
