	.file	"reg_u_sub.S"
/*---------------------------------------------------------------------------+
 |  reg_u_sub.S                                                              |
 |                                                                           |
 | Core floating point subtraction routine.                                  |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void reg_u_sub(reg *arg1, reg *arg2, reg *answ)                         |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*
 |    Kernel subtraction routine reg_u_sub(reg *arg1, reg *arg2, reg *answ).
 |    Takes two valid reg f.p. numbers (TW_Valid), which are
 |    treated as unsigned numbers,
 |    and returns their difference as a TW_Valid or TW_Zero f.p.
 |    number.
 |    The first number (arg1) must be the larger.
 |    The returned number is normalized.
 |    Basic checks are performed if PARANOID is defined.
 */

#include "exception.h"
#include "fpu_asm.h"


.text
	.align 2,144
.globl _reg_u_sub
_reg_u_sub:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi	/* source 1 */
	movl	PARAM2,%edi	/* source 2 */

//	xorl	%ecx,%ecx
	movl	EXP(%esi),%ecx
	subl	EXP(%edi),%ecx	/* exp1 - exp2 */

#ifdef PARANOID
	/* source 2 is always smaller than source 1 */
//	jc	L_bugged
	js	L_bugged_1

	testl	$0x80000000,SIGH(%edi)	/* The args are assumed to be be normalized */
	je	L_bugged_2

	testl	$0x80000000,SIGH(%esi)
	je	L_bugged_2
#endif PARANOID

/*--------------------------------------+
 |	Form a register holding the     |
 |	smaller number                  |
 +--------------------------------------*/
	movl	SIGH(%edi),%eax	// register ms word
	movl	SIGL(%edi),%ebx	// register ls word

	movl	PARAM3,%edi	/* destination */
	movl	EXP(%esi),%edx
	movl	%edx,EXP(%edi)	/* Copy exponent to destination */

	xorl	%edx,%edx	// register extension

/*--------------------------------------+
 |	Shift the temporary register	|
 |      right the required number of	|
 |	places.				|
 +--------------------------------------*/
L_shift_r:
	cmpl	$32,%ecx		/* shrd only works for 0..31 bits */
	jnc	L_more_than_31

/* less than 32 bits */
	shrd	%cl,%ebx,%edx
	shrd	%cl,%eax,%ebx
	shr	%cl,%eax
	jmp	L_shift_done

L_more_than_31:
	cmpl	$64,%ecx
	jnc	L_more_than_63

	subb	$32,%cl
	shrd	%cl,%eax,%edx
	movl	%eax,%ebx
	shr	%cl,%ebx
	xorl	%eax,%eax
	jmp	L_shift_done

L_more_than_63:
	cmpl	$66,%ecx
	jnc	L_more_than_65

	subb	$64,%cl
	movl	%eax,%edx
	shr	%cl,%edx
	xorl	%ebx,%ebx
	xorl	%eax,%eax
	jmp	L_shift_done

L_more_than_65:
	/* just copy the larger reg to dest */
	movw	SIGN(%esi),%ax
	movw	%ax,SIGN(%edi)
	movl	EXP(%esi),%eax
	movl	%eax,EXP(%edi)
	movl	SIGL(%esi),%eax
	movl	%eax,SIGL(%edi)
	movl	SIGH(%esi),%eax
	movl	%eax,SIGH(%edi)
	jmp	L_exit		// Does not underflow

L_shift_done:
L_subtr:
/*------------------------------+
 |	Do the subtraction	|
 +------------------------------*/
	xorl	%ecx,%ecx
	subl	%edx,%ecx
	movl	%ecx,%edx
	movl	SIGL(%esi),%ecx
	sbbl	%ebx,%ecx
	movl	%ecx,%ebx
	movl	SIGH(%esi),%ecx
	sbbl	%eax,%ecx
	movl	%ecx,%eax

#ifdef PARANOID
	/* We can never get a borrow */
	jc	L_bugged
#endif PARANOID

/*--------------------------------------+
 |	Normalize the result		|
 +--------------------------------------*/
	testl	$0x80000000,%eax
	jnz	L_round		/* no shifting needed */

	orl	%eax,%eax
	jnz	L_shift_1	/* shift left 1 - 31 bits */

	orl	%ebx,%ebx
	jnz	L_shift_32	/* shift left 32 - 63 bits */

//	 A rare case, the only one which is non-zero if we got here
//         is:           1000000 .... 0000
//                      -0111111 .... 1111 1
//                       -------------------- 
//                       0000000 .... 0000 1 

	cmpl	$0x80000000,%edx
	jnz	L_must_be_zero

	/* Shift left 64 bits */
	subl	$64,EXP(%edi)
	movl	%edx,%eax
	jmp	L_store

L_must_be_zero:
#ifdef PARANOID
	orl	%edx,%edx
	jnz	L_bugged_3
#endif PARANOID

	/* The result is zero */
	movb	TW_Zero,TAG(%edi)
	movl	$0,EXP(%edi)		/* exponent */
	movl	$0,SIGL(%edi)
	movl	$0,SIGH(%edi)
	jmp	L_exit		// Does not underflow

L_shift_32:
	movl	%ebx,%eax
	movl	%edx,%ebx
	movl	$0,%edx
	subl	$32,EXP(%edi)	/* Can get underflow here */

/* We need to shift left by 1 - 31 bits */
L_shift_1:
	bsrl	%eax,%ecx	/* get the required shift in %ecx */
	subl	$31,%ecx
	negl	%ecx
	shld	%cl,%ebx,%eax
	shld	%cl,%edx,%ebx
	shl	%cl,%edx
	subl	%ecx,EXP(%edi)	/* Can get underflow here */

L_round:
/*------------------------------+
 |	Round the result	|
 +------------------------------*/
	cmpl	$0x80000000,%edx
	jc	L_store

	jne	L_round_up

	testb	$1,%dl
	jz	L_store

L_round_up:
	addl	$1,%ebx
	adcl	$0,%eax
	jnc	L_store

	/* We just rounded up to (1) 00 00 */
	/* This *is* possible, if the subtraction is of the
	   form (1. + x) - (x + y) where x is small and y is
	   very small. */
	incl	EXP(%edi)
	movl	$0x80000000,%eax

L_store:
/*------------------------------+
 |	Store the result	|
 +------------------------------*/
	movl	%eax,SIGH(%edi)
	movl	%ebx,SIGL(%edi)

	movb	TW_Valid,TAG(%edi)		/* Set the tags to TW_Valid */

	cmpl	EXP_UNDER,EXP(%edi)
	jle	L_underflow

L_exit:
	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret


L_underflow:
	push	%edi
	call	_arith_underflow
	pop	%ebx
	jmp	L_exit


#ifdef PARANOID
L_bugged_1:
	pushl	EX_INTERNAL|0x206
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit

L_bugged_2:
	pushl	EX_INTERNAL|0x209
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit

L_bugged_3:
	pushl	EX_INTERNAL|0x210
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit

L_bugged_4:
	pushl	EX_INTERNAL|0x211
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit

L_bugged:
	pushl	EX_INTERNAL|0x212
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit
#endif PARANOID
