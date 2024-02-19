/*---------------------------------------------------------------------------+
 |  fpu_etc.c                                                                |
 |                                                                           |
 | Implement a few FPU instructions.                                         |
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
#include "reg_constant.h"


static void fchs()
{
  if ( NOT_EMPTY_0 )
    {
      st0_ptr->sign ^= SIGN_POS^SIGN_NEG;
      status_word &= ~SW_C1;
    }
  else
    stack_underflow();
}

static void fabs()
{
  if ( st0_tag ^ TW_Empty )
    {
      st0_ptr->sign = SIGN_POS;
      status_word &= ~SW_C1;
    }
  else
    stack_underflow();
}


static void ftst_()
{
  switch (st0_tag)
    {
    case TW_Zero:
      setcc(SW_C3);
      break;
    case TW_Valid:
      if (st0_ptr->sign == SIGN_POS)
        setcc(0);
      else
        setcc(SW_C0);
      break;
    case TW_NaN:
      setcc(SW_C2);    /* Operand is not comparable */ 
      EXCEPTION(EX_Invalid);
      break;
    case TW_Infinity:
      if (st0_ptr->sign == SIGN_POS)
        setcc(0);
      else
        setcc(SW_C3);
      /*      setcc(SW_C0|SW_C2|SW_C3); */
      EXCEPTION(EX_Invalid);
      break;
    case TW_Empty:
      setcc(SW_C0|SW_C2|SW_C3);
      EXCEPTION(EX_StackUnder);
      break;
    default:
      setcc(SW_C2);    /* Operand is not comparable */ 
      EXCEPTION(EX_INTERNAL|0x14);
      break;
    }
}

static void fxam()
{
  int c=0;
  switch (st0_tag)
    {
    case TW_Empty:
      c = SW_C3|SW_C0;
      break;
    case TW_Zero:
      c = SW_C3;
      break;
    case TW_Valid:
      if (st0_ptr->sigh & 0x80000000)
        c = SW_C2;
      else
        c = SW_C3|SW_C2;
      break;
    case TW_NaN:
      c = SW_C0;
      break;
    case TW_Infinity:
      c = SW_C2|SW_C0;
      break;
    }
  if (st0_ptr->sign == SIGN_NEG)
    c |= SW_C1;
  setcc(c);
}

static FUNC fp_etc_table[] = {
  fchs, fabs, Un_impl, Un_impl, ftst_, fxam, Un_impl, Un_impl
};

void fp_etc()
{
  (fp_etc_table[FPU_rm])();
}
