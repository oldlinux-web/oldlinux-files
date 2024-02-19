/*---------------------------------------------------------------------------+
 |  polynomial.S                                                             |
 |                                                                           |
 | Fixed point arithmetic polynomial evaluation.                             |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void polynomial(unsigned accum[], unsigned x[], unsigned terms[][2],    |
 |                   int n)                                                  |
 |                                                                           |
 | Computes:                                                                 |
 | terms[0] + (terms[1] + (terms[2] + ... + (terms[n-1]*x)*x)*x)*x) ... )*x  |
 | The result is returned in accum.                                          |
 |                                                                           |
 +---------------------------------------------------------------------------*/

	.file	"fpolynom.s"

#include "fpu_asm.h"


//	#define	EXTRA_PRECISE

#define	TERM_SIZE	$8


.text
	.align 2,144
.globl _polynomial
_polynomial:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$32,%esp
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi		// accum
	movl	PARAM2,%edi		// x
	movl	PARAM3,%ebx		// terms
	movl	PARAM4,%ecx		// n

	movl	TERM_SIZE,%eax
	mull	%ecx
	movl	%eax,%ecx

	movl	4(%ebx,%ecx,1),%edx	// terms[n]
	movl	%edx,-20(%ebp)
	movl	(%ebx,%ecx,1),%edx	// terms[n]
	movl	%edx,-24(%ebp)		
	xor	%eax,%eax
	movl	%eax,-28(%ebp)

	subl	TERM_SIZE,%ecx
	js	xL_accum_done

xL_accum_loop:
	xor	%eax,%eax
	movl	%eax,-4(%ebp)
	movl	%eax,-8(%ebp)

#ifdef EXTRA_PRECISE
	movl	-28(%ebp),%eax
	mull	4(%edi)			// x ms long
	movl	%edx,-12(%ebp)
#endif EXTRA_PRECISE

	movl	-24(%ebp),%eax
	mull	(%edi)			// x ls long
//	movl	%eax,-16(%ebp)		// Not needed
	addl	%edx,-12(%ebp)
	adcl	$0,-8(%ebp)

	movl	-24(%ebp),%eax
	mull	4(%edi)			// x ms long
	addl	%eax,-12(%ebp)
	adcl	%edx,-8(%ebp)
	adcl	$0,-4(%ebp)

	movl	-20(%ebp),%eax
	mull	(%edi)
	addl	%eax,-12(%ebp)
	adcl	%edx,-8(%ebp)
	adcl	$0,-4(%ebp)

	movl	-20(%ebp),%eax
	mull	4(%edi)
	addl	%eax,-8(%ebp)
	adcl	%edx,-4(%ebp)

/* Now add the next term */
	movl	(%ebx,%ecx,1),%eax
	addl	%eax,-8(%ebp)
	movl	4(%ebx,%ecx,1),%eax
	adcl	%eax,-4(%ebp)

/* And put into the second register */
	movl	-4(%ebp),%eax
	movl	%eax,-20(%ebp)
	movl	-8(%ebp),%eax
	movl	%eax,-24(%ebp)

#ifdef EXTRA_PRECISE
	movl	-12(%ebp),%eax
	movl	%eax,-28(%ebp)
#else
	testb	$128,-25(%ebp)
	je	xL_no_poly_round

	addl	$1,-24(%ebp)
	adcl	$0,-20(%ebp)
xL_no_poly_round:
#endif EXTRA_PRECISE

	subl	TERM_SIZE,%ecx
	jns	xL_accum_loop

xL_accum_done:
#ifdef EXTRA_PRECISE
/* And round the result */
	testb	$128,-25(%ebp)
	je	xL_poly_done

	addl	$1,-24(%ebp)
	adcl	$0,-20(%ebp)
#endif EXTRA_PRECISE

xL_poly_done:
	movl	-24(%ebp),%eax
	movl	%eax,(%esi)
	movl	-20(%ebp),%eax
	movl	%eax,4(%esi)

	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret
