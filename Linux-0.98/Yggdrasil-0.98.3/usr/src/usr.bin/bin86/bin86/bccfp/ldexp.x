! bcc 386 floating point routines (version 2) -- _ldexp
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	fpoverflow
	.extern	fpunderflow

! void ldexp(double value, int exponent);
! returns value * (2 ** exponent)

	.globl	_ldexp
	.align	ALIGNMENT
_ldexp:
push ebx
#undef PC_SIZE
#define PC_SIZE 8
	mov	ebx,PC_SIZE+D_HIGH[esp]	! upper dword of x
	mov	ecx,PC_SIZE+D_SIZE[esp]	! exponent arg
	mov	eax,ebx		! extract exponent (of x) here
	and	eax,#D_EXP_MASK
!	jz	exp_y_0		! may need check for preposterous exponent arg too

	shr	eax,#D_EXP_SHIFT	! shift to low bits just for testing
	jz	underflow	! denormal?
	add	eax,ecx		! test-add the exponents
	jz	underflow	! XXX probably need to fiddle norm bit
	cmp	eax,#D_EXP_INFINITE	! check if still within range
	jae	outofbounds	! the unsigned compare catches all overflow cases
				! because the exponent of x is non-negative

	shl	ecx,#D_EXP_SHIFT	! shift exponent arg bits into final position ...
	add	ebx,ecx		! ... safe to add it to exponent of x now
	mov	eax,PC_SIZE+D_LOW[esp]	! lower dword of x
mov edx,ebx
pop ebx
	ret


	.align	ALIGNMENT
outofbounds:
	test	ecx,ecx		! overflow or underflow?
	jns	overflow
underflow:
	mov	edx,ebx		! put sign in usual reg
	push	edi
	push	esi
	mov	edi,eax		! put exponent in usual reg
	mov	eax,2*GENREG_SIZE+PC_SIZE+D_LOW[esp]
				! put lower dword of x in usual reg
	mov	esi,ebx		! put upper dword of x in usual reg
	and	esi,#D_EXP_MASK | D_FRAC_MASK
	test	esi,#D_EXP_MASK
	jz	foo
	and	esi,#D_FRAC_MASK
	or	esi,#D_NORM_MASK
foo:
	neg	edi
!	inc	edi		! XXX ?
	call	fpunderflow
	pop	esi
	pop	edi
	mov	ebx,edx		! XXX = wrong reg
pop ebx
	ret

	.align	ALIGNMENT
overflow:
	mov	edx,ebx		! put sign in usual reg
	call	fpoverflow
	mov	eax,ecx		! XXX = wrong reg
	mov	ebx,edx		! XXX = wrong reg
pop ebx
	ret
