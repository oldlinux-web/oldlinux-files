/*---------------------------------------------------------------------------+
 |  get_address.c                                                            |
 |                                                                           |
 | Get the effective address from an FPU instruction.                        |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/


#include <linux/stddef.h>
#include <linux/math_emu.h>

#include <asm/segment.h>

#include "fpu_system.h"
#include "exception.h"
#include "fpu_emu.h"



static int reg_offset[] = {
	offsetof(struct info,___eax),
	offsetof(struct info,___ecx),
	offsetof(struct info,___edx),
	offsetof(struct info,___ebx),
	offsetof(struct info,___esp),
	offsetof(struct info,___ebp),
	offsetof(struct info,___esi),
	offsetof(struct info,___edi)
};

#define REG_(x) (*(long *)(reg_offset[(x)]+(char *) FPU_info))


void  *FPU_data_address;


/* Decode the SIB byte. This function assumes mod != 0 */
static void *sib(int mod)
{
  unsigned char ss,index,base;
  long offset;
  
  base = get_fs_byte((char *) FPU_EIP);   /* The SIB byte */
  FPU_EIP++;
  ss = base >> 6;
  index = (base >> 3) & 7;
  base &= 7;

  if ((mod == 0) && (base == 5))
    offset = 0;              /* No base register */
  else
    offset = REG_(base);

  if (index == 4)
    {
      /* No index register */
      /* A non-zero ss is illegal */
      if ( ss )
	EXCEPTION(EX_Invalid);
    }
  else
    {
      offset += (REG_(index)) << ss;
    }

  if (mod == 1)
    {
      /* 8 bit signed displacement */
      offset += (signed char) get_fs_byte((char *) FPU_EIP);
      FPU_EIP++;
    }
  else if (mod == 2 || base == 5) /* The second condition also has mod==0 */
    {
      /* 32 bit displacment */
      offset += (signed) get_fs_long((unsigned long *) FPU_EIP);
      FPU_EIP += 4;
    }

  return (void *) offset;
}


/*
       MOD R/M byte:  MOD == 3 has a special use for the FPU
                      SIB byte used iff R/M = 100b

       7   6   5   4   3   2   1   0
       .....   .........   .........
        MOD    OPCODE(2)     R/M


       SIB byte

       7   6   5   4   3   2   1   0
       .....   .........   .........
        SS      INDEX        BASE

*/

void get_address(void)
{
  unsigned char mod;
  long *cpu_reg_ptr;
  int offset;
  
  mod = (FPU_modrm >> 6) & 3;

  if (FPU_rm == 4 && mod != 3)
    {
      FPU_data_address = sib(mod);
      return;
    }

  cpu_reg_ptr = & REG_(FPU_rm);
  switch (mod)
    {
    case 0:
      if (FPU_rm == 5)
	{
	  /* Special case: disp32 */
	  offset = get_fs_long((unsigned long *) FPU_EIP);
	  FPU_EIP += 4;
	  FPU_data_address = (void *) offset;
	  return;
	}
      else
	{
	  FPU_data_address = (void *)*cpu_reg_ptr;  /* Just return the contents
						   of the cpu register */
	  return;
	}
    case 1:
      /* 8 bit signed displacement */
      offset = (signed char) get_fs_byte((char *) FPU_EIP);
      FPU_EIP++;
      break;
    case 2:
      /* 32 bit displacement */
      offset = (signed) get_fs_long((unsigned long *) FPU_EIP);
      FPU_EIP += 4;
      break;
    case 3:
      /* Not legal for the FPU */
      EXCEPTION(EX_Invalid);
    }

  FPU_data_address = offset + (char *)*cpu_reg_ptr;
}
