/*---------------------------------------------------------------------------+
 |  load_store.c                                                             |
 |                                                                           |
 | This file contains most of the code to interpret the FPU instructions     |
 | which load and store from user memory.                                    |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include <asm/segment.h>

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"
#include "status_w.h"


#define _NONE_ 0
#define _REG0_ 1   /* Need to check for not empty st(0) */
#define _REGI_ 2   /* Need to check for not empty st(0) and st(rm) */
#define _REGi_ 0   /* Uses st(rm) */
#define _PUSH_ 3   /* Need to check for space to push onto stack */
#define _null_ 4   /* Function illegal or not implemented */

static unsigned char type_table[32] = {
  _PUSH_, _PUSH_, _PUSH_, _PUSH_,
  _null_, _null_, _null_, _null_,
  _REG0_, _REG0_, _REG0_, _REG0_,
  _REG0_, _REG0_, _REG0_, _REG0_,
  _null_, _null_, _NONE_, _PUSH_,
  _NONE_, _PUSH_, _null_, _PUSH_,
  _null_, _null_, _NONE_, _REG0_,
  _NONE_, _REG0_, _NONE_, _REG0_
  };

void load_store_instr(char type)
{
  switch ( type_table[(int) (unsigned) type] )
    {
    case _NONE_:
      break;
    case _REG0_:
      break;
    case _PUSH_:
      {
	REG *st_new_ptr;
	if ( STACK_OVERFLOW )
	  { stack_overflow(); return; }
	push();
      }
      break;
    case _null_:
      return Un_impl();
#ifdef PARANOID
    default:
      return EXCEPTION(EX_INTERNAL);
#endif PARANOID
    }

switch ( type )
  {
  case 000:       /* fld m32real */
    reg_load_single();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 001:      /* fild m32int */
    reg_load_int32();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 002:      /* fld m64real */
    reg_load_double();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 003:      /* fild m16int */
    reg_load_int16();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 010:      /* fst m32real */
    reg_store_single();
    break;
  case 011:      /* fist m32int */
    reg_store_int32();
    break;
  case 012:     /* fst m64real */
    reg_store_double();
    break;
  case 013:     /* fist m16int */
    reg_store_int16();
    break;
  case 014:     /* fstp m32real */
    if ( reg_store_single() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 015:     /* fistp m32int */
    if ( reg_store_int32() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 016:     /* fstp m64real */
    if ( reg_store_double() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 017:     /* fistp m16int */
    if ( reg_store_int16() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 020:     /* fldenv  m14/28byte */
    fldenv();
    break;
  case 022:     /* frstor m94/108byte */
    frstor();
    break;
  case 023:     /* fbld m80dec */
    reg_load_bcd();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 024:     /* fldcw */
    control_word = get_fs_word((unsigned short *) FPU_data_address);
#ifdef NO_UNDERFLOW_TRAP
    if ( !(control_word & EX_Underflow) )
      {
	control_word |= EX_Underflow;
      }
#endif
    FPU_data_address = (void *)data_operand_offset; /* We want no net effect */
    FPU_entry_eip = ip_offset;               /* We want no net effect */
    break;
  case 025:      /* fld m80real */
    reg_load_extended();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 027:      /* fild m64int */
    reg_load_int64();
    reg_move(&FPU_loaded_data, st0_ptr);
    break;
  case 030:     /* fstenv  m14/28byte */
    fstenv();
    FPU_data_address = (void *)data_operand_offset; /* We want no net effect */
    FPU_entry_eip = ip_offset;               /* We want no net effect */
    break;
  case 032:      /* fsave */
    fsave();
    FPU_data_address = (void *)data_operand_offset; /* We want no net effect */
    FPU_entry_eip = ip_offset;               /* We want no net effect */
    break;
  case 033:      /* fbstp m80dec */
    if ( reg_store_bcd() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 034:      /* fstcw m16int */
    verify_area(FPU_data_address,2);
    put_fs_word(control_word, (short *) FPU_data_address);
    FPU_data_address = (void *)data_operand_offset; /* We want no net effect */
    FPU_entry_eip = ip_offset;               /* We want no net effect */
    break;
  case 035:      /* fstp m80real */
    if ( reg_store_extended() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  case 036:      /* fstsw m2byte */
    status_word &= ~SW_TOP;
    status_word |= (top&7) << SW_TOPS;
    verify_area(FPU_data_address,2);
    put_fs_word(status_word,(short *) FPU_data_address);
    FPU_data_address = (void *)data_operand_offset; /* We want no net effect */
    FPU_entry_eip = ip_offset;               /* We want no net effect */
    break;
  case 037:      /* fistp m64int */
    if ( reg_store_int64() )
      pop();  /* pop only if the number was actually stored
		 (see the 80486 manual p16-28) */
    break;
  }
}
