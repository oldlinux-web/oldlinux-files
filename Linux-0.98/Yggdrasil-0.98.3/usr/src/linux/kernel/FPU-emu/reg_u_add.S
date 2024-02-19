	.file	"reg_u_add.S"
/*---------------------------------------------------------------------------+
 |  reg_u_add.S                                                              |
 |                                                                           |
 | Add two valid (TW_Valid) REG numbers, of the same sign, and put result in |
 | a destination REG.                                                        |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void reg_u_add(reg *arg1, reg *arg2, reg *answ)                         |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*
 |    Kernel addition routine reg_u_add(reg *arg1, reg *arg2, reg *answ).
 |    Takes two valid reg f.p. numbers (TW_Valid), which are
 |    treated as unsigned numbers,
 |    and returns their sum as a TW_Valid or TW_S f.p. number.
 |    The returned number is normalized.
 |    Basic checks are performed if PARANOID is defined.
 */

#include "exception.h"
#include "fpu_asm.h"

.text
	.align 2,144
.globl _reg_u_add
_reg_u_add:
	pushl	%ebp
	movl	%esp,%ebp
//	subl	$16,%esp
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi		/* source 1 */
	movl	PARAM2,%edi		/* source 2 */

	xorl	%ecx,%ecx
	movl	EXP(%esi),%ecx
	subl	EXP(%edi),%ecx		/* exp1 - exp2 */
//	jnc	L_arg1_larger
	jge	L_arg1_larger

	/* num1 is smaller */
	movl	SIGL(%esi),%ebx
	movl	SIGH(%esi),%eax

	movl	%edi,%esi
	negw	%cx
	jmp	L_accum_loaded

L_arg1_larger:
	/* num1 has larger or equal exponent */
	movl	SIGL(%edi),%ebx
	movl	SIGH(%edi),%eax

L_accum_loaded:
	movl	16(%ebp),%edi	/* destination */

	movl	EXP(%esi),%edx
	movl	%edx,EXP(%edi)	/* Copy exponent to destination */

	xorl	%edx,%edx		/* clear the extension */

#ifdef PARANOID
	testl	$0x80000000,%eax
	je	L_bugged

	testl	$0x80000000,SIGH(%esi)
	je	L_bugged
#endif PARANOID

	cmpw	$32,%cx		/* shrd only works for 0..31 bits */
	jnc	L_more_than_31

/* less than 32 bits */
	shrd	%cl,%ebx,%edx
	shrd	%cl,%eax,%ebx
	shr	%cl,%eax
	jmp	L_shift_done

L_more_than_31:
	cmpw	$64,%cx
	jnc	L_more_than_63

	subb	$32,%cl
	shrd	%cl,%eax,%edx
	shr	%cl,%eax
	movl	%eax,%ebx
	xorl	%eax,%eax
	jmp	L_shift_done

L_more_than_63:
	cmpw	$66,%cx
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
	movl	SIGL(%esi),%eax
	movl	%eax,SIGL(%edi)
	movl	SIGH(%esi),%eax
	movl	%eax,SIGH(%edi)
	jmp	L_exit		// Does not overflow

L_shift_done:
	/* Now do the addition */
	addl	SIGL(%esi),%ebx
	adcl	SIGH(%esi),%eax
	jnc	L_round_the_result

	/* Overflow, adjust the result */
	rcrl	$1,%eax
	rcrl	$1,%ebx
	rcrl	$1,%edx

	incl	EXP(%edi)
	
L_round_the_result:
	/* Round the result */
	cmpl	$0x80000000,%edx
	jc	L_no_round_up

	jne	L_do_round_up

	/* Now test for round-to-even */
	testb	$1,%ebx
	jz	L_no_round_up

L_do_round_up:
	addl	$1,%ebx
	adcl	$0,%eax
	jnc	L_no_round_up		/* Rounding done, no overflow */

	/* Overflow, adjust the result */
	rcrl	$1,%eax
	rcrl	$1,%ebx
	incl	EXP(%edi)

L_no_round_up:
	/* store the result */
	movl	%eax,SIGH(%edi)
	movl	%ebx,SIGL(%edi)

	movb	TW_Valid,TAG(%edi)		/* Set the tags to TW_Valid */
	movb	SIGN(%esi),%al
	movb	%al,SIGN(%edi)		/* Copy the sign from the first arg */

	// The number may have overflowed
	cmpl	EXP_OVER,EXP(%edi)
	jge	L_overflow

L_exit:
	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret

/* The addition resulted in a number too large to represent */
L_overflow:
	push	%edi
	call	_arith_overflow
	pop	%ebx
	jmp	L_exit


#ifdef PARANOID
/* If we ever get here then we have problems! */
L_bugged:
	pushl	EX_INTERNAL|0x201
	call	EXCEPTION
	pop	%ebx
	jmp	L_exit
#endif PARANOID
