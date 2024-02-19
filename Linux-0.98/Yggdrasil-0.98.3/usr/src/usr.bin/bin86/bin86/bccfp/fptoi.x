! bcc 386 floating point routines (version 2)
! -- dtoi, dtol, dtoui, dtoul, ftoi, ftol (todo: ftoui, ftoul)
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

	.extern	fpoverflow
	.extern	Fpushf

! Convert double x at [ebx] to int and return in eax

	.globl dtoi
	.globl dtol
	.align	ALIGNMENT
dtoi:
dtol:
	mov	eax,D_HIGH[ebx]
	mov	ecx,eax
	and	ecx,#D_EXP_MASK	! extract exponent
	jz	retz		! if 0 return 0
	test	eax,#D_SIGN_MASK
	jnz	negative
	call	into_dtoui
	cmp	eax,#INT_MAX
	ja	overflow_int_max
	ret

	.align	ALIGNMENT
negative:
	and	eax,#~D_SIGN_MASK
	call	into_dtoui
	cmp	eax,#INT_MIN
	ja	overflow_int_min
	neg	eax
	ret

	.align	ALIGNMENT
overflow_int_max:
	call	fpoverflow
	mov	eax,#INT_MAX
	ret

	.align	ALIGNMENT
overflow_int_min:
	js	return		! actually INT_MIN is OK
	call	fpoverflow
	mov	eax,#INT_MIN
return:
	ret

	.align	ALIGNMENT
retz:
	sub	eax,eax		! clear return value
	ret

! Convert double x at [ebx] to unsigned and return in eax

	.globl dtoui
	.globl dtoul
	.align	ALIGNMENT
dtoui:
dtoul:
	mov	eax,D_HIGH[ebx]
	mov	ecx,eax
	and	ecx,#D_EXP_MASK	! extract exponent
	jz	retz		! if 0 return 0
	test	eax,#D_SIGN_MASK
	jnz	overflow_0
into_dtoui:
	mov	edx,D_LOW[ebx]

	and	eax,#D_FRAC_MASK	! extract fraction
	or	eax,#D_NORM_MASK	! restore normalization bit

	shr	ecx,#D_EXP_SHIFT	! convert exponent to number
	sub	ecx,#D_EXP_BIAS+D_NORM_BIT	! adjust radix point
	jl	dtoui_rightshift	! should we shift left or right?
	cmp	ecx,#D_BIT-D_FRAC_BIT	! can shift left by at most this
	ja	overflow_uint_max	! if more, overflow
	shld	eax,edx,cl
	ret

	.align	ALIGNMENT
dtoui_rightshift:
	neg	ecx		! make shift count > 0
	cmp	ecx,#REG_BIT	! big shifts would be taken mod REG_BIT ...
	jae	retz		! ... no good
	shr	eax,cl		! otherwise it is faster to do the shift ...
	ret			! ... then to jump for the slightly smaller
				! ... shift counts that shift out all bits

	.align	ALIGNMENT
overflow_0:
	call	fpoverflow
	sub	eax,eax
	ret

	.align	ALIGNMENT
overflow_uint_max:
	call	fpoverflow
	mov	eax,#UINT_MAX
	ret

! ftoi is like dtoi except ebx points to a float instead of a double.
! This is a quickly-written slowish version that does not take advantage
! of the float being smaller.

	.globl ftoi
	.globl ftol
	.align	ALIGNMENT
ftoi:
ftol:
	call	Fpushf
	mov	ebx,esp
	call	dtoi
	add	esp,#D_SIZE
	ret
