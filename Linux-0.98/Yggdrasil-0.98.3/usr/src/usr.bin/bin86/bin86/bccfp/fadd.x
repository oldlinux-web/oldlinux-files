! bcc 386 floating point routines (version 2)
! -- Fadd, Faddd, Faddf, Fsub, Fsubd, Fsubf, normalize2
! author: Bruce Evans

#include "fplib.h"

#define FRAME_SIZE	(3 * GENREG_SIZE + PC_SIZE)

	.extern	Fpushf
	.extern	fpdenormal
	.extern	fpoverflow
	.extern	fpunderflow

	.globl	Fadd
	.align	ALIGNMENT
Fadd:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ebx,FRAME_SIZE+D_SIZE+D_LOW[esp]
	mov	ecx,FRAME_SIZE+D_SIZE+D_HIGH[esp]
	call	addition
	mov	FRAME_SIZE+D_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret	#D_SIZE

	.globl	Faddd
	.align	ALIGNMENT
Faddd:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ecx,D_HIGH[ebx]
	mov	ebx,D_LOW[ebx]
	call	addition
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.globl	Faddf
	.align	ALIGNMENT
Faddf:
	push	ebp
	push	edi
	push	esi
	call	Fpushf
	pop	ebx		! yl
	pop	ecx		! yu
	mov	eax,FRAME_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_HIGH[esp]	! xu
	call	addition
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.globl	Fsub
	.align	ALIGNMENT
Fsub:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ebx,FRAME_SIZE+D_SIZE+D_LOW[esp]
	mov	ecx,FRAME_SIZE+D_SIZE+D_HIGH[esp]
	xor	ecx,#D_SIGN_MASK	! complement sign
	call	addition
	mov	FRAME_SIZE+D_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret	#D_SIZE

	.globl	Fsubd
	.align	ALIGNMENT
Fsubd:
	push	ebp
	push	edi
	push	esi
	mov	eax,FRAME_SIZE+D_LOW[esp]
	mov	edx,FRAME_SIZE+D_HIGH[esp]
	mov	ecx,D_HIGH[ebx]
	mov	ebx,D_LOW[ebx]
	xor	ecx,#D_SIGN_MASK	! complement sign
	call	addition
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.globl	Fsubf
	.align	ALIGNMENT
Fsubf:
	push	ebp
	push	edi
	push	esi
	call	Fpushf
	pop	ebx		! yl
	pop	ecx		! yu
	mov	eax,FRAME_SIZE+D_LOW[esp]	! xl
	mov	edx,FRAME_SIZE+D_HIGH[esp]	! xu
	xor	ecx,#D_SIGN_MASK	! complement sign
	call	addition
	mov	FRAME_SIZE+D_LOW[esp],eax
	mov	FRAME_SIZE+D_HIGH[esp],edx
	pop	esi
	pop	edi
	pop	ebp
	ret

	.align	ALIGNMENT
exp_y_0:

! Check for x denormal, to split off special case where both are denormal,
! so the norm bit (or 1 higher) is known to be set for addition, so addition
! can be done faster

	test	esi,#D_EXP_MASK
	jnz	x_normal_exp_y_0
	test	esi,esi		! test top bits of x fraction
	jnz	both_denorm	! denormal iff nonzero fraction with zero exp
	test	eax,eax		! test rest of fraction
	jz	return_edx_eax	! everything 0 (XXX - do signs matter?)
both_denorm:
	call	fpdenormal
	test	ebp,#D_SIGN_MASK
	jnz	denorm_subtract

! Add denormal x to denormal or zero y

#if D_NORM_BIT != D_EXP_SHIFT
#include "error, carry into norm bit does not go into exponent"
#endif

	add	eax,ebx
	adc	esi,edi
	or	edx,esi
	ret

denorm_subtract:
	sub	eax,ebx
	sbb	esi,edi
	or	edx,esi
	ret

	.align	ALIGNMENT
x_normal_exp_y_0:
	test	edi,edi		! this is like the check for x denormal
	jnz	y_denorm
	test	ebx,ebx	
	jz	return_edx_eax	! y = 0
y_denorm:
	call	fpdenormal
	or	ecx,#1 << D_EXP_SHIFT	! normalize y by setting exponent to 1
	jmp	got_y

	.align	ALIGNMENT
return_edx_eax:
	ret

	.align	ALIGNMENT
add_bigshift:
	cmp	ecx,#D_FRAC_BIT+2
	jae	return_edx_eax	! x dominates y
	sub	ecx,#REG_BIT
	shrd	ebp,ebx,cl
	shrd	ebx,edi,cl
	shr	edi,cl
	add	eax,edi
	adc	esi,#0
	xchg	ebp,ebx
	br	normalize

	.align	ALIGNMENT
addition:
	mov	esi,edx		! this mainly for consistent naming
	and	esi,#D_EXP_MASK | D_FRAC_MASK	! discard sign so comparison is simple
	mov	edi,ecx		! free cl for shifts
	and	edi,#D_EXP_MASK | D_FRAC_MASK
	cmp	esi,edi
	ja	xbigger
	jb	swap
	cmp	eax,ebx
	jae	xbigger
swap:
	xchg	edx,ecx
	xchg	eax,ebx
	xchg	esi,edi
xbigger:

! edx holds sign of result from here on
! and exponent of result before the normalization step

	mov	ebp,edx		! prepare difference of signs
	xor	ebp,ecx

	and	ecx,#D_EXP_MASK	! extract exp_y and check for y 0 or denormal
	beq	exp_y_0		! otherwise x is not 0 or denormal either
	and	edi,#D_FRAC_MASK	! extract fraction
	or	edi,#D_NORM_MASK	! normalize
got_y:
	and	esi,#D_FRAC_MASK	! extract fraction
	or	esi,#D_NORM_MASK	! normalize

	sub	ecx,edx		! carries from non-exp bits in edx killed later
	neg	ecx
	and	ecx,#D_EXP_MASK
	shr	ecx,#D_EXP_SHIFT	! difference of exponents

got_x_and_y:
	and	ebp,#D_SIGN_MASK 	! see if signs are same
	bne	subtract	! else roundoff reg ebp has been cleared

	cmp	cl,#REG_BIT
	bhis	add_bigshift
	shrd	ebp,ebx,cl
	shrd	ebx,edi,cl
	shr	edi,cl
	add	eax,ebx
	adc	esi,edi

! result edx(D_SIGN_MASK | D_EXP_MASK bits):esi:eax:ebp but needs normalization

	mov	edi,edx
	and	edi,#D_EXP_MASK
	test	esi,#D_NORM_MASK << 1
	jnz	add_loverflow

add_round:
	cmp	ebp,#1 << (REG_BIT-1)	! test roundoff register
	jb	add_done	! no rounding
	jz	tie
add_roundup:
	add	eax,#1
	adc	esi,#0
	test	esi,#D_NORM_MASK << 1
	jnz	pre_add_loverflow	! rounding may cause overflow!
add_done:
 	mov	ecx,edx		! duplicated code from 'done'
	and	edx,#D_SIGN_MASK
	or	edx,edi
	and	esi,#D_FRAC_MASK
	or	edx,esi
	ret

	.align	ALIGNMENT
tie:
	test	al,#1		! tie case, round to even
	jz	add_done	! even, no rounding
	jmp	add_roundup

	.align	ALIGNMENT
pre_add_loverflow:
	sub	ebp,ebp		! clear rounding register
				! probably avoiding tests for more rounding
add_loverflow:
	shrd	ebp,eax,#1
	jnc	over_set_sticky_bit
	or	ebp,#1
over_set_sticky_bit:
	shrd	eax,esi,#1
	shr	esi,#1
	add	edi,1 << D_EXP_SHIFT
	cmp	edi,#D_EXP_INFINITE << D_EXP_SHIFT
	jl	add_round
overflow:
	call	fpoverflow
	mov	eax,ecx		! XXX - wrong reg
	ret

! result edx(D_SIGN_MASK | D_EXP_MASK bits):
!        esi((D_NORM_MASK << 1) | D_NORM_MASK | D_FRAC_MASK bits):eax:ebp:ebx
! but needs normalization

	.align	ALIGNMENT
normalize:
	mov	edi,edx
	and	edi,#D_EXP_MASK
	test	esi,#D_NORM_MASK << 1
	bne	loverflow

! result edx(D_SIGN_MASK bit):edi(D_EXP_MASK bits):
!        esi(D_NORM_MASK | D_FRAC_MASK bits):eax:ebp:ebx
! but needs normalization

	.globl	normalize2
normalize2:
	test	esi,#D_NORM_MASK	! already-normalized is very common
	jz	normalize3
round:
	cmp	ebp,#1 << (REG_BIT-1)	! test roundoff register
	jb	done		! no rounding
	jz	near_tie
roundup:
	add	eax,#1
	adc	esi,#0
	test	esi,#D_NORM_MASK << 1
	bne	pre_loverflow	! rounding may cause overflow!
done:
cmp	edi,#D_EXP_INFINITE << D_EXP_SHIFT
jae	overflow
	and	edx,#D_SIGN_MASK	! extract sign of largest and result
	or	edx,edi		! include exponent with sign
	and	esi,#D_FRAC_MASK	! discard norm bit
	or	edx,esi		! include fraction with sign and exponent
	ret

	.align	ALIGNMENT
near_tie:
	test	ebx,ebx
	jnz	roundup
	test	al,#1		! tie case, round to even
	jz	done		! even, no rounding
	jmp	roundup

	.align	ALIGNMENT
not_in_8_below:
	shld	ecx,esi,#REG_BIT-D_NORM_BIT+16	! in 9 to 16 below?
	jz	not_in_16_below	! must be way below (17-20 for usual D_NORM_BIT)
	mov	cl,bsr_table[ecx]	! bsr(esi) - (D_NORM_BIT-16)
	neg	ecx		! (D_NORM_BIT-16) - bsr(esi)
	add	ecx,#16
	jmp	got_shift

	.align	ALIGNMENT
not_in_16_below:
	mov	cl,bsr_table[esi]	! bsr(esi) directly
	neg	ecx			! -bsr(esi)
	add	ecx,#D_NORM_BIT		! D_NORM_BIT - bsr(esi)
	jmp	got_shift

	.align	ALIGNMENT
normalize3:
	test	esi,esi
	jz	shift32

! Find first nonzero bit in esi
! Don't use bsr, it is very slow (const + 3 * bit_found)
! We know that there is some nonzero bit, and the norm bit and above are clear

	sub	ecx,ecx		! prepare unsigned extension of cl
	shld	ecx,esi,#REG_BIT-D_NORM_BIT+8	! any bits in 8 below norm bit?
	jz	not_in_8_below
	mov	cl,bsr_table[ecx]	! bsr(esi) - (D_NORM_BIT-8)
	neg	ecx		! (D_NORM_BIT-8) - bsr(esi)
	add	ecx,#8		! D_NORM_BIT - bsr(esi)
got_shift:
	shld	esi,eax,cl
	shld	eax,ebp,cl
	shld	ebp,ebx,cl
	shl	ebx,cl
	shl	ecx,D_EXP_SHIFT
	sub	edi,ecx
	bhi	round		! XXX - can rounding change the exponent to > 0?
				! not bgt since edi may be 0x80000000
	neg	edi
	shr	edi,#D_EXP_SHIFT
	inc	edi
	br	fpunderflow

	.align	ALIGNMENT
pre_loverflow:
	sub	ebp,ebp		! clear rounding registers
	sub	ebx,ebx		! probably avoiding tests for more rounding

loverflow:
	shr	esi,#1		! carry bit stayed in the reg
	rcr	eax,#1
	rcr	ebp,#1
	rcr	ebx,#1
	add	edi,1 << D_EXP_SHIFT
	cmp	edi,#D_EXP_INFINITE << D_EXP_SHIFT
	blt	round
	call	fpoverflow
	mov	eax,ecx		! XXX - wrong reg
	ret

	.align	ALIGNMENT
shift32:
	test	eax,eax
	jz	shift64
	mov	esi,eax
	mov	eax,ebp
	mov	ebp,ebx
	sub	ebx,ebx
	sub	edi,#REG_BIT << D_EXP_SHIFT
shiftxx:
	test	esi,#~(D_NORM_MASK | D_FRAC_MASK)
	jz	over_adjust	! else too big already
	shrd	ebx,ebp,#D_BIT-D_FRAC_BIT
	shrd	ebp,eax,#D_BIT-D_FRAC_BIT
	shrd	eax,esi,#D_BIT-D_FRAC_BIT
	shr	esi,#D_BIT-D_FRAC_BIT
	add	edi,#(D_BIT-D_FRAC_BIT) << D_EXP_SHIFT
over_adjust:
	test	edi,edi
	bgt	normalize2
	neg	edi
	shr	edi,#D_EXP_SHIFT
	inc	edi
	br	fpunderflow

	.align	ALIGNMENT
shift64:
	test	ebp,ebp
	jz	shift96
	mov	esi,ebp
	mov	eax,ebx
	sub	ebp,ebp
	mov	ebx,ebp
	sub	edi,#(2*REG_BIT) << D_EXP_SHIFT
	jmp	shiftxx

	.align	ALIGNMENT
shift96:
	test	ebx,ebx		! XXX - this test is probably unnecessary
				! since the shift must be small unless we
				! are subtracting 2 almost-equal numbers,
				! and then the bits beyond 64 will mostly
				! be 0
	jz	return_esi_eax	! all zero
	mov	esi,ebx
	sub	ebx,ebx
	sub	edi,#(3*REG_BIT) << D_EXP_SHIFT
	jmp	shiftxx

	.align	ALIGNMENT
return_esi_eax:
	mov	edx,esi
	ret

	.align	ALIGNMENT
subtract:
	sub	ebp,ebp		! set up roundoff register
	cmp	ecx,#REG_BIT
	jae	subtract_bigshift
	shrd	ebp,ebx,cl
	shrd	ebx,edi,cl
	shr	edi,cl
	neg	ebp		! begin subtraction esi:eax:0 - edi:ebx:ebp
	sbb	eax,ebx
	sbb	esi,edi
	sub	ebx,ebx
	mov	edi,edx
	and	edi,#D_EXP_MASK
	br	normalize2

	.align	ALIGNMENT
subtract_bigshift:
	cmp	ecx,#D_FRAC_BIT+2
	bhis	return_edx_eax	! x dominates y
	sub	ecx,#REG_BIT
	shrd	ebp,ebx,cl
	shrd	ebx,edi,cl
	shr	edi,cl
	not	ebp		! begin subtraction esi:eax:0:0 - 0:edi:ebx:ebp
	not	ebx
	add	ebp,#1
	adc	ebx,#0
	cmc
	sbb	eax,edi
	sbb	esi,#0
	xchg	ebp,ebx
	mov	edi,edx
	and	edi,#D_EXP_MASK
	br	normalize2

	.data
	.extern	bsr_table
