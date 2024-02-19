/*---------------------------------------------------------------------------+
 |  poly_mul64.S                                                             |
 |                                                                           |
 | Multiply two 64 bit integers.                                             |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void mul64(long long *a, long long *b, long long *result)               |
 |                                                                           |
 +---------------------------------------------------------------------------*/


#include "fpu_asm.h"

.text
	.align 2,144
.globl _mul64
_mul64:
	pushl %ebp
	movl %esp,%ebp
	subl $16,%esp
	pushl %esi
	pushl %ebx

	movl PARAM1,%esi
	movl PARAM2,%ecx
	movl PARAM3,%ebx

	xor %eax,%eax
	movl %eax,-4(%ebp)
	movl %eax,-8(%ebp)

	movl (%esi),%eax
	mull (%ecx)
	movl %eax,-16(%ebp)	// Not used
	movl %edx,-12(%ebp)

	movl (%esi),%eax
	mull 4(%ecx)
	addl %eax,-12(%ebp)
	adcl %edx,-8(%ebp)
	adcl $0,-4(%ebp)

	movl 4(%esi),%eax
	mull (%ecx)
	addl %eax,-12(%ebp)
	adcl %edx,-8(%ebp)
	adcl $0,-4(%ebp)

	movl 4(%esi),%eax
	mull 4(%ecx)
	addl %eax,-8(%ebp)
	adcl %edx,-4(%ebp)

	testb $128,-9(%ebp)
	je L_no_round

	addl $1,-8(%ebp)
	adcl $0,-4(%ebp)

L_no_round:
	movl -8(%ebp),%esi
	movl %esi,(%ebx)
	movl -4(%ebp),%esi
	movl %esi,4(%ebx)

	popl %ebx
	popl %esi
	leave
	ret
