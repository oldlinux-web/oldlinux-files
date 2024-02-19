/*---------------------------------------------------------------------------+
 |  fpu_aux.c                                                                |
 |                                                                           |
 | Code to implement some of the FPU auxiliary instructions.                 |
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

extern struct info *FPU_info;

#define EAX_REG ((long)(FPU_info->___eax))



static void fclex()
{
  status_word &= ~(SW_B|SW_ES|SW_SF|SW_PE|SW_UE|SW_OE|SW_ZE|SW_DE|SW_IE);
  FPU_entry_eip = ip_offset;               /* We want no net effect */
}

/* Needs to be externally visible */
void finit()
{
  int r;
  control_word = 0x037e;
  status_word = 0;
  top = 0;
  for (r = 0; r < 8; r++)
    {
      regs[r].sign = 0;
      regs[r].tag = TW_Empty;
      regs[r].exp = 0;
      regs[r].sigh = 0;
      regs[r].sigl = 0;
    }
  FPU_entry_eip = ip_offset;               /* We want no net effect */
}

static FUNC finit_table[] = {
  Un_impl, Un_impl, fclex, finit, Un_impl, Un_impl, Un_impl, Un_impl
};

void finit_()
{
  (finit_table[FPU_rm])();
}


static void fstsw_ax()
{

  status_word &= ~SW_TOP;
  status_word |= (top&7) << SW_TOPS;

  *(short *) &EAX_REG = status_word;

}

static FUNC fstsw_table[] = {
  fstsw_ax, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl
};

void fstsw_()
{
  (fstsw_table[FPU_rm])();
}



static void fnop()
{
}

FUNC fp_nop_table[] = {
  fnop, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl, Un_impl
};

void fp_nop()
{
  (fp_nop_table[FPU_rm])();
}


void fld_i_()
{
  REG *st_new_ptr;

  if ( STACK_OVERFLOW )
    { stack_overflow(); return; }

  /* fld st(i) */
  if ( NOT_EMPTY(FPU_rm) )
    { reg_move(&st(FPU_rm), st_new_ptr); push(); }
  else
    {
      if ( control_word & EX_Invalid )
	{
	  /* The masked response */
	  push();
	  stack_underflow();
	}
      else
	EXCEPTION(EX_StackUnder);
    }

}


void fxch_i()
{
  /* fxch st(i) */
  REG t;
  register REG *sti_ptr = &st(FPU_rm);
  reg_move(st0_ptr, &t);
  reg_move(sti_ptr, st0_ptr);
  reg_move(&t, sti_ptr);
}


void ffree_()
{
  /* ffree st(i) */
  st(FPU_rm).tag = TW_Empty;
}


void fst_i_()
{
  /* fst st(i) */
  reg_move(st0_ptr, &st(FPU_rm));
}


void fstp_i()
{
  /* fstp st(i) */
  reg_move(st0_ptr, &st(FPU_rm));
  pop();
}

