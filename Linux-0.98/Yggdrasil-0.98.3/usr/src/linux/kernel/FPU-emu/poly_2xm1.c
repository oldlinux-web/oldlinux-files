/*---------------------------------------------------------------------------+
 |  poly_2xm1.c                                                              |
 |                                                                           |
 | Function to compute 2^x-1 by a polynomial approximation.                  |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "reg_constant.h"
#include "fpu_emu.h"



#define	HIPOWER	13
static unsigned short	lterms[HIPOWER][4] =
	{
	{ 0x79b5, 0xd1cf, 0x17f7, 0xb172 },
	{ 0x1b56, 0x058b, 0x7bff, 0x3d7f },
	{ 0x8bb0, 0x8250, 0x846b, 0x0e35 },
	{ 0xbc65, 0xf747, 0x556d, 0x0276 },
	{ 0x17cb, 0x9e39, 0x61ff, 0x0057 },
	{ 0xe018, 0x9776, 0x1848, 0x000a },
	{ 0x66f2, 0xff30, 0xffe5, 0x0000 },
	{ 0x682f, 0xffb6, 0x162b, 0x0000 },
	{ 0xb7ca, 0x2956, 0x01b5, 0x0000 },
	{ 0xcd3e, 0x4817, 0x001e, 0x0000 },
	{ 0xb7e2, 0xecbe, 0x0001, 0x0000 },
	{ 0x0ed5, 0x1a27, 0x0000, 0x0000 },
	{ 0x101d, 0x0222, 0x0000, 0x0000 },
	};


/*--- poly_2xm1() -----------------------------------------------------------+
 |                                                                           |
 +---------------------------------------------------------------------------*/
int	poly_2xm1(REG *arg, REG *result)
{
  short		exponent;
  long long     Xll;
  REG           accum;


  exponent = arg->exp - EXP_BIAS;

  if ( arg->tag == TW_Zero )
    {
      /* Return 0.0 */
      reg_move(&CONST_Z, result);
      return 0;
    }

  if ( exponent >= 0 )	/* Can't hack a number >= 1.0 */
    {
      arith_invalid(result);
      return 1;
    }

  if ( arg->sign != SIGN_POS )	/* Can't hack a number < 0.0 */
    {
      arith_invalid(result);
      return 1;
    }
  
  if ( exponent < -64 )
    {
      reg_move(&CONST_LN2, result);
      return 0;
    }

  *(unsigned *)&Xll = arg->sigl;
  *(((unsigned *)&Xll)+1) = arg->sigh;
  if ( exponent < -1 )
    {
      /* shift the argument right by the required places */
      if ( shrx(&Xll, -1-exponent) >= 0x80000000U )
	Xll++;	/* round up */
    }

  *(short *)&(accum.sign) = 0; /* will be a valid positive nr with expon = 0 */
  accum.exp = 0;

  /* Do the basic fixed point polynomial evaluation */
  polynomial((unsigned *)&accum.sigl, (unsigned *)&Xll, lterms, HIPOWER-1);

  /* Convert to 64 bit signed-compatible */
  accum.exp += EXP_BIAS - 1;

  reg_move(&accum, result);

  normalize(result);

  return 0;

}
