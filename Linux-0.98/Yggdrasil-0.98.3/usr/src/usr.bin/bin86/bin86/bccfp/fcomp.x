! bcc 386 floating point routines (version 2) -- Fcomp, Fcompd, Fcompf
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	Fpushf

! Pop 2 doubles from stack and compare them, return result in flags so
! normal signed branches work (unlike 80x87 which returns the result in
! the zero and carry flags).

	.globl Fcomp
	.align	ALIGNMENT
Fcomp:
	pop	ecx		! get return address
	pop	eax		! xl
	pop	edx		! xu
	push	ecx		! put back ret address - pop 2nd double later

! All this popping is bad on 486's since plain mov takes 1+ cycle and pop
! takes 4 cycles. But this code is designed for 386's where popping is
! nominally the same speed and saves code space and so maybe instruction
! fetch time as well as the instruction to adjust the stack (ret #n takes
! no longer than plain ret but inhibits gotos).

	mov	ebx,PC_SIZE+D_LOW[esp]	! yl
	mov	ecx,PC_SIZE+D_HIGH[esp]	! yu
	jmp	compare

! Pop double from stack and compare with double at [ebx]

	.globl Fcompd
	.align	ALIGNMENT
Fcompd:
	mov	eax,PC_SIZE+D_LOW[esp]	! xl
	mov	edx,PC_SIZE+D_HIGH[esp]	! xu
	mov	ecx,D_HIGH[ebx]	! yu
	mov	ebx,D_LOW[ebx]	! yl

compare:
	test	edx,#D_SIGN_MASK	! is x >= 0?
	jz	cmp0		! yes; just compare x and y
	test	ecx,#D_SIGN_MASK	! no; but is y >= 0?
	jz	cmp0		! yes; just compare x and y

	xchg	edx,ecx		! x, y < 0, so ...
	xchg	eax,ebx		! ... swap x and y ...
	xor	edx,#D_SIGN_MASK	! ... and toggle signs
	xor	ecx,#D_SIGN_MASK

cmp0:
	cmp	edx,ecx		! compare upper dwords
	jnz	checkneg0	! if upper dwords differ, job is almost done
	mov	edx,eax		! upper dwords equal, so ...
	mov	ecx,ebx		! ... must make unsigned comparison of lower dwords
	shr	edx,#1		! shift past sign
	shr	ecx,#1
	cmp	edx,ecx		! compare top 31 bits of lower dwords
	jnz	return		! if these differ, job is done
	and	eax,#1		! compare lowest bits
	and	ebx,#1
	cmp	eax,ebx

return:
	ret	#D_SIZE		! return, popping 1 double from stack

checkneg0:
	test	edx,#D_EXP_MASK	| D_FRAC_MASK ! check to catch unusual case ...
	jnz	recheck
	test	eax,eax
	jnz	recheck
	test	ecx,#D_EXP_MASK | D_FRAC_MASK
	jnz	recheck
	test	ebx,ebx
	jz	return		! ... both are (+-) zero, return 'z'

recheck:
	cmp	edx,ecx		! the upper words were really different
	ret	#D_SIZE

	.globl Fcompf
	.align	ALIGNMENT
Fcompf:
	call	Fpushf
	pop	ebx		! yl
	pop	ecx		! yu
	mov	eax,PC_SIZE+D_LOW[esp]	! xl
	mov	edx,PC_SIZE+D_HIGH[esp]	! xu
	jmp	compare
