! bcc 386 floating point routines (version 2)
! -- Fpushi, Fpushl, Fpushs, Fpushc, Fpushuc, Fpushui, Fpushul, Fpushus
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

! Convert the short in ax to double and push on stack

	.globl	Fpushs
	.align	ALIGNMENT
Fpushs:
	cwde
	add	eax,#0		! fast 3-byte instruction to align

! Convert the int or long in eax to double and push on stack

	.globl	Fpushi
	.globl	Fpushl
! 	.align	ALIGNMENT	! don't do this until it pads with nop's
Fpushi:
Fpushl:
	test	eax,eax	
	jz	return_eax	! got 0 in eax
	mov	ebx,#(D_EXP_BIAS+D_NORM_BIT) << D_EXP_SHIFT ! set no-sign and exponent
	jns	normalize	! sign and fraction bits already set up
	mov	ebx,#D_SIGN_MASK | ((D_EXP_BIAS+D_NORM_BIT) << D_EXP_SHIFT)	! adjust sign
	neg	eax		! adjust fraction
	jmp	normalize

	.align	ALIGNMENT
ret1:
	mov	eax,#D_EXP_BIAS << D_EXP_SHIFT
	add	eax,#0		! fast 3-byte instruction to align

! 	.align	ALIGNMENT	! don't do this until it pads with nop's
return_eax:
	pop	ecx
	push	eax		! upper dword
	push	dword #0	! lower dword = 0
	jmp	ecx		! return

! Convert the (unsigned) char in al to double and push on stack

	.globl	Fpushc
	.globl	Fpushuc
	.align	ALIGNMENT
Fpushc:
Fpushuc:
	and	eax,#(1 << CHAR_BIT)-1
	add	eax,#0		! fast 3-byte instruction to align

! Convert the unsigned short in ax to double and push on stack

	.globl	Fpushus
! 	.align	ALIGNMENT	! don't do this until it pads with nop's
Fpushus:
	and	eax,#(1 << SHORT_BIT)-1
	add	eax,#0		! fast 3-byte instruction to align

! Convert the unsigned int or long in eax to double and push on stack

	.globl	Fpushui
	.globl	Fpushul
! 	.align	ALIGNMENT	! don't do this until it pads with nop's
Fpushui:
Fpushul:
	cmp	eax,#1		! this tests for both 0 and 1
	jb	return_eax	! got 0 in eax
	jz	ret1
	mov	ebx,#(D_EXP_BIAS+D_NORM_BIT) << D_EXP_SHIFT ! set no-sign and exponent

! 	.align	ALIGNMENT	! don't do this until it pads with nop's
normalize:
	sub	edx,edx		! clear lower dword of result

! Find first nonzero bit
! Don't use bsr, it is slow (const + 3n on 386, const + n on 486)

	sub	ecx,ecx		! prepare unsigned extension of cl
	test	eax,#~D_FRAC_MASK
	jnz	large
	test	eax,#0xFF << (D_NORM_BIT-8)
	jnz	middle
	shl	eax,#8
	sub	ebx,#8 << D_EXP_SHIFT
	test	eax,#0xFF << (D_NORM_BIT-8)
	jnz	middle
	shl	eax,#8
	sub	ebx,#8 << D_EXP_SHIFT
middle:
	shld	ecx,eax,#D_NORM_BIT
	mov	cl,bsr_table[ecx]
	add	ecx,#REG_BIT-D_NORM_BIT-D_NORM_BIT
	neg	ecx
	shl	eax,cl
	shl	ecx,#D_EXP_SHIFT
	sub	ebx,ecx
return:
	and	eax,#D_FRAC_MASK	! remove normalization bit
	or	eax,ebx		! include exponent (and sign) to fraction
	pop	ecx
	push	eax		! upper dword
	push	edx		! lower dword
	jmp	ecx		! return

	.align	ALIGNMENT
large:
	shld	ecx,eax,#REG_BIT-(D_NORM_BIT+8)
	jnz	huge
	shld	ecx,eax,#REG_BIT-D_NORM_BIT
	mov	cl,bsr_table[ecx]
got_shift_right:
	shrd	edx,eax,cl
	shr	eax,cl
	shl	ecx,#D_EXP_SHIFT
	add	ebx,ecx
	jmp	return

	.align	ALIGNMENT
huge:
	mov	cl,bsr_table[ecx]
	add	cl,#8
	jmp	got_shift_right

	.data
	.extern	bsr_table
