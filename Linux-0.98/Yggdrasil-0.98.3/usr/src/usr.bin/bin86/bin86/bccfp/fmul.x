! bcc 386 floating point routines (version 2) -- Fmul, Fmuld, Fmulf
! author: Bruce Evans

#include "fplib.h"

#define FRAME_SIZE	(3 * GENREG_SIZE + PC_SIZE)

	.extern	Fpushf
	.extern	fpoverflow
	.extern	fpunderflow
	.extern	normalize2

	.globl	Fmul
	.align	ALIGNMENT
Fmul:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ebx,FRAME_SIZE+D_SIZE+D_LOW[esp]
	mov	ecx,FRAME_SIZE+D_SIZE+D_HIGH[esp]
	call	multiplication
	mov	FRAME_SIZE+D_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret	#D_SIZE

	.globl	Fmuld
	.align	ALIGNMENT
Fmuld:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ecx,D_HIGH[ebx]
	mov	ebx,D_LOW[ebx]
	call	multiplication
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.globl	Fmulf
	.align	ALIGNMENT
Fmulf:
	push	ebp
	push	edi
	push	esi
	call	Fpushf
	pop	ebx	! yl
	pop	ecx	! xu
	mov	eax,FRAME_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_HIGH[esp]	! xu
	call	multiplication
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.align	ALIGNMENT
exp_x_0:
	mov	edx,#1 << D_EXP_SHIFT	! change exponent from 0 to 1
	jmp	x_unpacked		! XXX - check for denormal?

	.align	ALIGNMENT
exp_y_0:
	mov	ecx,#1 << D_EXP_SHIFT
	jmp	y_unpacked

	.align	ALIGNMENT
multiplication:
	mov	ebp,edx		! xu
	xor	ebp,ecx		! xu ^ yu
	and	ebp,#D_SIGN_MASK	! sign of result is difference of signs

	mov	esi,edx		! free edx for multiplications
	and	esi,#D_FRAC_MASK	! discard sign and exponent
	and	edx,#D_EXP_MASK	! exponent(x)
	jz	exp_x_0
	or	esi,#D_NORM_MASK	! normalize
x_unpacked:

	mov	edi,ecx		! this mainly for consistent naming
	and	edi,#D_FRAC_MASK
	and	ecx,#D_EXP_MASK	! exponent(y)
	jz	exp_y_0
	or	edi,#D_NORM_MASK
y_unpacked:

	add	ecx,edx		! add exponents

! exponent is in ecx, sign in ebp, operands in esi:eax and edi:ebx, edx is free
! product to go in esi:eax:ebp:ebx
! terminology: x * y = (xu,xl) * (yu,yl)
! = (xu * yu,0,0) + (0,xu * yl + xl * yu,0) + (0,0,xl * yl)

	push	ecx
	push	ebp
	mov	ecx,eax
	mul	ebx		! xl * yl
	mov	ebp,edx		! (xl * yl).u in ebp
	xchg	ebx,eax		! (xl * yl).l in ebx (final), yl in eax
	mul	esi		! xu * yl
	push	eax		! (xu * yl).l on stack
	push	edx		! (xu * yl).u on stack
	mov	eax,esi		! xu
	mul	edi		! xu * yu
	mov	esi,edx		! (xu * yu).u in esi (final except carries)
	xchg	ecx,eax		! (xu * yu).l in ecx, xl in eax
	mul	edi		! xl * yu

	add	ebp,eax		! (xl * yl).u + (xl * yu).l
	pop	eax		! (xu * yl).u
	adc	eax,edx		! (xu * yl).u + (xl * yu).u
	adc	esi,#0
	pop	edx		! (xu * yl).l
	add	ebp,edx		! ((xl * yl).u + (xl * yu).l) + (xu * yl).l
	adc	eax,ecx		! ((xu * yl).u + (xl * yu).u) + (xu * yu).l
	adc	esi,#0
	pop	edx		! sign
	pop	edi		! exponent
	sub	edi,#(D_EXP_BIAS+1-(D_EXP_BIT+2)) << D_EXP_SHIFT	! adjust
!	cmp	edi,#(D_EXP_INFINITE-1+(D_EXP_BIT+2)) << D_EXP_SHIFT
!	jae	outofbounds	! 0 will be caught as underflow by normalize2
cmp edi,#(2*D_EXP_INFINITE-(D_EXP_BIAS+1)+(D_EXP_BIT+2)) << D_EXP_SHIFT
ja underflow
	br	normalize2

	.align	ALIGNMENT
overflow:
	mov	edx,ebp		! put sign in usual reg
	call	fpoverflow
	mov	eax,ecx		! XXX - wrong reg
	ret

	.align	ALIGNMENT
underflow:
	mov	edx,ebp		! put sign in usual reg
	neg	edi
	shr	edi,#D_EXP_SHIFT
	inc	edi
	br	fpunderflow
