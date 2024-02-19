! bcc 386 floating point routines (version 2) -- dtof, Fpushd, Fneg, Fnegd
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	Fpullf

! dtof converts the double at [ebx] to a float and pushes the float onto
! the stack (D_SIZE bytes are allocated for the float although only the bottom
! F_SIZE are used).
! This is a quickly-written slowish version.

	.globl dtof
	.align	ALIGNMENT
dtof:
	pop	eax
	sub	esp,#D_SIZE	! build result here
	push	eax		! put back return address
	call	Fpushd
	lea	ebx,D_SIZE+PC_SIZE[esp]
	call	Fpullf
	ret

! Push double at address [ebx] onto stack

	.globl	Fpushd
	.align	ALIGNMENT
Fpushd:
	pop	ecx
	push	dword D_HIGH[ebx]
	push	dword D_LOW[ebx]
	jmp	ecx		! return
	
! Push double at address [ebx] onto stack, negating it on the way.

! Don't worry about generating -0 because other routines have to allow for
! it anyway.

! Perhaps this and Fneg should check for denormals and illegal operands
! (I think only signalling NaNs are illegal).
! fchs doesn't check, but fld does.
! Our Fpushd is not quite like fld because no conversions are involved.

	.globl	Fnegd
	.align	ALIGNMENT
Fnegd:
	pop	ecx
	mov	eax,D_HIGH[ebx]
	xor	eax,#D_SIGN_MASK	! toggle sign
	push	eax
	push	dword D_LOW[ebx]
	jmp	ecx		! return

! Negate double on stack

	.globl	Fneg
	.align	ALIGNMENT
Fneg:
	xorb	PC_SIZE+D_SIZE-1[esp],D_SIGN_MASK >> (REG_BIT-CHAR_BIT)	! toggle sign
	ret
