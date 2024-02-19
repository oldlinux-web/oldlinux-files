	.file	"poly_div.S"
/*---------------------------------------------------------------------------+
 |  poly_div.S                                                               |
 |                                                                           |
 | A set of functions to divide 64 bit integers by fixed numbers.            |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void poly_div2(long long *x)                                            |
 |   void poly_div4(long long *x)                                            |
 |   void poly_div16(long long *x)                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "fpu_asm.h"

.text

/*---------------------------------------------------------------------------*/
	.align 2,144
.globl _poly_div2
_poly_div2:
	pushl %ebp
	movl %esp,%ebp

	movl PARAM1,%ecx
	movw (%ecx),%ax

	shrl $1,4(%ecx)
	rcrl $1,(%ecx)

	testw $1,%ax
	je poly_div2_exit

	addl $1,(%ecx)
	adcl $0,4(%ecx)
poly_div2_exit:

	leave
	ret
/*---------------------------------------------------------------------------*/
	.align 2,144
.globl _poly_div4
_poly_div4:
	pushl %ebp
	movl %esp,%ebp

	movl PARAM1,%ecx
	movw (%ecx),%ax

	movl 4(%ecx),%edx
	shll $30,%edx

	shrl $2,4(%ecx)
	shrl $2,(%ecx)

	orl %edx,(%ecx)

	testw $2,%ax
	je poly_div4_exit

	addl $1,(%ecx)
	adcl $0,4(%ecx)
poly_div4_exit:

	leave
	ret
/*---------------------------------------------------------------------------*/
	.align 2,144
.globl _poly_div16
_poly_div16:
	pushl %ebp
	movl %esp,%ebp

	movl PARAM1,%ecx
	movw (%ecx),%ax

	movl 4(%ecx),%edx
	shll $28,%edx

	shrl $4,4(%ecx)
	shrl $4,(%ecx)

	orl %edx,(%ecx)

	testw $8,%ax
	je poly_div16_exit

	addl $1,(%ecx)
	adcl $0,4(%ecx)
poly_div16_exit:

	leave
	ret
/*---------------------------------------------------------------------------*/
