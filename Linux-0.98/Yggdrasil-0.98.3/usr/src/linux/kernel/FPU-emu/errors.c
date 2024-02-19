/*---------------------------------------------------------------------------+
 |  errors.c                                                                 |
 |                                                                           |
 |  The error handling functions for wm-FPU-emu                              |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include <linux/signal.h>

#include <asm/segment.h>

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"
#include "status_w.h"
#include "control_w.h"
#include "reg_constant.h"
#include "version.h"


extern unsigned char  FPU_lookahead;

/* */
#undef PRINT_MESSAGES
/* */


void Un_impl(void)
{
  unsigned char byte1 = get_fs_byte((unsigned char *) FPU_ORIG_EIP);

  printk("Unimplemented FPU Opcode at eip=%p : %02x ",
	 FPU_ORIG_EIP, byte1);

  if (FPU_modrm >= 0300)
    printk("%02x (%02x+%d)\n", FPU_modrm, FPU_modrm & 0xf8, FPU_modrm & 7);
  else
    printk("/%d\n", (FPU_modrm >> 3) & 7);

  EXCEPTION(EX_Invalid);

}




void emu_printall()
{
  int i;
  static char *tag_desc[] = { "Valid", "Zero", "ERROR", "ERROR",
                              "DeNorm", "Inf", "NaN", "Empty" };
  unsigned char byte1 = get_fs_byte((unsigned char *) FPU_ORIG_EIP);

#ifdef DEBUGGING
if ( status_word & SW_B ) printk("SW: backward compatibility (=ES)\n");
if ( status_word & SW_C3 ) printk("SW: condition bit 3\n");
if ( status_word & SW_C2 ) printk("SW: condition bit 2\n");
if ( status_word & SW_C1 ) printk("SW: condition bit 1\n");
if ( status_word & SW_C0 ) printk("SW: condition bit 0\n");
if ( status_word & SW_ES ) printk("SW: exception summary\n");
if ( status_word & SW_SF ) printk("SW: stack fault\n");
if ( status_word & SW_PE ) printk("SW: loss of precision\n");
if ( status_word & SW_UE ) printk("SW: underflow\n");
if ( status_word & SW_OE ) printk("SW: overflow\n");
if ( status_word & SW_ZE ) printk("SW: divide by zero\n");
if ( status_word & SW_DE ) printk("SW: denormalized operand\n");
if ( status_word & SW_IE ) printk("SW: invalid operation\n");
#endif DEBUGGING

  status_word = status_word & ~SW_TOP;
  status_word |= (top&7) << SW_TOPS;

  printk("At %p: %02x ", FPU_ORIG_EIP, byte1);
  if (FPU_modrm >= 0300)
    printk("%02x (%02x+%d)\n", FPU_modrm, FPU_modrm & 0xf8, FPU_modrm & 7);
  else
    printk("/%d, mod=%d rm=%d\n",
	   (FPU_modrm >> 3) & 7, (FPU_modrm >> 6) & 3, FPU_modrm & 7);

  printk(" SW: b=%d st=%d es=%d sf=%d cc=%d%d%d%d ef=%d%d%d%d%d%d\n",
	 status_word & 0x8000 ? 1 : 0,   /* busy */
	 (status_word & 0x3800) >> 11,   /* stack top pointer */
	 status_word & 0x80 ? 1 : 0,     /* Error summary status */
	 status_word & 0x40 ? 1 : 0,     /* Stack flag */
	 status_word & SW_C3?1:0, status_word & SW_C2?1:0, /* cc */
	 status_word & SW_C1?1:0, status_word & SW_C0?1:0, /* cc */
	 status_word & SW_PE?1:0, status_word & SW_UE?1:0, /* exception fl */
	 status_word & SW_OE?1:0, status_word & SW_ZE?1:0, /* exception fl */
	 status_word & SW_DE?1:0, status_word & SW_IE?1:0); /* exception fl */
  
printk(" CW: ic=%d rc=%d%d pc=%d%d iem=%d     ef=%d%d%d%d%d%d\n",
	 control_word & 0x1000 ? 1 : 0,
	 (control_word & 0x800) >> 11, (control_word & 0x400) >> 10,
	 (control_word & 0x200) >> 9, (control_word & 0x100) >> 8,
	 control_word & 0x80 ? 1 : 0,
	 control_word & SW_PE?1:0, control_word & SW_UE?1:0, /* exception */
	 control_word & SW_OE?1:0, control_word & SW_ZE?1:0, /* exception */
	 control_word & SW_DE?1:0, control_word & SW_IE?1:0); /* exception */

  for ( i = 0; i < 8; i++ )
    {
      struct reg *r = &st(i);
      switch (r->tag)
	{
	case TW_Empty:
	  continue;
	  break;
	case TW_Zero:
	  printk("st(%d)  %c .0000 0000 0000 0000         ",
		 i, r->sign ? '-' : '+');
	  break;
	case TW_Valid:
	case TW_NaN:
	case TW_Denormal:
	case TW_Infinity:
	  printk("st(%d)  %c .%04x %04x %04x %04x e%+-6d ", i,
		 r->sign ? '-' : '+',
		 (long)(r->sigh >> 16),
		 (long)(r->sigh & 0xFFFF),
		 (long)(r->sigl >> 16),
		 (long)(r->sigl & 0xFFFF),
		 r->exp - EXP_BIAS + 1);
	  break;
	default:
	  printk("Whoops! Error in errors.c      ");
	  break;
	}
      printk("%s\n", tag_desc[(int) (unsigned) r->tag]);
    }

  printk("[data] %c .%04x %04x %04x %04x e%+-6d ",
	 FPU_loaded_data.sign ? '-' : '+',
	 (long)(FPU_loaded_data.sigh >> 16),
	 (long)(FPU_loaded_data.sigh & 0xFFFF),
	 (long)(FPU_loaded_data.sigl >> 16),
	 (long)(FPU_loaded_data.sigl & 0xFFFF),
	 FPU_loaded_data.exp - EXP_BIAS + 1);
  printk("%s\n", tag_desc[(int) (unsigned) FPU_loaded_data.tag]);

}

static struct {
  int type;
  char *name;
} exception_names[] = {
  EX_StackOver, "stack overflow",
  EX_StackUnder, "stack underflow",
  EX_Precision, "loss of precision",
  EX_Underflow, "underflow",
  EX_Overflow, "overflow",
  EX_ZeroDiv, "divide by zero",
  EX_Denormal, "denormalized operand",
  EX_Invalid, "invalid operation",
  EX_INTERNAL, "INTERNAL BUG in "FPU_VERSION,
  0,0
};

/*
 EX_INTERNAL is always given with a code which indicates where the
 error was detected.

 Internal error types:
       0x14   in e14.c
       0x1nn  in a *.c file:
              0x101  in reg_add_sub.c
              0x102  in reg_mul.c
              0x103  in poly_sin.c
              0x104  in poly_tan.c
              0x105  in reg_mul.c
	      0x106  in reg_mov.c
              0x107  in fpu_trig.c
	      0x108  in reg_compare.c
	      0x109  in reg_compare.c
	      0x110  in reg_add_sub.c
	      0x111  in interface.c
	      0x112  in fpu_trig.c
	      0x113  in reg_add_sub.c
	      0x114  in reg_ld_str.c
	      0x115  in fpu_trig.c
	      0x116  in fpu_trig.c
	      0x117  in fpu_trig.c
	      0x118  in fpu_trig.c
	      0x119  in fpu_trig.c
	      0x120  in poly_atan.c
	      0x121  in reg_compare.c
	      0x122  in reg_compare.c
	      0x123  in reg_compare.c
       0x2nn  in an *.s file:
              0x201  in reg_u_add.S
              0x202  in reg_u_div.S
              0x203  in reg_u_div.S
              0x204  in reg_u_div.S
              0x205  in reg_u_mul.S
              0x206  in reg_u_sub.S
              0x207  in wm_sqrt.S
	      0x208  in reg_div.S
              0x209  in reg_u_sub.S
              0x210  in reg_u_sub.S
              0x211  in reg_u_sub.S
              0x212  in reg_u_sub.S
 */

void exception(int n)
{
  int i, int_type;
  
  if ( n & EX_INTERNAL )
    {
      int_type = n - EX_INTERNAL;
      n = EX_INTERNAL;
      /* Set lots of exception bits! */
      status_word |= (0x3f | EX_ErrorSummary | FPU_BUSY);
    }
  else
    {
      /* Set the corresponding exception bit */
      status_word |= (n | EX_ErrorSummary | FPU_BUSY);
      if (n == EX_StackUnder)    /* Stack underflow */
	/* This bit distinguishes over- from underflow */
	status_word &= ~SW_C1;
    }

  if ( (~control_word & n & CW_EXM) || (n == EX_INTERNAL) )
    {
#ifdef PRINT_MESSAGES
      /* My message from the sponsor */
      printk(FPU_VERSION" "__DATE__" (C) W. Metzenthen.\r\n");
#endif PRINT_MESSAGES
      
      /* Get a name string for error reporting */
      for (i=0; exception_names[i].type; i++)
	if (exception_names[i].type == n)
	  break;
      
      if (exception_names[i].type)
	{
#ifdef PRINT_MESSAGES
	  printk("FP Exception: %s!\n", exception_names[i].name);
#endif PRINT_MESSAGES
	}
      else
	printk("FP emulator: Unknown Exception: 0x%04x!\n", n);
      
      if ( n == EX_INTERNAL )
	{
	  printk("FP emulator: Internal error type 0x%04x\n", int_type);
	  emu_printall();
	}
#ifdef PRINT_MESSAGES
      else
	emu_printall();
#endif PRINT_MESSAGES

      send_sig(SIGFPE, current, 1);
    }

#ifdef __DEBUG__
  math_abort(FPU_info,SIGFPE);
#endif __DEBUG__

  /* Cause the look-ahead mechanism to terminate */
  FPU_lookahead = 0;
}


/* Real operation attempted on two operands, one a NaN */
void real_2op_NaN(REG *a, REG *b, REG *dest)
{
  REG *x;
  
  x = a;
  if (a->tag == TW_NaN)
    {
      if (b->tag == TW_NaN)
	{
	  /* find the "larger" */
	  if ( *(long long *)&(a->sigl) < *(long long *)&(b->sigl) )
	    x = b;
	}
      /* else return the quiet version of the NaN in a */
    }
  else if (b->tag == TW_NaN)
    {
      x = b;
    }
#ifdef PARANOID
  else
    {
      EXCEPTION(EX_INTERNAL|0x113);
      x = &CONST_QNaN;
    }
#endif PARANOID
  
  if ( control_word & EX_Invalid )
    {
      /* The masked response */
      reg_move(x, dest);
      /* ensure a Quiet NaN */
      dest->sigh |= 0x40000000;
    }

  EXCEPTION(EX_Invalid);
  
  return;
}

/* Invalid arith operation on valid registers */
void arith_invalid(REG *dest)
{
  
  if ( control_word & EX_Invalid )
    {
      /* The masked response */
      reg_move(&CONST_QNaN, dest);
    }

  EXCEPTION(EX_Invalid);
  
  return;

}


/* Divide a finite number by zero */
void divide_by_zero(int sign, REG *dest)
{

  if ( control_word & EX_ZeroDiv )
    {
      /* The masked response */
      reg_move(&CONST_INF, dest);
      dest->sign = (unsigned char)sign;
    }
 
  EXCEPTION(EX_ZeroDiv);

  return;

}


void arith_overflow(REG *dest)
{

  if ( control_word & EX_Overflow )
    {
      char sign;
      /* The masked response */
      sign = dest->sign;
      reg_move(&CONST_INF, dest);
      dest->sign = sign;
    }
  else
    {
      /* Subtract the magic number from the exponent */
      dest->exp -= (3 * (1 << 13));
    }

  EXCEPTION(EX_Overflow);

  return;

}


void arith_underflow(REG *dest)
{

  if ( control_word & EX_Underflow )
    {
      /* The masked response */
      if ( dest->exp <= EXP_UNDER - 63 )
	reg_move(&CONST_Z, dest);
    }
  else
    {
      /* Add the magic number to the exponent */
      dest->exp += (3 * (1 << 13));
    }

  EXCEPTION(EX_Underflow);

  return;
}


void stack_overflow(void)
{

 if ( control_word & EX_Invalid )
    {
      /* The masked response */
      top--;
      reg_move(&CONST_QNaN, st0_ptr = &st(0));
    }

  EXCEPTION(EX_StackOver);

  return;

}


void stack_underflow(void)
{

 if ( control_word & EX_Invalid )
    {
      /* The masked response */
      reg_move(&CONST_QNaN, st0_ptr);
    }

  EXCEPTION(EX_StackUnder);

  return;

}

