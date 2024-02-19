	.file	"reg_u_mul.S"
/*---------------------------------------------------------------------------+
 |  reg_u_mul.S                                                              |
 |                                                                           |
 | Core multiplication routine                                               |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 |   Basic multiplication routine.                                           |
 |   Does not check the resulting exponent for overflow/underflow            |
 |                                                                           |
 |   Internal working is at approx 96 bits.                                  |
 |   Result is rounded to nearest 64 bits, using "nearest or even".          |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "fpu_asm.h"



.data
	.align 2,0
accum_1:
	.long	0


.text
	.align 2,144

.globl _reg_u_mul
_reg_u_mul:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi
	movl	PARAM2,%ecx

#ifdef PARANOID
	testl	$0x80000000,SIGH(%esi)
	jz	xL_bugged
	testl	$0x80000000,SIGH(%ecx)
	jz	xL_bugged
#endif PARANOID

	xorl	%edi,%edi
	xorl	%ebx,%ebx

	movl	SIGL(%esi),%eax
	mull	SIGL(%ecx)
//	movl	%eax,accum_0
	movl	%edx,accum_1

	movl	SIGL(%esi),%eax
	mull	SIGH(%ecx)
	addl	%eax,accum_1
	adcl	%edx,%ebx
//	adcl	$0,%edi	// overflow here is not possible

	movl	SIGH(%esi),%eax
	mull	SIGL(%ecx)
	addl	%eax,accum_1
	adcl	%edx,%ebx
	adcl	$0,%edi

	movl	SIGH(%esi),%eax
	mull	SIGH(%ecx)
	addl	%eax,%ebx
	adcl	%edx,%edi

	movl	EXP(%esi),%eax	/* Compute the exponent */
	addl	EXP(%ecx),%eax
//  Have now finished with the sources
	movl	PARAM3,%esi	// Point to the destination
	movl	%eax,EXP(%esi)

//  Now make sure that the result is normalized
	testl	$0x80000000,%edi
	jnz	L20

	/* Normalize by shifting left one bit */
//	shll	$1,accum_0	// If using this, change next to rcll
	shll	$1,accum_1
	rcll	$1,%ebx
	rcll	$1,%edi
	decl	EXP(%esi)

L20:
	/* Do the rounding */
	cmpl	$0x80000000,accum_1
	jc	L40

	jne	L30

	/* 0x80000000, round up only if previous bit is 1 */
	testl	$1,%ebx
	jz L40

L30:
	addl	$1,%ebx
	adcl	$0,%edi

	/* An overflow can occur here (rare!) */
	jc	xL_overflow_adjust

L40:
	/* Copy the result to the destination register */
	movl	%ebx,SIGL(%esi)
	movl	%edi,SIGH(%esi)

xL_exit:
	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret


xL_overflow_adjust:
	rcrl	%edi
	incl	EXP(%esi)
	jmp	L40

#ifdef PARANOID
xL_bugged:
	pushl	EX_INTERNAL|0x205
	call	EXCEPTION
	pop	%ebx
	jmp	xL_exit
#endif PARANOID
