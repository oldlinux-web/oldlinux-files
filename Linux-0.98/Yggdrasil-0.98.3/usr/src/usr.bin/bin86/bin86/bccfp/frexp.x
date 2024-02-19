! bcc 386 floating point routines (version 2) -- _frexp
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	fpdenormal

! void frexp(double value, int *exponent);
! splits a double into exponent and fraction (where 0.5 <= fraction < 1.0)

	.globl	_frexp
	.align	ALIGNMENT
_frexp:
push ebx
#undef PC_SIZE
#define PC_SIZE 8
	mov	eax,PC_SIZE+D_LOW[esp]	! lower dword of x
	mov	ebx,PC_SIZE+D_HIGH[esp]	! upper dword of x
	mov	edx,PC_SIZE+D_SIZE[esp]	! exponent pointer
	mov	ecx,ebx		! extract exponent here
	and	ecx,#D_EXP_MASK
	jz	exp_x_0

	shr	ecx,#D_EXP_SHIFT	! exponent + bias
got_x:
	sub	ecx,#D_EXP_BIAS-1	! D_EXP_BIAS is for 1.x form, we want 0.1x form
	mov	[edx],ecx	! return exponent
	and	ebx,#D_SIGN_MASK | D_FRAC_MASK	! extract sign and fraction
	or	ebx,#(D_EXP_BIAS-1) << D_EXP_SHIFT	! set new exponent for 0.1x
mov edx,ebx
pop ebx
	ret

	.align	ALIGNMENT
exp_x_0:
	test	ebx,#D_FRAC_MASK
	jnz	xu_denorm
	test	eax,eax
	jnz	xl_denorm
	mov	[edx],ecx	! return zero exponent
	mov	ebx,ecx		! guard against -0 (may not be necessary)
mov edx,ebx
pop ebx
	ret

	.align	ALIGNMENT
xl_denorm:
	call	fpdenormal
	bsr	ecx,eax		! zzzz
	neg	ecx
	add	ecx,#REG_BIT-1
	shl	eax,cl
	shld	ebx,eax,#D_NORM_BIT+1
	shl	eax,#D_NORM_BIT+1
	sub	ecx,#D_NORM_BIT+1
	jmp	got_x

	.align	ALIGNMENT
xu_denorm:
	call	fpdenormal
	bsr	ecx,ebx
	neg	ecx
	add	ecx,#D_NORM_BIT
	shld	ebx,eax,cl
	shl	eax,cl
	jmp	got_x
