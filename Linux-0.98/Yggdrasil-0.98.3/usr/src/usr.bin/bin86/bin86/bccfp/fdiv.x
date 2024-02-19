#define EF_SIZE		4

! bcc 386 floating point routines (version 2) -- Fdiv, Fdivd, Fdivf
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

#define FRAME_SIZE	(3 * GENREG_SIZE + PC_SIZE)

	.extern	Fpushf
	.extern	fpdivzero
	.extern	fpoverflow
	.extern	fpunderflow

! double Fdiv(double x, double y) returns x / y

! pop 2 doubles from stack, divide first by second, and push quotient on stack

! we denote upper and lower dwords of x and y (or their fractions)
! by (xu,xl), (yu,yl)

	.globl	Fdivf
	.align	ALIGNMENT
Fdivf:
	sub	esp,#D_SIZE	! make space for dummy double on stack
	push	ebp
	push	edi		! save some regs
	push	esi
	mov	eax,FRAME_SIZE-PC_SIZE+D_SIZE[esp]	! move return address ...
	mov	FRAME_SIZE-PC_SIZE[esp],eax	! ... to usual spot
	call	Fpushf
	pop	esi		! yl
	pop	edi		! yu
	mov	eax,FRAME_SIZE+D_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_SIZE+D_HIGH[esp]	! xu
	jmp	division

	.globl	Fdiv
	.align	ALIGNMENT
Fdiv:
	push	ebp
	push	edi		! save some regs
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_HIGH[esp]	! xu
	mov	esi,FRAME_SIZE+D_SIZE+D_LOW[esp]	! yl
	mov	edi,FRAME_SIZE+D_SIZE+D_HIGH[esp]	! yu
	jmp	division

	.align	ALIGNMENT
exp_y_0:
	mov	ebx,edi
	or	ebx,esi
	beq	zerodivide
	mov	ebx,#1
fix_y:
	test	edi,edi		! XXX - sloow
	js	y_unpacked
	shld	edi,esi,#1
	shl	esi,#1
	dec	bx
	jmp	fix_y

	.align	ALIGNMENT
exp_x_0:
	mov	ecx,edx
	or	ecx,eax
	beq	retz
	mov	ecx,#1	! change exponent from 0 to 1
fix_x:
	test	edx,#1 << (REG_BIT-1-2)		! XXX - sloow
	jnz	x_unpacked
	shld	edx,eax,#1
	shl	eax,#1
	dec	cx
	jmp	fix_x

! Fdivd pops double from stack, divides it by double at [ebx],
! and pushes quotient back on stack

	.globl	Fdivd
	.align	ALIGNMENT
Fdivd:
	sub	esp,#D_SIZE	! make space for dummy double on stack
	push	ebp
	push	edi		! save some regs
	push	esi
	mov	eax,FRAME_SIZE-PC_SIZE+D_SIZE[esp]	! move return address ...
	mov	FRAME_SIZE-PC_SIZE[esp],eax	! ... to usual spot
	mov	eax,FRAME_SIZE+D_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_SIZE+D_HIGH[esp]	! xu
	mov	esi,D_LOW[ebx]	! yl
	mov	edi,D_HIGH[ebx]	! yu

division:

! The full calculations are

!	(xu,xl,0) = yu * (zu,zl) + (0,r,0)  (normal 96/32 -> 64 bit division)
!	yl * zu = yu * q1 + r1  (32*32 -> 64 bit mul and 64/32 -> 32 bit div)

! so

!	(xu,xl,0,0) = (yu,yl) * (zu,zl-q1) + (0,0,r-r1,yl*(q1-zl))

! where the calculations zl-q1, r-r1 and yl*(q1-zl) are more complicated
! than the notation suggests. They may be negative and the one with the
! multiplication may not fit in 32 bits and in both cases the overflow
! has to be moved into higher bit positions.

! See Knuth for why (zu,zl-q1) is the correct 64-bit quotient to within
! 1 bit either way (assuming the normalization x < 2 * y).

! We only need to calculate the remainder (0,0,r-r1,yl*(q1-zl)) to resolve
! tie cases. It tells whether the approximate quotient is too high or too
! low.

#define NTEMPS	5

	sub	esp,#NTEMPS*GENREG_SIZE	! space to remember values for rounding of tie case

! Offsets from esp for these values (offsets using FRAME_SIZE are invalid
! while these temps are active)
r	=	0
q1	=	4
r1	=	8
yl	=	12
zl	=	16

! Step 1: unpack and normalize x to fraction in edx:eax (left shifted as
! far as possible less 2 so that x < y, and later z < y); unpack and normalize
! y to a fraction in edi:esi (left shifted as far as possible), put difference
! of signs (= sign of quotient) in ecx(D_SIGN_MASK) and difference of exponents 
! (= exponent of quotient before normalization) in cx.

	mov	ebp,edx		! xu
	xor	ebp,edi		! xu ^ yu
	and	ebp,#D_SIGN_MASK	! sign of result is difference of signs

! Unpack y first to trap 0 / 0

	mov	ebx,edi		! remember yu for exponent of y
	shld	edi,esi,#D_BIT-D_FRAC_BIT	! extract fraction of y ...
	shl	esi,#D_BIT-D_FRAC_BIT
	and	ebx,#D_EXP_MASK	! exponent of y
	jz	exp_y_0
	shr	ebx,#D_EXP_SHIFT	! in ebx (actually in bx, with high bits 0)
	or	edi,#D_NORM_MASK << (D_BIT-D_FRAC_BIT)	! normalize
y_unpacked:

! Unpack x

	mov	ecx,edx		! remember xu for exponent of x
	shld	edx,eax,#D_BIT-D_FRAC_BIT-2	! extract fraction of x ...
	shl	eax,#D_BIT-D_FRAC_BIT-2
	and	edx,#(D_NORM_MASK << (D_BIT-D_FRAC_BIT-2+1))-1
				! XXX - above may be shifted 1 extra unnecessarily
	and	ecx,#D_EXP_MASK	! exponent of x
	jz	exp_x_0
	shr	ecx,#D_EXP_SHIFT	! in ecx (actually in cx, with high bits 0)
	or	edx,#D_NORM_MASK << (D_BIT-D_FRAC_BIT-2) ! normalize
x_unpacked:

	sub	cx,bx		! not ecx,ebx because we want to use high bit for sign
	add	cx,#D_EXP_BIAS	! adjust exponent of quotient

	or	ecx,ebp		! include sign with exponent

! Step 2: quotient of fractions -> (edx,eax)

! 2a: (xu,xl,0) div yu = (zu,zl) -> (ebx,esi)

	div	eax,edi		! (xu,xl) div yu = zu in eax; remainder (rem) in edx
	mov	ebx,eax		! save zu in ebx
	sub	eax,eax		! clear eax: (edx,eax) = (rem,0)
	div	eax,edi		! (rem,0) div yu = zl in eax
	mov	r[esp],edx
	mov	zl[esp],eax
	xchg	eax,esi		! store zl in esi; save yl in eax
	mov	yl[esp],eax

! 2b: (yl * zu) div yu -> (0,eax)

	mul	eax,ebx		! yl * zu -> (edx,eax)
	div	eax,edi		! (yl * zu) div yu in eax
	mov	q1[esp],eax
	mov	r1[esp],edx

! 2c: (xu,xl) / (yu,yl) = (zu,zl) - (yl * zu) div yu -> (edx,eax)

	mov	edx,ebx		! zu
	xchg	eax,esi		! eax <- zl; esi <- (yl * zu) div yu
	sub	eax,esi
	sbb	edx,#0

! Step 3: normalise quotient

	test	edx,#1 << (REG_BIT-2)	! is fraction too small? (can only be by 1 bit)
	jnz	div4
	shld	edx,eax,#1	! yes; multiply fraction ...
	shl	eax,#1		! ... by 2 ...
	dec	cx		! ... and decrement exponent

! Step 4: shift and round

div4:
	mov	ebx,eax		! save for rounding
	shrd	eax,edx,#D_BIT-D_FRAC_BIT-1	! shift fraction of result ...
	shr	edx,#D_BIT-D_FRAC_BIT-1		! ... to proper position
	and	ebx,#(1 << (D_BIT-D_FRAC_BIT-1))-1	! look at bits shifted out
	cmp	ebx,#D_NORM_MASK >> (D_BIT-D_FRAC_BIT)	! compare with middle value
	jb	div5		! below middle, don't round up
	ja	roundup		! above middle, round up

! The low bits don't contain enough information to resolve the tie case,
! because the quotient itself is only an approximation.
! Calculate the exact remainder.
! This case is not very common, so don't worry much about speed.
! Unfortunately we had to save extra in all cases to prepare for it.

	push	edx
	push	eax

	sub	esi,esi		! the calculation requires 33 bits - carry to here
	mov	eax,2*GENREG_SIZE+q1[esp]
	sub	eax,2*GENREG_SIZE+zl[esp]
	pushfd
	mul	dword EF_SIZE+2*GENREG_SIZE+yl[esp]
	popfd
	jnc	foo
	sub	edx,2*GENREG_SIZE+yl[esp]
	sbb	esi,#0
foo:
	add	edx,2*GENREG_SIZE+r[esp]
	adc	esi,#0
	sub	edx,2*GENREG_SIZE+r1[esp]
	sbb	esi,#0
	mov	ebx,eax
	mov	edi,edx

	pop	eax
	pop	edx

! Can finally decide rounding of tie case

	js	div5		! remainder < 0 from looking at top 64 bits
	jnz	roundup		! remainder > 0 from looking at top 64 bits
	or	edi,ebx		! test bottom 64 bits
	jnz	roundup		! remainder > 0

	test	al,#1		! at last we know it is the tie case, check parity bit
	jz	div5		! already even, otherwise round up to make even

roundup:
	add	eax,#1		! add rounding bit
	adc	edx,#0
	test	edx,#D_NORM_MASK << 1	! has fraction overflowed (very unlikely)
	jz	div5
! Why were the shifts commented out?
	shrd	eax,edx,#1	! yes, divide fraction ...
	shr	edx,#1		! ... by 2 ...
	inc	cx		! ... and increment exponent

! Step 5: put it all together

div5:
	mov	ebx,ecx		! extract sign
	and	ebx,D_SIGN_MASK
	cmp	cx,#D_EXP_INFINITE	! is exponent too big?
	jge	overflow
	test	cx,cx
	jle	underflow
	shl	ecx,#D_EXP_SHIFT

	and	edx,#D_FRAC_MASK	! remove norm bit
	or	edx,ecx		! include exponent ...
	or	edx,ebx		! ... and sign

return:
	add	esp,#NTEMPS*GENREG_SIZE	! reclaim temp space
	mov	FRAME_SIZE+D_SIZE+D_LOW[esp],eax	! "push" lower dword of product ...
	mov	FRAME_SIZE+D_SIZE+D_HIGH[esp],edx	! ... and upper dword
	pop	esi		! restore registers
	pop	edi
	pop	ebp
	ret	#D_SIZE

retz:
	sub	edx,edx		! clear upper dword
	sub	eax,eax		! ... and lower dword
	jmp	return

overflow:
	mov	edx,ecx		! put sign in usual reg
	call	fpoverflow
	mov	eax,ecx		! XXX - wrong reg
	jmp	return

underflow:
	mov	esi,edx		! put upper part of fraction in usual reg
	mov	edx,ecx		! sign
	movsx	edi,cx		! put shift in usual reg
	neg	edi
	inc	edi
	call	fpunderflow
	jmp	return

zerodivide:
	mov	edx,ebp		! sign
	call	fpdivzero
	mov	eax,ecx		! XXX - wrong reg
	jmp	return
