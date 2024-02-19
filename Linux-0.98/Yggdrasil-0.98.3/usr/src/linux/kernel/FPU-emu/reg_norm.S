/*---------------------------------------------------------------------------+
 |  reg_norm.S                                                               |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Normalize the value in a REG.                                             |
 |                                                                           |
 | Call from C as:                                                           |
 |   void normalize(REG *n)                                                  |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "fpu_asm.h"


.text

	.align 2,144
.globl _normalize

_normalize:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ebx

	movl	PARAM1,%ebx

	movl	SIGH(%ebx),%edx
	movl	SIGL(%ebx),%eax

	orl	%edx,%edx	// ms bits
	js	L_done		// Already normalized
	jnz	L_shift_1	// Shift left 1 - 31 bits

	orl	%eax,%eax
	jz	L_zero		// The contents are zero

L_shift_32:
	movl	%eax,%edx
	xorl	%eax,%eax
	subl	$32,EXP(%ebx)	// This can cause an underflow

/* We need to shift left by 1 - 31 bits */
L_shift_1:
	bsrl	%edx,%ecx	/* get the required shift in %ecx */
	subl	$31,%ecx
	negl	%ecx
	shld	%cl,%eax,%edx
	shl	%cl,%eax
	subl	%ecx,EXP(%ebx)	// This can cause an underflow

	movl	%edx,SIGH(%ebx)
	movl	%eax,SIGL(%ebx)

L_done:
	cmpl	EXP_OVER,EXP(%ebx)
	jge	L_overflow

	cmpl	EXP_UNDER,EXP(%ebx)
	jle	L_underflow

L_exit:
	popl	%ebx
	leave
	ret


L_zero:
	movl	EXP_UNDER,EXP(%ebx)
	movb	TW_Zero,TAG(%ebx)
	jmp	L_exit

L_underflow:
	push	%ebx
	call	_arith_underflow
	pop	%ebx
	jmp	L_exit

L_overflow:
	push	%ebx
	call	_arith_overflow
	pop	%ebx
	jmp	L_exit
