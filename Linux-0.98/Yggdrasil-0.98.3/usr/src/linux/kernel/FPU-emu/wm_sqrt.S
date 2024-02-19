	.file	"wm_sqrt.S"
/*---------------------------------------------------------------------------+
 |  wm_sqrt.S                                                                |
 |                                                                           |
 | Fixed point arithmetic square root evaluation.                            |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void wm_sqrt(REG *n)                                                    |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
 |  wm_sqrt(REG *n)                                                          |
 |    returns the square root of n in n.                                     |
 |                                                                           |
 |  Use Newton's method to compute the square root of a number, which must   |
 |  be in the range  [1.0 .. 4.0),  to 64 bits accuracy.                     |
 |  Does not check the sign or tag of the argument.                          |
 |  Sets the exponent, but not the sign or tag of the result.                |
 |                                                                           |
 |  The guess is kept in %esi:%edi                                           |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "fpu_asm.h"


.data
/*
	Local storage:
 */
	.align 4,0
accum_2:
	.long	0		// ms word
accum_1:
	.long	0
accum_0:
	.long	0

sq_2:
	.long	0		// ms word
sq_1:
	.long	0
sq_0:
	.long	0

.text
	.align 2,144

.globl _wm_sqrt

_wm_sqrt:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
//	pushl	%ebx

	movl	PARAM1,%esi

	movl	SIGH(%esi),%eax
	movl	SIGL(%esi),%ecx
	xorl	%edx,%edx

// We use a rough linear estimate for the first guess..

	cmpl	EXP_BIAS,EXP(%esi)
	jnz	L10

	shrl	$1,%eax			/* arg is in the range  [1.0 .. 2.0) */
	rcrl	$1,%ecx
	rcrl	$1,%edx

L10:
// From here on, n is never accessed directly again until it is
// replaced by the answer.

	movl	%eax,sq_2		// ms word of n
	movl	%ecx,sq_1
	movl	%edx,sq_0

	shrl	$1,%eax
	addl	$0x40000000,%eax
	movl	$0xaaaaaaaa,%ecx
	mull	%ecx
	shll	%edx			/* max result was 7fff... */
	testl	$0x80000000,%edx	/* but min was 3fff... */
	jnz	no_adjust

	movl	$0x80000000,%edx	/* round up */

no_adjust:
	movl	%edx,%esi	// Our first guess

/* We have now computed (approx)   (2 + x) / 3, which forms the basis
   for a few iterations of Newton's method */

	movl	sq_2,%ecx	// ms word

// From our initial estimate, three iterations are enough to get us
// to 30 bits or so. This will then allow two iterations at better
// precision to complete the process.

// Compute  (g + n/g)/2  at each iteration (g is the guess).
	shrl	%ecx		// Doing this first will prevent a divide
				// overflow later.

	movl	%ecx,%edx
	divl	%esi
	shrl	%esi
	addl	%eax,%esi

	movl	%ecx,%edx
	divl	%esi
	shrl	%esi
	addl	%eax,%esi

	movl	%ecx,%edx
	divl	%esi
	shrl	%esi
	addl	%eax,%esi

// Now that an estimate accurate to about 30 bits has been obtained,
// we improve it to 60 bits or so.

// The strategy from now on is to compute new estimates from
//      guess := guess + (n - guess^2) / (2 * guess)

// First, find the square of the guess
	movl	%esi,%eax
	mull	%esi
// guess^2 now in %edx:%eax

	movl	sq_1,%ecx
	subl	%ecx,%eax
	movl	sq_2,%ecx	// ms word of normalized n
	sbbl	%ecx,%edx
	jnc	l40

// subtraction gives a negative result
// negate the reult before division
	notl	%edx
	notl	%eax
	addl	$1,%eax
	adcl	$0,%edx

	divl	%esi
	movl	%eax,%ecx

	movl	%edx,%eax
	divl	%esi
	jmp	l45

l40:
	divl	%esi
	movl	%eax,%ecx

	movl	%edx,%eax
	divl	%esi

	notl	%ecx
	notl	%eax
	addl	$1,%eax
	adcl	$0,%ecx

l45:
	sarl	$1,%ecx		// divide by 2
	rcrl	$1,%eax

	movl	%eax,%edi
	addl	%ecx,%esi

// Now the square root has been computed to better than 60 bits

// Find the square of the guess
	movl	%edi,%eax		// ls word of guess
	mull	%edi
	movl	%edx,accum_0

	movl	%esi,%eax
	mull	%esi
	movl	%edx,accum_2
	movl	%eax,accum_1

	movl	%edi,%eax
	mull	%esi
	addl	%eax,accum_0
	adcl	%edx,accum_1
	adcl	$0,accum_2

	movl	%esi,%eax
	mull	%edi
	addl	%eax,accum_0
	adcl	%edx,accum_1
	adcl	$0,accum_2

// guess^2 now in accum_2:accum_1:accum_0

	movl	sq_1,%eax		// get normalized n
	subl	%eax,accum_1
	movl	sq_2,%eax		// ms word of normalized n
	sbbl	%eax,accum_2
	jnc	l60

// subtraction gives a negative result
// negate the reult before division
	notl	accum_0
	notl	accum_1
	notl	accum_2
	addl	$1,accum_0
	adcl	$0,accum_1

#ifdef PARANOID
	adcl	$0,accum_2	// This must be zero
	jz	l51

	pushl	EX_INTERNAL|0x207
	call	EXCEPTION

l51:
#endif PARANOID

	movl	accum_1,%edx
	movl	accum_0,%eax
	divl	%esi
	movl	%eax,%ecx

	movl	%edx,%eax
	divl	%esi

	sarl	$1,%ecx		// divide by 2
	rcrl	$1,%eax

	// round the result
	addl	$0x80000000,%eax
	adcl	$0,%ecx

	addl	%ecx,%edi
	adcl	$0,%esi

	jmp	l65

l60:
	movl	accum_1,%edx
	movl	accum_0,%eax
	divl	%esi
	movl	%eax,%ecx

	movl	%edx,%eax
	divl	%esi

	sarl	$1,%ecx		// divide by 2
	rcrl	$1,%eax

	// round the result
	addl	$0x80000000,%eax
	adcl	$0,%ecx

	subl	%ecx,%edi
	sbbl	$0,%esi

l65:
	movl	PARAM1,%ecx

	movl	%edi,SIGL(%ecx)
	movl	%esi,SIGH(%ecx)

	movl	EXP_BIAS,EXP(%ecx)	/* Result is in  [1.0 .. 2.0) */

//	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret
