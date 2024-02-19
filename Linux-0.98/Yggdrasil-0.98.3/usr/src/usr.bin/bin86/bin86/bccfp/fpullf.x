! bcc 386 floating point routines (version 2) -- Fpullf
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	fpoverflow
	.extern	fpunderflow

! pop double from stack, convert to float and store at address [ebx]

	.globl Fpullf
	.align	ALIGNMENT
Fpullf:

! Step 1: load and shift left

	mov	eax,PC_SIZE+D_LOW[esp]	! lower dword
	mov	edx,PC_SIZE+D_HIGH[esp]	! upper dword
	mov	ecx,edx		! copy upper dword into ecx ...
	and	ecx,#D_SIGN_MASK	! ... and extract sign
	and	edx,#D_EXP_MASK | D_FRAC_MASK	! extract exponent and fraction
	sub	edx,#(D_EXP_BIAS-F_EXP_BIAS) << D_EXP_SHIFT	! adjust exponent bias
	jz	underflow
	cmp	edx,#F_EXP_INFINITE << D_EXP_SHIFT	! check if exponent lies in reduced range
	jae	outofbounds
	shld	edx,eax,#D_EXP_BIT-F_EXP_BIT	! shift exponent and fraction

! Step 2: round

	test	eax,#1 << (REG_BIT-1-(D_EXP_BIT-F_EXP_BIT))	! test upper rounding bit
	jz	step3		! below middle, don't round up
	test	eax,#(1 << (REG_BIT-1-(D_EXP_BIT-F_EXP_BIT)))-1	! test other rounding bits
	jnz	roundup		! above middle, round up
	test	dl,#1		! in middle, check parity bit
	jz	step3		! already even, otherwise round up to make even

roundup:
	inc	edx		! carry 1
	test	edx,#F_FRAC_MASK	! is fraction now 0? (carry into F_EXPMASK)
	jnz	step3		! no -- carry complete
	cmp	edx,#(F_EXP_INFINITE << F_EXP_SHIFT) & ~F_NORM_MASK	! yes (very unlikely): check for overflow
				! XXX - I think these tests say 0x7e7fffff overflows
	jae	overflow

! Step 3: put it all together

step3:
	or	edx,ecx		! include sign
	mov	F_HIGH[ebx],edx	! store the result in [ebx]
	ret	#D_SIZE		! return and release double from stack

	.align	ALIGNMENT
outofbounds:
	jns	overflow	! have just compared exponent with the max
underflow:
!	call	fpunderflow	! XXX
	push	ecx		! save sign
	mov	ecx,edx
	and	ecx,#~D_FRAC_MASK	! assume fraction is below exp
	cmp	ecx,#-((D_EXP_BIAS-F_EXP_BIAS) << D_EXP_SHIFT)	! was exp = 0?
	jz	exp_x_0
	shr	ecx,#D_EXP_SHIFT
	neg	ecx
	and	edx,#D_FRAC_MASK
	or	edx,#D_NORM_MASK
	shld	edx,eax,#D_EXP_BIT-F_EXP_BIT-1
	shl	eax,#D_EXP_BIT-F_EXP_BIT-1
	push	ebx		! save to use for rounding
	sub	ebx,ebx
	shrd	ebx,eax,cl
	shrd	eax,edx,cl
	shr	edx,cl
	cmp	eax,#1 << (REG_BIT-1)
	jb	over_denorm_roundup
	ja	denorm_roundup
	test	dl,#1
	jz	over_denorm_roundup
denorm_roundup:
#if F_NORM_BIT != F_EXP_SHIFT
#include "carry into norm bit doesn't go into low exp bit"
#endif
	inc	edx
over_denorm_roundup:
	pop	ebx
	pop	ecx
	or	edx,ecx
	mov	F_HIGH[ebx],edx
	ret	#D_SIZE

	.align	ALIGNMENT
exp_x_0:			! XXX check for denormals - they underflow
	pop	ecx
	mov	dword F_HIGH[ebx],#0
	ret	#D_SIZE

	.align	ALIGNMENT
overflow:
	mov	edx,ebx		! put sign in usual reg
	call	fpoverflow
	mov	F_HIGH[ebx],dword #F_HUGE_HIGH	! XXX - should use infinity
	ret	#D_SIZE		! ... if fpoverflow does
