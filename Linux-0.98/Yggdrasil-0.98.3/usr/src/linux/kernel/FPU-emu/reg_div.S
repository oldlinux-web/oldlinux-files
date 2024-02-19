	.file	"reg_div.S"
/*---------------------------------------------------------------------------+
 |  reg_div.S                                                                |
 |                                                                           |
 | Divide one REG by another and put the result in a destination REG.        |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 | Call from C as:                                                           |
 |   void reg_div(REG *a, REG *b, REG *dest)                                 |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "exception.h"
#include "fpu_asm.h"

.text
	.align 2

.globl	_reg_div
_reg_div:
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	PARAM1,%esi
	movl	PARAM2,%ebx
	movl	PARAM3,%edi

	movb	TAG(%esi),%al
	orb	TAG(%ebx),%al

	jne	xL_div_special		// Not (both numbers TW_Valid)


// Both arguments are TW_Valid
	movl	EXP(%esi),%edx
	movl	EXP(%ebx),%eax
	subl	%eax,%edx
	addl	EXP_BIAS,%edx
	movl	%edx,EXP(%edi)

	movb	TW_Valid,TAG(%edi)

	movb	SIGN(%esi),%cl
	cmpb	%cl,SIGN(%ebx)
	setneb	(%edi)		// Set the sign, requires neg=1, pos=0

	add	$SIGL_OFFSET,%ebx
	add	$SIGL_OFFSET,%esi

	jmp	_divide_kernel


/*-----------------------------------------------------------------------*/
xL_div_special:
	cmpb	TW_NaN,TAG(%esi)	// A NaN with anything to give NaN
	je	xL_arg1_NaN

	cmpb	TW_NaN,TAG(%ebx)	// A NaN with anything to give NaN
	jne	xL_no_NaN_arg

// Operations on NaNs
xL_arg1_NaN:
xL_arg2_NaN:
	pushl	%edi
	pushl	%ebx
	pushl	%esi
	call	_real_2op_NaN
	jmp	xL78

// Invalid operations
xL_zero_zero:
xL_inf_inf:
	pushl	%esi
	call	_arith_invalid
	jmp	xL78

xL_no_NaN_arg:
	cmpb	TW_Infinity,TAG(%esi)
	jne	xL_arg1_not_inf

	cmpb	TW_Infinity,TAG(%ebx)
	je	xL_inf_inf	// invalid operation

	// Note that p16-9 says that infinity/0 returns infinity
	jmp	xL_copy_arg1		// Answer is Inf

xL_arg1_not_inf:
	cmpb	TW_Zero,TAG(%ebx)		// Priority to div-by-zero error
	jne	xL_arg2_not_zero

	cmpb	TW_Zero,TAG(%esi)
	je	xL_zero_zero	// invalid operation

// Division by zero error
	pushl	%esi
	movb	SIGN(%esi),%al
	xorb	SIGN(%ebx),%al
	pushl	%eax		// lower 8 bits have the sign
	call	_divide_by_zero
	jmp	xL78

xL_arg2_not_zero:
	cmpb	TW_Infinity,TAG(%ebx)
	jne	xL_arg2_not_inf

	jmp	xL_return_zero		// Answer is zero

xL_arg2_not_inf:
	cmpb	TW_Zero,TAG(%esi)
	jne	xL_unknown_tags

xL_copy_arg1:
	movb	TAG(%esi),%ax
	movb	%ax,TAG(%edi)
	movl	EXP(%esi),%eax
	movl	%eax,EXP(%edi)
	movl	SIGL(%esi),%eax
	movl	%eax,SIGL(%edi)
	movl	SIGH(%esi),%eax
	movl	%eax,SIGH(%edi)

	movb	SIGN(%esi),%cl
	cmpb	%cl,SIGN(%ebx)
	jne	xL76

	movb	SIGN_POS,SIGN(%edi)
	jmp	xL78

xL71:
	movb	SIGN(%esi),%cl
	cmpb	%cl,SIGN(%edi)
	jne	xL76

	movb	SIGN_POS,SIGN(%ebx)
	jmp	xL78

	.align 2,0x90
xL76:
	movb	SIGN_NEG,SIGN(%edi)

xL78:
	leal	-12(%ebp),%esp

	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret


xL_return_zero:
	movb	TW_Zero,TAG(%edi)
	jmp	xL71

xL_unknown_tags:
	push	EX_INTERNAL | 0x208
	call	EXCEPTION

	// Generate a NaN for unknown tags
	movl	_CONST_QNaN,%eax
	movl	%eax,(%edi)
	movl	_CONST_QNaN+4,%eax
	movl	%eax,SIGL(%edi)
	movl	_CONST_QNaN+8,%eax
	movl	%eax,SIGH(%edi)
	jmp	xL78
