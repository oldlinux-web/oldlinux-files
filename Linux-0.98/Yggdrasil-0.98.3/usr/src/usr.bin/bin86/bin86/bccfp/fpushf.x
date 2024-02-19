! bcc 386 floating point routines (version 2) -- Fpushf, Fnegf
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	fpdenormal

! Load float at [ebx], convert to double and push on stack

	.globl	Fpushf
	.align	ALIGNMENT
Fpushf:
	mov	edx,F_HIGH[ebx]
into_Fpushf:
	test	edx,#F_EXP_MASK	! is exponent 0?
	jz	exp_x_0

	mov	ecx,edx		! extract sign
	and	ecx,#F_SIGN_MASK

	and	edx,#F_EXP_MASK | F_FRAC_MASK	! extract exponent and fraction
	sub	eax,eax		! clear lower dword
	shrd	eax,edx,#D_EXP_BIT-F_EXP_BIT	! shift exponent and fraction to new position
	shr	edx,#D_EXP_BIT-F_EXP_BIT

	add	edx,#(D_EXP_BIAS-F_EXP_BIAS) << D_EXP_SHIFT	! adjust exponent bias
	or	edx,ecx		! include sign

	pop	ecx
	push	edx		! upper dword
	push	eax		! lower dword
	jmp	ecx		! return

	.align	ALIGNMENT
exp_x_0:
	mov	eax,edx
	and	eax,#F_FRAC_MASK
	jnz	x_denorm
	pop	ecx
	push	eax		! upper dword = 0
	push	eax		! lower dword = 0
	jmp	ecx		! return

	.align	ALIGNMENT
x_denorm:
	call	fpdenormal
	bsr	ecx,eax		! zzzz
	neg	ecx
	add	ecx,#F_NORM_BIT
	shl	eax,cl
	and	eax,#F_FRAC_MASK
	neg	ecx
	add	ecx,#D_EXP_BIAS-F_EXP_BIAS+1
	shl	ecx,#D_EXP_SHIFT
	and	edx,#F_SIGN_MASK	! assumed same as D_SIGN_MASK
	or	edx,ecx
	sub	ecx,ecx
	shrd	ecx,eax,#D_EXP_BIT-F_EXP_BIT
	shr	eax,#D_EXP_BIT-F_EXP_BIT
	or	edx,eax

	pop	eax
	push	edx		! upper dword
	push	ecx		! lower dword
	jmp	eax		! return

! Fnegf: as Fpushf, but negate double before pushing onto stack

	.globl	Fnegf
	.align	ALIGNMENT
Fnegf:
	mov	edx,F_HIGH[ebx]
	xor	edx,#F_SIGN_MASK	! toggle sign
	jmp	into_Fpushf	! join Fpushf
