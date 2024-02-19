	.file	"reg_u_div.S"
/*---------------------------------------------------------------------------+
 |  reg_u_div.S                                                              |
 |                                                                           |
 | Core division routines                                                    |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 |  Kernel for the division routines.                                        |
 |                                                                           |
 |  void reg_u_div(unsigned long long *a, unsigned long long *a, REG *dest)  |
 |                                                                           |
 |  Does not compute the destination exponent, but does adjust it.           |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "fpu_asm.h"


#define	dSIGL(x)	(x)
#define	dSIGH(x)	4(x)


.data
/*
	Local storage:
	Result:		accum_3:accum_2:accum_1:accum_0
	Overflow flag:	ovfl_flag
 */
	.align 2,0
accum_3:
	.long	0
accum_2:
	.long	0
accum_1:
	.long	0
accum_0:
	.long	0
result_1:
	.long	0
result_2:
	.long	0
ovfl_flag:
	.byte	0


.text
	.align 2,144

.globl _reg_u_div

.globl _divide_kernel

_reg_u_div:
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi	/* pointer to num */
	movl	PARAM2,%ebx	/* pointer to denom */
	movl	PARAM3,%edi	/* pointer to answer */

_divide_kernel:
#ifdef PARANOID
//	testl	$0x80000000, dSIGH(%esi)
//	je	xL_bugged
	testl	$0x80000000, dSIGH(%ebx)
	je	xL_bugged
#endif PARANOID

/* Check if the denominator can be treated as having just 32 bits */
	cmpl	$0,dSIGL(%ebx)
	jnz	L_Full_Division	/* Can't do a quick divide */

/* We should be able to zip through the division here */
	movl	dSIGH(%ebx),%ecx	/* The denominator */
	movl	dSIGH(%esi),%edx	/* Get the current num */
	movl	dSIGL(%esi),%eax	/* Get the current num */

	cmpl	%ecx,%edx
	setaeb	ovfl_flag	/* Keep a record */
	jb	xL_no_adjust

	subl	%ecx,%edx	/* Prevent the overflow */

xL_no_adjust:
	/* Divide the 64 bit number by the 32 bit denominator */
	divl	%ecx
	movl	%eax,SIGH(%edi)	/* Put the result in the answer */

	/* Work on the remainder of the first division */
	xorl	%eax,%eax
	divl	%ecx
	movl	%eax,SIGL(%edi)	/* Put the result in the answer */

	/* Work on the remainder of the 64 bit division */
	xorl	%eax,%eax
	divl	%ecx

	testb	$255,ovfl_flag	/* was the num > denom ? */
	je	xL_no_overflow

	/* Do the shifting here */
	/* increase the exponent */
	incl	EXP(%edi)

	/* shift the mantissa right one bit */
	stc			/* To set the ms bit */
	rcrl	SIGH(%edi)
	rcrl	SIGL(%edi)
	rcrl	%eax

xL_no_overflow:
	cmpl	$0x80000000,%eax
	jc	xL_no_round
	jnz	xL_round_up

	/* "round to even" used */
	testb	$1,SIGL(%edi)
	jz	xL_no_round

xL_round_up:
	addl	$1,SIGL(%edi)
	adcl	$0,SIGH(%edi)

#ifdef PARANOID
	jc	xL_bugged2
#endif PARANOID

xL_no_round:
	jmp	xL_done


#ifdef PARANOID
xL_bugged:
	pushl	EX_INTERNAL|0x203
	call	EXCEPTION
	pop	%ebx
	jmp	xL_exit

xL_bugged2:
	pushl	EX_INTERNAL|0x204
	call	EXCEPTION
	pop	%ebx
	jmp	xL_exit
#endif PARANOID


/*---------------------------------------------------------------------------+
 |  Divide:   Return  arg1/arg2 to arg3.                                     |
 |                                                                           |
 |  This routine does not use the exponents of arg1 and arg2, but does       |
 |  adjust the exponent of arg3.                                             |
 |                                                                           |
 |  The maximum returned value is (ignoring exponents)                       |
 |               .ffffffff ffffffff                                          |
 |               ------------------  =  1.ffffffff fffffffe                  |
 |               .80000000 00000000                                          |
 | and the minimum is                                                        |
 |               .80000000 00000000                                          |
 |               ------------------  =  .80000000 00000001   (rounded)       |
 |               .ffffffff ffffffff                                          |
 |                                                                           |
 +---------------------------------------------------------------------------*/


L_Full_Division:
	movl	dSIGL(%esi),%eax	/* Save extended num in local register */
	movl	%eax,accum_2
	movl	dSIGH(%esi),%eax
	movl	%eax,accum_3
	xorl	%eax,%eax
	movl	%eax,accum_1	/* zero the extension */
	movl	%eax,accum_0	/* zero the extension */

	movl	dSIGL(%esi),%eax	/* Get the current num */
	movl	dSIGH(%esi),%edx

/*----------------------------------------------------------------------*/
/* Initialization done */
/* Do the first 32 bits */

	movb	$0,ovfl_flag
	cmpl	dSIGH(%ebx),%edx	/* Test for imminent overflow */
	jb	L02
	ja	L01

	cmpl	dSIGL(%ebx),%eax
	jb	L02

L01:
/* The numerator is greater or equal, would cause overflow */
	setaeb	ovfl_flag		/* Keep a record */

	subl	dSIGL(%ebx),%eax
	sbbl	dSIGH(%ebx),%edx	/* Prevent the overflow */
	movl	%eax,accum_2
	movl	%edx,accum_3

L02:
/* At this point, we have a num < denom, with a record of
   adjustment in ovfl_flag */

	/* We will divide by a number which is too large */
	movl	dSIGH(%ebx),%ecx
	addl	$1,%ecx
	jnc	L04

	/* here we need to divide by 100000000h,
	   i.e., no division at all.. */

	mov	%edx,%eax
	jmp	L05

L04:
	divl	%ecx		/* Divide the numerator by the augmented
				   denom ms dw */

L05:
	movl	%eax,result_2	/* Put the result in the answer */

	mull	dSIGH(%ebx)		/* mul by the ms dw of the denom */

	subl	%eax,accum_2	/* Subtract from the num local reg */
	sbbl	%edx,accum_3

	movl	result_2,%eax	/* Get the result back */
	mull	dSIGL(%ebx)		/* now mul the ls dw of the denom */

	subl	%eax,accum_1	/* Subtract from the num local reg */
	sbbl	%edx,accum_2
	sbbl	$0,accum_3
	je	L10		/* Must check for non-zero result here */

#ifdef PARANOID
	jb	L_bugged
#endif PARANOID

	/* need to subtract another once of the denom */
	incl	result_2		/* Correct the answer */

	movl	dSIGL(%ebx),%eax
	movl	dSIGH(%ebx),%edx
	subl	%eax,accum_1	/* Subtract from the num local reg */
	sbbl	%edx,accum_2

#ifdef PARANOID
	sbbl	$0,accum_3
	jne	L_bugged	/* Must check for non-zero result here */
#endif PARANOID

/*----------------------------------------------------------------------*/
/* Half of the main problem is done, there is just a reduced numerator
   to handle now */
/* Work with the second 32 bits, accum_0 not used from now on */
L10:
	movl	accum_2,%edx	/* get the reduced num */
	movl	accum_1,%eax

	/* need to check for possible subsequent overflow */
	cmpl	dSIGH(%ebx),%edx
	jb	L22
	ja	L21

	cmpl	dSIGL(%ebx),%eax
	jb	L22

L21:
/* The numerator is greater or equal, would cause overflow */
	/* prevent overflow */
	subl	dSIGL(%ebx),%eax
	sbbl	dSIGH(%ebx),%edx
	movl	%edx,accum_2
	movl	%eax,accum_1

	incl	result_2		/* Reflect the subtraction in the answer */

#ifdef PARANOID
	je	L_bugged
#endif PARANOID

L22:
	cmpl	$0,%ecx
	jnz	L24

	/* %ecx == 0, we are dividing by 1.0 */
	mov	%edx,%eax
	jmp	L25

L24:
	divl	%ecx		/* Divide the numerator by the denom ms dw */

L25:
	movl	%eax,result_1	/* Put the result in the answer */

	mull	dSIGH(%ebx)		/* mul by the ms dw of the denom */

	subl	%eax,accum_1	/* Subtract from the num local reg */
	sbbl	%edx,accum_2

#ifdef PARANOID
	jc	L_bugged
#endif PARANOID

	movl	result_1,%eax	/* Get the result back */
	mull	dSIGL(%ebx)		/* now mul the ls dw of the denom */

	/* Here we are throwing away some ls bits */
	subl	%eax,accum_0	/* Subtract from the num local reg */
	sbbl	%edx,accum_1	/* Subtract from the num local reg */
	sbbl	$0,accum_2

#ifdef PARANOID
	jc	L_bugged
#endif PARANOID

	jz	L35		/* Just deal with rounding now */

#ifdef PARANOID
	cmpl	$1,accum_2
	jne	L_bugged
#endif PARANOID

L32:
	/* need to subtract another once of the denom */
	movl	dSIGL(%ebx),%eax
	movl	dSIGH(%ebx),%edx
	subl	%eax,accum_0	/* Subtract from the num local reg */
	sbbl	%edx,accum_1
	sbbl	$0,accum_2

#ifdef PARANOID
	jc	L_bugged
	jne	L_bugged
#endif PARANOID

	addl	$1,result_1	/* Correct the answer */
	adcl	$0,result_2

#ifdef PARANOID
	/* Do we ever really need this check? ***** */
	jc	L_bugged	/* Must check for non-zero result here */
#endif PARANOID

/*----------------------------------------------------------------------*/
/* The division is essentially finished here, we just need to perform
   tidying operations. */
/* deal with the 3rd 32 bits */
L35:
	movl	accum_1,%edx	/* get the reduced num */
	movl	accum_0,%eax

	/* need to check for possible subsequent overflow */
	cmpl	dSIGH(%ebx),%edx
	jb	L42
	ja	L41

	cmpl	dSIGL(%ebx),%eax
	jb	L42

L41:
	/* prevent overflow */
	subl	dSIGL(%ebx),%eax
	sbbl	dSIGH(%ebx),%edx
	movl	%edx,accum_1
	movl	%eax,accum_0	/* ***** not needed unless extended rounding */

	addl	$1,result_1	/* Reflect the subtraction in the answer */
	adcl	$0,result_2
	jne	L42
	jnc	L42

	/* This is a tricky spot, there is an overflow of the answer */
	movb	$255,ovfl_flag		/* Overflow -> 1.000 */
	
L42:
	/* Now test for rounding */
	movl	accum_1,%edx	/* ms byte of accum extension */

L44:
	cmpl	$0,%ecx
	jnz	L241

	/* %ecx == 0, we are dividing by 1.0 */
	mov	%edx,%eax
	jmp	L251

L241:
	divl	%ecx		/* Divide the numerator by the denom ms dw */

L251:
/* We are now ready to deal with rounding, but first we must get
   the bits properly aligned */
	testb	$255,ovfl_flag	/* was the num > denom ? */
	je	L45

	incl	EXP(%edi)

	/* shift the mantissa right one bit */
	stc			// Will set the ms bit
	rcrl	result_2
	rcrl	result_1
	rcrl	%eax

L45:
	cmpl	$0x80000000,%eax
	jc	xL_no_round_2		// No round up
	jnz	xL_round_up_2

	/* "round to even" used here for now... */
	testb	$1,result_1
	jz	xL_no_round_2		// No round up

xL_round_up_2:
	addl	$1,result_1
	adcl	$0,result_2

#ifdef PARANOID
	jc	L_bugged
#endif PARANOID

xL_no_round_2:
	movl	result_1,%eax
	movl	%eax,SIGL(%edi)
	movl	result_2,%eax
	movl	%eax,SIGH(%edi)

xL_done:
	decl	EXP(%edi)	/* binary point between 1st & 2nd bits */

xL_check_exponent:
	cmpl	EXP_OVER,EXP(%edi)
	jge	xL_overflow

	cmpl	EXP_UNDER,EXP(%edi)
	jle	xL_underflow

xL_exit:
	popl	%ebx
	popl	%edi
	popl	%esi

	leave
	ret


xL_overflow:
	pushl	%edi
	call	_arith_overflow
	popl	%ebx
	jmp	xL_exit

xL_underflow:
	pushl	%edi
	call	_arith_underflow
	popl	%ebx
	jmp	xL_exit


#ifdef PARANOID
/* The logic is wrong if we got here */
L_bugged:
	pushl	EX_INTERNAL|0x202
	call	EXCEPTION
	pop	%ebx
	jmp	xL_exit
#endif PARANOID
